/* $Id: nim_dm_cpss_extserv.c,v 1.2 2019/12/11 10:10:26 lucywang Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/dreamliner_cpss41/nim_dm_cpss_extserv.c,v $
 *------------------------------------------------------------------
 * nim_dm_cpps_extserv.c : DM CPSS lib external service
 *
 * Christine Wen -- Nov. 2013
 *
 * Copyright (c) 2014-2019 by Cisco Systems, Inc.
 * All rights reserved.
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

#include "nim_dm_cpss_extserv.h"
#include "cross_platform.h"
#include "plat_defs.h"
#include "dash_fpga.h"



extern int get_bus_num(uint mod_type, uint slot);

char nim_dm_driver_name[] = "nim_dm";

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
unsigned long exregs_addr = 0;
unsigned long exregs_size = 0;

int onboard_xcat3_init = GT_FALSE;

pcie_mapping_array_t * nim_dm_get_pcie_mapping(void)
{

    pcie_mapping_array_t *map_array;
    pcie_switch_port_mapping_t *ents;
#ifdef YWEN
    INFO("The dm instance of slot/bay(%u/%u) get the pcie port mapping.",
         dm_cpss_slot, dm_cpss_bay);
#endif
    map_array = malloc(sizeof(pcie_mapping_array_t));
    if(map_array == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) fail to alloc memory for "
            "pcie port mapping.", dm_cpss_slot, dm_cpss_bay);
        return NULL;
    }

    /* extern to 6 for neptune */
    map_array->num = TOTAL_NGIO_SLOT_NUM;
    ents = calloc(TOTAL_NGIO_SLOT_NUM, sizeof(pcie_switch_port_mapping_t));
    if(ents == NULL) {
        ERR("The dm instance of slot/bay(%u/%u) fail to alloc memory for "
            "pcie port mapping ents.", dm_cpss_slot, dm_cpss_bay);
        free(map_array);
        return NULL;
    }

    /* Juno only supports 3 NIMs.
       Dagger only supports 2 NIMs.
       Sword only supports 2 NIMs and 1 SM.
       O2 and Utah support 3 NIMs and 2 SMs.
       Neptune supports 3 NIMs and 3 SMs. 
    */
    /*
     * NGWIC1
     */
    ents[0].slot = 0;
    ents[0].bay = 1;


    if ((is_nanook_plus()) && (onboard_xcat3_init == GT_TRUE)) {
        /*For Nanook + AC3 device initiailization.*/
	 ents[0].sec_bus = 0x1C;
        ents[0].sub_bus = 0x1C;
    } else {
        ents[0].sec_bus = get_bus_num(WIC_MODULE, NGWIC1_SLOT);

        if (is_goldbeach()) {
            ents[0].sub_bus = ents[0].sec_bus; /* GB platform didn't have PCIe SW */
        } else {
            ents[0].sub_bus = ents[0].sec_bus + 8;
        }
    }
    /*
     * NGWIC2
     */
    ents[1].slot = 0;
    ents[1].bay = 2;
#ifndef TACHI_INTEL 
    ents[1].sec_bus = get_bus_num(WIC_MODULE, NGWIC2_SLOT);

    if (is_goldbeach()) {
        ents[1].sub_bus = ents[1].sec_bus; /* GB platform didn't have PCIe SW */
    } else {
        ents[1].sub_bus = ents[1].sec_bus + 8;
    }
#else
    ents[1].sec_bus = 0; 
    ents[1].sub_bus = ents[1].sec_bus + 8;
#endif 

    /*
     * NGWIC3
     */
    ents[2].slot = 0;
    ents[2].bay = 3;
#ifndef TACHI_INTEL 
    if (is_dagger() || is_sword() || is_goldbeach()) 
	ents[2].sec_bus = 0;
    else
	ents[2].sec_bus = get_bus_num(WIC_MODULE, NGWIC3_SLOT);
#else
    ents[2].sec_bus = 0; 
#endif 
    ents[2].sub_bus = ents[2].sec_bus + 8;

    /*
     * NGSM1
     */
    ents[3].slot = 1;
    ents[3].bay = 0;
#ifndef TACHI_INTEL 
    if (is_dagger() || is_juno() || is_goldbeach()) 
	ents[3].sec_bus = 0;
    else
	ents[3].sec_bus = get_bus_num(SM_MODULE, NGSM1_SLOT);
#else 
    ents[3].sec_bus = 0;
#endif 
    ents[3].sub_bus = ents[3].sec_bus + PCI_RESOURCE_RANGE;

    /*
     * NGSM2
     */
    ents[4].slot = 2;
    ents[4].bay = 0;
#ifndef TACHI_INTEL 
    if (is_dagger() || is_sword() || is_juno() || is_goldbeach()) 
	ents[4].sec_bus = 0;
    else
	ents[4].sec_bus = get_bus_num(SM_MODULE, NGSM2_SLOT);
#else 
    ents[4].sec_bus = 0;
#endif 
    ents[4].sub_bus = ents[4].sec_bus + PCI_RESOURCE_RANGE;

    /*
     * NGSM3
     */
    ents[5].slot = 3;
    ents[5].bay = 0;
#ifndef TACHI_INTEL
    if (is_neptune()) {
        ents[5].sec_bus = get_bus_num(SM_MODULE, NGSM3_SLOT);
    } else {
        ents[5].sec_bus = 0;
    }
#else
    ents[5].sec_bus = 0;
#endif
    ents[5].sub_bus = ents[5].sec_bus + PCI_RESOURCE_RANGE;

    map_array->ents = (unsigned long)ents;

    return map_array;

}


void nim_dm_put_pcie_mapping(pcie_mapping_array_t *map_array)
{
#ifdef YWEN
    INFO("The dm instance of slot/bay(%u/%u) put the pcie pcie_mapport mapping.",
         dm_cpss_slot, dm_cpss_bay);
#endif
    free((void *)(unsigned long)map_array->ents);
    free(map_array);
}


int nim_dm_cpss_extserv_init_ex(unsigned int slot, unsigned int bay)
{
    int ret = 0;
    int count = 5;
    char cdev_pathname[256];
    char cmd[256];
    pcie_mapping_array_t *map_array;
#ifdef YWEN
    INFO("Initialize dm cpss external services for slot/bay(%u/%u).\n",
         slot, bay);
#endif
    dm_cpss_slot = slot;
    dm_cpss_bay = bay;

    snprintf(cdev_pathname, sizeof(cdev_pathname),
            "/dev/%s", nim_dm_driver_name);

    main_fd = open(cdev_pathname, O_RDWR);
   if(main_fd < 0) {
        if(errno == EBUSY) {
            /*
             * Another instance has token the ownership of main cdev
             */
            INFO("The dm main cdev has been opened by other instance, "
                 "not slot/bay(%u/%u).", slot, bay);

        } else if (errno == ENOENT) {
            /*
             * The first instance is responsible for loading kernel module
             * And set the pcie port mapping for the host
             */
#ifdef YWEN
            INFO("The dm instance of slot/bay(%u/%u) load the dm kennel "
                 "module.", slot, bay);
#endif
            snprintf(cmd, sizeof(cmd), "modprobe %s dm_drv_log_level=0x100",
                     nim_dm_driver_name);

            ret = system(cmd);
            if(ret == -1) {
                ERR("The dm instance of slot/bay(%u/%u) fail to load the dm "
                    "kennel module. Error = %d", slot, bay, ret);
                return ret;
            }

            do {
                main_fd = open(cdev_pathname, O_RDWR);
                if(main_fd >= 0)
                    break;

                if(errno == EBUSY) {
                    /*
                     * Another instance has token the ownership of main cdev
                     */
                    INFO("The dm main cdev has been opened by other instance, "
                         "not slot/bay(%u/%u). Although it has loaded the "
                         "kernel module.", slot, bay);
                    return 0;
                } else if (errno == ENOENT) {
#ifdef YWEN
                    INFO("The dm instance of slot/bay(%u/%u) wait for the main "
                         "cdev file created by udev. Count = %u",
                         slot, bay, count);
#endif
                    usleep(200*2000);
                    continue;
                } else {
                    ERR("The dm instance of slot/bay(%u/%u) fail to open the "
                        "main cdev file after loading the dm kennel module. "
                        "Errno = %d", slot, bay, errno);
                    return errno;
                }

            }while(--count > 0);

            if(count == 0) {
                ERR("The dm instance of slot/bay(%u/%u) fail to wait for the "
                    "main cdev file created by udev after loading dm kennel "
                    "module. Errno = %d", slot, bay, errno);

                return errno;
            }

        } else {
            ERR("The dm instance of slot/bay(%u/%u) fail to open the main cdev "
                "file. Errno = %d", slot, bay, errno);
            return errno;
        }
    }

    /*
     * the main fd is opened successfully. Maybe it is the first dm instance
     * Or the original first instance has removed.
     */
    if(main_fd >= 0) {
#ifdef YWEN
        INFO("The dm instance of slot/bay(%u/%u) has opened the main cdev file. "
             "main_fd = %d", slot, bay, main_fd);
#endif

        if (is_nanook_plus()) {
            onboard_xcat3_init = GT_TRUE;
        }        
        /*
         * Get the pcie port mapping fort the host
         */
        map_array = nim_dm_get_pcie_mapping();
        if(map_array == NULL) {
            ERR("The dm instance of slot/bay(%u/%u) fail to get the pcie port "
                "mapping.", slot, bay);
            return -1;
        }
	 if (is_nanook_plus()) {
            onboard_xcat3_init = GT_FALSE;
        }     

        ret = ioctl(main_fd, DM_MAIN_SET_PCIE_MAPPING, (unsigned long)map_array);
        if(ret) {
            ERR("The dm instance of slot/bay(%u/%u) fail to set the pcie port "
                "mapping. Error = %d", slot, bay, ret);
            close(main_fd);
            main_fd = -1;
            nim_dm_put_pcie_mapping(map_array);
            return ret;
        }
#ifdef YWEN
        INFO("The dm instance of slot/bay(%u/%u) has set the pcie port "
             "mapping.", slot, bay);
#endif
        nim_dm_put_pcie_mapping(map_array);
    }

   system(PCI_RESCAN);

    /*
     * open the sub cdev for special dm instance
     */
    snprintf(cdev_pathname, sizeof(cdev_pathname),
             "/dev/%s_%u_%u", nim_dm_driver_name, slot, bay);
    count = 10;
    /*
     * wait for the dm powered on in a loop. 
     */
    do {
        sub_fd = open(cdev_pathname, O_RDWR);
        if(sub_fd >= 0)
            break;

        if(errno == EBUSY) {
            /*
             * Another instance has token the ownership of sub cdev
             */
            ERR("The dm sub cdev(%u/%u) has been opened by other instance.",
                 slot, bay);

            return -1;
        } else if (errno == ENOENT) {
#ifdef YWEN
            INFO("The dm instance of slot/bay(%u/%u) wait for the sub cdev file "
                 "created by udev.", slot, bay);
#endif
            usleep(200*2000);
            continue;
        } else {
            ERR("The dm instance of slot/bay(%u/%u) fail to open the sub cdev "
                "file.Errno = %d", slot, bay, errno);
            return errno;
        }

    } while(--count > 0);

    if(count == 0) {
	ERR("The dm instance of slot/bay(%u/%u) fail to wait for the "
	    "sub cdev file created by udev after loading dm kennel "
	    "module. Errno = %d", slot, bay, errno);
	close(main_fd);
	main_fd = -1;
	return errno;
    }

#ifdef YWEN
    INFO("The dm instance of slot/bay(%u/%u) has opened the sub cdev file. "
         "sub_fd = %d", slot, bay, sub_fd);
#endif
    /*
     * map the config and ppresg area to user space for cpss lib
     */
    ret = extDrvPciMapEx((GT_UINTPTR *)&ppregs_addr, (GT_SIZE_T *)&ppregs_size,
                       (GT_UINTPTR *)&config_addr, (GT_SIZE_T *)&config_size, (GT_UINTPTR *)&exregs_addr, (GT_SIZE_T *)&exregs_size);
    if(ret) {
        ERR("The dm instance of slot/bay(%u/%u) fail to map the config and "
            "ppreg memory space.", slot, bay);

    } else {
#ifdef YWEN
        INFO("The dm instance of slot/bay(%u/%u) succeeds to map the config "
             ",ppreg and exreg memory space(%lx, %lx)(%lx, %lx).",
             slot, bay,
             config_addr, config_size, ppregs_addr, ppregs_size, exregs_addr, exregs_size);
#endif
    }

    return ret;
}


int nim_dm_cpss_extserv_init(unsigned int slot, unsigned int bay)
{
    int ret = 0;
    int count = 5;
    char cdev_pathname[256];
    char cmd[256];
    pcie_mapping_array_t *map_array;
#ifdef YWEN
    INFO("Initialize dm cpss external services for slot/bay(%u/%u).\n",
         slot, bay);
#endif
    dm_cpss_slot = slot;
    dm_cpss_bay = bay;

    snprintf(cdev_pathname, sizeof(cdev_pathname),
            "/dev/%s", nim_dm_driver_name);

    main_fd = open(cdev_pathname, O_RDWR);
   if(main_fd < 0) {
        if(errno == EBUSY) {
            /*
             * Another instance has token the ownership of main cdev
             */
            INFO("The dm main cdev has been opened by other instance, "
                 "not slot/bay(%u/%u).", slot, bay);

        } else if (errno == ENOENT) {
            /*
             * The first instance is responsible for loading kernel module
             * And set the pcie port mapping for the host
             */
#ifdef YWEN
            INFO("The dm instance of slot/bay(%u/%u) load the dm kennel "
                 "module.", slot, bay);
#endif
            snprintf(cmd, sizeof(cmd), "modprobe %s dm_drv_log_level=0x100",
                     nim_dm_driver_name);

            ret = system(cmd);
            if(ret == -1) {
                ERR("The dm instance of slot/bay(%u/%u) fail to load the dm "
                    "kennel module. Error = %d", slot, bay, ret);
                return ret;
            }

            do {
                main_fd = open(cdev_pathname, O_RDWR);
                if(main_fd >= 0)
                    break;

                if(errno == EBUSY) {
                    /*
                     * Another instance has token the ownership of main cdev
                     */
                    INFO("The dm main cdev has been opened by other instance, "
                         "not slot/bay(%u/%u). Although it has loaded the "
                         "kernel module.", slot, bay);
                    return 0;
                } else if (errno == ENOENT) {
#ifdef YWEN
                    INFO("The dm instance of slot/bay(%u/%u) wait for the main "
                         "cdev file created by udev. Count = %u",
                         slot, bay, count);
#endif
                    usleep(200*2000);
                    continue;
                } else {
                    ERR("The dm instance of slot/bay(%u/%u) fail to open the "
                        "main cdev file after loading the dm kennel module. "
                        "Errno = %d", slot, bay, errno);
                    return errno;
                }

            }while(--count > 0);

            if(count == 0) {
                ERR("The dm instance of slot/bay(%u/%u) fail to wait for the "
                    "main cdev file created by udev after loading dm kennel "
                    "module. Errno = %d", slot, bay, errno);

                return errno;
            }

        } else {
            ERR("The dm instance of slot/bay(%u/%u) fail to open the main cdev "
                "file. Errno = %d", slot, bay, errno);
            return errno;
        }
    }

    /*
     * the main fd is opened successfully. Maybe it is the first dm instance
     * Or the original first instance has removed.
     */
    if(main_fd >= 0) {
#ifdef YWEN
        INFO("The dm instance of slot/bay(%u/%u) has opened the main cdev file. "
             "main_fd = %d", slot, bay, main_fd);
#endif
        /*
         * Get the pcie port mapping fort the host
         */
        map_array = nim_dm_get_pcie_mapping();
        if(map_array == NULL) {
            ERR("The dm instance of slot/bay(%u/%u) fail to get the pcie port "
                "mapping.", slot, bay);
            return -1;
        }

        ret = ioctl(main_fd, DM_MAIN_SET_PCIE_MAPPING, (unsigned long)map_array);
        if(ret) {
            ERR("The dm instance of slot/bay(%u/%u) fail to set the pcie port "
                "mapping. Error = %d", slot, bay, ret);
            close(main_fd);
            main_fd = -1;
            nim_dm_put_pcie_mapping(map_array);
            return ret;
        }
#ifdef YWEN
        INFO("The dm instance of slot/bay(%u/%u) has set the pcie port "
             "mapping.", slot, bay);
#endif
        nim_dm_put_pcie_mapping(map_array);
    }

    /*
     * open the sub cdev for special dm instance
     */
    snprintf(cdev_pathname, sizeof(cdev_pathname),
             "/dev/%s_%u_%u", nim_dm_driver_name, slot, bay);
    count = 10;
    /*
     * wait for the dm powered on in a loop. 
     */
    do {
        sub_fd = open(cdev_pathname, O_RDWR);
        if(sub_fd >= 0)
            break;

        if(errno == EBUSY) {
            /*
             * Another instance has token the ownership of sub cdev
             */
            ERR("The dm sub cdev(%u/%u) has been opened by other instance.",
                 slot, bay);

            return -1;
        } else if (errno == ENOENT) {
#ifdef YWEN
            INFO("The dm instance of slot/bay(%u/%u) wait for the sub cdev file "
                 "created by udev.", slot, bay);
#endif
            usleep(200*2000);
            continue;
        } else {
            ERR("The dm instance of slot/bay(%u/%u) fail to open the sub cdev "
                "file.Errno = %d", slot, bay, errno);
            return errno;
        }

    } while(--count > 0);

    if(count == 0) {
	ERR("The dm instance of slot/bay(%u/%u) fail to wait for the "
	    "sub cdev file created by udev after loading dm kennel "
	    "module. Errno = %d", slot, bay, errno);
	return errno;
    }

#ifdef YWEN
    INFO("The dm instance of slot/bay(%u/%u) has opened the sub cdev file. "
         "sub_fd = %d", slot, bay, sub_fd);
#endif
    /*
     * map the config and ppresg area to user space for cpss lib
     */
    ret = extDrvPciMap((GT_UINTPTR *)&ppregs_addr, (GT_SIZE_T *)&ppregs_size,
                       (GT_UINTPTR *)&config_addr, (GT_SIZE_T *)&config_size);
    if(ret) {
        ERR("The dm instance of slot/bay(%u/%u) fail to map the config and "
            "ppreg memory space.", slot, bay);

    } else {
#ifdef YWEN
        INFO("The dm instance of slot/bay(%u/%u) succeeds to map the config "
             "and ppreg memory space(%lx, %lx)(%lx, %lx).",
             slot, bay,
             ppregs_addr, ppregs_size, config_addr, config_size);
#endif
    }

    return ret;
}

void nim_dm_cpss_extserv_cleanup_ex(void)
{
    int ret = 0;
    GT_VOID *int_vect;
#ifdef YWEN
    INFO("The dm instance of slot/bay(%u/%u) cleans up the main cdev(%d) and "
         "sub cdev(%d).", dm_cpss_slot, dm_cpss_bay, main_fd, sub_fd);
#endif
    if (extDrvGetPciIntVec(0, (void **)&int_vect) != GT_OK) {
	ERR("Failed to get interrupt vector number.");
    } else {
	if (extDrvIntDisable((unsigned long)int_vect) != GT_OK) {
	    ERR("Failed to disable interrupt.");
	}
    }

    if(sub_fd >= 0) {

        if(int_tid != 0) {
            int_exit = 1;
            ret = ioctl(sub_fd, DM_SUB_IOC_INTDISCONNECT);
            if(ret) {
                ERR("The dm instance of slot/bay(%u/%u) fail to delete the ISR "
                    "routine thread(%d).", dm_cpss_slot, dm_cpss_bay, ret);
            } 
	    int_tid = 0;
        }

        if(config_addr != 0 && config_size != 0 &&
           ppregs_addr != 0 && ppregs_size != 0 &&
           exregs_addr != 0 && exregs_size != 0) {
            ret = extDrvPciUnMapEx((GT_UINTPTR)ppregs_addr, (GT_SIZE_T)ppregs_size,
                                 (GT_UINTPTR)config_addr, (GT_SIZE_T)config_size,
                                 (GT_UINTPTR)exregs_addr, (GT_SIZE_T)exregs_size);
            if(ret) {
                ERR("The dm instance of slot/bay(%u/%u) fail to unmap config "
                    "and ppreg memory space(%d).",
                    dm_cpss_slot, dm_cpss_bay, ret);

            } else {
#ifdef YWEN
                INFO("The dm instance of slot/bay(%u/%u) succeeds to unmap "
                     "config and ppreg memory space.",
                     dm_cpss_slot, dm_cpss_bay);
#endif
            }
            ppregs_addr = 0;
            ppregs_size = 0;
            config_addr = 0;
            config_size = 0;
	     exregs_addr = 0;
            exregs_size = 0;
        }

        ret = close(sub_fd);
        if(ret) {
            ERR("The dm instance of slot/bay(%u/%u) fail to close the sub fd"
                "(%d, %d).", dm_cpss_slot, dm_cpss_bay, sub_fd, errno);

        } else {
            INFO("The dm instance of slot/bay(%u/%u) succeeds to close the "
                 "sub fd(%d).", dm_cpss_slot, dm_cpss_bay, sub_fd);
        }
        sub_fd = -1;
    }

    if(main_fd >= 0) {
        ret = close(main_fd);
        if(ret) {
            ERR("The dm instance of slot/bay(%u/%u) fail to close the main fd"
                "(%d, %d).", dm_cpss_slot, dm_cpss_bay, main_fd, errno);

        } else {
            INFO("The dm instance of slot/bay(%u/%u) succeeds to close the "
                 "main fd(%d).", dm_cpss_slot, dm_cpss_bay, main_fd);
        }
        main_fd = -1;
    }
}


void nim_dm_cpss_extserv_cleanup(void)
{
    int ret = 0;
    GT_VOID *int_vect;
#ifdef YWEN
    INFO("The dm instance of slot/bay(%u/%u) cleans up the main cdev(%d) and "
         "sub cdev(%d).", dm_cpss_slot, dm_cpss_bay, main_fd, sub_fd);
#endif
    if (extDrvGetPciIntVec(0, (void **)&int_vect) != GT_OK) {
	ERR("Failed to get interrupt vector number.");
    } else {
	if (extDrvIntDisable((unsigned long)int_vect) != GT_OK) {
	    ERR("Failed to disable interrupt.");
	}
    }

    if(sub_fd >= 0) {

        if(int_tid != 0) {
            int_exit = 1;
            ret = ioctl(sub_fd, DM_SUB_IOC_INTDISCONNECT);
            if(ret) {
                ERR("The dm instance of slot/bay(%u/%u) fail to delete the ISR "
                    "routine thread(%d).", dm_cpss_slot, dm_cpss_bay, ret);
            } 
	    int_tid = 0;
        }

        if(config_addr != 0 && config_size != 0 &&
           ppregs_addr != 0 && ppregs_size != 0) {
            ret = extDrvPciUnMap((GT_UINTPTR)ppregs_addr, (GT_SIZE_T)ppregs_size,
                                 (GT_UINTPTR)config_addr, (GT_SIZE_T)config_size);
            if(ret) {
                ERR("The dm instance of slot/bay(%u/%u) fail to unmap config "
                    "and ppreg memory space(%d).",
                    dm_cpss_slot, dm_cpss_bay, ret);

            } else {
#ifdef YWEN
                INFO("The dm instance of slot/bay(%u/%u) succeeds to unmap "
                     "config and ppreg memory space.",
                     dm_cpss_slot, dm_cpss_bay);
#endif
            }
            ppregs_addr = 0;
            ppregs_size = 0;
            config_addr = 0;
            config_size = 0;
        }

        ret = close(sub_fd);
        if(ret) {
            ERR("The dm instance of slot/bay(%u/%u) fail to close the sub fd"
                "(%d, %d).", dm_cpss_slot, dm_cpss_bay, sub_fd, errno);

        } else {
            INFO("The dm instance of slot/bay(%u/%u) succeeds to close the "
                 "sub fd(%d).", dm_cpss_slot, dm_cpss_bay, sub_fd);
        }
        sub_fd = -1;
    }

    if(main_fd >= 0) {
        ret = close(main_fd);
        if(ret) {
            ERR("The dm instance of slot/bay(%u/%u) fail to close the main fd"
                "(%d, %d).", dm_cpss_slot, dm_cpss_bay, main_fd, errno);

        } else {
            INFO("The dm instance of slot/bay(%u/%u) succeeds to close the "
                 "main fd(%d).", dm_cpss_slot, dm_cpss_bay, main_fd);
        }
        main_fd = -1;
    }
}


void nim_dm_cpss_get_extserv (CPSS_EXT_DRV_FUNC_BIND_STC *extDrv,
                              CPSS_OS_FUNC_BIND_STC      *os,
                              CPSS_TRACE_FUNC_BIND_STC   *trace)

{
#ifdef YWEN
    INFO("The dm instance of slot/bay(%u/%u) get the cpss external services.",
         dm_cpss_slot, dm_cpss_bay);
#endif

    if(extDrv != NULL) {
        memset(extDrv, 0, sizeof(CPSS_EXT_DRV_FUNC_BIND_STC));

        extDrv->extDrvDmaBindInfo.extDrvDmaReadFunc = extDrvDmaRead;
        extDrv->extDrvDmaBindInfo.extDrvDmaWriteDriverFunc = extDrvDmaWrite;

        extDrv->extDrvIntBindInfo.extDrvIntConnectFunc = extDrvIntConnect;
        extDrv->extDrvIntBindInfo.extDrvIntEnableFunc = extDrvIntEnable;
        extDrv->extDrvIntBindInfo.extDrvIntDisableFunc = extDrvIntDisable;
        extDrv->extDrvIntBindInfo.extDrvIntLockModeSetFunc =
                                  extDrvSetIntLockUnlock;

        extDrv->extDrvPciInfo.extDrvPciConfigWriteRegFunc =
                              extDrvPciConfigWriteReg;
        extDrv->extDrvPciInfo.extDrvPciConfigReadRegFunc =
                              extDrvPciConfigReadReg;
        extDrv->extDrvPciInfo.extDrvPciDevFindFunc = extDrvPciFindDev;
        extDrv->extDrvPciInfo.extDrvPciIntVecFunc = extDrvGetPciIntVec;
        extDrv->extDrvPciInfo.extDrvPciIntMaskFunc = extDrvGetIntMask;
        extDrv->extDrvPciInfo.extDrvPciCombinedAccessEnableFunc =
                              extDrvEnableCombinedPciAccess;
        extDrv->extDrvPciInfo.extDrvPciDoubleWriteFunc = extDrvPciDoubleWrite;
        extDrv->extDrvPciInfo.extDrvPciDoubleReadFunc = extDrvPciDoubleRead;
    }

    if(os != NULL) {
        memset(os, 0, sizeof(CPSS_OS_FUNC_BIND_STC));

        os->osMemBindInfo.osMemBzeroFunc = osMemBzero;
        os->osMemBindInfo.osMemSetFunc = osMemSet;
        os->osMemBindInfo.osMemCpyFunc = osMemCpy;
        os->osMemBindInfo.osMemCmpFunc = osMemCmp;
        os->osMemBindInfo.osMemStaticMallocFunc = osMemStaticMalloc;
        os->osMemBindInfo.osMemMallocFunc = osMemMalloc;
        os->osMemBindInfo.osMemReallocFunc = osMemRealloc;
        os->osMemBindInfo.osMemFreeFunc = osMemFree;
        os->osMemBindInfo.osMemCacheDmaMallocFunc = osMemCacheDmaMalloc;
        os->osMemBindInfo.osMemCacheDmaFreeFunc = osMemCacheDmaFree;
        os->osMemBindInfo.osMemPhyToVirtFunc = osMemPhyToVirt;
        os->osMemBindInfo.osMemVirtToPhyFunc = osMemVirtToPhy;

        os->osStrBindInfo.osStrlenFunc = osStrlen;
        os->osStrBindInfo.osStrCpyFunc = osStrCpy;
        os->osStrBindInfo.osStrNCpyFunc = osStrNCpy;
        os->osStrBindInfo.osStrChrFunc = osStrChr;
        os->osStrBindInfo.osStrCmpFunc = osStrCmp;
        os->osStrBindInfo.osStrNCmpFunc = osStrNCmp;
        os->osStrBindInfo.osStrCatFunc = osStrCat;
        os->osStrBindInfo.osStrStrNCatFunc = osStrNCat;
        os->osStrBindInfo.osStrChrToUpperFunc = osToUpper;
        os->osStrBindInfo.osStrTo32Func = osStrTo32;
        os->osStrBindInfo.osStrToU32Func = osStrToU32;

        os->osSemBindInfo.osMutexCreateFunc = osMutexCreate;
        os->osSemBindInfo.osMutexDeleteFunc = osMutexDelete;
        os->osSemBindInfo.osMutexLockFunc = osMutexLock;
        os->osSemBindInfo.osMutexUnlockFunc = osMutexUnlock;
        os->osSemBindInfo.osSigSemBinCreateFunc = osSemBinCreate;
        os->osSemBindInfo.osSigSemMCreateFunc = osSemMCreate;
        os->osSemBindInfo.osSigSemCCreateFunc = osSemCCreate;
        os->osSemBindInfo.osSigSemDeleteFunc = osSemDelete;
        os->osSemBindInfo.osSigSemWaitFunc = osSemWait;
        os->osSemBindInfo.osSigSemSignalFunc = osSemSignal;

        os->osIoBindInfo.osIoBindStdOutFunc = osBindStdOut;

        os->osIoBindInfo.osIoPrintfFunc = osPrintf;
        os->osIoBindInfo.osIoVprintfFunc = osVprintf;
        os->osIoBindInfo.osIoSprintfFunc = osSprintf;
        os->osIoBindInfo.osIoVsprintfFunc = osVsprintf;
	 os->osIoBindInfo.osIoSnprintfFunc =	osSnprintf;
	 os->osIoBindInfo.osIoVsnprintfFunc = osVsnprintf; 
        os->osIoBindInfo.osIoPrintSynchFunc = osIoPrintSynch;
        os->osIoBindInfo.osIoGetsFunc = osGets;

        os->osInetBindInfo.osInetNtohlFunc = osNtohl;
        os->osInetBindInfo.osInetHtonlFunc = osHtonl;
        os->osInetBindInfo.osInetNtohsFunc = osNtohs;
        os->osInetBindInfo.osInetHtonsFunc = osHtons;
        os->osInetBindInfo.osInetNtoaFunc = osInetNtoa;

        os->osTimeBindInfo.osTimeWkAfterFunc = osTimerWkAfter;
        os->osTimeBindInfo.osTimeTickGetFunc = osTickGet;
        os->osTimeBindInfo.osTimeGetFunc = osTimeGet;
        os->osTimeBindInfo.osTimeRTFunc = osTimeRT;
        os->osTimeBindInfo.osGetSysClockRateFunc = osGetSysClockRate;
        os->osTimeBindInfo.osDelayFunc = osDelay;

        os->osIntBindInfo.osIntEnableFunc =
                (CPSS_OS_INT_ENABLE_FUNC)extDrvIntEnable;
        os->osIntBindInfo.osIntDisableFunc =
                (CPSS_OS_INT_DISABLE_FUNC)extDrvIntDisable;
        os->osIntBindInfo.osIntModeSetFunc =
                (CPSS_OS_INT_MODE_SET_FUNC)extDrvSetIntLockUnlock;
        os->osIntBindInfo.osIntConnectFunc =
                (CPSS_OS_INT_CONNECT_FUNC)extDrvIntConnect;

        os->osRandBindInfo.osRandFunc = osRand;
        os->osRandBindInfo.osSrandFunc = osSrand;

        os->osTaskBindInfo.osTaskCreateFunc = osTaskCreate;
        os->osTaskBindInfo.osTaskDeleteFunc = osTaskDelete;
        os->osTaskBindInfo.osTaskGetSelfFunc = osTaskGetSelf;
        os->osTaskBindInfo.osTaskLockFunc = osTaskLock;
        os->osTaskBindInfo.osTaskUnLockFunc = osTaskUnLock;

        os->osStdLibBindInfo.osQsortFunc = osQsort;
        os->osStdLibBindInfo.osBsearchFunc = osBsearch;

        os->osMsgQBindInfo.osMsgQCreateFunc = osMsgQCreate;
        os->osMsgQBindInfo.osMsgQDeleteFunc = osMsgQDelete;
        os->osMsgQBindInfo.osMsgQSendFunc = osMsgQSend;
        os->osMsgQBindInfo.osMsgQRecvFunc = osMsgQRecv;
        os->osMsgQBindInfo.osMsgQNumMsgsFunc = osMsgQNumMsgs;
    }

    if(trace != NULL) {
        memset(trace, 0, sizeof(CPSS_TRACE_FUNC_BIND_STC));

        trace->traceHwBindInfo.traceHwAccessWriteFunc = traceHwAccessWrite;
        trace->traceHwBindInfo.traceHwAccessReadFunc = traceHwAccessRead;
        trace->traceHwBindInfo.traceHwAccessDelayFunc = traceHwAccessDelay;
    }
}

void nim_dm_cpss_get_pciemap_ex(GT_UINTPTR *pciBaseAddr,
                             GT_UINTPTR *internalPciBase, GT_UINTPTR *expciBaseAddr)
{
    *pciBaseAddr = ppregs_addr;
    *internalPciBase = config_addr;
    *expciBaseAddr = exregs_addr;

    return;
}

void nim_dm_cpss_get_pciemap(GT_UINTPTR *pciBaseAddr,
                             GT_UINTPTR *internalPciBase)
{
    *pciBaseAddr = ppregs_addr;
    *internalPciBase = config_addr;

    return;
}

void nim_dm_cpss_event_signal(void)
{
    if(ioctl(sub_fd, DM_SUB_IOC_EVENT)) {
        ERR("The dm instance of slot/bay(%u/%u) fail to send a event.",
            dm_cpss_slot, dm_cpss_bay);
    }
    return;
}

/*
 *------------------------------------------------------------------
 * $Log: nim_dm_cpss_extserv.c,v $
 * Revision 1.2  2019/12/11 10:10:26  lucywang
 * Merged Nanook to main trunk
 *
 *
 *------------------------------------------------------------------
 * $Endlog$
 */
