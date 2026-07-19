/* $Id: cpss_extserv.c,v 1.2 2021/09/24 01:21:05 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/elixir/cpss_extserv.c,v $
 *------------------------------------------------------------------
 *
 * Filename:	cpss_extserv.c
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "cpss_extserv.h"
#include "cross_platform.h"
#include "plat_defs.h"
#include "dash_fpga.h"



extern int get_bus_num(uint mod_type, uint slot);

char cpss_pcie_driver_name[] = "ac5_driver";

/*
 * the slot/bay bound to the dm hw lib instance
 */
unsigned int dm_cpss_slot = 0xFFFFFFFF, dm_cpss_bay = 0xFFFFFFFF;

/*
 * the main_fd is used to global configuration, it may not be opened
 * if it is hold by other instance.
 * the sub_fd is used for slot/bay specific operation and should be valid.
 */
int main_fd = -1, sub_fd = -1;

/*
 * the base address and size of the mapped config space and ppregs space.
 */
unsigned long config_addr = 0;
unsigned long config_size = 0;
unsigned long ppregs_addr = 0;
unsigned long ppregs_size = 0;


int cpss_pcie_extserv_init_ex ()
{
    int ret = 0;
    int count = 5;
    char cdev_pathname[256];
    char cmd[256];
    pcie_mapping_array_t *map_array;
    

    snprintf(cdev_pathname, sizeof(cdev_pathname),
            "/dev/%s", cpss_pcie_driver_name);

    main_fd = open(cdev_pathname, O_RDWR);
    if(main_fd < 0) {
        if(errno == EBUSY) {
            /*
             * Another instance has token the ownership of main cdev
             */
            printf("The dm main cdev has been opened by other instance.");

        } else if (errno == ENOENT) {
            /*
             * The first instance is responsible for loading kernel module
             * And set the pcie port mapping for the host
             */
#ifdef DEBUG
            printf("The dm instance load the dm kennel "
                 "module.");
#endif
            snprintf(cmd, sizeof(cmd), "modprobe %s dm_drv_log_level=0x100",
                     cpss_pcie_driver_name);

            ret = system(cmd);
            if (ret == -1) {
                printf("The dm instance fail to load the dm "
                       "kennel module. Error = %d", ret);
                return (ret);
            }

            do {
                main_fd = open(cdev_pathname, O_RDWR);
                if(main_fd >= 0)
                    break;

                if (errno == EBUSY) {
                    /*
                     * Another instance has token the ownership of main cdev
                     */
                    printf("The dm main cdev has been opened by other instance, "
                           " Although it has loaded the "
                           "kernel module.");
                    return (0);
                } else if (errno == ENOENT) {
#ifdef DEBUG
                    printf("The dm instance wait for the main "
                          "cdev file created by udev. Count = %u", count);
#endif
                    usleep(200*2000);
                    continue;
                } else {
                    printf("The dm instance fail to open the "
                           "main cdev file after loading the dm kennel module. "
                           "Errno = %d", errno);
                    return (errno);
                }

            } while(--count > 0);

            if (count == 0) {
                printf("The dm instance fail to wait for the "
                       "main cdev file created by udev after loading dm kennel "
                       "module. Errno = %d", errno);

                return (errno);
            }

        } else {
            printf("The dm instance  fail to open the main cdev "
                "file. Errno = %d", errno);
            return (errno);
        }
    }

    /*
     * the main fd is opened successfully. Maybe it is the first dm instance
     * Or the original first instance has removed.
     */
    if (main_fd >= 0) {
#ifdef DEBUG
        INFO("The dm instance has opened the main cdev file. "
             "main_fd = %d", main_fd);
#endif
        /*
         * Get the pcie port mapping fort the host
         */
        map_array = cpss_pcie_get_pcie_mapping();
        if (map_array == NULL) {
            printf("The dm instance  fail to get the pcie port "
                   "mapping.");
            return (-1);
        }

        ret = ioctl(main_fd, DM_MAIN_SET_PCIE_MAPPING, (unsigned long)map_array);
        if (ret) {
            printf("The dm instance  fail to set the pcie port "
                   "mapping. Error = %d", ret);
            close(main_fd);
            main_fd = -1;
            cpss_pcie_put_pcie_mapping(map_array);
            return (ret);
        }
#ifdef DEBUG
        INFO("The dm instance of slot/bay(%u/%u) has set the pcie port "
             "mapping.", slot, bay);
#endif
        cpss_pcie_put_pcie_mapping(map_array);
    }

    system(PCI_RESCAN);

    /*
     * open the sub cdev for special dm instance
     */
    snprintf(cdev_pathname, sizeof(cdev_pathname),
             "/dev/%s_0_1", cpss_pcie_driver_name);
    count = 10;

    /*
     * wait for the dm powered on in a loop. 
     */
    do {
        sub_fd = open(cdev_pathname, O_RDWR);
        if (sub_fd >= 0)
            break;

        if (errno == EBUSY) {
            /*
             * Another instance has token the ownership of sub cdev
             */
            printf("The dm sub cdev has been opened by other instance.");

            return (-1);
        } else if (errno == ENOENT) {
#ifdef DEBUG
            INFO("The dm instance  wait for the sub cdev file "
                 "created by udev.");
#endif
            usleep(200*2000);
            continue;
        } else {
            printf("The dm instance fail to open the sub cdev "
                "file.Errno = %d", errno);
            return (errno);
        }

    } while(--count > 0);

    if (count == 0) {
	    return (errno);
    }

    /*
     * map the config and ppresg area to user space for cpss lib
     */
    ret = extDrvPciMapEx((GT_UINTPTR *)&ppregs_addr, (GT_SIZE_T *)&ppregs_size,
                         (GT_UINTPTR *)&config_addr, (GT_SIZE_T *)&config_size);

    if (ret) {
        printf("The dm instance  fail to map the config and "
               "ppreg memory space.");

    } else {
#ifdef DEBUG
        INFO("The dm instance of slot/bay(%u/%u) succeeds to map the config "
             ",ppreg and exreg memory space(%lx, %lx).",
             slot, bay,
             config_addr, config_size, ppregs_addr, ppregs_size);
#endif
    }
    return (ret);
    
}

void cpss_pcie_bar2_resize ()
{
    unsigned long long bar2Phys = 0x900400000;
    long BAR2_SIZE_REG_ADDR = 0x800A4018;
    int sizeRegBasePhys = BAR2_SIZE_REG_ADDR & 0xfff00000;
    int sizeRegOffset = BAR2_SIZE_REG_ADDR & 0x000fffff;
    int bar2LimitPhys = bar2Phys | 0x000fffff;

    /*  Open window in BAR2, remapped to CnM RFU base */
    *((volatile GT_U32 *)(config_addr + 0x1300)) = 0;
    *((volatile GT_U32 *)(config_addr + 0x1304)) = 0x80000000;
    *((volatile GT_U32 *)(config_addr + 0x1308)) = bar2Phys;
    *((volatile GT_U32 *)(config_addr + 0x130c)) = 0;
    *((volatile GT_U32 *)(config_addr + 0x1310)) = bar2LimitPhys;
    *((volatile GT_U32 *)(config_addr + 0x1314)) = sizeRegBasePhys;
    *((volatile GT_U32 *)(config_addr + 0x1318)) = 0;
    
    /* Memory barrier before writing to register.
     * GT_SYNC doesn't help al all, and the below asm memory instruction
     * does not work with kernel 4.14 (when 64 bit addresses are assiged
     * to the BARs).
     * __asm__ __volatile__ ("" : : : "memory")
     */
    usleep(500000);

    /* Write the new size to register 0x1c164018 */
    *((volatile GT_U32 *)(ppregs_addr + sizeRegOffset)) = 0x800000 - 1;
    usleep(500000);

}

void cpss_pcie_extserv_cleanup_ex (void)
{
    int ret = 0;
    GT_VOID *int_vect;
#ifdef DEBUG
    INFO("Cleans up the main cdev(%d) and sub cdev(%d).", main_fd, sub_fd);
#endif
    if (extDrvGetPciIntVec(0, (void **)&int_vect) != GT_OK) {
        ERR("Failed to get interrupt vector number.");
    } else {
	    if (extDrvIntDisable((unsigned long)int_vect) != GT_OK) {
	        ERR("Failed to disable interrupt.");
	    }
    }

    if (sub_fd >= 0) {

        if (int_tid != 0) {
            int_exit = 1;
            ret = ioctl(sub_fd, DM_SUB_IOC_INTDISCONNECT);
            if (ret) {
                ERR("Fail to delete the ISR routine thread(%d).", ret);
            }
	        int_tid = 0;
        }

        if (config_addr != 0 && config_size != 0 &&
           ppregs_addr != 0 && ppregs_size != 0) { 
           
            ret = extDrvPciUnMapEx((GT_UINTPTR)ppregs_addr, (GT_SIZE_T)ppregs_size,
                                   (GT_UINTPTR)config_addr, (GT_SIZE_T)config_size);
                                 
            if (ret) {
                ERR("Fail to unmap config and ppreg memory space(%d).", ret);

            } else {
#ifdef DEBUG
                INFO("Succeeds to unmap config and ppreg memory space.");
#endif
            }
            ppregs_addr = 0;
            ppregs_size = 0;
            config_addr = 0;
            config_size = 0;
        }

        ret = close(sub_fd);
        if (ret) {
            ERR("Fail to close the sub fd (%d, %d).", sub_fd, errno);

        } else {
            INFO("Succeeds to close the sub fd(%d).", sub_fd);
        }
        sub_fd = -1;
    }

    if (main_fd >= 0) {
        ret = close(main_fd);
        if (ret) {
            ERR("Fail to close the main fd (%d, %d).", main_fd, errno);

        } else {
            INFO("Succeeds to close the main fd(%d).", main_fd);
        }
        main_fd = -1;
    }
}

void cpss_pcie_get_pciemap_ex (GT_UINTPTR *pciBaseAddr,
                               GT_UINTPTR *internalPciBase)
{
    *pciBaseAddr = ppregs_addr;
    *internalPciBase = config_addr;

    return;
}

void cpss_pcie_get_pciemap_size_ex (GT_UINTPTR *pciBaseAddr_size,
                                    GT_UINTPTR *internalPciBase_size)
{
    *pciBaseAddr_size = ppregs_size;
    *internalPciBase_size = config_size;

    return;
}

/*
 *------------------------------------------------------------------
 * $Log: cpss_extserv.c,v $
 * Revision 1.2  2021/09/24 01:21:05  harrchan
 * Collapse Elixir-branch to Main Trunk.
 *
 * Revision 1.1.2.2  2021/04/23 02:35:01  illiu
 * Clean up code
 *
 * Revision 1.1.2.1  2021/04/12 08:41:12  illiu
 * Add file: cpss platform code
 *
 * Revision 1.1.2.3  2021/03/03 06:57:21  illiu
 * Rename AC5 driver from nim_dm.ko to ac5_driver.ko
 *
 * Revision 1.1.2.2  2021/02/22 02:41:42  illiu
 * Clean up code
 *
 * Revision 1.1.2.1  2021/01/26 02:55:06  illiu
 * Rename nim_dm prefix file to cpss prefix
 *
 * Revision 1.1.2.4  2020/11/05 06:35:01  harrchan
 * 1.Base on P1A bring up result to Modify the AC5 MAC/internal/external loopback test
 * 2.Remove some debug message on AC5 init process
 *
 * Revision 1.1.2.3  2020/10/26 07:54:49  harrchan
 * Base on AC5 bring up reseult to modify AC5 code
 *
 * Revision 1.1.2.2  2020/10/07 09:12:43  illiu
 * Clean up code
 *
 * Revision 1.1.2.1  2020/09/09 09:18:08  illiu
 * Modified to support Dreamliner with CPSS 4.2 library.
 *
 * Revision 1.2  2019/12/11 10:10:26  lucywang
 * Merged Nanook to main trunk
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
