/* $Id: ipmi_sprom_cmds.c,v 1.2 2016/04/20 08:41:35 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/ipmi_sprom_cmds.c,v $
 *
 *      File:   sprom_cmds.c
 *      Name:   Curt Brune, matt strathman
 *
 *      Description:
 *       sprom misc cmd handlers
 *
 * Copyright (c) 2007-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 ********************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "diag_sprom.h"
#include "ipmi_sprom.h"
#include "ipmi_sprom_ops.h"

#if defined (DIAG_IOM2)
#include "diag_dev.h"
#include "diag_cmn.h"
#include "diag_ops.h"
#include "mv88e6095.h"
extern int is_test_backplane();
#endif

#ifdef DIAG_TCL
#include <tcl.h>
extern struct Tcl_Interp* diag_get_tcl_interp_ptr();
#endif
extern int brd_slot_id_get(uint8_t *);
extern int is_bmc_enable();

static int 
_ipmi_dump_data( diag_sprom_t *sprom, uint8_t *buf )
{
    int    i;

    sprom_printf ( "Dumping %s SPROM", ipmi_sprom_get_name( sprom ) );
    sprom_printf ( " contents in hex.\n" );

    for (i = 0; i < ipmi_sprom_get_size ( sprom ); i++) {
        if (i % 16==0) {
            sprom_printf ( "\n0x%04x: ", i );
        }
        sprom_printf ( "%02x ", buf[i] );
    }
    sprom_printf ( "\n" );
    
    return ( 0 );
}

int
ipmi_sprom_dump ( diag_sprom_t *const sprom, uint32_t offset, uint32_t len )
{
    int rc = 0;
    uint8_t *buf;

    // set number of bytes to dump as passed from CLI
    if ( len > (uint32_t)ipmi_sprom_get_size ( sprom ) )
        len = (uint32_t)ipmi_sprom_get_size ( sprom );
    
    buf = malloc( len );
    if ( buf == NULL ) {
        ipmi_sprom_err ( "malloc failed\n");
        return ( -SPROM_ERR_NOMEM );
    }

    if ( ( rc = diag_sprom_rd ( sprom, offset, len, buf ) ) < 0 ) {
        ipmi_sprom_err ("Unable to read sprom contents.\n");
        if ( buf ) free ( buf );
        return ( -SPROM_ERR_IO );
    }

    rc = _ipmi_dump_data( sprom, buf );

    if ( buf ) free( buf );

    return rc;
}

static int
_ipmi_sprom_fill_raw(diag_sprom_t *sprom, uint32_t offset, int len, 
  uint8_t fill_value)
{
    int rc;
    char   input;  
    uint8_t *buf;

    if (len < 0) {
	printf("Invalid Length %d\n", len);
	return (-1);
    }

    sprom_printf ( "Do you really want to fill the SPROM with "
      "0x%02x (y/n) [n]? ", fill_value);
    input = getchar();
    while( getchar() != '\n' );

    if ( input != 'y' && input != 'Y') {
        sprom_printf ( "Aborting SPROM modification ...\n" );
        return 0;
    } 
    
    // allocate data buffers
    if ( len > ipmi_sprom_get_size ( sprom ) ) 
        len = ipmi_sprom_get_size ( sprom );

    buf = malloc( len );
    if ( buf == NULL ) {
        ipmi_sprom_err ( "malloc failed\n" );
        return ( -SPROM_ERR_NOMEM );
    }
 
    memset( buf, fill_value, len );
    
    if ( ( rc = diag_sprom_wr ( sprom, offset, len, buf ) ) < 0 ) {
         ipmi_sprom_err ( "Unable to write sprom contents.\n" );
         if ( buf ) free( buf );
         return ( -SPROM_ERR_IO );
    }

    if ( buf ) free( buf );

    return ( 0 );

}

int 
ipmi_sprom_invalidate( diag_sprom_t *const sprom )
{
    // A "fresh from the factory" untouched SPROM usually is filled
    // with 0xFF.  This appears to the software as being blank.
    return( _ipmi_sprom_fill_raw( sprom, 0,
		ipmi_sprom_get_size ( sprom ), 0xFF) );
}

int
ipmi_sprom_zero( diag_sprom_t *const sprom )
{
    return ( _ipmi_sprom_fill_raw ( sprom, 0, 
		ipmi_sprom_get_size ( sprom ), 0x00) );
}

int ipmi_sprom_ui (int argc, char *argv[])
{
	int	 rc = 0;
	char	 *endp;
	int	 len = 0;
	uint32_t cmd  = 0;
	uint32_t type = 0;
	uint32_t slot = 0, sup_slot = 0, subslot=1;
	uint32_t offset = 0;
	uint32_t length = 0;

	sprom_platform_t platform;
	diag_sprom_t *sprom;
	uint8_t *rwbuf;

	if ( argc < 3 ) {
		return (-1);
	}

	argc--;
	argv++;
	if ( strcasecmp (*argv, "iom" ) == 0 ) {
		platform = SPROM_PLATFORM_IOM;
		argc--;
		argv++;
	} else if ( strcasecmp (*argv, "bmc" ) == 0 ) {
		platform = SPROM_PLATFORM_IBMC;
		argc--;
		argv++;
	} else if ( strcasecmp (*argv, "host" ) == 0 ) {
                platform = SPROM_PLATFORM_HOST;
                argc--;
                argv++;
        } else {
		if ((platform = ipmi_sprom_get_platform ( ) ) < 0 ) {
			fprintf ( stderr, "Err: Can't determine platform type,"
					" please specify\n" );
			return (-2);
		}
	}

	if ( ipmi_sprom_get_slot_sup ( platform, &sup_slot) < 0 ) {
		fprintf ( stderr, "Err: Could not get supervisor slot\n" );
		return (-3);
	}

	//
	// get cmd
	len = strlen ( *argv );

	if ( strncmp ( *argv, "set", len ) == 0 ) {
		cmd = SPROM_CMD_SET;
	} else if ( strncmp ( *argv, "show", len ) == 0 ) {
		cmd = SPROM_CMD_SHOW;
	} else if ( strncmp ( *argv, "set-serial", len ) == 0 ) {
		cmd = SPROM_CMD_SET_SERIAL;
	} else if ( strncmp ( *argv, "set-part-num", len ) == 0 ) {
		cmd = SPROM_CMD_SET_PART_NUM;
	} else if ( strncmp ( *argv, "set-part-rev", len ) == 0 ) {
		cmd = SPROM_CMD_SET_PART_REV;
	} else if ( strncmp ( *argv, "set-license", len ) == 0 ) {
		cmd = SPROM_CMD_SET_LICENSE;
	} else if ( strncmp ( *argv, "set-file-id", len ) == 0 ) {
		cmd = SPROM_CMD_SET_FFID;
	} else if ( strncmp ( *argv, "set-mac", len ) == 0 ) {
		cmd = SPROM_CMD_SET_MAC;
	} else if ( strncmp ( *argv, "dump", len ) == 0 ) {
		cmd = SPROM_CMD_DUMP;
		argc--;
		argv++;

		if ( argc ) {
			offset = strtoul (*argv, &endp, 0 );
			if (*endp) {
				return ( -SPROM_ERR_INVALID );
			}

			argc--;
			argv++;

			if ( argc ) {
				length = strtoul ( *argv, &endp, 0 );

				if ( *endp ) {
					return ( -SPROM_ERR_INVALID );
				}
			} else {
				return ( -SPROM_ERR_INVALID );
			}
		} else {
			return ( -SPROM_ERR_INVALID );
		}
	} else if ( strncmp ( *argv, "invalidate", len ) == 0 ) {
		cmd = SPROM_CMD_INVALIDATE;
	} else if ( strncmp ( *argv, "zero", len ) == 0 ) {
		cmd = SPROM_CMD_ZERO;
	} else {
		return (-1);
	}

	argc--;
	argv++;

	//  get type
	if (!argc) {
		printf("WARN: Forcing SPROM type to SUP\n");
		type = SPROM_UTIL_TYPE_SUP;
		if ((rc = ipmi_sprom_get_slot_sup (platform, &slot)) < 0) {
			return  ( rc );
		}
 	} else {
		len = strlen ( *argv );
		if ( strncmp ( *argv, "supervisor", len ) == 0 ) {
			type = SPROM_UTIL_TYPE_SUP;
			if ( ( rc = ipmi_sprom_get_slot_sup (platform, 
				&slot)) < 0 ) {
				return  ( rc );
			}
		} else if ( ( strncmp ( *argv, "backplane", len ) == 0 ) ||
				( strcmp ( *argv, "bp" ) == 0 ) ) {
			type = SPROM_UTIL_TYPE_BP;
			argc--;
			argv++;
	
			if (argc) {
				slot = strtoul ( *argv, &endp, 0 );
	
				if ( *endp ) {
					return  ( -SPROM_ERR_INVALID );
				}
			} else {
				slot = 0;
			}
		} else if ( strncmp ( *argv, "mod", len ) == 0 ) {
			type = SPROM_UTIL_TYPE_MOD_HOST;
			argc--;
			argv++;
	
			if (argc) {
				slot = strtoul ( *argv, &endp, 0 );
	
				if ( *endp ) {
					return  ( -SPROM_ERR_INVALID );
				}
				if (slot == 0) {
					printf("  Error: Slot is 1 based of BLADE\n");
					return  ( -SPROM_ERR_INVALID );
				}
				slot -= 1;
				argc--;
				argv++;
				if (argc) {
					subslot = strtoul ( *argv, &endp, 0 );
		
					if ( *endp ) {
						return  ( -SPROM_ERR_INVALID );
					}
					if ((subslot < 1) && (subslot > 3))
						return  ( -SPROM_ERR_INVALID );
					if (subslot >= 2)
						type = SPROM_UTIL_TYPE_MEZZ;
				}
			}

		} else if ( strncmp ( *argv, "fan", len ) == 0 ) {
			type = SPROM_UTIL_TYPE_FAN;
	
			argc--;
			argv++;
	
			if (argc) {
				slot = strtoul ( *argv, &endp, 0 );
	
				if ( *endp ) {
					return  ( -SPROM_ERR_INVALID );
				}
				if (slot == 0) {
					printf("  Error: Slot is 1 based of FAN\n");
					return  ( -SPROM_ERR_INVALID );
				}
				slot -= 1;
			}
		} else if ((strncmp(*argv, "powersupply", len ) == 0 ) ||
			( strcmp ( *argv, "ps" ) == 0 ) ) {
			type = SPROM_UTIL_TYPE_PS;

			argc--;
			argv++;
	
			if (argc) {
				slot = strtoul ( *argv, &endp, 0 );
				if ( *endp ) {
					return  ( -SPROM_ERR_INVALID );
				}
				if (slot == 0) {
					printf("  Error: Slot is 1 based of PSU\n");
					return  ( -SPROM_ERR_INVALID );
				}
				slot -= 1;
			}
		} else if ((strncmp(*argv, "bmc", len ) == 0)) {
			type = SPROM_UTIL_TYPE_IBMC;
		} else if ((strncmp(*argv, "menloe", strlen("menloe")) == 0)) {
			type = SPROM_UTIL_TYPE_MENLO_E;
			argc--;
			argv++;
	
			if (argc) {
				slot = strtoul ( *argv, &endp, 0 );
	
				if ( *endp ) {
					return  ( -SPROM_ERR_INVALID );
				}
			} else {
				slot = 0;
			}
		} else if ((strncmp(*argv, "menloq", strlen("menloq")) == 0)) {
			type = SPROM_UTIL_TYPE_MENLO;
			argc--;
			argv++;
	
			if (argc) {
				slot = strtoul ( *argv, &endp, 0 );
	
				if ( *endp ) {
					return  ( -SPROM_ERR_INVALID );
				}
			} else {
				slot = 0;
			}
		} else if ((strncmp(*argv, "oplin", len ) == 0)) {
			type = SPROM_UTIL_TYPE_OPLIN;
			argc--;
			argv++;
	
			if (argc) {
				slot = strtoul ( *argv, &endp, 0 );
	
				if ( *endp ) {
					return  ( -SPROM_ERR_INVALID );
				}
			} else {
				slot = 0;
			}
		} else if ((strncmp(*argv, "mezz", len ) == 0)) { 
			type = SPROM_UTIL_TYPE_MEZZ;
			argc--;
			argv++;
			slot = argc? strtoul ( *argv, &endp, 0 ) : 0;
		} else if ((strncmp(*argv, "mpark", len ) == 0)) {
			type = SPROM_UTIL_TYPE_MONTEREYPARK;
			argc--;
			argv++;
			slot = argc? strtoul ( *argv, &endp, 0 ) : 0;
		} else if ((strncmp(*argv, "niantic", len ) == 0)) {
			type = SPROM_UTIL_TYPE_NIANTIC;
			argc--;
			argv++;
			slot = argc? strtoul ( *argv, &endp, 0 ) : 0;
		} else if ((strncmp(*argv, "neteffect", len ) == 0)) {
			type = SPROM_UTIL_TYPE_NETEFFECT;
			argc--;
			argv++;
			slot = argc? strtoul ( *argv, &endp, 0 ) : 0;
		} else if ((strncmp(*argv, "schultz", len ) == 0)) {
			type = SPROM_UTIL_TYPE_SCHULTZ;
			argc--;
			argv++;
			slot = argc? strtoul ( *argv, &endp, 0 ) : 0;
		} else if ((strncmp(*argv, "tigershark", len ) == 0)) {
			type = SPROM_UTIL_TYPE_TIGERSHARK;
			argc--;
			argv++;
			slot = argc? strtoul ( *argv, &endp, 0 ) : 0;
		} else if ((strncmp(*argv, "everest", len ) == 0)) {
			type = SPROM_UTIL_TYPE_EVEREST;
			argc--;
			argv++;
			slot = argc? strtoul ( *argv, &endp, 0 ) : 0;
		} else if ((strncmp(*argv, "dublin", len ) == 0)) {
			type = SPROM_UTIL_TYPE_DUBLIN;
			argc--;
			argv++;
			slot = argc? strtoul ( *argv, &endp, 0 ) : 0;
		} else if ((strncmp(*argv, "fremont", len ) == 0)) {
			type = SPROM_UTIL_TYPE_FREMONT;
			argc--;
			argv++;
			slot = argc? strtoul ( *argv, &endp, 0 ) : 0;
		} else if ((strncmp(*argv, "livermore", len ) == 0)) {
			type = SPROM_UTIL_TYPE_LIVERMORE;
			argc--;
			argv++;
			slot = argc? strtoul ( *argv, &endp, 0 ) : 0;
		} else if ((strncmp(*argv, "bbu", len ) == 0)) {
			type = SPROM_UTIL_TYPE_BBU;
			argc--;
			argv++;
			slot = argc? strtoul ( *argv, &endp, 0 ) : 0;
		} else if ((strncmp(*argv, "turlock", len ) == 0)) { 
			type = SPROM_UTIL_TYPE_TURLOCK;
			argc--;
			argv++;
			slot = argc? strtoul ( *argv, &endp, 0 ) : 0;
		} else if ((strncmp(*argv, "tpm", len ) == 0)) { 
			type = SPROM_UTIL_TYPE_ALCATRAZ;
			argc--;
			argv++;
			slot = argc? strtoul ( *argv, &endp, 0 ) : 0;
		} else if ((strncmp(*argv, "palo", len ) == 0)) {
			type = SPROM_UTIL_TYPE_PALO;
			argc--;
			argv++;
		
			if (argc) {
				slot = strtoul ( *argv, &endp, 0 );
	
				if ( *endp ) {
					return  ( -SPROM_ERR_INVALID );
				}
			} else {
				slot = 0;
			}
		} else if ((strncmp(*argv, "memfront", len ) == 0)) {
			type = SPROM_UTIL_TYPE_MEMFRONT;
			argc--;
			argv++;
		} else if ((strncmp(*argv, "memback", len ) == 0)) {
			type = SPROM_UTIL_TYPE_MEMBACK;
			argc--;
			argv++;
			//slot = argc? strtoul ( *argv, &endp, 0 ) : 0;
	
			if (argc) {
				slot = strtoul ( *argv, &endp, 0 );
	
				if ( *endp ) {
					return  ( -SPROM_ERR_INVALID );
				}
			} else {
				slot = 0;
			}
		} else {
			return  ( -SPROM_ERR_INVALID );
		}
	}


	sprom = ipmi_sprom_init ( platform, type, slot);
	if ( sprom == NULL ) {
		fprintf ( stderr, "Err: No Device\n" );
			return  ( -SPROM_ERR_NODEV );
	}

	if ( sprom->type != type ) {
		fprintf ( stderr, "Err: Sprom type does not match\n" );
		return  ( 1 );
	}

	switch ( cmd ) {
		case	SPROM_CMD_SET:
			rc = ipmi_sprom_set ( sprom );
			break;

		case	SPROM_CMD_SHOW:
#if defined (DIAG_IOM2)
        {
            uint32_t data;
            uint16_t port;

            port = MARVELL_MIIPHY7 - slot;

            if ( type == SPROM_UTIL_TYPE_MOD_HOST ) {
                if ( is_test_backplane() ) {
                    printf("  Diag in back plane mode!\n");
                    break;
                } else {
                    printf("  Reset PHY%d\n", port);
                    data = MVPHY_PHYCONTROL_RESET | MVPHY_PHYCONTROL_SPEED_LSB | MVPHY_PHYCONTROL_DUPLEXMODE;
                    rc = diag_ioctl(DEV_MV88E6095, 0, IOCTL_REG_WR, MVPHY_PHYCONTROL, data, port);
                    if (rc) {
                        printf("Error: Failed to program the marvell switch reg%x\n",MVPHY_PHYCONTROL);
                        return(rc);
                    }

                    data = (1<<MARVELL_MIIPHY9)|(1<<MARVELL_MIIPHY10);
                    rc = diag_ioctl(DEV_MV88E6095, 0, IOCTL_REG_WR, MARVELL_PORTVLANMAP, data, port);
                    if (rc) {
                        printf("Error: Failed to program the marvell switch reg%x\n",MARVELL_PORTVLANMAP);
                        return(rc);
                    }

                    printf("  Initialize MV6095 port%d\n", port);
                    rc = diag_ioctl(DEV_MV88E6095, 0, IOCTL_REG_RD, MARVELL_PORTCTRL, &data, port|0x10);
                    if (rc) {
                        printf("Error: Failed to program the marvell switch reg%x\n",MARVELL_PORTCTRL);
                        return(rc);
                    }
                    data |= PORTCTRL_PORTSTATE_FW;
                    rc = diag_ioctl(DEV_MV88E6095, 0, IOCTL_REG_WR, MARVELL_PORTCTRL, data, port|0x10);
                    if (rc) {
                        printf("Error: Failed to program the marvell switch reg%x\n",MARVELL_PORTCTRL);
                        return(rc);
                    }
                }
            }
        }
#endif
#ifdef DIAG_TCL
#if defined(DIAG_IOM) || defined(DIAG_IOM2)
                        if (((type == SPROM_UTIL_TYPE_MOD_HOST) ||
			     (type == SPROM_UTIL_TYPE_MEZZ)) && (!is_bmc_enable())) {
                                struct Tcl_Interp* pInterp = diag_get_tcl_interp_ptr();
                                char cmd_str[80];
                                uint8_t cmc_slot;

                                brd_slot_id_get(&cmc_slot);
                                memset(cmd_str, 0, sizeof(cmd_str));
                                snprintf(cmd_str, 80, "get_bmc_seeprom %d %d %d",
                                                cmc_slot, slot+1, subslot);
                                if (Tcl_Eval (pInterp, cmd_str) != TCL_OK) {
                                        printf ( "Error: Sprom read from cmc%d mod%d subslot%d\n" ,
                                                cmc_slot, slot, subslot);
                                }
			} else 
#endif
#endif
			{
				rwbuf = malloc ( ipmi_sprom_get_size ( sprom ) );
				if ( rwbuf == NULL ) {
					return  ( -ENOMEM );
				}
	
				if ( ( rc = ipmi_sprom_read ( sprom, rwbuf ) ) < 0 ) {
					fprintf(stderr, "Error: sprom read %d\n", rc );
					free ( rwbuf );
					return  ( rc );
				}

				if ( ( rc = ipmi_sprom_show ( sprom, rwbuf ) ) < 0 ) {
					fprintf(stderr, "Error: sprom_show %d\n", rc);
					free ( rwbuf );
					return  ( rc );
				}
				free ( rwbuf );
			}
			break;

		case	SPROM_CMD_SET_SERIAL:
			rc = ipmi_sprom_set_serial (sprom);
			break;

		case	SPROM_CMD_SET_FFID:
			rc = ipmi_sprom_set_fru_file_id ( sprom );
			break;

		case	SPROM_CMD_SET_MAC:
			rc = ipmi_sprom_set_mac ( sprom );
			break;

		case	SPROM_CMD_DUMP:
			rc = ipmi_sprom_dump ( sprom, offset, length );
			break;

		case	SPROM_CMD_INVALIDATE:
			rc = ipmi_sprom_invalidate ( sprom );
			break;

		case	SPROM_CMD_ZERO:
			rc = ipmi_sprom_zero ( sprom );
			break;

		default:
			return(-SPROM_ERR_INVALID );
	}

        return 0;
}


int diag_ipmi_sprom_ui (int platform, int type , int slot, int cmd) {
    int  rc = 0;
    uint8_t *rwbuf;
    uint32_t offset = 0;
    uint32_t length = 0;

	diag_sprom_t *sprom;
    sprom = ipmi_sprom_init ( platform, type, slot);
	if ( sprom == NULL ) {
		fprintf ( stderr, "Err: No Device\n" );
			return  ( -SPROM_ERR_NODEV );
	}

	if ( sprom->type != type ) {
		fprintf ( stderr, "Err: Sprom type does not match\n" );
		return  ( 1 );
	}

	switch ( cmd ) {
		case	SPROM_CMD_SET:
			rc = ipmi_sprom_set ( sprom );
			break;

		case	SPROM_CMD_SHOW:
#if defined (DIAG_IOM2)
        {
            uint32_t data;
            uint16_t port;

            port = MARVELL_MIIPHY7 - slot;

            if ( type == SPROM_UTIL_TYPE_MOD_HOST ) {
                if ( is_test_backplane() ) {
                    printf("  Diag in back plane mode!\n");
                    break;
                } else {
                    printf("  Reset PHY%d\n", port);
                    data = MVPHY_PHYCONTROL_RESET | MVPHY_PHYCONTROL_SPEED_LSB | MVPHY_PHYCONTROL_DUPLEXMODE;
                    rc = diag_ioctl(DEV_MV88E6095, 0, IOCTL_REG_WR, MVPHY_PHYCONTROL, data, port);
                    if (rc) {
                        printf("Error: Failed to program the marvell switch reg%x\n",MVPHY_PHYCONTROL);
                        return(rc);
                    }

                    data = (1<<MARVELL_MIIPHY9)|(1<<MARVELL_MIIPHY10);
                    rc = diag_ioctl(DEV_MV88E6095, 0, IOCTL_REG_WR, MARVELL_PORTVLANMAP, data, port);
                    if (rc) {
                        printf("Error: Failed to program the marvell switch reg%x\n",MARVELL_PORTVLANMAP);
                        return(rc);
                    }

                    printf("  Initialize MV6095 port%d\n", port);
                    rc = diag_ioctl(DEV_MV88E6095, 0, IOCTL_REG_RD, MARVELL_PORTCTRL, &data, port|0x10);
                    if (rc) {
                        printf("Error: Failed to program the marvell switch reg%x\n",MARVELL_PORTCTRL);
                        return(rc);
                    }
                    data |= PORTCTRL_PORTSTATE_FW;
                    rc = diag_ioctl(DEV_MV88E6095, 0, IOCTL_REG_WR, MARVELL_PORTCTRL, data, port|0x10);
                    if (rc) {
                        printf("Error: Failed to program the marvell switch reg%x\n",MARVELL_PORTCTRL);
                        return(rc);
                    }
                }
            }
        }
#endif
#ifdef DIAG_TCL
#if defined(DIAG_IOM) || defined(DIAG_IOM2)
                        if (((type == SPROM_UTIL_TYPE_MOD_HOST) ||
			     (type == SPROM_UTIL_TYPE_MEZZ)) && (!is_bmc_enable())) {
                                struct Tcl_Interp* pInterp = diag_get_tcl_interp_ptr();
                                char cmd_str[80];
                                uint8_t cmc_slot;

                                brd_slot_id_get(&cmc_slot);
                                memset(cmd_str, 0, sizeof(cmd_str));
                                snprintf(cmd_str, 80, "get_bmc_seeprom %d %d %d",
                                                cmc_slot, slot+1, subslot);
                                if (Tcl_Eval (pInterp, cmd_str) != TCL_OK) {
                                        printf ( "Error: Sprom read from cmc%d mod%d subslot%d\n" ,
                                                cmc_slot, slot, subslot);
                                }
			} else 
#endif
#endif
			{
				rwbuf = malloc ( ipmi_sprom_get_size ( sprom ) );
				if ( rwbuf == NULL ) {
					return  ( -ENOMEM );
				}
	
				if ( ( rc = ipmi_sprom_read ( sprom, rwbuf ) ) < 0 ) {
					fprintf(stderr, "Error: sprom read %d\n", rc );
					free ( rwbuf );
					return  ( rc );
				}

				if ( ( rc = ipmi_sprom_show ( sprom, rwbuf ) ) < 0 ) {
					fprintf(stderr, "Error: sprom_show %d\n", rc);
					free ( rwbuf );
					return  ( rc );
				}
				free ( rwbuf );
			}
			break;

		case	SPROM_CMD_SET_SERIAL:
			rc = ipmi_sprom_set_serial (sprom);
			break;

		case	SPROM_CMD_SET_FFID:
			rc = ipmi_sprom_set_fru_file_id ( sprom );
			break;

		case	SPROM_CMD_SET_MAC:
			rc = ipmi_sprom_set_mac ( sprom );
			break;

		case	SPROM_CMD_DUMP:
			rc = ipmi_sprom_dump ( sprom, offset, length );
			break;

		case	SPROM_CMD_INVALIDATE:
			rc = ipmi_sprom_invalidate ( sprom );
			break;

		case	SPROM_CMD_ZERO:
			rc = ipmi_sprom_zero ( sprom );
			break;

		default:
			return(-SPROM_ERR_INVALID );
	}

        return 0;
}

