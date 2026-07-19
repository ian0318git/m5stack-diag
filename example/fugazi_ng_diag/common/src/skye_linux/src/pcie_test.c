/* $Id: pcie_test.c,v 1.2 2015/05/25 03:59:16 steja Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/pcie_test.c,v $
 *------------------------------------------------------------------
 *
 * pcie_test: Chip-To-Chip PCIe data transmission 
 *
 * June 2013 - Ian Chang
 *
 * Copyright (c) 2013-2015 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
/***************************************************************************
 * Copyright 2012 Tilera Corporation. All Rights Reserved.                
 *
 *   The source code contained or described herein and all documents
 *   related to the source code ("Material") are owned by Tilera
 *   Corporation or its suppliers or licensors.  Title to the Material
 *   remains with Tilera Corporation or its suppliers and licensors. The
 *   software is licensed under the Tilera MDE License.
 *
 *   Unless otherwise agreed by Tilera in writing, you may not remove or
 *   alter this notice or any other notice embedded in Materials by Tilera
 *   or Tilera's suppliers or licensors in any way.
 **************************************************************************/


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <tmc/alloc.h>
#include <tmc/mspace.h>
#include <gxio/trio.h>
#include <gxpci/gxpci.h>
#include <sys/stat.h>
#include <tmc/cpus.h>
#include <tmc/task.h>
#include <tmc/sync.h>
#include <tmc/perf.h>
#include <arch/cycle.h>
#include "common.h" 
#include "proto.h" 
#include "error.h" 
#include "nvmonvars.h"

#define DATA_VALIDATION
#define SKYE_PCIE_INFO_BUF_SZ          256

#define VERIFY_ZERO(VAL, WHAT)                                  \
  do {                                                          \
    long long __val = (VAL);                                    \
    if (__val != 0)                                             \
      tmc_task_die("Failure in '%s': %lld: %s.",                \
                   (WHAT), __val, gxpci_strerror(__val));       \
  } while (0)

/* #define MIN(A,B) (((A) < (B)) ? (A) : (B)) */
/* Fixed MDE-4.2.beta1 re-define issue */

#define MAX_PKT_SIZE		(1 << (GXPCI_MAX_CMD_SIZE_BITS - 1))

#define PKTS_IN_POOL		(MAP_LENGTH / MAX_PKT_SIZE)

/* The packet pool memory size. */
#define MAP_LENGTH 		(16 * 1024 * 1024)

// The number of packets that this program sends.
#define SEND_PKT_COUNT		10


// The size of a single packet.
#define SEND_PKT_SIZE		4096

#define MAX_CMDS_BATCH		64

gxio_trio_context_t trio_context_body;
gxio_trio_context_t *trio_context = &trio_context_body;

gxpci_context_t gxpci_context_send;
gxpci_context_t *send_context = &gxpci_context_send;
gxpci_context_t gxpci_context_recv;
gxpci_context_t *recv_context = &gxpci_context_recv;

cpu_set_t desired_cpus;

// By default, this test conducts duplex transfers.
int dont_recv = 0;
int dont_send = 0;

// The running cpu number.
int send_cpu_rank = 1;
int recv_cpu_rank = 2;

// The TRIO index.
int trio_index = 0;
int trio_asid;

int rem_link_index = 1;

// The local MAC index.
int loc_mac = 0;

// The queue index of the C2C send queue and receive queue.
int send_queue_index = 1;
int recv_queue_index = 2;

int send_pkt_count = SEND_PKT_COUNT;

int send_pkt_size = SEND_PKT_SIZE;

int pkt_size_order;

// The size of space that the receiver wants to preserve
// at the beginning of the packet, e.g. for packet header
// that is to be filled after the packet is received.
#define RECV_PKT_HEADROOM       0

/*******************************************************************************
 * Function   : usage
 *
 * Description: print out usage
 *
 * Inputs     : none
 *
 * Outputs    : none
 *******************************************************************************
 */
static void
usage(void)
{
    fprintf(stderr, "Usage: c2c_sender [--mac=<local mac port #>] "
            "[--rem_link_index=<remote port link index>] "
            "[--queue_index=<send queue index>] " 
            "[--send_pkt_size=<packet size>] "
            "[--cpu_rank=<cpu_id>] "
            "[--send_pkt_count=<packet count>]\n");
    exit(EXIT_FAILURE);
}

/*******************************************************************************
 * Function   : shift_option
 *
 * Description: shift option
 *
 * Inputs     : arglist - argument list
 *              option  - option list
 *
 * Outputs    : NULL or val
 *******************************************************************************
 */
static char *
shift_option(char ***arglist, const char* option)
{
    char** args = *arglist;
    char* arg = args[0], **rest = &args[1];
    int optlen = strlen(option);
    char* val = arg + optlen;
    if (option[optlen - 1] != '=')
    {
      if (strcmp(arg, option))
        return NULL;
    }
    else
    {
      if (strncmp(arg, option, optlen - 1))
        return NULL;
      if (arg[optlen - 1] == '\0')
        val = *rest++;
      else if (arg[optlen - 1] != '=')
        return NULL;
    }
    *arglist = rest;
    return val;
}

/* Parses command line arguments in order to fill in the MAC and bus
   address variables. */
//void parse_args(int argc, char** argv)
void parse_args(int argc, char** argv)
{
  char **args = &argv[1];

  // Scan options.
  while (*args)
  {
    char* opt = NULL;

    if ((opt = shift_option(&args, "--mac=")))
      loc_mac = atoi(opt);
    else if ((opt = shift_option(&args, "--dont_recv=")))
      dont_recv = atoi(opt);
    else if ((opt = shift_option(&args, "--dont_send=")))
      dont_send= atoi(opt);
    else if ((opt = shift_option(&args, "--rem_link_index=")))
      rem_link_index = atoi(opt);
    else if ((opt = shift_option(&args, "--send_queue_index=")))
      send_queue_index = atoi(opt);
    else if ((opt = shift_option(&args, "--recv_queue_index=")))
      recv_queue_index = atoi(opt);
    else if ((opt = shift_option(&args, "--send_pkt_size=")))
      send_pkt_size = atoi(opt);
    else if ((opt = shift_option(&args, "--send_cpu_rank=")))
      send_cpu_rank = atoi(opt);
    else if ((opt = shift_option(&args, "--recv_cpu_rank=")))
      recv_cpu_rank = atoi(opt);
    else if ((opt = shift_option(&args, "--send_pkt_count=")))
      send_pkt_count = atoi(opt);
    else
      usage();
  }
}

/*******************************************************************************
 * Function   : next_power_of_2
 *
 * Inputs     : pkt_size - packet size
 *
 * Outputs    : return value
 *******************************************************************************
 */
static unsigned int next_power_of_2(int pkt_size)
{
  if (pkt_size & (pkt_size - 1)) {
    return 32 - __builtin_clz(pkt_size);
  } else {
    return __builtin_ctz(pkt_size);
  }
}

/*******************************************************************************
 * Function   : do_send
 *
 * Inputs     : context -  pci structure
 *              buf_mem - buffer memory
 *
 * Outputs    : return none
 *******************************************************************************
 */
void do_send(gxpci_context_t* context, void* buf_mem)
{
    gxpci_comp_t comp[MAX_CMDS_BATCH];
    gxpci_cmd_t cmd[MAX_CMDS_BATCH];
    uint64_t start_cycles;
    uint64_t finish_cycles;
    float gigabits;
    float cycles;
    float gbps;
    int result;
    uint32_t cmds_to_post;
    uint32_t completions = 0;
    uint32_t sends = 0;
    uint32_t credits;
#ifdef TEST_DATA_PATTERN
    uint32_t data_pattern = 0;
    int j;
#endif

  start_cycles = get_cycle_count();

  while (completions < send_pkt_count)
  {
    if (sends < send_pkt_count)
    {
      credits = gxpci_c2c_send_get_credits(context);
      cmds_to_post = MIN(credits, (send_pkt_count - sends));
      cmds_to_post = MIN(MAX_CMDS_BATCH, cmds_to_post);

      for (int i = 0; i < cmds_to_post; i++)
      {
        cmd[i].buffer =
          buf_mem + ((sends++ << pkt_size_order) & (MAP_LENGTH - 1));
        cmd[i].size = send_pkt_size;

#ifdef TEST_DATA_PATTERN
        for (j = 0; j < send_pkt_size; j += 4)
        {
          *((uint32_t*)(cmd[i].buffer + j)) = data_pattern++;
        }
        // Make data visible to push DMA command.
        __insn_mf();
#endif

      }

      result = gxpci_c2c_send_cmds(context, cmd, cmds_to_post);
      if (result == GXPCI_ERESET)
      {
        printf("do_send: channel is reset, posted %u comps %u\n",
               sends - cmds_to_post, completions);
        goto send_reset;
      }
    }

    result = gxpci_c2c_get_send_comps(context, comp, 0, MAX_CMDS_BATCH);
    if (result == GXPCI_ERESET)
    {
      printf("do_send: channel is reset, posted %u comps %u\n",
             sends, completions);
      goto send_reset;
    }
    completions += result;
  }

send_reset:

    finish_cycles = get_cycle_count();
    gigabits = (float)completions * send_pkt_size * 8;
    cycles = finish_cycles - start_cycles;
    gbps = gigabits / cycles * tmc_perf_get_cpu_speed() / 1e9;
    printf("Transferred %d %d-byte packets: %f gbps\n",
           completions, send_pkt_size, gbps);
  
    gxpci_c2c_send_destroy(context);
}

/*******************************************************************************
 * Function   : thread_send
 *
 * Inputs     : arg - argument
 *
 * Outputs    : return none
 *******************************************************************************
 */
void *thread_send(void* arg)
{
    gxpci_context_t *gxpci_context = (gxpci_context_t *)arg;
    int rank = send_cpu_rank;
    int result;
  
    // Bind to the rank'th tile in the cpu set.
    if (tmc_cpus_set_my_cpu(tmc_cpus_find_nth_cpu(&desired_cpus, rank)) < 0) {
        tmc_task_die("tmc_cpus_set_my_cpu(thread_send) failed.");
    }
   
    if (dont_send) {
        return (void*)NULL;
    }
  
    // Allocate and register data buffers.
    tmc_alloc_t alloc = TMC_ALLOC_INIT;
    tmc_alloc_set_huge(&alloc);
    void* tx_buf_mem = tmc_alloc_map(&alloc, MAP_LENGTH);
    assert(tx_buf_mem);
  
    result = gxio_trio_register_page(trio_context, trio_asid, tx_buf_mem,
                                     MAP_LENGTH, 0);
    VERIFY_ZERO(result, "gxio_trio_register_page()");
  
    result = gxpci_c2c_open_send_queue(gxpci_context, send_queue_index);
    VERIFY_ZERO(result, "gxpci_c2c_open_send_queue()");
  
    printf("\nthread_send(tile %d): queue %d opened, waiting to send ...\n",
           rank, send_queue_index);
  
    // Run the test.
    do_send(gxpci_context, tx_buf_mem);
    tmc_alloc_unmap(tx_buf_mem, MAP_LENGTH);  
    return (void*)NULL;
}

/*******************************************************************************
 * Function   : do_recv
 *
 * Inputs     : context - PCI receive structrue
 *              buf_mem - buffer memory
 *
 * Outputs    : return none
 *******************************************************************************
 */
void do_recv(gxpci_context_t* context, void* buf_mem)
{
    gxpci_comp_t comp[MAX_CMDS_BATCH];
    gxpci_cmd_t cmd[MAX_CMDS_BATCH];
    uint32_t recv_pkt_count = 0;
    unsigned int sent_pkts = 0;
    unsigned int cmds_to_post;
    unsigned int credits;
    int result;
#ifdef TEST_DATA_PATTERN
    uint32_t actual_data;
    uint32_t expected_data = 0;
    uint32_t *chk_buf;
#endif

    while (recv_pkt_count < send_pkt_count)
    {
        credits = gxpci_c2c_recv_get_credits(context);
        cmds_to_post = MIN(credits, send_pkt_count - sent_pkts);
        cmds_to_post = MIN(cmds_to_post, MAX_CMDS_BATCH);
      
        for (int i = 0; i < cmds_to_post; i++)
        {
            cmd[i].buffer =
                buf_mem + ((sent_pkts++ << pkt_size_order) & (MAP_LENGTH - 1));
            cmd[i].size = send_pkt_size;
        }
      
        result = gxpci_c2c_recv_cmds(context, cmd, cmds_to_post);
        if (result == GXPCI_ERESET)
        {
            printf("do_recv: channel is reset, posted %u received %u\n",
                    sent_pkts - cmds_to_post, recv_pkt_count);
            goto recv_reset;
        }
      
        result = gxpci_c2c_get_recv_comps(context, comp, 0, MAX_CMDS_BATCH);
        if (result == GXPCI_ERESET)
        {
            printf("do_recv: channel is reset, posted %u received %u\n",
                    sent_pkts, recv_pkt_count);
            goto recv_reset;
        }
  
#ifdef TEST_DATA_PATTERN
    for (int i = 0; i < result; i++)
    {
      chk_buf = (uint32_t *)(comp[i].buffer);
      for (int j = 0; j < (send_pkt_size >> 2); j++)
      {
        actual_data = chk_buf[j];
        if (actual_data != expected_data)
        {
          printf("do_recv: comp %d of %d word %d expect %d get %d %p "
                 "posted %u recved %u\n", i, result, j, expected_data,
                 actual_data, chk_buf + j, sent_pkts, recv_pkt_count);

          goto recv_reset;
        }
        expected_data++;
      }
      recv_pkt_count++;
    }
#else
    recv_pkt_count += result;
#endif
  }

recv_reset:

  printf("Received %d %d-byte packets\n", recv_pkt_count, send_pkt_size);

  gxpci_c2c_recv_destroy(context);
}

/*******************************************************************************
 * Function   : thread_recv
 *
 * Inputs     : arg - thread received argument
 *
 * Outputs    : return none
 *******************************************************************************
 */
void *thread_recv(void *arg)
{
    gxpci_context_t *gxpci_context = (gxpci_context_t *)arg;
    int rank = recv_cpu_rank;
    int result;
   
    // Bind to the rank'th tile in the cpu set (RECV).
    if (tmc_cpus_set_my_cpu(tmc_cpus_find_nth_cpu(&desired_cpus, rank)) < 0) {
        tmc_task_die("tmc_cpus_set_my_cpu() failed.");
    }
   
    if (dont_recv) {
        return (void*)NULL;
    }
  
    // Allocate and register data buffers.
    tmc_alloc_t alloc = TMC_ALLOC_INIT;
    tmc_alloc_set_huge(&alloc);
    void* rx_buf_mem = tmc_alloc_map(&alloc, MAP_LENGTH);
    assert(rx_buf_mem);
  
#ifdef TEST_DATA_PATTERN
  for (int i = 0; i < MAP_LENGTH;)
  {
        *(uint32_t *)(rx_buf_mem + i) = 11;
        i += 4;
  }
#endif

    result = gxio_trio_register_page(trio_context, trio_asid, rx_buf_mem,
                                     MAP_LENGTH, 0);
    VERIFY_ZERO(result, "gxio_trio_register_page()");
    
    result = gxpci_c2c_open_recv_queue(gxpci_context, RECV_PKT_HEADROOM,
                                       MAP_LENGTH, rx_buf_mem, recv_queue_index);
    VERIFY_ZERO(result, "gxpci_c2c_open_recv_queue()");
    
    printf("\nthread_recv(tile %d): queue %d opened, waiting to receive ...\n",
           rank, recv_queue_index);
    
    // Run the test.
    do_recv(gxpci_context, rx_buf_mem);
    tmc_alloc_unmap(rx_buf_mem, MAP_LENGTH);  
    return (void*)NULL;	
}

/*******************************************************************************
 * Function   : pcie_c2c
 *
 * Inputs     : argc - argument counter
 *              argv - argument vector
 *
 * Outputs    : return value
 *******************************************************************************
 */
int pcie_c2c(int argc, char* argv[])
{
    pthread_t send_thread;
    pthread_t recv_thread;
    int result;
  
//    parse_args(argc, argv);
  
    assert(send_pkt_size <= GXPCI_MAX_CMD_SIZE);
  
    /* We must bind to a single CPU. */
    if (tmc_cpus_get_my_affinity(&desired_cpus) != 0) {
      tmc_task_die("tmc_cpus_get_my_affinity() failed.");
    }
    
    // Initialize TRIO context and get a TRIO asid.
    // They are shared by the send and the receiv threads.
    result = gxio_trio_init(trio_context, trio_index);
    VERIFY_ZERO(result, "gxio_trio_init()");
    
    trio_asid = gxio_trio_alloc_asids(trio_context, 1, 0, 0);
    if (trio_asid < 0) {
	    tmc_task_die("Failure in gxio_trio_alloc_asids(), asid = %d",
		            trio_asid);
    }
    
    result = gxpci_init(trio_context, send_context, trio_index, loc_mac);
    VERIFY_ZERO(result, "gxpci_init(send_context)");
    
    result = gxpci_init(trio_context, recv_context, trio_index, loc_mac);
    VERIFY_ZERO(result, "gxpci_init(recv_context)");
    
    result = gxpci_open_duplex_queue(recv_context, send_context, trio_asid,
                                     GXPCI_C2C_DUPLEX, rem_link_index, 0);
    VERIFY_ZERO(result, "gxpci_open_duplex_queue");
    
    pkt_size_order = next_power_of_2(send_pkt_size);
    
    // Start pthreads to send and receive data.
    
    if (pthread_create(&recv_thread, NULL, thread_recv,
                        (void*)recv_context) != 0) {
        tmc_task_die("pthread_create(recv_thread) failed.");
    }
    if (pthread_create(&send_thread, NULL, thread_send,
                        (void*)send_context) != 0) {
        tmc_task_die("pthread_create(send_thread) failed.");
    }
    if (pthread_join(recv_thread, NULL) != 0) {
        tmc_task_die("thread_join(recv_thread) failed.");
    }
    if (pthread_join(send_thread, NULL) != 0) {
        tmc_task_die("thread_join(send_thread) failed.");
    }
    gxpci_destroy_duplex(recv_context, send_context, GXPCI_C2C_DUPLEX);

    return (PASSED); 
}

/*****************************************************************************
 *
 * Function   : cpu1_c2c_send
 * Description: c2c pcie data transfer function
 * Inputs     : void
 * Outputs    : exit status
 *
 *****************************************************************************/
int cpu1_c2c_send (void)
{
    int result = 0;
    char *arg[4];
    testname("Skye PCIe");
    prpass(testpass, "CPU0 Data Transfer ");

#ifdef NO_USED
    arg[0]="--mac=0";
    arg[1]="--rem_link_index=1";
    arg[2]="--send_queue_index=0";
    arg[3]="--recv_queue_index=1";
#endif
    loc_mac = 0;
    rem_link_index = 1;
    send_queue_index = 0;
    recv_queue_index=1;
    /* Used core 1 to send packets */
    result = pcie_c2c(4, arg);

    return(result);
}

/*****************************************************************************
 *
 * Function   : cpu0_c2c_receive
 * Description: c2c pcie data receive function
 * Inputs     : void
 * Outputs    : exit status
 *
 *****************************************************************************/
int cpu0_c2c_receive (void)
{
    int result = 0;
    char *arg[4];
    testname("Skye PCIe");
    prpass(testpass, "CPU1 Data Transfer ");
#ifdef NO_USED
    arg[0]="--mac=0";
    arg[1]="--rem_link_index=0";
    arg[2]="--send_queue_index=1";
    arg[3]="--recv_queue_index=0";
#endif
    loc_mac = 0;
    rem_link_index = 0;
    send_queue_index = 1;
    recv_queue_index=0;

    /* Used core 2 to receive packets */
    result = pcie_c2c(4, arg);
    return(result);
}

/*******************************************************************************
 *
 * Function   : skye_check_pcie_lanes
 * Description: Function to check the PCIe lanes and show the result
 *              corresponding to the script "generic_pcie_lane.sh"
 * Inputs     : None
 * Outputs    : PASSED/FAILED
 *
 *******************************************************************************
 */
int
skye_check_pcie_lanes (void)
{
    testname("Skye PCIe lanes Scan");
    prpass(testpass, "");
    system("sh /diag/generic_pcie_lane.sh");
    char pcie_info[SKYE_PCIE_INFO_BUF_SZ];
    char buffer[SKYE_PCIE_INFO_BUF_SZ];
    FILE  *fp;
    struct stat f_st;
    int    f_size = 0;

    char pcie_bus_num[][13] = {"0000:01:00.0"};
    char device_id[][10] = {"1a41:0200"};
    char pcie_lanes[][4] = {"x8"};

    int bus_index = 0;
    int error = 0;

    stat("/skye_pcie_check.txt", &f_st);
    f_size = f_st.st_size;
    if (f_size == 0) {
        cterr('f', 0, "PCIe: Nothing is detected, /skye_pcie_check.txt is empty.");
        return (FAILED);
    }

    fp = fopen("/skye_pcie_check.txt", "r");
    if (fp == NULL) {
        printf("Failed to open /skye_pcie_check.txt");
        return (FAILED);
    }

    memset((char *)pcie_info, 0, sizeof(pcie_info));
    memset((char *)buffer, 0, sizeof(buffer));

    /* Scan file "skye_pcie_check.txt" to check the pcie lanes number */
    while (fscanf(fp, "%s", pcie_info) != EOF) {
        if (strcmp(pcie_info,device_id[bus_index]) == 0) {
            do {
                fscanf(fp, "%s", pcie_info);
            }while ( strcmp(pcie_info, "Width") != 0 );

            fscanf(fp, "%s", pcie_info);
            pcie_info[strlen(pcie_info)-1] = '\0';

            if (strcmp(pcie_info,pcie_lanes[bus_index]) != 0) {
                sprintf(buffer, "For bus number %s; Device ID %s ;" 
                    "detected %s lanes, expected %s lanes.",
                    pcie_bus_num[bus_index],device_id[bus_index], 
                    pcie_info+1,pcie_lanes[bus_index]+1);
                cterr('w', 0, buffer);
                error++ ;
            }
            bus_index++ ;
        }
    }
    if (error == 0) {
        if ((NVRAM)->diagflag & D_VERBOSE) {
            printf("\nPCIe Bus number %s; Device ID %s ;" 
                    "detected %s lanes.", pcie_bus_num[bus_index - 1],
                    device_id[bus_index - 1], pcie_lanes[bus_index - 1]+1);
        }
        printf("\nPCIe lanes scan passed!!\n");
    }
    else {
        printf("\nPCIe lanes scan failed!!\n");
    }

    fclose(fp);

    return (PASSED);
}


/******** History ********/ 
/*
 *------------------------------------------------------------------
 * $Log: pcie_test.c,v $
 * Revision 1.2  2015/05/25 03:59:16  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.4  2015/05/11 13:45:45  steja
 * Code clean up <CSCuu14285>
 *
 * Revision 1.1.4.3  2015/04/30 03:01:43  palin2
 * code clean up.
 *
 * Revision 1.1.4.2  2015/04/29 11:36:33  steja
 * Code check-in to skye-branch2 for ER code review
 *
 *------------------------------------------------------------------
 * Revision 1.1.2.2  2014/11/27 07:25:20  palin2
 * 1. Fixed PCIe lanes Scan test.
 * 2. Added PCIe lanes Scan test to 2-CPUs Skye default tests.
 * 3. Added SKYE_P1A compile flag to tell difference between P1A and P1B.
 *
 * Revision 1.1.2.1  2014/07/21 01:56:54  palin2
 * Initial check-in Skye module side Diag code.
 *
 *------------------------------------------------------------------
 * Revision 1.2.8.1  2014/05/25 16:54:22  iachang
 * CSCum84765: Dual CPU PCIe Data Transfer Test
 * Fixed segmentation fault when run 2nd time
 *
 * Revision 1.2  2014/02/27 15:01:46  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.6  2014/02/18 09:19:02  steja
 * Add print refer CDETS info for pci lane check and remove continue and do all flag
 *
 * Revision 1.1.4.5  2014/02/07 03:36:52  steja
 * code clean up
 *
 * Revision 1.1.4.4  2014/01/13 09:33:14  iachang
 * PCIe test Support pthreads to send and receive data
 *
 * Revision 1.1.4.3  2013/09/16 09:50:15  iachang
 * Code review and update
 *
 * Revision 1.1.4.2  2013/09/13 07:00:08  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.5  2013/09/04 08:01:06  iachang
 * Fixed MDE-4.2.beta1 re-define issue
 *
 * Revision 1.1.2.4  2013/08/23 06:32:48  iachang
 * Fix compile issue with MDE-4.2.alpha3
 *
 * Revision 1.1.2.3  2013/07/30 08:21:06  iachang
 * Modify the PCIe Endpoint Device.
 *
 * Revision 1.1.2.2  2013/07/11 15:55:46  iachang
 * Support PCIe Lanes Check Test
 *
 * Revision 1.1.2.1  2013/06/28 09:45:41  iachang
 * Support Dual CPU PCIe interface data transfer test
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
