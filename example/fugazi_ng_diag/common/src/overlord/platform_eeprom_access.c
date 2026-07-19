/* $Id: platform_eeprom_access.c,v 1.5 2014/05/29 00:37:41 mcharon Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/platform_eeprom_access.c,v $
 *------------------------------------------------------------------
 *  
 * platform_eeprom_access.c  
 * access eeprom via ethtool
 *
 * Oct 2011 Alan Peng
 * Copyright (c) 2014 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */

#include<stdio.h>
#include<stdlib.h>
#include<strings.h>  /* for bzero*/
#include<string.h>
#include<sys/socket.h>
#include<features.h>
#include<linux/if_packet.h>
#include<linux/if_ether.h>
#include<linux/ethtool.h> /*struct ethtool */
#include<linux/sockios.h> /* SIOCETHTOOL */
#include <sys/types.h> /* getpid */
#include <sys/stat.h>
#include <unistd.h>  /* getpid */
#include <netinet/in.h>  /* for including the linux_eth.h */

#include<errno.h>
#include<sys/ioctl.h>
#include<net/if.h>
#include<assert.h>
	
#include "defs.h"
#include "types.h"
#include "common.h"
#include "queryflags.h"
#include "platform_eeprom_access.h"
#include "linux_api.h"


#define GEEPROM 0
#define SEEPROM 1

extern const unsigned int igb_eeprom_size;
extern const unsigned char *igb_eeprom;
static unsigned char *igb_eeprom_new = NULL;

int ovld_dump_eeprom(int, int, boolean);
static int do_seeprom(int fd, struct ifreq *ifr, uint, uint, uint);

/*------------------------------------------------------------------
 *
 * Function: create_socket
 *     Create the socket for eeeprom read write.
 *
 * Input:  protocol - seclect protocol
 *
 * Output: rawsock - return created socket num.
 *
 *------------------------------------------------------------------
 */
static int
create_socket (void)
{ 
    int fd;
    if((fd = socket(AF_INET, SOCK_DGRAM, 0))== -1) {
        perror("Cannot get control socket");
        exit(-1);
    }
    return fd;
}

/*------------------------------------------------------------------
 *
 * Function: send_ioctl 
 * Description: wrapper function for calling iotcl
 *
 * Input:  fd - file descript; ifr -- pointer to struct ifreq
 *
 * Output: return status for ioctl
 *
 *------------------------------------------------------------------
 */
static int
send_ioctl (int fd, struct ifreq *ifr)
{
    return ioctl(fd, SIOCETHTOOL, ifr);
}

/*------------------------------------------------------------------
 *
 * Function: dump_eeprom 
 * Description: utility display cavecreek eeprom
 *
 * Input: info - pointer to struct ethtool_drvinfo ;
 *        ee   - pointer to struct ethtool_eeprom;
 *
 * Output: return passed
 *
 *------------------------------------------------------------------
 */
static int
dump_eeprom (struct ethtool_drvinfo *info, struct ethtool_eeprom *ee)
{
    int i;

    fprintf(stdout, "Offset\t\tValues\n");
    fprintf(stdout, "------\t\t------");
    for (i = 0; i < ee->len; i++) {
        if(!(i%16)) fprintf(stdout, "\n0x%04x\t\t", i + ee->offset);
        fprintf(stdout, "%02x ", ee->data[i]);
    }
    fprintf(stdout, "\n");
    return PASSED;
}

/*------------------------------------------------------------------
 *
 * Function: do_geeprom 
 * Description: lowe level api to get/read eeprom data
 *
 *
 *        ifr - pointer to struct ifreq
 *        geeprom_offset -- offset to eeprom 
 *        geeprom_length -- number of bytes to read from eeprom
 *
 * Output: return linux error code
 *
 *------------------------------------------------------------------
 */
static int
do_geeprom (int fd, struct ifreq *ifr, uint geeprom_offset, uint geeprom_length)
{
    int err = 0;
    struct ethtool_drvinfo drvinfo;
    struct ethtool_eeprom *eeprom;

    drvinfo.cmd = ETHTOOL_GDRVINFO;
    ifr->ifr_data = (caddr_t)&drvinfo;
    err = send_ioctl(fd, ifr);
    if (err < 0) {
        perror("Cannot get driver information");
        return 74;
    }

    if (geeprom_length <= 0)   
        geeprom_length = drvinfo.eedump_len;

    if (drvinfo.eedump_len < geeprom_offset + geeprom_length)
        geeprom_length = drvinfo.eedump_len - geeprom_offset;

    eeprom = calloc(1, sizeof(*eeprom)+geeprom_length);
    if (!eeprom) {
        perror("Cannot allocate memory for EEPROM data");
        return 75;
    }
    eeprom->cmd = ETHTOOL_GEEPROM;
    eeprom->len = geeprom_length;
    eeprom->offset = geeprom_offset;
    ifr->ifr_data = (caddr_t)eeprom;
    err = send_ioctl(fd, ifr);
    if (err < 0) {
        perror("Cannot get EEPROM data");
        free(eeprom);
        return 74;
    }
    err = dump_eeprom(&drvinfo, eeprom);
    free(eeprom);

    return err;
}

/*------------------------------------------------------------------
 *
 * Function: do_seeprom 
 * Description: lowe level api to get/read eeprom data
 *
 *                        
 * INPUT:
 *        fd - file descriptor
 *        ifr - pointer to struct ifreq
 *        seeprom_magic -- magic number required by driver to allow eeprom to be
 *        program. ususally this is the pci vendor id. (look at driver code to
 *        find out what magic number is required.
 *        geeprom_offset -- offset to eeprom 
 *        geeprom_value -- value to be written to eeprom
 *
 * Output: return linux error code
 *
 *------------------------------------------------------------------
 */
static int
do_seeprom (int fd, struct ifreq *ifr, uint seeprom_magic, uint seeprom_offset,
            uint seeprom_value)
{
    int err = 0;
    int seeprom_length = 0;
    struct ethtool_drvinfo drvinfo;
    struct ethtool_eeprom *eeprom;

    drvinfo.cmd = ETHTOOL_GDRVINFO;
    ifr->ifr_data = (caddr_t)&drvinfo;
    err = send_ioctl(fd, ifr);
    if (err < 0) {
        perror("Cannot get driver information");
        return 74;
    }
    /*	
drvinfo.eedump_len =  32768;
seeprom_length  =  1
    */

    if (seeprom_value != EOF)
        seeprom_length = 1;

    if (seeprom_length <= 0)
        seeprom_length = drvinfo.eedump_len;
        
    if (drvinfo.eedump_len < seeprom_offset + seeprom_length)
        seeprom_length = drvinfo.eedump_len - seeprom_offset;


    eeprom = calloc(1, sizeof(*eeprom)+seeprom_length);
    if (!eeprom) {
        perror("Cannot allocate memory for EEPROM data");
        return 75;
    }

    eeprom->cmd = ETHTOOL_SEEPROM;
    eeprom->len = 1;
    eeprom->offset = seeprom_offset;
    eeprom->magic = seeprom_magic;
    eeprom->data[0] = seeprom_value;

    /* Multi-byte write: read input from file */
    if (seeprom_value == EOF) {
        printf("programming cavecreek eeprom...firmware size = %d\n", seeprom_length);
        if (drvinfo.eedump_len != igb_eeprom_size) {
            assert(!" driver eeprom length is not the same as firmware file size");
        }
        eeprom->len = seeprom_length;
        if (igb_eeprom_new) {
            printf("using data from file.\n");
            memcpy(eeprom->data, igb_eeprom_new, seeprom_length);
        } else {
            printf("using default data file.\n");
            memcpy(eeprom->data, igb_eeprom, seeprom_length);
        }
    }

    ifr->ifr_data = (caddr_t)eeprom;
    err = send_ioctl(fd, ifr);
    if (err < 0) {
        perror("Cannot set EEPROM data");
        err = 87;
    }
    free(eeprom);

    return err;
}

/*------------------------------------------------------------------
 *
 * Function: program_mac_eeprom (int port, uchar *mac_addr)
 * Description:	helper api to program mac address into eeprom
 *
 * Input:  port; port number. on overlord there are 4 macs (4 ports)
 *         mac_address: pointer to mac address
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
program_mac_eeprom (int port, uchar *mac_addr)
{

    int fd, rd;
    int offset = 0;

    struct ifreq ifr;    
   
    /* Setup our control structures. */
    memset(&ifr, 0, sizeof(ifr));
    sprintf(ifr.ifr_name, "eth%1d", port);
  
    /* Create the socket */
    fd = create_socket();
    
    /* 6 bytes for mac address*/
    for(offset = 0; offset < 6; offset++, mac_addr++) {
        rd = do_seeprom(fd, &ifr, 0x04388086, offset, *mac_addr); 
        if(rd != 0) {
            perror("Access eeprom failed \n");
            return FAILED;
        }
    }
    
    close(fd);
    
    return PASSED;
}

/*------------------------------------------------------------------
 *
 * Function: dump_eeprom_util 
 * Description:	utility to display cavecreek eeprom
 *
 * Input:  NONE
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
dump_eeprom_util (void)
{

    unsigned char region;
    
    region = gethex_answer("\nEnter display region", 1, 1, 0xFF);
    ovld_dump_eeprom(1, region, TRUE);  /*hard coding in here */
   
    return 0;
}

/*------------------------------------------------------------------
 *
 * Function: dump_eeprom_256
 *	Main function to dump eeprom
 *
 * Input:  eth -- ethernet interface
 *         byte -- how many bytes to dump
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
display_mac_eeprom(int eth, int byte)
{

    ovld_dump_eeprom(eth, byte, FALSE);  /*hard coding in here */
   
    return PASSED;
}

/*------------------------------------------------------------------
 *
 * Function: ovld_dump_eeprom
 *	Main function to access the eeprom
 *
 * Input:  port - eth port to access the eeprom.
 *         length - how many bytes to dump
 *         UTIL - flag set if called from submenu utility
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
ovld_dump_eeprom (int port, int length, boolean UTIL)
{
    int fd, rd;
    int offset = 0;
    struct ifreq ifr;
   
   	
    /* Setup our control structures. */
    memset(&ifr, 0, sizeof(ifr));
    sprintf(ifr.ifr_name, "eth%1d", port);
	   
    /* Create the socket */
    fd = create_socket();
    
    if (UTIL) {
        do {
            rd = do_geeprom(fd, &ifr, offset, length); 
            offset = offset + length;
        } while(getc_answer("Continue Display", "yn", 'y') == 'y');
    } else {
        rd = do_geeprom(fd, &ifr, offset, length); 
    }
    
    close(fd);

    if(rd != 0) {
    	perror("Access eeprom failed \n");
        return 1;
    }
    
    return 0;
}


/*------------------------------------------------------------------
 *
 * Function: access_eeprom
 *	Main function to access the eeprom
 *
 * Input:  port - eth port to access the eeprom.
 *         command - select read/write 
 *         offset - start address/ modified address
 *         value - show address length/ modified value
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
static int
access_eeprom (char *port, int command, int offset, int value)
{
    int fd, rd;

    struct ifreq ifr;
   
    /* Setup our control structures. */
    memset(&ifr, 0, sizeof(ifr));
    sprintf(ifr.ifr_name, port);
	   
    /* Create the socket */
    fd = create_socket();
    
    if(command == GEEPROM){
        /* read */  
        rd = do_geeprom(fd, &ifr, offset, value); 
    } else {  /* magic 0x04388086 need to be fixed ? */
        /* write */
        rd = do_seeprom(fd, &ifr, 0x04388086, offset, value);  
    }

    close(fd);

    if(rd != 0) {
    	perror("Access eeprom failed \n");
        return 1;
    }
    
    return 0;
}

/*------------------------------------------------------------------
 *
 * Function: access_eeprom_utils
 *	Using the ethtool to access eeprom.
 *
 * Input:  None. 
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
access_eeprom_utils (void)
{
    int access_typ, port;
    char pname[IFNAMSIZ];
    uint offset, value;
    char c;
        
    port = 0;  /*hard coding in here */
    sprintf(pname,"%s%d", "eth", port);
    
    
    printf("\nAccess eeprom 0:Read  1:Write");
    access_typ = getdec_answer("\nEnter ", 0, 0, 1);


    if (access_typ == GEEPROM) {
        /* read */
        offset = gethex_answer("\nGet start address  ", 0, 0, 0x7FFF);
    
        value = gethex_answer("\nGet display length ", 0, 0, 0x7FFF);    
        return(access_eeprom(pname, GEEPROM, offset, value));

    } else {
        /* write */    	
        do {
            offset = getdec_answer("\nEnter write address number: ", 0, 0, 0x7FFF);
            printf("\nCurrent Value is :\n");
            access_eeprom(pname, GEEPROM, offset, 1);
            
            c = getc_answer("Do you want to change value?", "yn",'n');

            if (c == 'y') {
                value = gethex_answer("\nEnter value[hex]:", 0, 0, 0xFF);
                access_eeprom(pname, SEEPROM, offset, value);
                printf("\nRead back :\n");   
                access_eeprom(pname, GEEPROM, offset, 1);
            }
        } while(getc_answer("Continue?", "yn", 'y') == 'y');
    }
    
    return 0;
}

/*------------------------------------------------------------------
 *
 * Function: program_cavecreek_eeprom
 * Description:	utility to program cavecree eeprom. this uility
 *       requires user to specify firmware file name (binary format).
 *       hardware guys can provide this. do not use ttf formatted file.
 *
 * Input:  query - flag set if utility needs intervention from user;
 *         otherwise, flag is not set.
 *
 * Output: PASSED/FAILED
 *
 *------------------------------------------------------------------
 */
int
program_cavecreek_eeprom (int query)
{
    uint offset = 0;
    char str[128];
    struct stat st;
    
    if (!query) {


    } else {
        printf("Enter binary file name including path. >> ");
        fgets(str, 120,  stdin);
        if (*str == '\n') {
            return PASSED;
        }
        if (*str) {
            str[strlen(str)-1] = '\0';
        }

        stat(str, &st);
        if (st.st_size == igb_eeprom_size) {
         
            igb_eeprom_new = malloc(st.st_size);
            if (igb_eeprom_new) {
                if (readfile(str, igb_eeprom_new, st.st_size) < 0)
                    return FAILED;
            }
        } else {
            printf("file not found or invalid file size.\n");
            return FAILED;
        }
    }

    if (access_eeprom("eth0", SEEPROM, offset, EOF) < 0 ) {
        cterr('f', 0, "unable to write %#x @%#x", offset);
        return FAILED;
    }

    if (igb_eeprom_new) {
        free(igb_eeprom_new);
        igb_eeprom_new = NULL;
                                  
    }

    ovld_dump_eeprom(0, 0x20, 0);
    printf("\n 32 bytes of eeprom are shown above.\n");
    
    return PASSED;
}

/*
$Log: platform_eeprom_access.c,v $
Revision 1.5  2014/05/29 00:37:41  mcharon
rename bin2hex to readfile

Revision 1.4  2014/05/02 18:24:15  mcharon
replace strcpy with sprint when copying interface name to struct ifr

Revision 1.3  2014/05/01 13:43:15  mcharon
fix memory coruption which causes ngvm to fail during bind_socket

Revision 1.2  2013/11/26 08:40:36  hroni
fix compiler warning

Revision 1.1  2013/05/09 05:42:36  alpeng
moving overlord common code from x86

Revision 1.7  2012/11/07 10:58:16  alpeng
remove useless file and clean up code

Revision 1.6  2012/11/06 20:39:50  mcharon
add headers/cleanup/remove unneeded functions/files

Revision 1.5  2012/08/07 17:48:20  mcharon
allow user to specify fm file to program eeprom

Revision 1.4  2012/06/25 23:33:46  mcharon
support programming cavecreek eeprom

Revision 1.3  2012/06/06 07:34:05  palin2
Clean up compiler warnings.

Revision 1.2  2012/03/28 00:38:23  mcharon
remove forward slash from second line

Revision 1.1.1.1  2012/03/23 23:02:09  ptong
Initial archive of ng_diag module


$Endlog$
*/
