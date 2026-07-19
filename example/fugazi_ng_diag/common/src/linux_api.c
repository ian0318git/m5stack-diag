/* $Id: linux_api.c,v 1.23 2021/09/24 01:27:20 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/linux_api.c,v $
 *--------------------------------------------------------------------------
 * File: linux_mmap.c
 *
 * Dec 2008, mcharon
 *
 * Copyright (c) 2014-2018 by cisco Systems, Inc.
 * All rights reserved.
 *
 *--------------------------------------------------------------------------
 */
#include "types.h"
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/poll.h>
#include <ctype.h>

#include "pci.h"
#include "linux_api.h"
#include "common.h"
#include "proto.h"
#include "error.h"

static int fd_diag_timer = 0;

/********************************
is_le (void)
{
    int num = 1;
    unsigned char *ptr = (unsigned char*)&num;
    if (*ptr) {
        return 1;
    }
    return 0;
}
********************************/


/*
 * Function: wastetime 
 *
 * Input . n....in mircro secs
 * 
 * Output: - value left in alarm
 */
void
wastetime (long n)
{
    int delay;

    /* open driver if it's not already open */
    diag_timer_open();

    if (n == 0) {
        /* we just want to perform memory barrier */
        read(fd_diag_timer, &delay, sizeof(delay));
        return;
    }
    
    delay = (int)n;
    if (write(fd_diag_timer, &delay, sizeof(delay))<0) {
        perror("fail write: does /dev/diag_timer have the right major number?");
        fflush(0);
        exit(0);
    }

}

void
smart_cookie_delay (long us_delay_count)
{
    wastetime(us_delay_count);
}

void
msleep (int t)
{
    usleep(t*1000);
}

void
mdelay (unsigned long t)
{
    usleep(t*1000);
}

void
udelay (unsigned long t)
{
    usleep(t);
}

uint32_t
pci_domain_config_read (uint32_t domain, uint32_t bus, uint16_t device, uint32_t fn, int offset)
{
    char proc_name[128];
    int handle;
    uint32_t value = 0xFFFFFFFF;

    sprintf(proc_name, "/proc/bus/pci/%04x:%02x/%02x.%01x", domain, bus, device, fn);

    handle = open(proc_name, O_RDWR);
    if (handle<0) {
        printf("%s: PCI device on domain %04x, bus %02x, device %02x, function %01x doesn't exist\n",
               __FUNCTION__, domain, bus, device, fn);
        printf("Please run 'lspci' and check that the device is correctly enumerated on the bus.\n");
        cterr('f', 0,"unable to read frome pci device.");
        return(value);
    }
    lseek(handle, offset, SEEK_SET); /* read value of bar 0 */
    read(handle, &value, sizeof(value));
    close(handle);
    return(value);

}

uint32_t
pci_config_read (uint32_t bus, uint16_t device, uint32_t fn, int offset)
{
    char proc_name[128];
    int handle;
    uint32_t value = 0xFFFFFFFF;
    sprintf(proc_name, "/proc/bus/pci/%02x/%02x.%01x", bus, device, fn);
#ifdef ELIXIR
    sprintf(proc_name, "/proc/bus/pci/0001:%02x/%02x.%01x", bus, device, fn);
#endif
    handle = open(proc_name, O_RDWR);
    if (handle<0) {
        printf("%s: PCI device on bus %d, device %d, function %d doesn't exist\n",
               __FUNCTION__, bus, device, fn);
        printf("Please run 'lspci' and check that the device is correctly enumerated on the bus.\n");
        cterr('f', 0,"unable to read frome pci device.");
        return(value);
    }
    lseek(handle, offset, SEEK_SET); /* read value of bar 0 */
    read(handle, &value, sizeof(value));
    //    printf("pci_config_read:%s: offset: %#x: read %#x\n", proc_name, offset, BYTESWAP(value));
    close(handle);
    return(value);

}

uint32_t
pci_config_read_byte (uint32_t bus, uint16_t device, uint32_t fn, int offset)
{
    return (pci_config_read_byte(bus, device, fn, offset) & 0xFF);

}

uint32_t
pci_domain_config_write (uint32_t domain, uint32_t bus, uint16_t device, uint32_t fn, int offset, uint32_t value)
{
    char proc_name[128];
    int handle;
    uint32_t value_swapped = (value);
    ssize_t size;

    sprintf(proc_name, "/proc/bus/pci/%04x:%02x/%02x.%01x", domain, bus, device, fn);
    handle = open(proc_name, O_RDWR);
    if (handle<0) {
        printf("%s: PCI device on bus %d, device %d, function %d doesn't exist.\n",
               __FUNCTION__, bus, device, fn);
        printf("Please run 'lspci' and check that the device is correctly enumerated on the bus.\n");
        cterr('f', 0, "Unable to write to pci device.");
        return(FAILED);
    }
    lseek(handle, offset, SEEK_SET); /* read value of bar 0 */
    size = write(handle, &value_swapped, sizeof(value_swapped));
    if (size < 0 ) {
        perror("Unable to config write\n");
    }
    fsync(handle);
    close(handle);
    return(PASSED);
}

uint32_t
pci_config_write (uint32_t bus, uint16_t device, uint32_t fn, int offset, uint32_t value)
{
    char proc_name[128];
    int handle;
    uint32_t value_swapped = (value);
    ssize_t size;
    sprintf(proc_name, "/proc/bus/pci/%02x/%02x.%01x", bus, device, fn);
    //    printf("pci_config_write:%s: offset: %#x; val=%#x\n", proc_name, offset, value);
    handle = open(proc_name, O_RDWR);
    if (handle<0) {
        printf("%s: PCI device on bus %d, device %d, function %d doesn't exist.\n",
               __FUNCTION__, bus, device, fn);
        printf("Please run 'lspci' and check that the device is correctly enumerated on the bus.\n");
        cterr('f', 0, "Unable to write to pci device.");
        return(FAILED);
    }
    lseek(handle, offset, SEEK_SET); /* read value of bar 0 */
    size = write(handle, &value_swapped, sizeof(value_swapped));
    if (size < 0 ) {
        perror("Unable to config write\n");
    }
    fsync(handle);
    close(handle);
    return(PASSED);
}

void
pci_config_write_byte (uint32_t bus, uint16_t device, uint32_t fn, int offset, uint32_t value)
{
    pci_config_write(bus, device, fn, offset, value & 0xFF);
}


int 
flush_io_wb (void)
{
#if 0
    asm volatile ("sync");
    asm volatile ("syncw");
    asm volatile ("": : :"memory");
    do {} while (0); //for mips
#endif
    /* by indicating wastetime of 0, we tell our driver we don't want to have
       any delay. instead, we want the driver to performer
       hardware 'barrier' as well as to prevent copmiler optimization across
       the barrier.
    */
    wastetime((long)0);
    return(0);
}

void
diag_timer_open (void)
{
    if (!fd_diag_timer) {
        if ((fd_diag_timer = open("/dev/diag_timer", O_RDWR)) < 0) {
            perror("Unable to open delay driver...\n");
            exit(0);
        }
    }
    return;
}

void
diag_timer_close (void)
{
    if (fd_diag_timer) {
        if (close(fd_diag_timer)) {
            perror("nm_klm_close: Can't close");
            exit(0);
        }
        fd_diag_timer = 0;
    }
}

void
print_offset_val (char *str1, unsigned long base, unsigned long addr,
                  unsigned int line, char *f)
{
    unsigned int *ptr32;
    if (f)
        printf("file %s", f);

    if (line)
        printf("line %d;  ", line);

    printf("\n");

    if (addr < base) {
        printf("line %d; file %s", line, f);
        printf("addresses not valid; base @%#lx; device @%#lx\n",
               base, addr);
        fflush(stdout);
        exit(0);
    }
    ptr32 =  (unsigned int *)addr;
    printf("%s @%#x = %#x;  ", str1, (unsigned int)(addr-base), *ptr32);
    
    fflush(stdout);
}

void
print_offset (char *str1, unsigned long base, unsigned long addr,
              unsigned int line, char *file)
{

    if (addr < base) {
        printf("line %d; file %s\n", line, file);
        printf("addresses not valid; base @%#lx; device @%#lx\n",
               base, addr);
        fflush(stdout);
        exit(0);
    }
    if (file)
        printf("file %s", file);

    if (line)
        printf("line %d;  ", line);
    
    printf("\n");
    printf("%s @%#x ", str1, (unsigned int)(addr-base));
    fflush(stdout);
}

int
get_line (char *ptr, unsigned int size)
{
    int i;
    unsigned int len = 0;
    fgets(ptr, size, stdin);
    len = strlen(ptr);
    for (i=0;i<2;i++) {
        if (len) {
            if (ptr[len - 1] == '\r' || ptr[len - 1] == '\n') {
                ptr[len - 1] = '\0';
                len--;
            }
        }
    }

    return len;
}

int
readfile (const char *varname, unsigned char *hex,
         unsigned int size)
{
    FILE *fp;

    //    printf("file name is %s\n", varname);

    fp = fopen(varname, "r");
    if (fp == NULL) {
        perror("can't open binary file.");
        return -1;
    }
    
    fread(hex, size, 1, fp);
    if (ferror(fp)) {
        perror("can't read from binary file.");
        return -1;
    }
#if 0    
    for (i=0;i<10;i++, tmp++) {
        printf("%#x, ", *tmp);
    }
#endif
    fclose(fp);

    return 0;
}

/*
 * Function: file_exist: check of file exists.
 * Input . char dest, file name including absoulute path.
 * 
 * Output:  size -- file size
 *
 * returns:  1 if file exists  0; otherwise 0.
 *
 */
int
file_exist (char *dest, size_t *size)
{
    struct stat st;

    memset(&st, 0, sizeof(struct stat));
    if ( (stat(dest, &st) < 0) ) {
        /* errno == ENOENT if file doens exist */
        return(0);
    }

    *size = st.st_size;
   
    return(1);
}

/*
 * Function: get_ip
 *   get ip address of the system, given the name of interface
 * Input . char if_name, iternface name, ie "eth0", 
 * 
 * Output: - char ip_addr, ip address of interface, ie, 192.168.0.252
 *
 */
int
get_ip (const char *if_name, char *ip_addr) {

    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(-1);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                            host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }
            // printf("<Interface>: %s \t <Address> %s\n", ifa->ifa_name, host);
            if (!strcmp(ifa->ifa_name, ip_addr)) {
                //  printf("hey <Interface>: %s \t <Address> %s\n", ifa->ifa_name, host);
                sprintf(ip_addr, "%s", host);
            }
        }
    }
    return 0;
}
/*************************************************************
 * Function: ttf2array
 * Description:  open ttf file, read its contents, swap bits,
 * and stored in array. 
 * Input: size - number of bytes to read,
 *        file - file name
 *        fpga - array to store file content.
 * Output: return number of bytes found from ttf file
 *************************************************************
 */
int
ttf2array (int size, const char *file, unsigned char *fpga)
{

    FILE *fp_s;
    unsigned int c, i, line;
    unsigned int val;
    char *tmp = (char *)fpga;
    
    fp_s = fopen(file, "r");
    if (!fp_s) {
        printf("\n\ncan't open %s\n\n", file);
        exit(0);
    }

    i = 1;
    line = 1;
    while (!feof(fp_s)) {
        if (fscanf(fp_s, "%d", &val) == EOF) {
            printf("problem scanning number.\n");
            goto out;
        }

        *tmp++ = swapbyte(val);

        if ((c = fgetc(fp_s)) == EOF) {
            printf("end of file. no more characters. %d values.  %d lines\n", i, line);
            goto out;

        } else {
            if (c == ',') {
                
            } else {
                printf("File read successfully. %d values found.\n", i);
                goto out;
            }
        }
        i++;
    }
    
 out:
    fclose(fp_s);
    return i;
}


/*
 * Function: logfile
 *  log data into a file as text
 * Input . char name : file name
 *         data: data to log
 *         len: len of data
 * Output: - always return pass for now.
 *
 */
int
logfile (const char *name, char *data, int len)
{
    FILE *fp;
    if (name) {
        if (!strcmp(name, "none")) {
            return(PASS);
        }
        fp = fopen(name, "w");
        if (!fp) {
            return(PASS);
        }
    } else {
        printf("\n");
        fp = stdout;
    }

    print_hex_dump(fp, 0, "@", DUMP_PREFIX_OFFSET, 16, 1, data, len, 1);
    fprintf(fp, "\n");
    fflush(fp);
    fclose(fp);
    
    return(PASS);
}

/* descrption: check /proc/modules to see if driver is loaded
 * input: char *name -- name of driver
 * output: 1 if driver has been loaded; 0 , otherwise
 *
 */
int
driver_loaded (char *name)
{
    char line[TESTNAMEBUFSIZ];
    FILE *fp;
    int ix;

    /* check if modules loaded ok */
    line[0] = '\0';
    if ((fp = fopen("/proc/modules", "r")) == NULL)
        return(0);

    while (fgets(line, TESTNAMEBUFSIZ, fp) != NULL) {
        ix = 0;
        while(line[ix] != '\n' && ix < TESTNAMEBUFSIZ ) {
            if (line[ix] == ' ') {
                line[ix] = '\0';
                break;
            }
            ix++;
        }
        if (ix == TESTNAMEBUFSIZ)
            continue;
        
        if (strcmp(line, name)==0) {
            fclose(fp);
            return(1);
        }
    }
    fclose(fp);
    return(0);
}

int
linux_echo (char *name, char * buf, char *option)
{
    FILE *fp;
    int ret;
    /* check if modules loaded ok */
    if ((fp = fopen(name, option)) == NULL)
        return(FAIL);

    ret = fputs(buf, fp);
        
    fclose(fp);
    
    return(ret);
}
#ifdef RHEL8_GCC_4_7_0_P5
/* On x86-64 Linux with glibc, RHEL7 link against the 2.2.5 version of clock_gettime() so
 * that we avoid depending on the 2.17 version of the symbol on RHEL8 server. 
 * Some project still using old version kenerl, and glibc didn't support 2.17 versions.
 * Define a wrapper function to "Symbolic Specific Version" at the link stage to take over 
 * all references with clock_gettime() function.
 * This wrapper is enabled by passing the linker flags -Wl,--wrap=clock_gettime in Makefile
 */
void *__clock_gettime_glibc_2_2_5(clockid_t, struct timespec *);
__asm__(".symver __clock_gettime_glibc_2_2_5, clock_gettime@GLIBC_2.2.5");
void *__wrap_clock_gettime(clockid_t clk_id, struct timespec *tp)
{
    return __clock_gettime_glibc_2_2_5(clk_id, tp); 
}
#endif
/*
$Log: linux_api.c,v $
Revision 1.23  2021/09/24 01:27:20  harrchan
Collapse Elixir-branch to Main Trunk.

Revision 1.22  2021/09/13 02:03:46  iachang
CSCvz45877 : Migrate ISR Platform common code From RHEL7 to RHEL8

Revision 1.21  2018/05/18 09:24:48  alpeng
 Neptune merge to trunk with tag <neptune-branch-0518>

Revision 1.20.40.1  2016/10/28 08:30:52  alpeng
fixed enhance error msg bug, add more info on pcird/wr, update testcard plx scan test

Revision 1.20  2014/05/31 00:42:23  mcharon
change bytes to values to make it clear

Revision 1.19  2014/05/29 00:37:41  mcharon
rename bin2hex to readfile

Revision 1.18  2014/02/06 05:08:02  mcharon
fix get_ip; replace 'eth0' with function parameter

Revision 1.17  2014/02/04 18:53:04  mcharon
add function 'driver_loaded' to check if driver is loaded

Revision 1.16  2013/12/18 00:24:40  mcharon
file_exist now returns size of file

Revision 1.15  2013/11/26 08:40:33  hroni
fix compiler warning

Revision 1.14  2013/11/11 21:18:39  mcharon
pass string instead of number in first argum of host_send_packet ; add xaui support

Revision 1.13  2013/10/08 08:48:26  tirawan
Woodlawn collapsed to main trunk

Revision 1.12  2013/05/02 17:27:48  mcharon
move ttf2array to linux_api.c

Revision 1.11  2013/05/01 20:43:48  mcharon
add error messages to pci_config_read/write

Revision 1.10  2013/04/17 17:22:04  mcharon
improve pci error message

Revision 1.9  2013/02/13 18:25:13  mcharon
add function to get ip address of system interface

Revision 1.8  2013/01/15 02:19:18  ptong
Fix a type warning in the sprintf() in file_exist()

Revision 1.7  2013/01/14 21:45:06  mcharon
move file_exist to linux_api.c

Revision 1.6  2012/09/18 19:18:06  mcharon
 add clrtestname

Revision 1.5  2012/08/07 17:46:30  mcharon
include bin2hex support

$Endlog$
*/
