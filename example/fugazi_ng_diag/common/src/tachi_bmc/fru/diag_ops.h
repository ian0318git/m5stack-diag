/* $Id: diag_ops.h,v 1.2 2016/04/20 08:41:36 benchen2 Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/tachi_bmc/fru/diag_ops.h,v $
 *
 *      File:   diag_ops.h
 *      Name:   Sudharshan Kadari
 *
 *      Description:
 *
 *
 * Copyright (c) 1985-2016 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *
 *********************************************************************/

#ifndef _DIAG_OPS_H_
#define _DIAG_OPS_H_

extern void 		diag_dump_dev 		();
extern void		diag_dump_all		();
extern void		diag_dump_test		(test_toc_t *ptest);
extern void		diag_ttoc_dump		(test_toc_t *ptest);
extern void		diag_err_display	 (int errcode);
extern void		diag_err_code_set	(test_results_t *pr, uint32_t err);
extern void		diag_dump_section	(section_toc_t *psect);
extern void		diag_err_port_set	(test_results_t *pr, uint64_t pmask);
extern void		diag_test_toc_dump	(test_toc_t *p_test_toc);
extern void		diag_dev_tree_free	(diag_dev_t *ptopdev);
extern void		diag_err_device_set	(test_results_t *pr, uint8_t *dev);
extern void		diag_test_param_dump	(test_parameters_t *pparam);
extern void		diag_err_display_all	();
extern void		diag_dump_glob_param	();
extern void		diag_section_toc_dump	(section_toc_t *psect);
extern void		diag_test_results_dump	(test_results_t *presults);
extern void		diag_err_addr_data_set	(test_results_t *pre, uint32_t addr, 
						 uint32_t edata, uint32_t rdata);
extern void*		diag_get_global_param_ptr(uint8_t *parameter);


extern int		diag_main		();
extern int 		diag_cli_cmd_add 	(cli_cmds_t *pcmd);
extern int		dsh_add_internal_commands();


extern uint32_t		diag_dev_init		(board_info_t *p, diag_dev_t **pdev);
extern uint32_t		diag_run_all		(uint32_t runcount);
extern uint32_t		diag_run_test		(test_toc_t *ptest, uint32_t r);
extern uint32_t 	diag_ioctl_all 		(uint32_t opcode, ...);
extern uint32_t		diag_set_param		(int argc, char *argv[]);
extern uint32_t		diag_skip_test		(test_toc_t *ptest, uint8_t flag);
extern uint32_t		diag_results_all	(uint32_t flag);
extern uint32_t		diag_run_section	(section_toc_t *ps, uint32_t cnt);
extern uint32_t		diag_results_fail	();
extern uint32_t		diag_get_runcount	();
extern uint32_t		diag_get_nfsmode	();
extern uint32_t		diag_get_verbose	();
extern uint32_t		diag_get_detection	();
extern uint32_t		diag_get_debug		();
extern uint32_t		diag_get_rdwacc		();
extern uint32_t		diag_get_laneswap	();
extern uint32_t		diag_get_retry		();
extern uint32_t		diag_get_subslot	();
extern uint32_t		diag_get_cliloop	();
extern uint32_t		diag_get_chstest	();
extern uint32_t		diag_get_sysinit	();
extern uint32_t		diag_get_revision	();
extern uint32_t		diag_get_krphyrev	();
extern uint32_t		diag_get_extended	();
extern uint32_t		diag_get_preemp		();
extern uint32_t		diag_get_bcmemp		();
extern uint32_t		diag_get_psumask	();
extern uint32_t		diag_get_fanmask	();
extern uint32_t		diag_get_blademask	();

extern uint32_t		diag_results_pass	();
extern uint32_t		diag_results_test	(test_toc_t *ptest, uint32_t flag);
extern uint32_t		diag_skip_all		(uint8_t flag);
extern uint32_t		diag_skip_section	(section_toc_t *psect, uint8_t flag);
extern uint32_t		diag_board_dev_set	(uint32_t slot);
extern uint32_t		diag_results_enable	(test_results_t *re, uint8_t ena);
extern uint32_t		diag_results_update	(test_parameters_t*, test_results_t*);
extern uint32_t		diag_param_size_get	(test_parameters_t *pparams);
extern uint32_t		diag_results_section	(section_toc_t* psect, uint32_t flag);
extern uint32_t		diag_get_stop_on_fail	();
extern uint32_t		diag_convert_str_to_mask(char *pstr, uint64_t *data);
extern uint32_t		diag_ioctl_all_dev_types(uint32_t opcode, ...);
extern uint32_t		diag_ioctl_all_instances(dev_type_t dt, uint32_t opcode, ...);
extern uint32_t 	diag_ioctl		(dev_type_t dev_type, uint32_t inst, 
						uint32_t opcode, ...);

extern test_toc_t*	diag_test_get		(section_toc_t *psect, uint8_t *name);
extern diag_dev_t*	diag_board_dev_get 	();
extern section_toc_t*	diag_section_get	(uint8_t *name);


extern int 		dsh_cmd_add		(char * cmd_name, char * cmd_argdesc,
                                                 int  (*cmd_function)(TCL_ARGS
                                                                      int argc, char *argv[]));

extern diag_dev_t*	diag_get_dev_by_name	(char *name);


// Diag Reg ops
extern reg_desc_t*      find_next_reg           (reg_desc_t *p_reg);
extern reg_desc_t*      find_reg_desc           (reg_desc_t *p_reg, char *reg_name );
extern reg_desc_t*	find_reg_desc_by_typ	(reg_desc_t *p_reg, unsigned char *typ);
extern reg_desc_t*      find_reg_desc_by_addr   (reg_desc_t *p_reg, unsigned int reg_addr );
extern reg_desc_t*      find_reg_desc_by_blk_and_addr   (reg_desc_t *p_reg, 
                                                 char *block,
                                                 unsigned int reg_addr);
extern char*		find_reg_name_by_addr	(reg_desc_t *p, unsigned int );

extern int              decode_reg              (reg_desc_t *p_reg,  
						 unsigned int reg_addr, 
						 unsigned int reg_val );
extern void             reg_list_dump           (reg_desc_t *p_reg );
extern uint32_t		diag_reg_access		(int argc, char *argv[], int flag);
extern uint32_t	diag_reg_access_by_blk(int argc, char *argv[], int flag, char *);
extern int		diag_special_init	();
extern int		diag_boot_init		();
extern void		diag_set_flip_flag	();
extern int		is_diag_boot		();
extern int		diag_sw_alloc();
extern int		diag_sw_dealloc();


extern int 		find_dev_bus_by_inst	(uint8_t dev_type, uint8_t inst, 
						 uint16_t* bus);
extern int 		find_dev_addr_by_inst	(uint8_t dev_type, uint8_t inst, 
						 uint32_t* addr);
extern uint8_t*		find_dev_name_by_inst	(uint8_t dev_type, uint8_t inst);

extern int		diag_board_serial_no_get(uint8_t*, uint8_t);
extern int		diag_board_part_number_get(uint8_t*, uint8_t);

extern int		palo_map_port ( int port, int *palo, int *phy );
extern int		palo_map_enics ( int, int, char *, char * );
extern int		host_map_palo_inst ( int slot, int *palo );
extern void 		diag_fix_dev_addr (uint8_t *devname, uint32_t addr);
extern void		diag_test_param_set(const char *sect_name, const char *test_name, 
				const char *param_name, uint32_t value, uint32_t min, 
				uint32_t max);
extern int		is_dev_type_valid (uint8_t dev_type);
extern void		diag_test_str_param_set(const char *sect_name, 
				const char *test_name, const char *param_name, char *value);
#endif // _DIAG_OPS_H_
