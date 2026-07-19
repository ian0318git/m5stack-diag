/* $Id: diag_dev.c,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_dev.c,v $
 *
 *      File:   diag_dev.c
 *      Name:   Sudharshan Kadari
 *
 *      Description:
 *       Diag infra structure 
 *
 *
 * Copyright (c) 2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/

#include "diag_main.h"
#include <errno.h>
#include <regex.h>

extern reg_desc_t *find_reg_desc_by_exp(reg_desc_t *preg, regex_t *);

#define DATA_SIZE 80

test_results_t fail_params[] =
{
	{"Run Count",   FORMAT_TYPE_HEX, {0x00}, 0},
	{"Start Time",  FORMAT_TYPE_STR, {(uint32_t)NULL}, 0},
	{"End Time",    FORMAT_TYPE_STR, {(uint32_t)NULL}, 0},
	{"Test Name",   FORMAT_TYPE_STR, {(uint32_t)NULL}, 0},
	{"Test Status", FORMAT_TYPE_STR, {(uint32_t)NULL}, 0},
	{"Fail Count",  FORMAT_TYPE_HEX, {(uint32_t)0x00}, 0},
        {"Failed/Run History", FORMAT_TYPE_STR, {(uint32_t)NULL}, 0},
        {"Diag Version",FORMAT_TYPE_STR, {(uint32_t)NULL}, 0},
        {"Board S/N",   FORMAT_TYPE_STR, {(uint32_t)NULL}, 0},
	{"Err Code",    FORMAT_TYPE_DEC, {0x00}, 0},
	{"Err Info",    FORMAT_TYPE_STR, {0x00}, 0},
	{"Err LPorts",  FORMAT_TYPE_HEX, {0x00}, 0},
	{"Err HPorts",  FORMAT_TYPE_HEX, {0x00}, 0},
	{"Fail Addr",   FORMAT_TYPE_HEX, {0x00}, 0},
	{"Data Exp",    FORMAT_TYPE_HEX, {0x00}, 0},
	{"Data Read",   FORMAT_TYPE_HEX, {0x00}, 0},
	{NULL,          FORMAT_TYPE_UNKNOWN, {0x00}, 0},
};

static uint32_t diag_dev_count_get (board_info_t *pinfo)
{
	uint32_t	dev_cnt = 0;

	if (!pinfo) return dev_cnt;

	while (pinfo->dev_type != DEV_UNKNOWN) {
		dev_cnt += pinfo->dev_cnt;
		pinfo++;
	}

	return dev_cnt;
}

static uint32_t diag_result_size_get(test_results_t *results)
{  
        uint32_t        count = 0;
        while (results->parameter) {
                count++;
                results++;
        }
        return (count);       
}

uint32_t diag_results_enable(test_results_t *results, uint8_t ena)
{
	uint32_t num;
	uint32_t cnt;
	num = diag_result_size_get(results);
	num -= (sizeof(fail_params)/sizeof(test_results_t) - 1);

	if (num < 0) {
		printf("ERROR: Result Parameters are incorrect\n");
		return (DIAG_SOFTWARE_ERROR);
	}

	for (cnt = 0; cnt < num; cnt++, results++) {
		results->flag = ena ? 1 : 0;
	}	
	for (cnt = 0; cnt < sizeof(fail_params)/sizeof(test_results_t); cnt++, results++) {
		results->flag = 0;
	}	

	return (DIAG_SUCCESS);	
}

uint32_t diag_results_update(test_parameters_t *pparam, test_results_t *presults)
{
	while (pparam->parameter) {
		if ((presults->format == FORMAT_TYPE_STR) ||
		    (presults->format == FORMAT_TYPE_MASK)) {
			memcpy(presults->discrete.ptr, pparam->discrete.ptr,
                                        strlen(pparam->discrete.ptr)+1);
                } else {
                        presults->discrete.value = pparam->discrete.value;
                }

		presults++;
		pparam++;
	}

	// Clear Error Information from previous run.
	while (presults->parameter) {
		presults->flag = 0;
		if (presults->format == FORMAT_TYPE_STR) {
			bzero(presults->discrete.ptr, DATA_SIZE);
		} 
		presults++;
	}
	return (0);
}

uint32_t diag_param_size_get(test_parameters_t *pparams)
{
	uint32_t	count = 0;
	while (pparams->parameter) {
		count++;
		pparams++;
	}
	return (count+1);	
}

static uint32_t diag_ttoc_size_get(test_toc_t *ptoc)
{
	uint32_t	count = 0;
	while (ptoc->name) {
		count++;
		ptoc++;
	}
	return (count+1);	
}

static void diag_section_free (diag_dev_t *pdev)
{
	test_toc_t *ptoc = pdev->psection->test_toc;
	test_results_t	  *presults;
	test_parameters_t *pparams;

	while (ptoc->name) {
		presults = ptoc->test_results;
		while (presults->parameter) {
			if ((presults->format == FORMAT_TYPE_STR) ||
			    (presults->format == FORMAT_TYPE_MASK))
				free(presults->discrete.ptr);
			presults++;
		}
		pparams = ptoc->test_params;
		while (pparams->parameter) {
			if ((pparams->format == FORMAT_TYPE_STR) ||
			    (pparams->format == FORMAT_TYPE_MASK))
				free(pparams->discrete.ptr);

			if (pparams->format == FORMAT_TYPE_MASK)
				free(pparams->max.ptr);
			pparams++;
		}
		free(ptoc->test_results);
		free(ptoc->test_params);
	}

	free(pdev->psection->test_toc);
	free(pdev->psection);
}

void diag_dev_tree_free (diag_dev_t *ptopdev)
{
	diag_dev_t  *pdev = ptopdev;

	while(pdev) {
		diag_section_free(pdev);
		free(pdev->name);
		pdev = pdev->p_next;
	}
	free (pdev);
}

static uint32_t diag_section_alloc (board_info_t *pinfo, diag_dev_t *pdev)
{
	uint32_t test_param_size, test_toc_size;
	test_parameters_t *tparam = NULL, *pparam = NULL;
	test_results_t *presults = NULL, *fresults = NULL;
	test_toc_t	*ptoc, *ttoc;

	if (!pinfo->psection) {
		pdev->psection = NULL;
		return (0);
	}
	// Allocate Section
	pdev->psection = (section_toc_t*)malloc(sizeof(section_toc_t));
	if (!pdev->psection) return (DIAG_MALLOC_ERROR);
	*pdev->psection = *pinfo->psection;

	// Allocate Test Toc
	test_toc_size = diag_ttoc_size_get(pinfo->psection->test_toc);
	pdev->psection->test_toc = (test_toc_t*)malloc(sizeof(test_toc_t) * test_toc_size);
	if (!pdev->psection->test_toc) return (DIAG_MALLOC_ERROR);
	bzero (pdev->psection->test_toc, sizeof(test_toc_t) * test_toc_size);
	memcpy(pdev->psection->test_toc, pinfo->psection->test_toc, 
			sizeof(test_toc_t)*test_toc_size);

	ptoc = pinfo->psection->test_toc;
	ttoc = pdev->psection->test_toc;
	while (ptoc && ptoc->name && ttoc && ttoc->name) {
		// Allocate Test Parameters
		test_param_size = diag_param_size_get(ptoc->test_params);

		ttoc->test_params = (test_parameters_t*) 
				malloc(test_param_size * sizeof(test_parameters_t));
		if (!ttoc->test_params) 
			return (DIAG_MALLOC_ERROR);

		memcpy( ttoc->test_params, ptoc->test_params,
			test_param_size * sizeof(test_parameters_t));

		tparam  = ttoc->test_params; 
		pparam  = ptoc->test_params; 

		// Initilize string parameters.
		while(pparam->parameter) {
			if ((pparam->format == FORMAT_TYPE_STR) ||
			    (pparam->format == FORMAT_TYPE_MASK)) {
				tparam->discrete.ptr = (uint8_t*)malloc(DATA_SIZE); 
				if (!tparam->discrete.ptr)
					return (DIAG_MALLOC_ERROR);
				bzero(tparam->discrete.ptr, DATA_SIZE);
				memcpy(tparam->discrete.ptr, pparam->discrete.ptr, 
					strlen(pparam->discrete.ptr)+1);
			}
			if (pparam->format == FORMAT_TYPE_MASK) {
				uint64_t data64 = 0;
				tparam->max.ptr = (uint8_t*)malloc(sizeof(uint64_t)); 
				if (!tparam->max.ptr)
					return (DIAG_MALLOC_ERROR);

				if (diag_convert_str_to_mask(tparam->discrete.ptr, &data64) == 0) {
					*(uint64_t*)tparam->max.ptr = data64;
				}
			}
			pparam++;
			tparam++;
		}
		// Allocate Result Params
		test_param_size += sizeof(fail_params)/sizeof(test_results_t)-1;
		ttoc->test_results = (test_results_t*)malloc(test_param_size * sizeof(test_results_t));
		bzero(ttoc->test_results, test_param_size * sizeof(test_results_t));
	
		// Initialize Result Params
		tparam   = ttoc->test_params; 
		presults = ttoc->test_results; 
		
		while (tparam->parameter) {
			presults->parameter   = tparam->parameter;
			presults->format = tparam->format;
			presults->flag   = 0; 

			if ((presults->format == FORMAT_TYPE_STR) ||
			    (presults->format == FORMAT_TYPE_MASK)) {
				presults->discrete.ptr = (uint8_t*)malloc(DATA_SIZE); 
				if (!presults->discrete.ptr)
					return (DIAG_MALLOC_ERROR);
				bzero(presults->discrete.ptr, DATA_SIZE);
				memcpy(presults->discrete.ptr, tparam->discrete.ptr, 
					strlen(tparam->discrete.ptr)+1);
			} else {
				presults->discrete.value = tparam->discrete.value;
			}
			presults++;
			tparam++;
		}

		// Initialize fail parameters	
		fresults = fail_params; 
		while (fresults->parameter) {
			presults->parameter   = fresults->parameter;
			presults->format = fresults->format;
			presults->flag   = 0; 
	
			if ((presults->format == FORMAT_TYPE_STR) ||
			    (presults->format == FORMAT_TYPE_MASK)) {
				presults->discrete.ptr = (uint8_t*)malloc(DATA_SIZE); 
				if (!presults->discrete.ptr)
					return (DIAG_MALLOC_ERROR);
				bzero(presults->discrete.ptr, DATA_SIZE);
			} else {
				presults->discrete.value = tparam->discrete.value;
			}
			presults++;
			fresults++;
		}
		ptoc++;
		ttoc++;
	}
	return (0);
}

static uint32_t diag_dev_tree_alloc (board_info_t *pinfo, diag_dev_t *ptopdev, 
					uint32_t dev_cnt)
{
	uint32_t	cnt, tcnt = 0;
	diag_dev_t  *pparent = ptopdev;
	diag_dev_t  *pdev = ptopdev, *pcurdev;
	diag_dev_t  *pnext = NULL, *pprev = NULL;	

	while (pinfo->dev_type != DEV_UNKNOWN) {

		pnext = ((pinfo+1)->dev_type == DEV_UNKNOWN) ?
			NULL : pdev + pinfo->dev_cnt;

		pcurdev = pdev;
		// Update all instances in a device.
		for (cnt = 0; cnt < pinfo->dev_cnt; cnt++, pdev++) {
			pdev->p_parent      = pparent;
			pdev->p_prev        = cnt ? NULL : pprev;
			pdev->p_next        = cnt? NULL : pnext;
			pdev->p_prev_inst   = cnt ? (pdev-1): NULL;
			pdev->p_next_inst   = (cnt+1 == pinfo->dev_cnt) ? 
						NULL: (pdev+1);
			pdev->ioctl 	    = pinfo->ioctl;
			pdev->preg 	    = pinfo->preg;
			pdev->portreg 	    = pinfo->portreg;
			pdev->pmem 	    = pinfo->pmem;
			if (!cnt) {
				if (diag_section_alloc(pinfo, pdev)) {
					printf("Failed to allocate section\n");
					return (DIAG_SOFTWARE_ERROR);
				}
			} else {
				pdev->psection 	    = NULL; 
			}

			pdev->name	    = malloc(strlen(pinfo->name + 5)); 
			if (!pdev->name) {
				printf("Failed to allocate name\n");
				return (DIAG_SOFTWARE_ERROR);
			}
			sprintf(pdev->name, "%s%d", pinfo->name, cnt);
			pdev->type 	    = pinfo->dev_type;
			pdev->instance 	    = cnt; 
			pdev->dev_addr	    = pinfo->dev_addr ? pinfo->dev_addr[cnt] : 0;
			pdev->dev_bus	    = pinfo->dev_bus  ? pinfo->dev_bus[cnt] : 0;
			pdev->flags	    = 0x00; 
			pdev->port_cnt 	    = pinfo->port_cnt;

			if (tcnt++ >= dev_cnt) {
				printf(" ERROR: Dev Count is incorrect %d\n",
						dev_cnt);
				return (DIAG_SOFTWARE_ERROR);
			}
		}
		pprev=pcurdev;
		pinfo++;
	}

	return (0);
}

uint32_t diag_dev_init (board_info_t *pinfo, diag_dev_t **ppdev)
{
	diag_dev_t*		pdev = NULL;
	uint32_t	dev_cnt, rc = 0;

	if (!pinfo) {
		printf(" ERROR: Invalid Board Info\n");
		return (DIAG_SOFTWARE_ERROR);
	}

	dev_cnt = diag_dev_count_get(pinfo);
	pdev = (diag_dev_t*)malloc(sizeof(diag_dev_t) * dev_cnt);
	if (pdev == NULL) {
		printf(" ERROR: Failed to allocate %d * %d bytes\n", 
				(int)sizeof(diag_dev_t), dev_cnt);
		return (DIAG_MALLOC_ERROR);
	}

	bzero(pdev, sizeof(diag_dev_t) * dev_cnt);

	rc = diag_dev_tree_alloc(pinfo, pdev, dev_cnt);
	if (rc) {
		printf (" ERROR: Failed to Create the tree\n");
		return (DIAG_SOFTWARE_ERROR);
	}

	*ppdev = pdev;
	return (rc);
}


uint32_t diag_ioctl (dev_type_t dev_type, uint32_t inst, uint32_t opcode, ...)
{
	uint32_t rc = 0;
	va_list arglist;
	diag_dev_t *pdev = diag_board_dev_get();

	va_start(arglist, opcode);
	while (pdev) {
		if(pdev->type == dev_type) {
			while (pdev) {
				if (pdev->instance == inst) {
					if (pdev->ioctl) {
						rc = (*pdev->ioctl)(pdev, opcode, arglist);
						return (rc);
					}
				}
				pdev = pdev->p_next_inst;
			}
			printf("Invalid device (%d) Instance %d\n", dev_type, inst);
			return (DIAG_SOFTWARE_ERROR);
		}
		pdev = pdev->p_next;
	}

	printf("Invalid device (%d)\n", dev_type);
	va_end(arglist);
	return (DIAG_SOFTWARE_ERROR);
}

static uint32_t diag_dev_ioctl (diag_dev_t *pdev, uint32_t opcode, ...)
{
	va_list arglist;

	va_start(arglist, opcode);
	if (pdev && pdev->ioctl)
		return ((*pdev->ioctl)(pdev, opcode, arglist));
	va_end(arglist);
	return (DIAG_SOFTWARE_ERROR);
}

uint32_t diag_ioctl_all_instances (dev_type_t dev_type, uint32_t opcode, ...)
{
	uint32_t rc = 0;
	va_list arglist;
        va_list argcopy;
	diag_dev_t *pdev = diag_board_dev_get();

	va_start(arglist, opcode);
	while (pdev) {
		if(pdev->type == dev_type) {
			while (pdev) {
				if (pdev->ioctl) {
                                        va_copy(argcopy, arglist);
					rc = (*pdev->ioctl)(pdev, opcode, argcopy);
                                        va_end(argcopy);
					if (rc) return (rc);
				}
				pdev = pdev->p_next_inst;
			}
			return (rc);
		} 
		pdev = pdev ? pdev->p_next : NULL;
	}

	printf("Invalid device (%d)\n", dev_type);
	va_end(arglist);
	return (DIAG_SOFTWARE_ERROR);
}

static uint32_t diag_ioctl_all_device_types (diag_dev_t *pdev, uint32_t opcode, 
				      va_list arglist)
{
	int rc = 0;
	if (!pdev) return(0);

	rc = diag_ioctl_all_device_types(pdev->p_next, opcode, arglist);
	if (rc) return (rc);

	if (pdev->ioctl) {
		rc = (*pdev->ioctl)(pdev, opcode, arglist);
	}
	return (rc);
}

uint32_t diag_ioctl_all_dev_types (uint32_t opcode, ...)
{
	uint32_t rc = 0;
	va_list arglist;
	diag_dev_t *pdev = diag_board_dev_get();

	va_start(arglist, opcode);
	rc = diag_ioctl_all_device_types(pdev, opcode, arglist);
	va_end(arglist);

	return (rc);
}

static uint32_t diag_ioctl_all_devs (diag_dev_t *pdev, uint32_t opcode, va_list arglist)
{
	uint32_t rc = 0;
	if (!pdev) 
		return (rc);

	if (pdev->ioctl) {
		rc = (*pdev->ioctl)(pdev, opcode, arglist);
	}
	diag_ioctl_all_devs(pdev->p_next, opcode, arglist);
	diag_ioctl_all_devs(pdev->p_next_inst, opcode, arglist);

	return (rc);
}

uint32_t diag_ioctl_all (uint32_t opcode, ...)
{
	uint32_t rc = 0;
	va_list arglist;
	diag_dev_t *pdev = diag_board_dev_get();

	va_start(arglist, opcode);
	rc = diag_ioctl_all_devs(pdev, opcode, arglist);
	va_end(arglist);

	return (rc);
}

typedef enum _dev_ops_e_
{
	DEV_OP_NAME = 1,
} dev_ops_t;

void diag_op_on_all_devs (diag_dev_t *pdev, uint32_t op)
{
	if (!pdev) return;		

	if (pdev->p_next) 
		diag_op_on_all_devs(pdev->p_next, op);
	if (pdev->p_next_inst) 
		diag_op_on_all_devs(pdev->p_next_inst, op);

	switch(op) {
		case	DEV_OP_NAME:
			if (pdev && pdev->name)
				printf("%-8s [%d (%d): 0x%08X]\n", 
					pdev->name, pdev->instance,
					pdev->dev_bus, pdev->dev_addr);
			break;

		default:
			break;
	}
}

void diag_dump_dev ()
{
	diag_op_on_all_devs(diag_board_dev_get(), DEV_OP_NAME);
}


static void diag_set_dev_addr (diag_dev_t *pdev, uint8_t *dev_name, uint32_t addr)
{
	if (!pdev) return;		

	if (!strncasecmp(pdev->name, dev_name, strlen(dev_name)+1)) {
		pdev->dev_addr = addr;
		return;
	} 
	if (pdev->p_next) 
		diag_set_dev_addr(pdev->p_next, dev_name, addr);
	if (pdev->p_next_inst) 
		diag_set_dev_addr(pdev->p_next_inst, dev_name, addr);
}

void diag_fix_dev_addr (uint8_t *devname, uint32_t addr)
{
	diag_set_dev_addr(diag_board_dev_get(), devname, addr);
}

diag_dev_t *diag_get_dev_by_name (char *name)
{
	diag_dev_t *pdev = diag_board_dev_get(), *pnext;

	if (!pdev) return NULL;

	while (pdev) {
		// Check device.
		if (!strncasecmp(pdev->name, name, strlen(name)))
			return (pdev);
		pnext = pdev->p_next_inst;

		// Check instances
		while(pnext) {	
			if (!strncasecmp(pnext->name, name, strlen(name)))
				return (pnext);
			pnext = pnext->p_next_inst;
		}	
		pdev = pdev->p_next;
	}

	return NULL;
}


static uint32_t diag_get_arg_port_num (char *parg, uint32_t max_port, uint32_t *port)
{
	// get port number
	if ((*parg == 'p') || (*parg == 'P')) {
		*port= strtoul(++parg, NULL, 0);
		return (*port < max_port) ? 0 : 1;
	} 
	return (1);
}

uint32_t diag_reg_access (int argc, char *argv[], int flg)
{
	diag_dev_t *pdev;
	reg_desc_t *preg;
	uint32_t    addr, data, cnt; 
	uint32_t    port, rc, mdata, len=1;
	char	    pdata[64], pflag = 0;
	regex_t	    myregex;

	if (argc < 2+flg) {
		printf("ERROR: Fewer Arguments than expected (argc=%d, flag=%d\n",
				argc, flg);
		return (DIAG_SYNTAX_ERROR);
	}

	pdev = diag_get_dev_by_name(argv[flg]);
	if (!pdev) {
		printf("ERROR: Invalid device\n");
		return (DIAG_SYNTAX_ERROR);
	}

	switch (argv[1+flg][0]) {
		case	'i':			// Init Flag
			rc = diag_dev_ioctl(pdev, IOCTL_DEVICE_INIT); 
			break;

		case 	'r':
			addr = (argc >= (3+flg)) ? strtoul(argv[2+flg], NULL, 16) : 0;
			len  = (argc >= (4+flg)) ? strtoul(argv[3+flg], NULL, 10) : 1;

			// get port number
			if ((argc == (5+flg))) {
				if (diag_get_arg_port_num(argv[4+flg], pdev->port_cnt, &port)) {
					printf("ERROR: Invalid port number\n");
					return (DIAG_SYNTAX_ERROR);
				}
				pflag = 1;
			}

			preg = find_reg_desc_by_addr(pflag? pdev->portreg[port] : pdev->preg, addr);	

			if (!preg) {
				printf("Error: Invalid address\n");
				return (DIAG_SYNTAX_ERROR);
			}

			for (cnt = 0; cnt < len; cnt++) {	
				rc = diag_dev_ioctl(pdev, IOCTL_REG_RD, preg->addr, &data, port, preg); 
				if (rc) {
					printf("Error: Reading %s from 0x%08X\n",
						pdev->name, addr);
					return (rc);
				}
				decode_reg(preg, preg->addr, data);
				preg = find_next_reg(preg);
			}
			break;

		case 	'w':
			if (argc < (4+flg)) {
				printf("ERROR: Fewer arguments than expected\n");
				return (DIAG_SYNTAX_ERROR);
			}
			addr = (argc >= (3+flg)) ? strtoul(argv[2+flg], NULL, 16) : 0;
			data = (argc >= (4+flg)) ? strtoul(argv[3+flg], NULL, 16) : 0;
			len  = (argc >= (5+flg)) ? strtoul(argv[4+flg], NULL, 16) : 1;
		
			// get port number
			if ((argc == (6+flg))) {
				if (diag_get_arg_port_num(argv[5+flg], pdev->port_cnt, &port)) {
					printf("ERROR: Invalid port number\n");
					return (DIAG_SYNTAX_ERROR);
				}
				pflag = 1;
			}
			preg = find_reg_desc_by_addr(pflag? pdev->portreg[port] : pdev->preg, addr);	
			if (!preg) {
				printf("Error: Invalid address\n");
				return (DIAG_SYNTAX_ERROR);
			}

			for (cnt = 0; cnt < len; cnt++) {	
				rc = diag_dev_ioctl(pdev, IOCTL_REG_WR, preg->addr, data, port, preg); 
				if (rc) {
					printf("Error: Writing %s from 0x%08X\n",
						pdev->name, addr);
					return (rc);
				}
				preg = find_next_reg(preg);
			}
			break;

		case 	'm':
			addr = (argc >= (3+flg)) ? strtoul(argv[2+flg], NULL, 16) : 0;
			len  = (argc >= (4+flg)) ? strtoul(argv[3+flg], NULL, 10) : 1;

			// get port number
			if ((argc >= (5+flg))) {
				if (diag_get_arg_port_num(argv[4+flg], pdev->port_cnt, &port)) {
					printf("ERROR: Invalid port number\n");
					return (DIAG_SYNTAX_ERROR);
				}
				pflag = 1;
			}
			preg = find_reg_desc_by_addr(pflag? pdev->portreg[port] : pdev->preg, addr);	
			if (!preg) {
				printf("Error: Invalid address\n");
				return (DIAG_SYNTAX_ERROR);
			}

			for (cnt = 0; cnt < len; cnt++) {	
				rc = diag_dev_ioctl(pdev, IOCTL_REG_RD, preg->addr, &data, port, preg); 
				if (rc) {
					printf("Error: Reading %s from 0x%08X\n",
						pdev->name, addr);
					return (rc);
				}
				printf("%-36s [0x%08X : 0x%08X] :", preg->name, preg->addr, data);
				fgets(pdata, sizeof(pdata)-1,stdin);
				if (strlen(pdata)) {
					data = strtoul(pdata, NULL, 16);
					if (errno != EINVAL) {
						rc = diag_dev_ioctl(pdev, IOCTL_REG_WR, 
								preg->addr, data, port, preg); 
						if (rc) {
							printf("Error: Writing %s from 0x%08X\n",
								pdev->name, addr);
							return (rc);
						}
					}
				}
				preg = find_next_reg(preg);
			}
			break;

		case 	's':
		case 	'c':
			if (argc < (4+flg)) {
				printf("ERROR: Fewer arguments than expected\n");
				return (DIAG_SYNTAX_ERROR);
			}
			addr = (argc >= (3+flg)) ? strtoul(argv[2+flg], NULL, 16) : 0;
			data = (argc >= (4+flg)) ? strtoul(argv[3+flg], NULL, 16) : 0;
			len  = (argc >= (5+flg)) ? strtoul(argv[4+flg], NULL, 16) : 1;
		
			// get port number
			if ((argc >= (6+flg))) {
				if (diag_get_arg_port_num(argv[5+flg], pdev->port_cnt, &port)) {
					printf("ERROR: Invalid port number\n");
					return (DIAG_SYNTAX_ERROR);
				}
				pflag = 1;
			}
			preg = find_reg_desc_by_addr(pflag? pdev->portreg[port] : pdev->preg, addr);	
			if (!preg) {
				printf("Error: Invalid address\n");
				return (DIAG_SYNTAX_ERROR);
			}

			for (cnt = 0; cnt < len; cnt++) {	
				rc = diag_dev_ioctl(pdev, IOCTL_REG_RD, preg->addr, &mdata, port, preg); 
				if (rc) {
					printf("Error: Reading %s from 0x%08X\n",
						pdev->name, preg->addr);
					return (rc);
				}
				if (argv[1+flg][0] == 's') {
					mdata |= data;	
				} else {
					mdata &= ~data;
				}
				rc = diag_dev_ioctl(pdev, IOCTL_REG_WR, preg->addr, mdata, port, preg); 
				if (rc) {
					printf("Error: Writing %s from 0x%08X\n",
						pdev->name, addr);
					return (rc);
				}
				preg = find_next_reg(preg);
			}
			break;


		case 	'l':
			// get port number
			if ((argc >= (3+flg))) {
				if (diag_get_arg_port_num(argv[2+flg], pdev->port_cnt, &port)) {
					printf("ERROR: Invalid port number\n");
					return (DIAG_SYNTAX_ERROR);
				}
				pflag = 1;
			}
			reg_list_dump(pflag ? pdev->portreg[port] : pdev->preg);
			break;

		case 	'v':
			rc = diag_dev_ioctl(pdev, IOCTL_VERSION); 
			break;

		case 	'd':
			// get port number
			if ((argc >= (3+flg))) {
				if (diag_get_arg_port_num(argv[2+flg], pdev->port_cnt, &port)) {
					printf("ERROR: Invalid port number\n");
					return (DIAG_SYNTAX_ERROR);
				}
				pflag = 1;
			}
			preg = pflag? pdev->portreg[port] : pdev->preg;

			while (preg && preg->desc_type != TYP_NONE) {
				rc = diag_dev_ioctl(pdev, IOCTL_REG_RD, preg->addr, &data, port, preg); 
				if (rc) {
					printf("Error: Reading %s from 0x%08X\n",
						pdev->name, preg->addr);
					return (rc);
				}

				decode_reg(preg, preg->addr, data);
				preg = find_next_reg(preg);
			}
			break;

		case 	'n':
			if (argc < (3+flg)) {
				printf("ERROR: Fewer arguments then expected\n");
				return (DIAG_SYNTAX_ERROR);
			}

			// get port number
			if ((argc >= (4+flg))) {
				if (diag_get_arg_port_num(argv[3+flg], pdev->port_cnt, &port)) {
					printf("ERROR: Invalid port number\n");
					return (DIAG_SYNTAX_ERROR);
				}
				pflag = 1;
			}

			preg = find_reg_desc(pflag? pdev->portreg[port] : pdev->preg, argv[2+flg]);
			while (preg && (preg->desc_type != TYP_NONE)) {
				rc = diag_dev_ioctl(pdev, IOCTL_REG_RD, preg->addr, &mdata, port, preg); 
				if (rc) {
					printf("Error: Reading %s from 0x%08X\n",
						pdev->name, preg->addr);
					return (rc);
				}
				decode_reg(preg, preg->addr, mdata);
				preg = find_reg_desc(++preg, argv[2+flg]);
			}
			break;

		case 	'u':
			if (argc < (3+flg)) {
				printf("ERROR: Fewer arguments then expected\n");
				return (DIAG_SYNTAX_ERROR);
			}

			// get port number
			if ((argc >= (4+flg))) {
				if (diag_get_arg_port_num(argv[3+flg], pdev->port_cnt, &port)) {
					printf("ERROR: Invalid port number\n");
					return (DIAG_SYNTAX_ERROR);
				}
				pflag = 1;
			}

			preg = find_reg_desc_by_typ(pflag ? pdev->portreg[port] : pdev->preg, argv[2+flg]);
			while (preg && (preg->desc_type != TYP_NONE)) {
				rc = diag_dev_ioctl(pdev, IOCTL_REG_RD, preg->addr, &mdata, port, preg); 
				if (rc) {
					printf("Error: Reading %s from 0x%08X\n",
						pdev->name, preg->addr);
					return (rc);
				}
				printf( "%-24s  (0x%08x) : [%08x]\n", preg->name, preg->addr, mdata );
				preg = find_reg_desc_by_typ(++preg, argv[2+flg]);
			}
			break;

		case 	'g':
			if (argc < (3+flg)) {
				printf("ERROR: Fewer arguments then expected\n");
				return (DIAG_SYNTAX_ERROR);
			}
			rc = regcomp(&myregex, argv[2+flg], REG_EXTENDED);
			if (rc) {
				printf("ERROR: Failed to compile regular expression (%d)\n", rc);
				return (DIAG_SYNTAX_ERROR);
			}
			
			// get port number
			if ((argc >= (4+flg))) {
				if (diag_get_arg_port_num(argv[3+flg], pdev->port_cnt, &port)) {
					printf("ERROR: Invalid port number\n");
					return (DIAG_SYNTAX_ERROR);
				}
				pflag = 1;
			}

			// Get the register information if reg ex matched.
			preg = find_reg_desc_by_exp(pflag ? pdev->portreg[port] : pdev->preg, &myregex);
			while (preg && (preg->desc_type != TYP_NONE)) {
				rc = diag_dev_ioctl(pdev, IOCTL_REG_RD, preg->addr, &mdata, port, preg); 
				if (rc) {
					printf("Error: Reading %s from 0x%08X\n",
						pdev->name, preg->addr);
					return (rc);
				}
				printf( "%-24s  (0x%08x) : [%08x]\n", preg->name, preg->addr, mdata );
				preg = find_reg_desc_by_exp(++preg, &myregex);
			}
			break;

		default:
			return (DIAG_SYNTAX_ERROR);
			break;
	}

	return (0);
}

uint32_t diag_reg_access_by_blk (int argc, char *argv[], int flg, char *block)
{
	diag_dev_t *pdev;
	reg_desc_t *preg;
	uint32_t    addr, data, cnt; 
	uint32_t    port=0, rc, len=1;

	if (argc < 2+flg) {
		printf("ERROR: Fewer Arguments than expected "
                       "(argc=%d, flag=%d\n", argc, flg);
		return (DIAG_SYNTAX_ERROR);
	}

	pdev = diag_get_dev_by_name(argv[0]);
	if (!pdev) {
		printf("ERROR: Invalid device\n");
		return (DIAG_SYNTAX_ERROR);
	}

	switch (argv[1+flg][0]) {
		case 	'r':
			addr = (argc >= (3+flg)) ? 
                               strtoul(argv[2+flg], NULL, 16) : 0;
			len  = (argc >= (4+flg)) ? 
                               strtoul(argv[3+flg], NULL, 10) : 1;


			preg = find_reg_desc_by_blk_and_addr(pdev->preg, block,
                                                             addr);	

			if (!preg) {
				printf("Error: Invalid address\n");
				return (DIAG_SYNTAX_ERROR);
			}

			for (cnt = 0; cnt < len; cnt++) {	
				rc = diag_dev_ioctl(pdev, IOCTL_REG_RD, preg->addr, &data, port, preg); 
				if (rc) {
					printf("Error: Reading %s from 0x%08X\n",
						pdev->name, addr);
					return (rc);
				}
				decode_reg(preg, preg->addr, data);
				preg = find_next_reg(preg);
			}
			break;

		case 	'w':
			if (argc < (4+flg)) {
				printf("ERROR: Fewer arguments than "
                                       "expected\n");
				return (DIAG_SYNTAX_ERROR);
			}
			addr = (argc >= (3+flg)) ? 
                                strtoul(argv[2+flg], NULL, 16) : 0;
			data = (argc >= (4+flg)) ? 
                                strtoul(argv[3+flg], NULL, 16) : 0;
			len  = (argc >= (5+flg)) ? 
                                strtoul(argv[4+flg], NULL, 16) : 1;
		
			preg = find_reg_desc_by_blk_and_addr(pdev->preg, 
                                                             block, addr);	
			if (!preg) {
				printf("Error: Invalid address\n");
				return (DIAG_SYNTAX_ERROR);
			}

			for (cnt = 0; cnt < len; cnt++) {	
				rc = diag_dev_ioctl(pdev, IOCTL_REG_WR, 
                                               preg->addr, data, port, preg); 
				if (rc) {
					printf("Error: Writing %s from "
                                               "0x%08X\n", pdev->name, addr);
					return (rc);
				}
				preg = find_next_reg(preg);
			}
			break;

		default:
			return (DIAG_SYNTAX_ERROR);
			break;
	}
	return (0);
}

int diag_boot_init()
{
        printf(" Diag Boot Configuration.\n");
        return (diag_ioctl_all(IOCTL_BOOT_CONFIG));
}

int diag_special_init()
{
        printf(" Diag Special Configuration.\n");
	return (diag_ioctl(DEV_BOARD, 0, IOCTL_SPECIAL_INIT)); 
}

int diag_sw_alloc()
{
	return (diag_ioctl(DEV_BOARD, 0, IOCTL_SW_ALLOC)); 
}

int diag_sw_dealloc()
{
	return (diag_ioctl(DEV_BOARD, 0, IOCTL_SW_DEALLOC)); 
}

int find_dev_addr_by_inst(uint8_t dev_type, uint8_t inst, uint32_t* addr)
{
	diag_dev_t *pdev = diag_board_dev_get();

	while (pdev) {
		if (pdev->type == dev_type) {
			while(pdev) {
				if (pdev->instance == inst) {
					*addr = pdev->dev_addr;
					return (0);
				}
				pdev = pdev->p_next_inst;
			}
			return (-1);
		}
		pdev = pdev->p_next;
	}

	return (-1);
}

int find_dev_bus_by_inst(uint8_t dev_type, uint8_t inst, uint16_t* dbus)
{
	diag_dev_t *pdev = diag_board_dev_get();

	while (pdev) {
		if (pdev->type == dev_type) {
			while(pdev) {
				if (pdev->instance == inst) {
					*dbus = pdev->dev_bus;
					return (0);
				}
				pdev = pdev->p_next_inst;
			}
			return (-1);
		}
		pdev = pdev->p_next;
	}

	return (-1);
}

uint8_t *find_dev_name_by_inst(uint8_t dev_type, uint8_t inst)
{
	diag_dev_t *pdev = diag_board_dev_get();

	while (pdev) {
		if (pdev->type == dev_type) {
			while(pdev) {
				if (pdev->instance == inst) {
					return(pdev->name);
				}
				pdev = pdev->p_next_inst;
			}
			return (NULL);
		}
		pdev = pdev->p_next;
	}

	return (NULL);
}

int is_dev_type_valid(uint8_t dev_type)
{
    diag_dev_t *pdev = diag_board_dev_get();
    while (pdev) {
        if (pdev->type == dev_type) {
            return 1;
        }
        pdev = pdev->p_next;
    }
    return 0;
}
