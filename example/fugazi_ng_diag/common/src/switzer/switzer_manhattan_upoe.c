/* $Id: switzer_manhattan_upoe.c,v 1.1 2021/04/12 14:01:03 xiaolaya Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/switzer/switzer_manhattan_upoe.c,v $
 *------------------------------------------------------------------
 *
 * switzer_manhattan_upoe.c - Switzer-Manhattan NIM.
 *
 * Mar. 2020, Xuanyu Shi <xuashi@cisco.com>
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

#include "common.h"
#include "types.h"
#include "defs.h"
#include "menu.h"
#include "error.h"
#include "strings.h"
#include "common_utils.h"
#include "linux_api.h"
#include "platform_slot.h"
#include "dash_fpga.h"

#include "switzer_common.h"
#include "switzer_manhattan.h"
#include "switzer_manhattan_upoe.h"
#include "switzer_tps23881_sram_parity.h"


/* According to DE board mapping
 example: port 1 ==> channel 2, channel 1 refers to Gate number on chip
 For UPOE, each port has two channels , each channel refers to ALT-A mode or ALT-B mode,
*/

UpoePortMap_t port_mapping[4] ={{UPOE_PORT_1, UPOE_I2C_0, PSE_CHANNEL_2, PSE_CHANNEL_1},
                                {UPOE_PORT_2, UPOE_I2C_0, PSE_CHANNEL_3, PSE_CHANNEL_4},
                                {UPOE_PORT_3, UPOE_I2C_1, PSE_CHANNEL_1, PSE_CHANNEL_2},
                                {UPOE_PORT_4, UPOE_I2C_1, PSE_CHANNEL_3, PSE_CHANNEL_4}};

int switzer_pse_register_read(struct switzer_manhattan *mod, uint8_t port_num, uint8_t reg)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if(switzer_dash_i2c_slave_read(slave, reg, &data, sizeof(data)) < 0) {
        cterr('f', 0, "cannot read POE register %#.2x\n", reg);
    } else {
        prt("Read POE register  %#.2x, data %#.2x\n", reg, data);
    }
    return data;
}

int switzer_pse_register_write(struct switzer_manhattan *mod, uint8_t port_num, uint8_t reg, uint8_t data)
{
    struct switzer_dash_i2c_slave *slave;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if(switzer_dash_i2c_slave_write(slave, reg, &data, sizeof(data)) < 0) {
        cterr('f', 0, "cannot write POE register %#.2x\n", reg);
        return 1;
    } else {
        prt("Write POE register  %#.2x, data %#.2x\n", reg, data);
        return 0;
    }
    return 0;
}

int switzer_upoe_set_general_mask(struct switzer_manhattan *mod, uint8_t port_num, uint8_t value)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;
    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;

   // prt("the before value is 0x%x,", value);

    if (switzer_dash_i2c_slave_write(slave, POE_REG_GENERAL_MASK, &value, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__ );
        return 1;
    }

    if (switzer_dash_i2c_slave_read(slave, POE_REG_GENERAL_MASK, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__ );
        return 1;
    }
    //prt("the after value is 0x%x,", data);

    return 0;
}

int switzer_upoe_get_general_mask(struct switzer_manhattan *mod, uint8_t port_num, uint8_t *value)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if (switzer_dash_i2c_slave_write(slave, POE_REG_GENERAL_MASK, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__ );
        return 1;
    }
    *value = data;

    return 0;
}

/* each 4-pair port accounts for 2 channels
   HW: PORT 0 -- CHANNEL 3/4, PORT 1--CHANNEL 1/2 */
int switzer_display_port_mapping(void)
{
    uint8_t port_num;
    prt("\n****** 4Pair Mode port Mapping ******\n");
    prt(" Port.Number  Physical_channel_1/Physical_channel_2\n");

    for (port_num = 0; port_num < UPOE_PORTS; port_num++) {
        prt("%4d   %8d  /%2d \n", port_mapping[port_num].portNum,
                port_mapping[port_num].phy_chn1, port_mapping[port_num].phy_chn2);
    }
    return 0;
}

/*4-pair mode, both channels shall be set the same*/
int switzer_upoe_set_operating_mode(struct switzer_manhattan *mod, uint8_t port_num, operating_mode_t mode)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if (switzer_dash_i2c_slave_read(slave, POE_REG_OPERATING_MODE, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__ );
        return 1;
    }

    data |= REGISTER_OPERATION_LEFT_OR(port_num, mode, TWO_BITS_PER_CHNL);
    if (switzer_dash_i2c_slave_write(slave, POE_REG_OPERATING_MODE, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__ );
        return 1;
    }

    return 0;
}

int switzer_upoe_get_classification_detection_status(struct switzer_manhattan *mod,
                         uint8_t port_num, classStatus_t *classStatus, detStatus_t *detectStatus)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data, reg;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if (port_mapping[port_num].phy_chn1 < 2) {
        reg = POE_REG_CH1_DISCOVERY;
    } else {
        reg = POE_REG_CH3_DISCOVERY;
    }
    if (switzer_dash_i2c_slave_read(slave , reg, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
            return 1;
    }
    *classStatus = (data >> UPOE_DATA_SHIFT);
    *detectStatus = (data & UPOE_DATA_MASK);
    return 0;
}

// return value: 0 mean port power is not good, 1 means port pwr good
int switzer_upoe_get_port_pwr_good_status(struct switzer_manhattan *mod, uint8_t port_num)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data, pwr_good_status, port_status;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if (switzer_dash_i2c_slave_read(slave, POE_REG_POWER_STATUS, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
        return 1;
    }
    pwr_good_status = data >> UPOE_DATA_SHIFT;
    prt("port power good status value is 0x%x", pwr_good_status);
    data = REGISTER_OPERATION_LEFT_OR(port_num, SET_ENABLE_BIT, ONE_BIT_PER_CHNL);
    port_status = pwr_good_status & data;
    prt("port power status value is 0x%x", port_status);
    return port_status;
}

// 1 means port is on , 0 mean port is off
int switzer_upoe_get_port_pwr_enable_status(struct switzer_manhattan *mod, uint8_t port_num)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data, pwr_enable_status, port_status;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if (switzer_dash_i2c_slave_read(slave, POE_REG_POWER_STATUS, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
        return 1;
    }
    pwr_enable_status = data & UPOE_DATA_MASK;
    data = REGISTER_OPERATION_LEFT_OR(port_num, SET_ENABLE_BIT, ONE_BIT_PER_CHNL);
    port_status = pwr_enable_status & data;
    return port_status;
}

int switzer_upoe_set_detection_enable (struct switzer_manhattan *mod, uint8_t port_num)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    data = REGISTER_OPERATION_LEFT_OR(port_num, SET_ENABLE_BIT, ONE_BIT_PER_CHNL);
    if (switzer_dash_i2c_slave_write(slave, POE_REG_DETECT_CLASS_ENABLE, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }
    return 0;
}

int switzer_upoe_set_classification_enable (struct switzer_manhattan *mod, uint8_t port_num)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    data = REGISTER_OPERATION_LEFT_OR(port_num, SET_ENABLE_BIT, ONE_BIT_PER_CHNL);
    data = data << UPOE_DATA_SHIFT;
    if (switzer_dash_i2c_slave_write(slave, POE_REG_DETECT_CLASS_ENABLE, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }
    return 0;
}

int switzer_upoe_set_port_Pcut_disable(struct switzer_manhattan *mod, uint8_t port_num)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if (switzer_dash_i2c_slave_read(slave, POE_REG_DISCONNECT_ENABLE, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
        return 1;
    }
    data = REGISTER_OPERATION_LEFT_OR(port_num, SET_ENABLE_BIT, ONE_BIT_PER_CHNL);
    if (switzer_dash_i2c_slave_write(slave, POE_REG_DISCONNECT_ENABLE, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }
    return 0;
}

int switzer_upoe_set_timing_configuration(void)
{
    return 0;
}

int switzer_upoe_restart_detection (struct switzer_manhattan *mod, uint8_t port_num)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    data = REGISTER_OPERATION_LEFT_OR(port_num, SET_ENABLE_BIT, ONE_BIT_PER_CHNL);
    if (switzer_dash_i2c_slave_write(slave, POE_REG_DET_CLASS_RESTART, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }
    return 0;
}

int switzer_upoe_restart_classification (struct switzer_manhattan *mod, uint8_t port_num)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    data = REGISTER_OPERATION_LEFT_OR(port_num, SET_ENABLE_BIT, ONE_BIT_PER_CHNL);
    data |= data << UPOE_DATA_SHIFT;
    if (switzer_dash_i2c_slave_write(slave, POE_REG_DET_CLASS_RESTART, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }
    return 0;
}

int switzer_upoe_set_port_one_bit_oss(struct switzer_manhattan *mod, uint8_t port_num)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if (switzer_dash_i2c_slave_read(slave, POE_REG_PWRPR_ICUT_DISABLE, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
        return 1;
    }
    data = REGISTER_OPERATION_LEFT_OR(port_num, SET_ENABLE_BIT, ONE_BIT_PER_CHNL);
    data |= data << UPOE_DATA_SHIFT;
    if (switzer_dash_i2c_slave_write(slave, POE_REG_PWRPR_ICUT_DISABLE, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }
    return 0;
}

int switzer_upoe_set_detection_classification_enable(struct switzer_manhattan *mod, uint8_t port_num)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data_det, data_cls, data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if (switzer_dash_i2c_slave_read(slave, POE_REG_DETECT_CLASS_ENABLE, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }

    /* set classification enable */
    data_det = REGISTER_OPERATION_LEFT_OR(port_num, SET_ENABLE_BIT, ONE_BIT_PER_CHNL);
    /*set classification enable */
    data_cls = data_det << UPOE_DATA_SHIFT;
    data = data| data_det | data_cls;

    if (switzer_dash_i2c_slave_write(slave, POE_REG_DETECT_CLASS_ENABLE, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }

     if (switzer_dash_i2c_slave_read(slave, POE_REG_DETECT_CLASS_ENABLE, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }
       return 0;
}

int switzer_upoe_set_port_dc_disconnect_enable(struct switzer_manhattan *mod, uint8_t port_num, int disconnect_enable)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data,value;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    /*disconnect value */
    if (switzer_dash_i2c_slave_read(slave, POE_REG_DISCONNECT_ENABLE, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
        return 1;
    }
    value = REGISTER_OPERATION_LEFT_OR(port_num, SET_ENABLE_BIT, ONE_BIT_PER_CHNL);
    if (disconnect_enable == SET_ENABLE_BIT) {
        data |= value;
    } else {
        data &= ~value;
    }
    if (switzer_dash_i2c_slave_write(slave, POE_REG_DISCONNECT_ENABLE, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }

    return 0;
}

int switzer_upoe_set_port_power (struct switzer_manhattan *mod, uint8_t port_num, int on_off)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;

    data = REGISTER_OPERATION_LEFT_OR(port_num, SET_ENABLE_BIT, ONE_BIT_PER_CHNL);
    if (on_off == PD_PORT_ON) {
        data = data;
    } else {
        data <<= UPOE_DATA_SHIFT;
    }
    if (switzer_dash_i2c_slave_write(slave, POE_REG_PWRON, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }

    return 0;
}

int switzer_upoe_set_port_reset (struct switzer_manhattan *mod, uint8_t port_num)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    data = REGISTER_OPERATION_LEFT_OR(port_num, SET_ENABLE_BIT, ONE_BIT_PER_CHNL);
    if (switzer_dash_i2c_slave_write(slave, POE_REG_RESET, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }

    return 0;
}

int switzer_upoe_set_legacy_detection(struct switzer_manhattan *mod, uint8_t port_num, int enable)
{
    struct switzer_dash_i2c_slave *slave;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if (port_mapping[port_num].phy_chn1 >= PSE_CHANNEL_3)
        enable |= 0x50;
    else if (port_mapping[port_num].phy_chn1 <= PSE_CHANNEL_2)
        enable |= 0x05;
    if (switzer_dash_i2c_slave_write(slave, POE_REG_LEGACY_DETECT, &enable, sizeof(enable)) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }
    return 0;
}

int switzer_upoe_set_interrupt_mask(struct switzer_manhattan *mod, uint8_t port_num, uint8_t masked)
{
    struct switzer_dash_i2c_slave *slave;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if (switzer_dash_i2c_slave_write(slave, POE_REG_INTR_MASK, &masked, sizeof(masked)) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }
    return 0;
}

int switzer_upoe_remap_channel(struct switzer_manhattan *mod, uint8_t channel_map)
{
    struct switzer_dash_i2c_slave *slave;
    slave = mod->poe[UPOE_I2C_0].i2c;

    if (switzer_dash_i2c_slave_write(slave, POE_REG_PORTS_REMAPPING, &channel_map, sizeof(channel_map)) < 0) {
        cterr('f',0, "%s at %d: Failed to remap channel", __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

int switzer_upoe_get_interrupt_mask(struct switzer_manhattan *mod, uint8_t port_num, uint8_t* masked)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if (switzer_dash_i2c_slave_read(slave, POE_REG_INTR_MASK, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }
    *masked = data;
    return 0;
}

int switzer_upoe_get_interrupt_events(struct switzer_manhattan *mod, uint8_t port_num, uint8_t *pwr_enable_event,
                                                 uint8_t *pwr_good_status_event, uint8_t *disconnect_event, uint8_t *pwr_cut_event,
                                                 uint8_t *detect_event, uint8_t *cls_event, uint8_t *inrush_event,
                                                 uint8_t *ilim_event, uint8_t *supply_event)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if (switzer_dash_i2c_slave_read(slave, POE_REG_POWER_EVENT, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }
    *pwr_enable_event = data & UPOE_DATA_MASK;
    *pwr_good_status_event = data >> UPOE_DATA_SHIFT ;

    if (switzer_dash_i2c_slave_read(slave, POE_REG_FAULT_EVENT, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }
    *pwr_cut_event = data & UPOE_DATA_MASK;
    *disconnect_event = data >> UPOE_DATA_SHIFT ;

    if (switzer_dash_i2c_slave_read(slave, POE_REG_DETECT_EVENT, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }
    *detect_event = data & UPOE_DATA_MASK;
    *cls_event = data >> UPOE_DATA_SHIFT;

    if (switzer_dash_i2c_slave_read(slave, POE_REG_TSTART_ILIM_EVENT, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }
    *inrush_event = data & UPOE_DATA_MASK;
    *ilim_event = data >> UPOE_DATA_SHIFT;

    if (switzer_dash_i2c_slave_read(slave, POE_REG_SUPPLY_EVENT, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }
    *supply_event = data;

    return 0;
}

int switzer_upoe_get_poweron_fault(struct switzer_manhattan *mod, uint8_t port_num, power_on_fault_t *power_onfault)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if (switzer_dash_i2c_slave_read(slave, POE_REG_PWR_ON_FAULT, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
        return 1;
    }
    *power_onfault = (power_on_fault_t)data;

    return 0;
}

int switzer_upoe_set_4P_power_allocation (struct switzer_manhattan *mod, uint8_t port_num, pwr_allocation_t power_allocation)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;

    if (switzer_dash_i2c_slave_read(slave, POE_REG_4P_PWR_ALLOCATION, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }

    if (port_mapping[port_num].phy_chn1 >= PSE_CHANNEL_3) {
        data &= 0x0F;
        data |= (power_allocation << UPOE_DATA_SHIFT);
        data |= (SET_FOUR_PAIR_MODE << UPOE_DATA_SHIFT);
    } else {
        data &= 0xF0;
        data |= power_allocation;
        data |= SET_FOUR_PAIR_MODE;
    }
    if (switzer_dash_i2c_slave_write(slave, POE_REG_4P_PWR_ALLOCATION, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to write register ", __FUNCTION__, __LINE__);
        return 1;
    }

    return 0;
}

int switzer_upoe_get_4P_power_allocation(struct switzer_manhattan *mod, uint8_t port_num, int *power_allocation)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if (switzer_dash_i2c_slave_read(slave, POE_REG_4P_PWR_ALLOCATION, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
        return 1;
    }
    if (port_mapping[port_num].phy_chn1 >= PSE_CHANNEL_3) {
        data = (data >> UPOE_DATA_SHIFT) & 0x07;
    } else {
        data &= 0x07;
    }
    *power_allocation = data;
    return 0;
}


int switzer_upoe_set_4P_policing (void)
{
    return 0;
}

/*T = -20 +N*Tstep, Tstep = 0.652*/
int switzer_upoe_get_temperature (struct switzer_manhattan *mod, uint8_t port_num, unsigned long *temp_value)
{
    struct switzer_dash_i2c_slave *slave;
    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    uint8_t  data;

    if (switzer_dash_i2c_slave_read(slave, POE_REG_TEMP, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
        return 1;
    }
    *temp_value = data ;
    return 0;
}

int switzer_upoe_get_pse_mfrid (struct switzer_manhattan *mod, uint8_t *mfr_id, uint8_t *ic_id)
{
    struct switzer_dash_i2c_slave *slave;
    slave = mod->poe[UPOE_I2C_0].i2c;
    uint8_t data;

    if (switzer_dash_i2c_slave_read(slave, POE_REG_ID, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
        return 1;
    }
    *mfr_id = data >> MFR_ID_SHIFT;
    *ic_id = data & IC_ID_MASK;
    return 0;
}

int switzer_upoe_get_firmware_version(struct switzer_manhattan *mod, uint8_t *firmwarerev)
{
    struct switzer_dash_i2c_slave *slave;
    slave = mod->poe[UPOE_I2C_0].i2c;
    uint8_t data = 0;

    if (switzer_dash_i2c_slave_read(slave, POE_REG_FIRMWARE_REV, &data, 1) < 0) {
        cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
        return 1;
    }
    *firmwarerev = data;
    return 0;
}

/*step voltage value is  of 3.662 mVolts*/
int switzer_upoe_get_input_voltage(struct switzer_manhattan *mod, uint8_t port_num, unsigned long *voltage)
{
    struct switzer_dash_i2c_slave *slave;
    uint16_t data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if (switzer_dash_i2c_slave_read(slave,POE_REG_INPUT_VOLTAGE, &data, 2) < 0) {
        cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
        return 1;
    }
    /* calculate voltage value in mv*/
    *voltage = data;
    return 0;
}

/*full current scale value is 1.46A, and each step value is 89.5uA*/
int switzer_upoe_get_port_volt_current_measurements(struct switzer_manhattan *mod, uint8_t port_num,
                                                                   unsigned long *voltage, unsigned long *current)
{
    struct switzer_dash_i2c_slave *slave;
    uint16_t  data, overall_current, overall_voltage;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if (port_mapping[port_num].phy_chn1 >= PSE_CHANNEL_3) {
        if(switzer_dash_i2c_slave_read(slave,POE_REG_CURRENT_CH3, &data, 2) < 0) {
            cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
            return 1;
        }
        overall_current = data;
        if (switzer_dash_i2c_slave_read(slave,POE_REG_CURRENT_CH4, &data, 2) < 0) {
            cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
            return 1;
        }
        overall_current += data;
        if (switzer_dash_i2c_slave_read(slave,POE_REG_VOLTAGE_CH3, &data, 2) < 0) {
            cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
            return 1;
        }
        overall_voltage = data;

    } else if (port_mapping[port_num].phy_chn1 <= PSE_CHANNEL_2) {
        if (switzer_dash_i2c_slave_read(slave,POE_REG_CURRENT_CH1, &data, 2) < 0) {
            cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
            return 1;
        }
        overall_current = data;
        if (switzer_dash_i2c_slave_read(slave,POE_REG_CURRENT_CH2, &data, 2) < 0) {
            cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
            return 1;
        }
        overall_current += data;

        if (switzer_dash_i2c_slave_read(slave,POE_REG_VOLTAGE_CH1, &data, 2) < 0) {
            cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
            return 1;
        }
        overall_voltage = data;
    }
    *current = overall_current;
    *voltage = overall_voltage;
    return 0;
}

int switzer_upoe_set_foldback_curve (struct switzer_manhattan *mod, uint8_t port_num, int set_foldback)
{
    return 0;
}

int switzer_upoe_get_foldback_curve (struct switzer_manhattan *mod, uint8_t port_num, int switzer_upoe_set_foldback)
{
    return 0;
}

/*step capacitance is 0.05uF*
 * calculate value: detectcapacitance *50)/1000 */
int switzer_upoe_get_port_detect_capacitance(struct switzer_manhattan *mod, uint8_t port_num, unsigned long *detectcapacitance)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if (port_mapping[port_num].phy_chn1 >= PSE_CHANNEL_3) {
        if (switzer_dash_i2c_slave_read(slave, POE_REG_DETECT_CAPACITANCE_CH3, &data, 1) < 0) {
            cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
            return 1;
        }
    } else if (port_mapping[port_num].phy_chn1 <= PSE_CHANNEL_2) {
        if (switzer_dash_i2c_slave_read(slave, POE_REG_DETECT_CAPACITANCE_CH2, &data, 1) < 0) {
            cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
            return 1;
        }
    }
    *detectcapacitance = data ;
    return 0;
}

/*step resistance is 195.3125Ohm*/
int switzer_upoe_get_port_detect_resistance(struct switzer_manhattan *mod, uint8_t port_num, unsigned long *detectresistance)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if (port_mapping[port_num].phy_chn1 >= PSE_CHANNEL_3) {
        if (switzer_dash_i2c_slave_read(slave, POE_REG_DETECT_RESISTANCE_CH3, &data, 1) < 0) {
            cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
            return 1;
        }
    } else if (port_mapping[port_num].phy_chn1 <= PSE_CHANNEL_2) {
        if (switzer_dash_i2c_slave_read(slave, POE_REG_DETECT_RESISTANCE_CH2, &data, 1) < 0) {
            cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
            return 1;
        }
    }
    *detectresistance = data;
    return 0;
}

int switzer_upoe_get_port_assigned_class(struct switzer_manhattan *mod, uint8_t port_num, int *preclass, int *assignclass)
{
    struct switzer_dash_i2c_slave *slave;
    uint8_t data;

    slave = mod->poe[port_mapping[port_num].i2c_num].i2c;
    if (port_mapping[port_num].phy_chn1 >= PSE_CHANNEL_3) {
        if (switzer_dash_i2c_slave_read(slave, POE_REG_ASSIGNED_CLASS_CH3, &data, 1) < 0) {
            cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
            return 1;
        }
    } else if (port_mapping[port_num].phy_chn1 <= PSE_CHANNEL_2) {
        if (switzer_dash_i2c_slave_read(slave, POE_REG_ASSIGNED_CLASS_CH2, &data, 1) < 0) {
            cterr('f',0, "%s at %d: Failed to read register ", __FUNCTION__, __LINE__);
            return 1;
        }
    }
    *preclass = data & UPOE_DATA_MASK;
    *assignclass = data >> UPOE_DATA_SHIFT;
    return 0;

}

/*SRAM and Parity Programming steps during power up
 * Refering to DOC TPS23881_2 SRAM CODE v05 Release Notes which
 * downloaded from TI website */

int switzer_load_sram_and_parity_code (struct switzer_manhattan *mod)
{
    struct switzer_dash_i2c_slave *slave;
    slave = mod->poe[UPOE_I2C_0].i2c;
    uint8_t data;
    int index;

    /*Step 1, reset the memory address pointer*/
    data = 0x01;
    if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_CTRL_REG, &data, 1) < 0)
        goto err;
    /*Step 2, set start address LSB*/
    data = 0x00;
    if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_PROGRAM_START_ADDR_LSB, &data, 1) < 0)
        goto err;
    /*Step 3, set start address MSB */
    data = 0x80;
    if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_PROGRAM_START_ADDR_MSB, &data, 1) < 0)
        goto err;
   /*Check values*/
    if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_CTRL_REG, &data, 1) < 0)
        goto err;

    prt("the value is 0x%x", data);
    /*Step 4, Reset CPU and enable Parity Write*/
    data = 0xc4;
    if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_CTRL_REG, &data, 1) < 0)
        goto err;

    /*Step 5, Load Parity data*/
    prt("loading parity code\n");
    for (index = 0; index < PARITY_LENGTH; index ++) {
        if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_DATA_REG, &tps23881_parity_code[index], 1) < 0)
           goto err;
    }
    /*Step 6, Keep CPU in reset and reset my memory pointer*/
    data = 0xc5;
    if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_CTRL_REG, &data, 1) < 0)
            goto err;
    /*Step 7, Re-set LSB of start address*/
    data = 0x00;
    if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_PROGRAM_START_ADDR_LSB, &data, 1) < 0)
        goto err;
    /*Step 8, Re-set MSB of start address*/
    data = 0x80;
    if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_PROGRAM_START_ADDR_MSB, &data, 1) < 0)
        goto err;
    /* Step 9, Keep CPU in reset and enable SRAM*/
    data = 0xc0;
    if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_CTRL_REG, &data, 1) < 0)
        goto err;
    /*Step 10, Load SRAM data*/
    prt("loading sram code\n");
    for (index = 0; index < SRAM_LENGTH; index ++) {
        if(index%256 == 0) {
            prt(".");
            fflush(stdout);
        }
        if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_DATA_REG, &tps23881_sram_code[index], 1) < 0)
            goto err;
    }
    prt("finish sram code\n");
    /*Step 11, Clears CPU reset and enables SRAM and Parity*/
    data = 0x18;
    if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_CTRL_REG, &data, 1) < 0)
        goto err;
    prt("return value\n");
    return 0;

err:
    cterr('f',0, "%s : Failed to write register ", __FUNCTION__);
    return 1;
}

int safe_mode_load_code (struct switzer_manhattan *mod)
{
    struct switzer_dash_i2c_slave *slave;
    slave = mod->poe[0].i2c;
    uint8_t data;
    int index;
    uint8_t firmwareversion;

    switzer_upoe_get_firmware_version(mod, &firmwareversion);

    /*Step 1, reset the memory address pointer*/
    data = 0x01;
    if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_CTRL_REG, &data, 1) < 0)
        goto err;
    /*Step 2, set start address LSB*/
    data = 0x00;
    if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_PROGRAM_START_ADDR_LSB, &data, 1) < 0)
        goto err;
    /*Step 3, set start address MSB */
    data = 0x80;
    if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_PROGRAM_START_ADDR_MSB, &data, 1) < 0)
        goto err;
//    if (data & 0x10) {
        /*Step 4, enable Parity Write*/
        data = 0x84;
        if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_CTRL_REG, &data, 1) < 0)
            goto err;
        /*Step 5, Load Parity data*/
        prt("loading parity data\n");
        for (index = 0; index < PARITY_LENGTH; index ++) {
            if(index%64)
            {
                prt(".");
            }
            if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_DATA_REG, &tps23881_parity_code[index], 1) < 0)
                goto err;
        }
        prt("Finish loading parity data\n");

        /* Step 6, Reset memory pointer*/
        prt("Reset memory pointer\n");
        data = 0x85;
        if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_CTRL_REG, &data, 1) < 0)
            goto err;
        /* Step 7, Re-set LSB of start address*/
        prt("Reset lsb of start address\n");
        data = 0x00;
        if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_PROGRAM_START_ADDR_LSB, &data, 1)<0)
            goto err;
        /* Step 8, Re-start MSB of start address*/
        prt("Reset msb of start address\n");
        data = 0x80;
        if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_PROGRAM_START_ADDR_MSB, &data, 1) < 0)
            goto err;
 //   }
    /* Step 9, Enable SRAM I2C write*/
        prt("Reset SRAM I2C Write\n");
        data = 0x80;
        if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_CTRL_REG, &data, 1) < 0)
            goto err;

    prt("loading sram data\n");
    /* Step 10, Load SRAM data*/
    for (index = 0; index < SRAM_LENGTH; index++) {
        if(index%256)
        {
            prt(".");
        }
        if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_DATA_REG, &tps23881_sram_code[index], 1) < 0)
            goto err;
    }
    prt("Finish loading sram data\n");
    /* Step 11, Enable SRAM & Parity*/
    data = 0x18;
    if (switzer_dash_i2c_slave_write(slave, POE_REG_SRAM_CTRL_REG, &data, 1) < 0)
        goto err;

    switzer_mdelay(20);
    switzer_upoe_get_firmware_version(mod, &firmwareversion);

    return 0;
err:
    cterr('f',0, "%s : Failed to write register ", __FUNCTION__);
    return 1;
}

int switzer_upoe_pse_init(struct switzer_manhattan *mod, operating_mode_t mode, pwr_allocation_t pwr)
{
    uint8_t firmwareversion;
    uint8_t port_num, value, intrmask, rc;

    prt("PSE Initalizing....\n");
    /*check firmware revision*/
    rc = switzer_upoe_get_firmware_version(mod, &firmwareversion);
    prt("Firmware revision : %x\n\r", firmwareversion);
    if (firmwareversion != SRAM_VERSION) {
        if (switzer_load_sram_and_parity_code(mod)) {
            cterr('f',0, "%s at %d: Failed to load firmware ", __FUNCTION__, __LINE__);
            return 1;
        }
    }
    switzer_mdelay(20);
    rc = switzer_upoe_get_firmware_version(mod, &firmwareversion);

    if (switzer_upoe_remap_channel(mod, 0xe1)) {
        return -1;
    }

   for (port_num = 0; port_num < UPOE_PORTS; port_num ++) {
       /*Configure device's interrupt*/
       intrmask = 0xf7;
       value = INTEN_MASK | I2C_CONFIGURATION_A_MASK;
       /*Configure device's interrupt*/
       rc = switzer_upoe_set_interrupt_mask(mod, port_num, intrmask);
       //rc = switzer_upoe_get_interrupt_mask(mod, port_num, &intrmask);
       //prt("the interrupt mask is 0x%x\n", intrmask);
       /*set general mask*/
       rc = switzer_upoe_set_general_mask(mod, port_num, value);
       //rc = switzer_upoe_get_general_mask(mod, port_num, &value);
       //prt(" after setting general mask the general mask is 0x%x\n", value);
       /*Set semi-auto mode for all channels */
       rc = switzer_upoe_set_operating_mode(mod, port_num, mode);
       //prt("after semi auto mode set general mask is 0x%x\n", value);
       //rc = switzer_upoe_get_general_mask(mod, port_num, &value);
       /*Enables all channels's DC disconnect*/
      // prt("after all channels dc disconnect general mask is 0x%x\n", value);
      // rc = switzer_upoe_get_general_mask(mod, port_num, &value);
       rc = switzer_upoe_set_port_dc_disconnect_enable(mod, port_num, SET_ENABLE_BIT);
       /*set 4pair 90W mode*/
       rc = switzer_upoe_set_4P_power_allocation (mod, port_num, pwr);
       //prt("after power allocation general mask is 0x%x\n", value);
       //rc = switzer_upoe_get_general_mask(mod, port_num, &value);
       /*power off all ports*/
       rc = switzer_upoe_set_port_power(mod, port_num, PD_PORT_OFF);
       //rc = switzer_upoe_get_general_mask(mod, port_num, &value);
       //prt("after power set the general mask is 0x%x\n", value);
       /*enable all channels' detection and classification*/
       rc = switzer_upoe_set_detection_classification_enable(mod, port_num);
    //   rc = switzer_upoe_get_general_mask(mod, port_num, &value);
      // prt(" after detection and classification enable the general mask is 0x%x\n", value);

   }
    return rc;
}


