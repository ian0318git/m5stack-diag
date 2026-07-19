/* $Id: linux_api.c,v 1.2 2015/05/25 03:59:15 steja Exp $ 
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/skye_linux/src/linux_api.c,v $ 
 *------------------------------------------------------------------
 * File: linux_mmap.c
 *
 * April 29, 2013 - iachang ported from Overlord.
 *
 * Copyright (c) 2013 ~ 2015 by Cisco Systems, Inc.
 * All rights reserved.
 *------------------------------------------------------------------
 */
#include "types.h"
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/poll.h>

#include "linux_api.h"
#include "byteswap.h"
#include "menu.h"
#include "mem_mgr.h"
#include "common.h"
static int fd_diag_timer = 0;

/*
 * Function: is_le
 * Less endian
 *
 * Input NONE
 *
 * Output: 1 / 0
 */
int
is_le (void)
{
    int num = 1;
    unsigned char *ptr = (unsigned char*)&num;
    if (*ptr) {
        return 1;
    }
    return 0;
}


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

/*
 * Function: smart_cookie_delay
 *
 * Input  us_delay_count
 *
 * Output: None
 */
void
smart_cookie_delay (long us_delay_count)
{
    wastetime(us_delay_count);
}

/*
 * Function: msleep
 *
 * Input  time
 *
 * Output: None
 */
void
msleep(unsigned long t)
{
    usleep(t*1000);
}

/*
 * Function: mdelay
 *
 * Input  time
 *
 * Output: None
 */
void
mdelay(unsigned long t)
{
    usleep(t*1000);
}

/*
 * Function: udelay
 *
 * Input  time
 *
 * Output: None
 */
void
udelay(unsigned long t)
{
    usleep(t);
}

/*
 * Function: pci_config_read
 *
 * Input  bus - interface bus
 *        device - interface dev
 *        fn  - function dev
 *        offset - offset dev
 *
 * Output: return value
 */
uint32_t
pci_config_read(uint32_t bus, uint16_t device, uint32_t fn, int offset)
{
    char proc_name[128];
    int handle;
    uint32_t value = 0xFFFFFFFF;

    sprintf(proc_name, "/proc/bus/pci/%02x/%02x.%01x", bus, device, fn);
    handle = open(proc_name, O_RDWR);
    if (handle<0) {
        close(handle);
        printf("PCI device on bus %d, device %d, function %d doesn't exist\n",
               bus, device, fn);
        printf("Please run 'lspci' and check that the device is correctly enumerated on the bus.\n");
        cterr('f', 0,"unable to read frome pci device.");
        return(value);
    }
    lseek(handle, offset, SEEK_SET); /* read value of bar 0 */
    read(handle, &value, sizeof(value));
    close(handle);
    return (value);

}

/*
 * Function: pci_config_read_byte
 *
 * Input  bus - interface bus
 *        device - interface dev
 *        fn  - function dev
 *        offset - offset dev
 *
 * Output: return value
 */
uint32_t
pci_config_read_byte(uint32_t bus, uint16_t device, uint32_t fn, int offset)
{
    return (pci_config_read_byte(bus, device, fn, offset) & 0xFF);

}

/*
 * Function: pci_config_write
 *
 * Input  bus - interface bus
 *        device - interface dev
 *        fn  - function dev
 *        offset - offset dev
 *        value - input integer
 *
 * Output: return FAILED/PASSED
 */
uint32_t
pci_config_write(uint32_t bus, uint16_t device, uint32_t fn, int offset, uint32_t value)
{
    char proc_name[128];
    int handle;
    uint32_t value_swapped = (value);
    ssize_t size;
    sprintf(proc_name, "/proc/bus/pci/%02x/%02x.%01x", bus, device, fn);
    handle = open(proc_name, O_RDWR);
    if (handle<0) {
        printf("PCI device on bus %d, device %d, function %d doesn't exist.\n",
               bus, device, fn);
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

/*
 * Function: pci_config_write_byte
 *
 * Input  bus - interface bus
 *        device - interface dev
 *        fn  - function dev
 *        offset - offset dev
 *        value - input integer
 *
 * Output: return FAILED/PASSED
 */
void
pci_config_write_byte(uint32_t bus, uint16_t device, uint32_t fn, int offset, uint32_t value)
{
    pci_config_write(bus, device, fn, offset, value & 0xFF);
}

/*
 * Function: flush_io_wb
 *
 * Input  none
 *
 * Output: return 0
 */
int 
flush_io_wb (void)
{
    /* by indicating wastetime of 0, we tell our driver we don't want to have
       any delay. instead, we want the driver to performer
       hardware 'barrier' as well as to prevent copmiler optimization across
       the barrier.
    */
    wastetime((long)0);
    return(0);
}

/*
 * Function: diag_timer_open
 *
 * Input  none
 *
 * Output: return none
 */
void
diag_timer_open(void)
{
    if (!fd_diag_timer) {
        if ((fd_diag_timer = open("/dev/diag_timer", O_RDWR)) < 0) {
            perror("Unable to open delay driver...\n");
            exit(0);
        }
    }
    return;
}

/*
 * Function: diag_timer_close
 *
 * Input  none
 *
 * Output: none
 */
void
diag_timer_close(void)
{
    if (fd_diag_timer) {
        if (close(fd_diag_timer)) {
            perror("nm_klm_close: Can't close");
            exit(0);
        }
        fd_diag_timer = 0;
    }
}

/*
 * Function: print_offset_val
 *
 * Input  str1 - string
 *        base - base for device
 *        addr - address for device
 *        line - line for device
 *        f   - file for device
 *
 * Output: none
 */
void
print_offset_val (unsigned char *str1, unsigned long base, unsigned long addr,
                  unsigned int line, unsigned char *f)
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

/*
 * Function: print_offset
 *
 * Input  str1 - string
 *        base - base for device
 *        addr - address for device
 *        line - line for device
 *        file - file for device
 *
 * Output: none
 */
void
print_offset (unsigned char *str1, unsigned long base, unsigned long addr,
              unsigned int line, unsigned char *file)
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

/*
 * Function: get_line
 *
 * Input  ptr - pointer to string
 *        size - size of string
 *
 * Output: len of string
 */
int get_line(char *ptr, unsigned int size)
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

/*
 * Function: bin2hex
 *
 * Input  varname - pointer to variable
 *        hex - pointer to hex
 *        size - size of binary
 *
 * Output: -1 /0
 */
int
bin2hex (const char *varname, unsigned char *hex,
         unsigned int size)
{
    FILE *fp;

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
#if DEBUG
    for (i=0;i<10;i++, tmp++) {
        printf("%#x, ", *tmp);
    }
#endif
    fclose(fp);

    return 0;
}

/*
 * Function: file_exist
 *   check of file exists.
 * Input . char dest, file name including absoulute path.
 * 
 * Output: - returns TRUE if file exists; otherwise FALSE.
 *
 */
int
file_exist (unsigned char *dest)
{
    if (access((const char *)dest, F_OK) == 0) {
        return(TRUE);
    }
    return(FALSE);
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
            if (!strcmp(ifa->ifa_name, "eth0")) {
                sprintf(ip_addr, "%s", host);
            }
        }
    }
    return 0;
}

/*
** Return the value for the1;,.e ascii hex character or -1 if invalid.
*/
char
atoh(char c)
{
  if(c >= '0' && c <= '9') return(c - '0');
  if(c >= 'A' && c <= 'F') return(c - ('A' - 10));
  if(c >= 'a' && c <= 'f') return(c - ('a' - 10));

  return(-1);
}

int
getnnum(char *cptr, int base, utype_t *longret, int maxchars)
     /*cptr : character buffer pointer */
     /*longret : for the result */
{
  char cval;
  unsigned long value = 0; /* init */
  int count = 0; /* init */

  while(1) {
    cval = atoh(*cptr);
    if(cval < 0 || cval >= base) break;  /* invalid character encountered */
    value = (value * base) + cval;
    cptr++;
    count++;
    if(maxchars && count == maxchars) break;
  }
  *longret = value;  /* place result */
  return(count);
}

/*
** Convert the ascii string pointed to by cptr to binary according to base.
** Result is placed in *longret.
** Return value is the number of characters processed.
** Maxchars defines the maximum number of characters to process.  If
** maxchars == 0, process until an invalid character occurs.
** Getnum exists for historical reasons.
*/
int
getnum(char *cptr, int base, utype_t *longret)
     /*cptr : character buffer pointer */
     /*longret : for the result */
{
  return(getnnum(cptr, base, longret, 0));
}


/******** History ********/ 
/*
 *------------------------------------------------------------------
 * $Log: linux_api.c,v $
 * Revision 1.2  2015/05/25 03:59:15  steja
 * Add Support Skye SM
 *
 * Revision 1.1.4.5  2015/05/11 13:45:45  steja
 * Code clean up <CSCuu14285>
 *
 * Revision 1.1.4.4  2015/04/30 08:33:53  steja
 * Clean up code
 *
 * Revision 1.1.4.3  2015/04/30 03:01:43  palin2
 * code clean up.
 *
 * Revision 1.1.4.2  2015/04/29 11:36:32  steja
 * Code check-in to skye-branch2 for ER code review
 * 
 *------------------------------------------------------------------
 * Revision 1.1.2.1  2014/07/21 01:56:53  palin2
 * Initial check-in Skye module side Diag code.
 *
 *------------------------------------------------------------------
 * Revision 1.2  2014/02/27 15:01:48  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.4.5  2014/02/13 01:35:15  palin2
 * 1. Removed file "linux_stub.c" and update Makefile.
 * 2. Moved necessary function "getnum" from "linux_stub.c" to "linux_api.c".
 *
 * Revision 1.1.4.4  2014/02/07 03:36:52  steja
 * code clean up
 *
 * Revision 1.1.4.3  2013/09/16 09:50:15  iachang
 * Code review and update
 *
 * Revision 1.1.4.2  2013/09/13 07:00:07  palin2
 * Initial check-in ShrinkRay SM side Diag code.
 *
 * Revision 1.1.2.1  2013/04/29 08:25:07  iachang
 * Support memory test
 *
 *------------------------------------------------------------------
 * $Endlog$
 */

