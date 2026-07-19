/* $Id: epdm.c,v 1.2 2019/08/06 06:56:12 alpeng Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/common/src/overlord/neptune/curie_1RU/epdm.c,v $
 *-----------------------------------------------------------------------------
 * epdm.c - Leverage from BCM API
 * Quadra28_Stand_Alone_APis_v1_0/QUADRA28_1_0/bcm_quadra28_app/epdm.c
 *
 * Feb 2019, Leschen
 *
 * Copyright (c) 2016-2019 by Cisco Systems, Inc.
 * All rights reserved.
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bcm_pm_if_api.h"
#include "epdm_v1v2v0.h"

#define BCM_PM_IF_INVALID_PHY           -26
#define BCM_PM_IF_UNAVAIL               -27

char chip_id[plp_chip_count][MAX_CHIP_NAME_SIZE] = {
    "quadra28"
};

/*! \brief Static configurations
 *
 *  This API initializes the software database with static configurations
 *  specified by the user for the specified PHY ID. Static configurations
 *  are one time configurations. User needs to call this API before
 *  calling bcm_plp_init. But user can still skip calling this function,
 *  in which case the default configurations are applied to the device.
 *
 *  @param chip_name          String representing the chip family name
 *  @param phy_info           Structure for phy access information
 *  @param bcm_static_config  Static configuration structure
 *
 *  @return SUCCESS
 */
int bcm_plp_static_config_set(char* chip_name, bcm_plp_access_t phy_info, void* bcm_static_config)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_static_config_set((*(bcm_plp_quadra28_access_t*) (&phy_info)), bcm_static_config);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Static configurations
 *
 *  This API retrieves static configurations from software database
 *  for the PHY ID specified by the user.
 *  If no configurations specified by user through bcm_plp_static_config_set,
 *      this API returns default configurations.
 *
 *  @param chip_name          String representing the chip family name
 *  @param phy_info           Structure for phy access information
 *  @param bcm_static_config  Static configuration structure
 *
 *  @return SUCCESS
 */
int bcm_plp_static_config_get(char* chip_name, bcm_plp_access_t phy_info, void* bcm_static_config)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_static_config_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), bcm_static_config);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Initialize PHY
 *
 *  This API initializes the specified PHY-ID by creating software database and
 *  downloading firmware for the specified PHY-ID, This API needs to be called for each PHY-ID.
 *
 *  @param chip_name             String representing the chip family name
 *  @param phy_info              Structure for phy access information
 *  @param read                  User defined Function pointer for reading register.
 *  @param write                 User defined Function pointer for writing register.
 *  @param firmware_load_method  Represents Firmware download method to be followed
 *                               during initialization.
 *                               0 - Do not download  FW
 *                               1 - FW download through MDIO
 *                               2 - Load FW by a given function. This is for future use
 *                               3 - Load FW through MDIO and flash it on to EEPROM
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_init(char* chip_name, bcm_plp_access_t phy_info,
             int (*read)(void* user_acc, unsigned int core_addr, unsigned int reg_addr, unsigned int* val), 
             int (*write)(void* user_acc, unsigned int core_addr, unsigned int reg_addr, unsigned int val), 
             bcm_pm_firmware_load_method_t firmware_load_method)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_init((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                              read, write, firmware_load_method);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Cleanup the PHY
 *
 *  This API cleans up the allocated SW database and invalidates the given PHY-ID.
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_cleanup(char* chip_name, bcm_plp_access_t phy_info)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_cleanup((*(bcm_plp_quadra28_access_t*) (&phy_info)));
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Retrives the link status.
 *
 *  This API retrives link status of the specified PHY. If PHY supports PCS it will
 *  return PCS live link status, if not PMD link status will be returned.
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information
 *  @param link_status     [OUT] Retrives PMD/PCS link status
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_link_status_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int *link_status)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_link_status_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), link_status);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Mode Configuratio
 *
 *  This API configures speed, interface, reference clock and auxilary modes on the specified PHY-id.
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information
 *  @param speed           Represents datarate of a port(mentioned through lane_map) in PHY in Mbps
 *                         eg: speed = 40000 for 40G
 *  @param if_type         Represents electrical interface type of the PHY, interface are as follows,
 *                         0 - bcm_pm_InterfaceBypass, 1  - bcm_pm_InterfaceSR, 2 - bcm_pm_InterfaceSR4, 3 - bcm_pm_InterfaceKX, \n
 *                         4 - bcm_pm_InterfaceKX4, 5 - bcm_pm_InterfaceKR,  6 - bcm_pm_InterfaceKR2,  7 - bcm_pm_InterfaceKR4,  \n
 *                         8 - bcm_pm_InterfaceCX, 9 - bcm_pm_InterfaceCX2, 10 - bcm_pm_InterfaceCX4, 11 - bcm_pm_InterfaceCR, \n
 *                         12 - bcm_pm_InterfaceCR2, 13 - bcm_pm_InterfaceCR4, 14 - bcm_pm_InterfaceCR10, 15 - bcm_pm_InterfaceXFI,
 *                         16 - bcm_pm_InterfaceSFI, 17 - bcm_pm_InterfaceSFPDAC 18 - bcm_pm_InterfaceXGMII, 19 - bcm_pm_Interface1000X, \n
 *                         20 - bcm_pm_InterfaceSGMII, 21 - bcm_pm_InterfaceXAUI, 22 - bcm_pm_InterfaceRXAUI, 23 - bcm_pm_InterfaceX2,  24 - bcm_pm_InterfaceXLAUI, \n
 *                         25 - bcm_pm_InterfaceXLAUI2, 26 - bcm_pm_InterfaceCAUI, 27 - bcm_pm_interfaceQSGMII, 28 - bcm_pm_InterfaceLR4, 29 - bcm_pm_InterfaceLR \n
 *                         30 - bcm_pm_InterfaceLR2, 31 - bcm_pm_InterfaceER, 32 - bcm_pm_InterfaceER2, 33 - bcm_pm_InterfaceER4, 34 - bcm_pm_InterfaceSR2 \n
 *                         35 - bcm_pm_InterfaceSR10, 36 - bcm_pm_InterfaceCAUI4, 37 - bcm_pm_InterfaceVSR, 38 - bcm_pm_InterfaceLR10, 39 - bcm_pm_InterfaceKR10 \n
 *                         40 - bcm_pm_InterfaceCAUI4_C2C 41- bcm_pm_InterfaceCAUI4_C2M, 42 - bcm_pm_InterfaceZR, 43 - bcm_pm_InterfaceLRM, 44 - bcm_pm_InterfaceXLPPI \n
 *
 *  @param ref_clk         Represents reference clock of the PHY\n
 *                         0 - bcm_pm_RefClk156Mhz(156.25MHz) , 1 - bcm_pm_RefClk125Mhz \n
 *                         2 - bcm_pm_RefClk106Mhz , 3 - bcm_pm_RefClk161Mhz \n
 *                         4 - bcm_pm_RefClk174Mhz , 5 - bcm_pm_RefClk312Mhz \n
 *                         6 - bcm_pm_RefClk322Mhz , 7 - bcm_pm_RefClk349Mhz \n
 *                         8 - bcm_pm_RefClk644Mhz , 9 - bcm_pm_RefClk698Mhz \n
 *                         9 - bcm_pm_RefClk155Mhz , 10 - bcm_pm_RefClk156P6Mhz \n
 *                         11 - bcm_pm_RefClk157Mhz , 12 - bcm_pm_RefClk158Mhz \n
 *                         13 - bcm_pm_RefClk159Mhz , 14 - bcm_pm_RefClk168Mhz \n
 *                         15 - bcm_pm_RefClk172Mhz , 16 - bcm_pm_RefClk173Mhz \n
 *
 *  @param interface_mode  Represents mode of the PHY\n
 *                         0 - bcm_pm_Interface_mode_IEEE , 1 - bcm_pm_Interface_mode_HIGIG \n
 *                         2 - bcm_pm_Interface_mode_OTN \n
 *  @param device_aux_modes Structure that contains chip specific mode information such as pass-through or gear box\n
 *                          For details see phymod/chip/{chip name}/tier2/{chip name}.c file
 *
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_mode_config_set(char* chip_name, bcm_plp_access_t phy_info, int speed, int if_type,
                            int ref_clk, int interface_mode, void* device_aux_modes)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_mode_config_set((*(bcm_plp_quadra28_access_t*) (&phy_info)), speed, if_type,
                                     ref_clk, interface_mode, device_aux_modes);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Retrive the configured mode.
 *
 *  This API retrives speed, interface, refclock and auxilary mode of specified PHY-id.
 *
 *  @param chip_name        String representing the chip family name
 *  @param phy_info         Structure for phy access information\n
 *  @param speed            [OUT] Represents datarate of the PHY \n
 *  @param if_type          [OUT] Represents interface of the PHY, interface are as follows,\n
 *  @param ref_clk          [OUT] Represents reference clock of the PHY \n
 *  @param interface_mode   [OUT] Represents interface mode of the PHY\n
 *  @param device_aux_modes [OUT] Reserved
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_mode_config_get(char* chip_name, bcm_plp_access_t phy_info, int *speed, 
                            int *if_type, int *ref_clk, int *interface_mode,
                            void *device_aux_modes)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_mode_config_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), speed, if_type, ref_clk,
                                         interface_mode, device_aux_modes);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief SW version.
 *
 *  This API used to retrives SW version
 *
 *  @param chip_name       String representing the chip family name
 *  @param chip_ver        [OUT] Retrives chip version number
 *  @param api_ver         [OUT] Retrives API version number
 *  @param enahan_ver      [OUT] Retrives Enhancement version number
 *
 */
void bcm_plp_version_get(char* chip_name,
                         unsigned short *chip_ver,
                         unsigned short *api_ver, unsigned short *enahan_ver)
{
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        bcm_plp_quadra28_version_get(chip_ver, api_ver, enahan_ver);
    } else {
        *chip_ver = *api_ver = *enahan_ver = -1;
    }
}

/*! \brief PRBS configuration.
 *
 *  This API configures PRBS and enable/disable generator(TX) and checker(Rx) for PRBS operation.
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param tx_rx           Represents Transmit, receive or both\n
 *                         0 = Both transmit and receive PRBS \n 1 = Receive side PRBS\n 2 = Transmit side PRBS
 *  @param poly            Represents PRBS polynomial type \n
 *                         0 = PRBS polynomial 7 \n
 *                         1 = PRBS polynomial 9 \n
 *                         2 = PRBS polynomial 11\n
 *                         3 = PRBS polynomial 15\n
 *                         4 = PRBS polynomial 23\n
 *                         5 = PRBS polynomial 31\n
 *                         6 = PRBS polynomial 58
 *  @param invert          Represents PRBS inversion \n
 *                         1 = Set invert,\n 0 = reset invert
 *  @param loopback        Reserved for future use only
 *  @param ena_dis         Represents PRBS Enable/Disable\n
 *                         1 - Enable PRBS\n
 *                         0 - Disable PRBS
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_prbs_set(char* chip_name, bcm_plp_access_t phy_info, unsigned int tx_rx, 
                     unsigned int poly, unsigned int invert,
                     unsigned int loopback, unsigned int ena_dis)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_prbs_set((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                              tx_rx, poly, invert, loopback, ena_dis);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief PRBS configuration get.
 *
 *  This API retives the configured TX/RX PRBS.
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param tx_rx           Represents Transmit, receive or both\n
 *                         0 = Both transmit and receive PRBS \n 1 = Receive side PRBS\n 2 = Transmit side PRBS
 *  @param poly            [OUT] Represents PRBS polynomial type \n
 *                         0 = polynomial 7 \n
 *                         1 = polynomial 9 \n
 *                         2 = polynomial 11\n
 *                         3 = polynomial 15\n
 *                         4 = polynomial 23\n
 *                         5 = polynomial 31\n
 *                         6 = polynomial 58
 *  @param invert          [OUT] Represents PRBS inversion \n
 *                         1 = Invert got enable \n 0 = Invert got not enable
 *  @param loopback        Reserved for future use only
 *  @param ena_dis         [OUT] Represents PRBS Enable/Disable \n
 *                         1 - Enable PRBS \n
 *                         0 - Disable PRBS
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_prbs_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int tx_rx,
                     unsigned int *poly, unsigned int *invert,
                     unsigned int *loopback, unsigned int *ena_dis)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_prbs_get((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                               tx_rx, poly, invert, loopback, ena_dis);

    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Dump PRBS Status.
 *
 *  This API dump PRBS Checker(RX) Status.
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param time            Represents delay between the PRBS RX status read's
 *                         i.e. Read Rx STATUS;wait till 'time' expires; Re-read Rx status
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_prbs_rx_stat(char* chip_name, bcm_plp_access_t phy_info, unsigned int time)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_prbs_rx_stat((*(bcm_plp_quadra28_access_t*) (&phy_info)), time);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Clear PRBS
 *
 *  This API disable PRBS Generator(TX) / Checker(Rx) .
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param tx_rx           Represents Tx/Rx PRBS\n
 *                         1 = Receive side PRBS \n 2 = Transmit side PRBS\n
 *                         0 = Both Tranmit and Receive side
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_prbs_clear(char* chip_name, bcm_plp_access_t phy_info, unsigned int tx_rx)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_prbs_clear((*(bcm_plp_quadra28_access_t*) (&phy_info)), tx_rx);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Get PRBS configuration.
 *
 *  This API used to retrieve TX/RX PRBS configurations for the specified lane
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param tx_rx           Represents Tx/Rx PRBS\n
 *                         1 = Receive side PRBS \n 2 = Transmit side PRBS\n
 *                         0 = Both Tranmit and Receive side
 *  @param poly            [OUT] Represents PRBS polynomial type\n
 *                         0 = polynomial 7\n
 *                         1 = polynomial 9\n
 *                         2 = polynomial 11\n
 *                         3 = polynomial 15\n
 *                         4 = polynomial 23\n
 *                         5 = polynomial 31\n
 *                         6 = polynomial 58\n
 *  @param invert          [OUT] Represents PRBS inversion\n
 *                         1 = invert\n 0 = no inversion \n
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_prbs_config_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int tx_rx,
                            unsigned int *poly, unsigned int *invert)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_prbs_config_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), tx_rx, poly, invert);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Get PRBS Status.
 *
 *  This API retrives Rx checker status of the specified lane. It
 *  also provide lock lost and error count.
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param prbs_lock       [OUT] Represents whether PRBS is currently locked
 *  @param prbs_lock_loss  [OUT] Represents PRBS was unlocked since last call
 *  @param error_count     [OUT] Represents PRBS errors count
 *
 */
int bcm_plp_prbs_status_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int *prbs_lock,
                            unsigned int *prbs_lock_loss, unsigned int *error_count)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_prbs_status_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), prbs_lock, prbs_lock_loss, error_count);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}  

/*! \brief Register Write
 *
 *  This API is used to perform register write.
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param devaddr         device address\n 1 - PMA/PMD \n 7 - AN
 *  @param regaddr         Register address of the device
 *  @param data            Value to be written to Register
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_reg_value_set(char* chip_name, bcm_plp_access_t phy_info, unsigned int devaddr,
                          unsigned int regaddr, unsigned int data)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_reg_value_set((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                       devaddr, regaddr, data);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Register Read
 *
 *  This API is used to read the specified register address.
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param devaddr         device address\n 1 - PMA/PMD \n 3 - PCS \n 7 - AN
 *  @param regaddr         Register address to read
 *  @param data            [OUT] Value of the specified register
 *
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_reg_value_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int devaddr,
                          unsigned int regaddr, unsigned int *data)
{
    int rv;

        rv = bcm_plp_quadra28_reg_value_get((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                       devaddr, regaddr, data);
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_reg_value_get((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                       devaddr, regaddr, data);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Polarity set
 *
 *  This API is used to set the polarity of the specified lane
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param tx_pol          Tx Polarity\n
 *                             0 - no inversion\n
 *                             1 - invert polarity
 *  @param rx_pol          Rx Polarity\n
 *                             0 - no inversion\n
 *                             1 - invert polarity
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_polarity_set(char* chip_name, bcm_plp_access_t phy_info, unsigned int tx_pol,
                         unsigned int rx_pol)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_polarity_set((*(bcm_plp_quadra28_access_t*) (&phy_info)), tx_pol, rx_pol);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Polarity get
 *
 *  This API is used to get the polarity of a specified lane
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param tx_pol          [OUT] Tx polarity\n
 *                                 0 - no inversion on the specified lane\n
 *                                 1 - Represents lane polarity inverted
 *  @param rx_pol          [OUT] Rx Polarity \n
 *                                 0 - no inversion on the specified lane\n
 *                                 1 - lane polarity inverted
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_polarity_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int *tx_pol,
                         unsigned int *rx_pol)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_polarity_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), tx_pol, rx_pol);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Get RX PMD lock
 *
 *  This API is used to get RX PMD live lock status
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param rx_pmd_lock     [OUT]Status of Rx PMD, When lane map is multicast,
 *                         rx_pmd_lock consists 'AND' of all the lane status.
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_rx_pmd_lock_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int* rx_pmd_lock)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_rx_pmd_lock_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), rx_pmd_lock);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Chip revision ID
 *
 *  This API is used to retrieve revision id of the specified PHY
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param rev_id          [OUT] Chip Revision ID
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_rev_id(char* chip_name, bcm_plp_access_t phy_info, unsigned int* rev_id)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_rev_id((*(bcm_plp_quadra28_access_t*) (&phy_info)), rev_id);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Loopback set
 *
 *  This API is used to set the Remote or Digital loopback
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param lb_mode         Represents the mode of loopback\n
 *                         2 - Remote PMD loopback \n
 *                         4 - Digital PMD loopback
 *  @param enable          Represents Enable/Disable\n
 *                         0 - disable\n
 *                         1 - enable
 *
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_loopback_set(char* chip_name, bcm_plp_access_t phy_info, unsigned int lb_mode,
                         unsigned int enable)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_loopback_set((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                      lb_mode, enable);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Loopback get
 *
 *  This API is used to get the status of specified loopback whether or not enabled
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param lb_mode         Represents Loopback mode\n
 *                         2 - Remote PMD loopback \n
 *                         4 - Digital PMD loopback
 *  @param enable          [OUT] Represents whether specified loopback is enabled or not.\n
 *                         0 - Loopback disabled\n
 *                         1 - Loopback enabled
 *
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_loopback_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int lb_mode,
                         unsigned int *enable)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_loopback_get((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                      lb_mode, enable);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Set Tx analog values
 *
 *  This API is used to set Transmitter pre, main, post, post2 and post3 taps.
 *  It also set Tx current.
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param tx              Transmitter analog parameter\n
 *                          pre       Pretap value \n
 *                          main      Maintap value\n
 *                          post      Posttap value\n
 *                          post2     Post2tap value\n
 *                          post3     Post3tap value\n
 *                          amp       current value\n
 *
 *
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_tx_set(char* chip_name, bcm_plp_access_t phy_info, bcm_plp_tx_t* tx)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_tx_set((*(bcm_plp_quadra28_access_t*) (&phy_info)), (bcm_plp_quadra28_tx_t*) tx);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Get Tx analog values
 *
 *  This API is used to get Transmitter pre, main, post, post2 and post3 taps.
 *  It also get Tx current.
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param tx              [OUT]   Transmitter analog parameter
 *                          pre       Pretap value \n
 *                          main      Maintap value\n
 *                          post      Posttap value\n
 *                          post2     Post2tap value\n
 *                          post3     Post3tap value\n
 *                          amp       current value\n
 *
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_tx_get(char* chip_name, bcm_plp_access_t phy_info, bcm_plp_tx_t* tx)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_tx_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), (bcm_plp_quadra28_tx_t*) tx);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Set Rx analog values
 *
 *  This API is used to set value of the receiver analog paramters
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param rx              Receiver analog parameter\n
 *                         vga->enable               vga->value will be programmed only if enable is set
 *                         0 - disable; 1 - enable\n
 *                         vga->value                Value to be set to vga\n
 *                         num_of_dfe_taps           Number of DFE to program \n
 *                         dfe[i]->enable            This needs to be set for programming dfe[i]->value
 *                         0 - disable; 1 - enable\n
 *                         dfe[i]->value             value to set to DFE[i]\n
 *                         peaking_filter->enable    This has to be set for programming peaking_filter->value
 *                         0 - disable; 1 - enable\n
 *                         peaking_filter->value    value to be programmed to peaking_filter\n
 *                         low_freq_peaking_filter->enable This needs to be set for programming low_freq_peaking_filter->value
 *                         0 - disable; 1 - enable\n
 *                         low_freq_peaking_filter->value   Value to be set for low_freq_peaking_filter  \n
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_rx_set(char* chip_name, bcm_plp_access_t phy_info, bcm_plp_rx_t* rx)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_rx_set((*(bcm_plp_quadra28_access_t*) (&phy_info)), (bcm_plp_quadra28_rx_t*) rx);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Set Rx analog values
 *
 *  This API is used to set value of the receiver analog paramters
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Represents PHY access\n
 *  @param rx              Receiver analog parameter\n
 *                          vga->enable               vga->value will be programmed only if enable is set
 *                          0 - disable; 1 - enable\n
 *                          vga->value                Value to be set to vga\n
 *                          num_of_dfe_taps           Number of DFE to program \n
 *                          dfe[i]->enable            This needs to be set for programming dfe[i]->value
 *                          0 - disable; 1 - enable\n
 *                          dfe[i]->value             value to set to DFE[i]\n
 *                          peaking_filter->enable    This has to be set for programming peaking_filter->value
 *                          0 - disable; 1 - enable\n
 *                          peaking_filter->value    value to be programmed to peaking_filter\n
 *                          low_freq_peaking_filter->enable This needs to be set for programming low_freq_peaking_filter->value
 *                          0 - disable; 1 - enable\n
 *                          low_freq_peaking_filter->value   Value to be set for low_freq_peaking_filter  \n
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_rx_get(char* chip_name, bcm_plp_access_t phy_info, bcm_plp_rx_t* rx)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_rx_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), (bcm_plp_quadra28_rx_t*) rx);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Chip Reset
 *
 *  This API is used to do Hard/Soft reset on the specified PHY.
 *  Hard reset erases firmware along with the contents of PHY registers.
 *  Soft reset preserves firmware and erases the contents of PHY registers.
 *
 *  @param chip_name     String representing the chip family name
 *  @param phy_info      Structure for phy access information\n
 *  @param reset_mode    Reset modes\n
 *                        0 - Hard reset\n
 *                        1 - Soft reset
 *  @param reset_val     Reserved for future use
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_reset_set(char* chip_name, bcm_plp_access_t phy_info, unsigned int reset_mode,
                      unsigned int reset_val)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_reset_set((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                   reset_mode, reset_val);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief AFE Datapath Reset
 *
 *  This API is used to reset Tx/Rx Datapath. Perlane datapath also supported.
 *
 *  @param chip_name      String representing the chip family name
 *  @param phy_info       Structure for phy access information\n
 *  @param reset          TX/RX Reset direction,
 *                         0 - In\n
 *                         1 - Out\n
 *                         2 - In Out (toggle)
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_phy_lane_reset_set(char* chip_name, bcm_plp_access_t phy_info, bcm_plp_pm_phy_reset_t* reset)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_phy_lane_reset_set((*(bcm_plp_quadra28_access_t*) (&phy_info)), (bcm_plp_quadra28_pm_phy_reset_t*) reset);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Datapath Reset get
 *
 *  This API is used to get the reset that are programmmed using bcm_plp_phy_lane_reset_set
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param reset           [OUT] TX/RX Reset direction\n
 *                         0 - In\n
 *                         1 - Out\n
 *                         2 - In Out (toggle)
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_phy_lane_reset_get(char* chip_name, bcm_plp_access_t phy_info, bcm_plp_pm_phy_reset_t* reset)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_phy_lane_reset_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), (bcm_plp_quadra28_pm_phy_reset_t*) reset);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Lane control set
 *
 *  This API is used to perform Tx datapath reset/Traffic disable/Squelch for a specified lane and
 *  for a specified interface side. (system or line side)
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param tx_control      Represents Tx lane control structure \n
 *                         0 - Traffic disable  \n
 *                         1 - Traffic enable \n
 *                         2 - Tx Datapath reset\n
 *                         3 - Tx Squelch on\n
 *                         4 - Tx Squelch off\n
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_tx_lane_control_set(char* chip_name, bcm_plp_access_t phy_info, bcm_plp_pm_phy_tx_lane_control_t tx_control)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_tx_lane_control_set( (*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                                    tx_control);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Rx Lane control set
 *
 *  This API is used to perform Rx datapath reset /Traffic disable/Squelch for a specified lane and
 *  for a specified interface side. (system or line side)
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param rx_control      Represents Rx lane control struct\n
 *                         0 - Rx datapath reset \n
 *                         1 - Rx squelch on\n
 *                         2 - Rx squelch off
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_rx_lane_control_set(char* chip_name, bcm_plp_access_t phy_info, bcm_plp_pm_phy_rx_lane_control_t rx_control)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_rx_lane_control_set( (*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                                   rx_control);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Lane control get
 *
 *  This API is used to get the Tx datapath reset /Traffic disable/Squelch for a specified lane and
 *  for a specified interface side. (system or line side)
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
  * @param tx_control      [OUT]Represet Tx lane control structure\n
 *                         0 - Traffic disable\n
 *                         1 - Traffic enable\n
 *                         2 - Tx Datapath reset\n
 *                         3 - Tx Squelch on\n
 *                         4 - Tx Squelch off
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_tx_lane_control_get(char* chip_name, bcm_plp_access_t phy_info, bcm_plp_pm_phy_tx_lane_control_t *tx_control)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_tx_lane_control_get( (*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                                   tx_control);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Rx Lane control set
 *
 *  This API is used to perform Rx datapath reset /Traffic disable/Squelch for a specified lane and
 *  for a specified interface side. (system or line side)
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param rx_control      [OUT]Represents Rx lane control struct\n
 *                         0 - Rx datapath reset\n
 *                         1 - Rx squelch on\n
 *                         2 - Rx squelch off
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_rx_lane_control_get(char* chip_name, bcm_plp_access_t phy_info, bcm_plp_pm_phy_rx_lane_control_t *rx_control)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_rx_lane_control_get( (*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                                     rx_control);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Perform cross switch map
 *
 *  This API is used to perform cross switch mapping between the specified source and destination lane
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param tx_source_array Represents Tx lane of specified side to be mapped to Rx of the other side
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_lane_cross_switch_map_set(char* chip_name, bcm_plp_access_t phy_info, unsigned int* tx_source_array)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_lane_cross_switch_map_set((*(bcm_plp_quadra28_access_t*) (&phy_info)), tx_source_array);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Get cross switch map
 *
 *  This API is used to retrieve lane number to which the specified lane is mapped.
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param mapped_to       Represents Tx lane of specified side to be mapped to Rx of the other side
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_lane_cross_switch_map_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int *mapped_to)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_lane_cross_switch_map_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), mapped_to);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Force Tx training set
 *
 *  This API is used to set force Tx Training for a specified interface side.
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param enable          Represents Tx training enable or disable\n
 *                         0 - disabled \n
 *                         1 - enabled
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_force_tx_training_set(char* chip_name, bcm_plp_access_t phy_info, unsigned int enable)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_force_tx_training_set((*(bcm_plp_quadra28_access_t*) (&phy_info)), enable);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Force Tx training get
 *
 *  This API is used to get force Tx Training enable status for a specified interface side.
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param enable         [OUT] Represents Tx training enable status\n
 *                         0 - disabled \n
 *                         1 - enabled
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_force_tx_training_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int *enable)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_force_tx_training_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), enable);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Force Tx training status get
 *
 *  This API is used to get force Tx Training status for a specified interface side.
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param enabled         [OUT] Represents Tx training enabled status\n
 *                         0 - disabled \n
 *                         1 - enabled
 *  @param training_failure [OUT] Represents Tx training status\n
 *                          0 - no failure detected\n
 *                          1 - failure detected
 *  @param trained         [OUT] Represents Rx status\n
 *                         0 - receiver not trained\n
 *                         1 - receiver trained
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_force_tx_training_status_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int *enabled,
                                         unsigned int *training_failure, unsigned int *trained)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_force_tx_training_status_get((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                                  enabled, training_failure,  trained);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief CL73 Auto Negotiation Ablity set
 *
 *  This API is used to set CL73 Auto Negotiation ability
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param tech_ability    Represents tech ablity\n
 *                         1 = AN_CAP_1G_KX\n
 *                         2 = AN_CAP_10G_KX4\n
 *                         4 = AN_CAP_10G_KR\n
 *                         8 = AN_CAP_40G_KR4\n
 *                         0x10 = AN_CAP_40G_CR4\n
 *                         0x40 = AN_CAP_100G_CR4\n
 *                         0x80 = AN_CAP_100G_KR4
 *  @param fec_ability     Represents FEC ablity\n
 *                         0   = Hardware default \n
 *                         1   = FEC Ablity\n
 *                         2   = FEC requested
 *  @param pause_ability   Represents pause ablity\n
 *                         0     = Hardware default \n
 *                         0x40  = AN_CAPABILITIES_SYMM_PAUSE \n
 *                         0x80  = AN_CAPABILITIES_ASYM_PAUSE
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_cl73_ability_set(char* chip_name, bcm_plp_access_t phy_info, unsigned short tech_ability,
                             unsigned short fec_ability, unsigned short pause_ability)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_cl73_ability_set((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                      tech_ability, fec_ability, pause_ability);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief CL73 AN Ablity get
 *
 *  This API is used to get configured CL73 Auto Negotiation ability
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param tech_ability    [OUT] Represents tech ablity\n
 *                         4 = AN_CAP_10G_KR\n
 *                         8 = AN_CAP_40G_KR4\n
 *                         0x10 = AN_CAP_40G_CR4\n
 *                         0x40 = AN_CAP_100G_CR4\n
 *                         0x80 = AN_CAP_100G_KR4
 *  @param fec_ability     [OUT] Represents FEC ablity\n
 *                         0   = Hardware default \n
 *                         1   = FEC Ablity\n
 *                         2   = FEC requested
 *  @param pause_ability   [OUT] Represents pause ablity\n
 *                         0   = Hardware default \n
 *                         0x40  = AN_CAPABILITIES_SYMM_PAUSE \n
 *                         0x80  = AN_CAPABILITIES_ASYM_PAUSE
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_cl73_ability_get(char* chip_name, bcm_plp_access_t phy_info, unsigned short *tech_ability,
                             unsigned short *fec_ability, unsigned short *pause_ability)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_cl73_ability_get((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                      tech_ability, fec_ability, pause_ability);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}
/*! \brief CL73 Enable/Disable
 *
 *  This API is used to enable/Disable CL73
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param ena_dis         Represents enable/disable\n
 *                         0 = Disable\n     1= enable
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_cl73_set(char* chip_name, bcm_plp_access_t phy_info, unsigned short ena_dis)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_cl73_set((*(bcm_plp_quadra28_access_t*) (&phy_info)), ena_dis);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Get CL73 completion state
 *
 *  This API is used to get CL73 completion state
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param an              [OUT] Represents whether AN is enabled/disabled
 *  @param an_done         [OUT] Represents AN done state
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_cl73_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int *an,
                     unsigned int *an_done)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_cl73_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), an, an_done);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Eyescan
 *
 *  This API is used to display eyescan for a given lane
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 *
 *  ***************************************************************************\n
 *            Legend of Entries in display_lane_state  header\n
 *  ***************************************************************************\n
 *  \n
 *  LN               : lane index within IP core\n
 *  (CDRxN,UC_CFG)   : CDR type x OSR ratio, micro lane configuration variable\n
 *  SD               : signal detect\n
 *  LOCK             : pmd_rx_lock\n
 *  RXPPM            : Frequency offset of local reference clock with respect to RX data in ppm\n
 *  CLK90            : Delay of zero crossing slicer, m1, wrt to data in PI codes\n
 *  CLKP1            : Delay of diagnostic/lms slicer, p1, wrt to data in PI codes\n
 *  PF(M,L)          : Peaking Filter Main (0..15) and Low Frequency (0..7) settings\n
 *  VGA              : Variable Gain Amplifier settings (0..42)\n
 *  DCO              : DC offset DAC control value\n
 *  P1mV             : Vertical threshold voltage of p1 slicer\n
 *  DFE taps         : ISI correction taps in units of 2.35mV (for 1 & 2 even values are displayed, dcd = even-odd)\n"
 *  SLICER(ze,zo,pe,po,me,mo) : Slicer calibration control codes\n
 *  TXPPM            : Frequency offset of local reference clock with respect to TX data in ppm\n
 *  TXEQ(n1,m,p1,p2) : TX equalization FIR tap weights in units of 1Vpp/60 units\n
 *  EYE(L,R,U,D)     : Eye margin @ 1e-5 as seen by internal diagnostic slicer in mUI and mV\n
 *  LINK_TIME        : Link time in milliseconds\n
 *  \n
 *  ***************************************************************************\n
 */
int bcm_plp_display_eye_scan(char* chip_name, bcm_plp_access_t phy_info)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_display_eye_scan((*(bcm_plp_quadra28_access_t*) (&phy_info)));
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Firmware info
 *
 *    This API is used to get the firmware version and CRC.
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param fw_version      [OUT] Firmware version
 *  @param fw_crc          [OUT] Firmware checksum
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_firmware_info_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int *fw_version,
                              unsigned int *fw_crc)
{
    int rv;

    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_firmware_info_get((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                           fw_version, fw_crc);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}
/*! \brief Firmware set
 *
 *  This API is used to program firmware to non-volatile memory.
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param offset          Offset to the firmware binary data
 *  @param fw_data         Firmware binary data in char array
 *  @param fw_len          Lengh of the firmware binary data
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_firmware_set(char* chip_name, bcm_plp_access_t phy_info, const int offset,
                     const unsigned char *fw_data, const int fw_len)
{
    return BCM_PM_IF_UNAVAIL;
}
/*! \brief Set MDI pair swap
 *
 *  This API is used to set the MDI pair swap
 *
 *  @param chip_name      String representing the chip family name
 *  @param phy_info <pre> Represents PHY access\n
 *  @param laneswap_map   MDI pair swap map</pre>
 *
 *  @return BCM_PM_IF_SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_rxtx_laneswap_set(char* chip_name, bcm_plp_access_t phy_info, bcm_plp_laneswap_map_t* laneswap_map)
{
    return BCM_PM_IF_UNAVAIL;

}

/*! \brief Get MDI pair swap
 *
 *  This API is used to get the MDI pair swap map
 *
 *  @param chip_name      String representing the chip family name
 *  @param phy_info <pre> Represents PHY access\n
 *  @param laneswap_map   MDI pair swap map</pre>
 *
 *  @return BCM_PM_IF_SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_rxtx_laneswap_get(char* chip_name, bcm_plp_access_t phy_info, bcm_plp_laneswap_map_t* laneswap_map)
{
    return BCM_PM_IF_UNAVAIL;

}

/*! \brief PLL sequencer restart
 *
 *    This API is used to restart the PLL sequencer
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param flags           Reserved for future use
 *  @param operation       PLL operation to be performed\n
 *                         2 = bcmpmSeqOpRestart
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_pll_sequencer_restart(char* chip_name, bcm_plp_access_t phy_info, unsigned char flags,
                                  bcm_pm_sequencer_operation_t operation)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_pll_sequencer_restart((*(bcm_plp_quadra28_access_t*) (&phy_info)), flags, operation);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Enable/Disable FEC
 *
 *  This API is used to Enable/Disable FEC
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param enable          enable or disable \n
 *                         0 - disable\n
 *                         1 - enable
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_fec_enable_set(char* chip_name, bcm_plp_access_t phy_info, unsigned int enable)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_fec_enable_set((*(bcm_plp_quadra28_access_t*) (&phy_info)), enable);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief FEC enable get
 *
 *  This API is used to retrive FEC enable status
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param enable          [OUT] Represents enabled status \n
 *                         0 - Disable\n
 *                         1 - Enable
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_fec_enable_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int* enable)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_fec_enable_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), enable);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief PHY status dump
 *
 *  This API is used to dump status about PHY and its lanes
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_phy_status_dump(char* chip_name, bcm_plp_access_t phy_info)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_phy_status_dump((*(bcm_plp_quadra28_access_t*) (&phy_info)));
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief PHY DSC Diagnostics
 *
 *  This API is used to retrieve PHY DSC Diagnostics for a given lane
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param diag            [OUT]Attributes for Lane based diagnosis
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_phy_diagnostics_get(char* chip_name, bcm_plp_access_t phy_info, bcm_plp_pm_phy_diagnostics_t* diag)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_phy_diagnostics_get((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                         (bcm_plp_quadra28_pm_phy_diagnostics_t*) diag);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Interrupt status get
 *
 *  This API is used to get interrupt stauts
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param intr_type       Type of interrupt \n
 *  @param intr_status     [OUT] Interrupt status
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_intr_status_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int intr_type,
                            unsigned int* intr_status)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_intr_status_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), intr_type, intr_status);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Interrupt enable set
 *
 *  This API is used enable specified interrupt
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param intr_type       Type of interrupt, \n
 *  @param enable          Enable/Disable\n
 *                         1 - Enable\n
 *                         0 - Disable
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_intr_enable_set(char* chip_name, bcm_plp_access_t phy_info, unsigned int intr_type,
                            unsigned int enable)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_intr_enable_set((*(bcm_plp_quadra28_access_t*) (&phy_info)), intr_type, enable);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Interrupt enable get
 *
 *  This API is used get enabled interrupt
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param intr_type       Type of interrupt,
 *  @param enable          Get Enabled status\n
 *                         1 - Enable\n
 *                         0 - Disable
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_intr_enable_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int intr_type,
                            unsigned int* enable)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_intr_enable_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), intr_type, enable);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Interrupt status clear
 *
 *  This API is used clear the interrupt status
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param intr_type       Type of interrupt
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_intr_status_clear(char* chip_name, bcm_plp_access_t phy_info, unsigned int intr_type)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_intr_status_clear((*(bcm_plp_quadra28_access_t*) (&phy_info)), intr_type);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Fiber channel/PCS checker enable
 *
 *  This API is used to Enable/Diable fiber/PCS checker
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param fcpcs_chkr_mode checker mode\n
 *                         PCS49_1x10G   = 0x0, 10G-KR/LR/SR\n
 *                         PCS82_4x10G   = 0x1, 40G-KR4/LR4/SR4/CR4\n
 *                         PCS82_2x25G   = 0x2, 50G-KR2\n
 *                         PCS82_4x25G   = 0x3, 100G-KR4/LR4/CR4\n
 *                         FC4           = 0x4,\n
 *                         FC8           = 0x5,\n
 *                         FC16          = 0x6,\n
 *                         FC32          = 0x7,\n
 * @param enable           Represents enable/disable fc/pcs checker \n
 *                         1 - Enable\n
 *                         0 - Disable
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_fc_pcs_chkr_enable_set(char* chip_name, bcm_plp_access_t phy_info, unsigned int fcpcs_chkr_mode,
                                   unsigned int enable)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_fc_pcs_chkr_enable_set((*(bcm_plp_quadra28_access_t*) (&phy_info)), fcpcs_chkr_mode, enable);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief FC PCS chkr enable get
 *
 *   This API is used to get status whether FC PCS checker enabled
 *    or disabled
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param fcpcs_chkr_mode FC/PCS checker mode\n
 *                         PCS49_1x10G   = 0x0, 10G-KR/LR/SR\n
 *                         PCS82_4x10G   = 0x1, 40G-KR4/LR4/SR4/CR4\n
 *                         PCS82_2x25G   = 0x2, 50G-KR2\n
 *                         PCS82_4x25G   = 0x3, 100G-KR4/LR4/CR4\n
 *                         FC4           = 0x4,\n
 *                         FC8           = 0x5,\n
 *                         FC16          = 0x6,\n
 *                         FC32          = 0x7,
 *  @param enable          [OUT] Retrives enabled status\n
 *                         1 - Enable\n
 *                         0 - Disable
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_fc_pcs_chkr_enable_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int fcpcs_chkr_mode,
                                   unsigned int* enable)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_fc_pcs_chkr_enable_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), fcpcs_chkr_mode, enable);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief FC PCS chkr status get
 *
 *    This API is used to get the status of PCS checker
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param lock_status     [OUT] Represents lock status\n
 *                         1 - Locked\n
 *                         0 - Unlocked\n
 *  @param lock_lost_lh    [OUT] Represents loss of lock status\n
 *                         1 - Loss of lock set\n
 *                         0 - Loss of lock not set\n
 *  @param error_count     [OUT] Represents error count
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_fc_pcs_chkr_status_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int *lock_status,
                                   unsigned int* lock_lost_lh, unsigned int* error_count)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_fc_pcs_chkr_status_get((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                            lock_status, lock_lost_lh, error_count);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

#ifdef SERDES_API_FLOATING_POINT

/*! \brief Eye margin Projection
 *
 *  This API is used to display eye margin for a given lane
 *
 *  @param chip_name         String representing the chip family name
 *  @param phy_info          Structure for phy access information\n
 *  @param rate              Line rate in Hz used to calculate BER
 *  @param ber_scan_mode     Controls the direction and polarity of the test
 *  @param timer_control     Total measurement time in units of ~1.3 seconds
 *  @param max_error_control The error threshold it uses to step to next measurement in units of 16
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_eye_margin_proj(char* chip_name, bcm_plp_access_t phy_info, double rate,
                            unsigned char ber_scan_mode, unsigned char timer_control,
                            unsigned char max_error_control)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_eye_margin_proj((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                            rate, ber_scan_mode,  timer_control, max_error_control);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}
#else 
/*! \brief Eye margin Projection
 *
 *  This API is used to display eye margin for a given lane
 *
 *  @param chip_name         String representing the chip family name
 *  @param phy_info          Structure for phy access information\n
 *  @param rate              Line rate in Hz used to calculate BER
 *  @param ber_scan_mode     Controls the direction and polarity of the test
 *  @param timer_control     Total measurement time in units of ~1.3 seconds
 *  @param max_error_control The error threshold it uses to step to next measurement in units of 16
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_eye_margin_proj(char* chip_name, bcm_plp_access_t phy_info, int rate,
                            unsigned char ber_scan_mode, unsigned char timer_control,
                            unsigned char max_error_control)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_eye_margin_proj((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                            rate, ber_scan_mode,  timer_control, max_error_control);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}
#endif


/*! \brief Repeater mode
 *
 *  This API is used to enable/Disable Repeater/Retimer
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param ena_dis         Represents enable/disable\n
 *                          0 - retimer mode
 *                          1 - repeater mode
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_repeater_mode_set(char* chip_name, bcm_plp_access_t phy_info, unsigned int ena_dis)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_repeater_mode_set((*(bcm_plp_quadra28_access_t*) (&phy_info)), ena_dis);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Module Read
 *    This API is used read the data with I2C command on selected module
 *    on specified PHY-id.
 *
 *  @param chip_name         String representing the chip family name
 *    @param phy_info        Structure for phy access information\n
 *    @param slv_dev_addr    Module slave address (for supported Module types)
 *    @param start_addr      Start address of i2c Slave to be accessed
 *    @param no_of_bytes     No of bytes to be read.
 *    @param read_data       [OUT] Contains array of bytes read from the module
 *    @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_module_read(char* chip_name, bcm_plp_access_t phy_info, unsigned int slv_dev_addr,
                        unsigned int start_addr, unsigned int no_of_bytes,
                        unsigned char *read_data)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_module_read((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                 slv_dev_addr, start_addr, no_of_bytes, read_data);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Module Write
 *   This API is used write the data with I2C command on selected module
 *   on specified PHY-id.
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param slv_dev_addr    Module slave address (for supported Module types)
 *  @param start_addr      Start address of i2c Slave to be accessed
 *  @param no_of_bytes     No of bytes to be read.
 *  @param write_data      Contains array of bytes to be written to the module
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_module_write(char* chip_name, bcm_plp_access_t phy_info, unsigned int slv_dev_addr,
                         unsigned int start_addr, unsigned int no_of_bytes,
                         unsigned char *write_data)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_module_write((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                 slv_dev_addr, start_addr, no_of_bytes, write_data);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Repeater mode
 *
 *  This API is used to get enable/Disable status Repeater/Retimer
 *
 *  @param chip_name         String representing the chip family name
 *  @param phy_info          Structure for phy access information\n
 *  @param ena_dis           Represents enable/disable\n
 *                            0 - retimer mode
 *                            1 - repeater mode
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_repeater_mode_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int *ena_dis)
{
    return BCM_PM_IF_UNAVAIL;
}

/*! \brief Config set for CFP2/CFP4 modules
 *  Config set the pins of module card for CFP2/CFP4
 *  This API is used to set the configuration of the module controller IO of line card
 *  This function will configure IO in In/Out & Its Pu/PD mode based on Module need
 *  And it configures value of the pin for Outputs
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param mdcrtl_pins     Module control pins parameters
 *                         To configure tx_dis value:
 *                            tx_dis->enable = 1; tx_dis->value = <1/0>;
 *                         To configure rx_los value:
 *                            rx_los->enable = 1;
 *                         To configure mod_lopwr value:
 *                            mod_lopwr->enable = 1; mod_lopwr->value = <1/0>;
 *                         To configure mod_abs value:
 *                            mod_abs->enable = 1;
 *                         To configure glb_alrmn value:
 *                            glb_alrmn->enable = 1;
 *                         To configure mod_rstn value:
 *                            mod_rstn->enable = 1; mod_rstn->value = <1/0>
 *                         We can configure one or more parameters by calling
 *                         enable = 1 with particular parameter value <0/1>.
 *                         Pin value is ignored for the input pins
 *
 *                         If a Module pin is not supported in a Package or Chip
 *                         and user try to enable func throws error message
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_modctrl_cfg_cfp_linecard_set(char* chip_name, bcm_plp_access_t phy_info, 
                                         bcm_plp_modctrl_cfp_io_pins_t *mdcrtl_pins)
{
    return BCM_PM_IF_UNAVAIL;
}

/*! \brief Config set for QSFP28/QSFP+ modules
 *  Config set the pins of module card for QSFP28/QSFP+
 *  This API is used to set the configuration of the module controller of line card
 *  This function will configure IO in In/Out & Its Pu/PD mode based on Module need
 *  And it configures value of the pin for Outputs
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param mdcrtl_pins     Module control pins parameters
 *                         To configure lpmod value:
 *                              lpmod->enable = 1; lpmod->value = <1/0>;
 *                         To configure resetl value:
 *                               resetl->enable = 1; resetl->value = <1/0>;
 *                         To configure intl value:
 *                               intl->enable = 1; intl->value = <1/0>;
 *                         To configure mod_sell value:
 *                               mod_sell->enable = 1; mod_sell->value = <1/0>;
 *                         To configure mod_prsl value:
 *                                mod_prsl->enable = 1; mod_prsl->value = <1/0>;
 *                         We can configure one or more parameters by calling
 *                         enable = 1 with particular parameter value<0/1>.
 *                         Pin value is ignored for the input pins
 *
 *                         If a Module pin is not supported in a Package or Chip
 *                         and user try to enable func throws error message
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_modctrl_cfg_qsfp_linecard_set(char* chip_name, bcm_plp_access_t phy_info,
                                         bcm_plp_modctrl_qsfp_io_pins_t  *mdcrtl_pins)
{
    return BCM_PM_IF_UNAVAIL;
}

/*! \brief Config get for QSFP28/QSFP+ modules
 *  Config get the pins of module card for QSFP28/QSFP+
 *  This API is used to get the configuration of the module controller of line card
 *  This API gets the value of the Pin
 *  Reads from either IN or OUT data register of a GPIO based on Module pin type
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param mdcrtl_pins     [OUT] Module control pin status
 *                         Returns pin values for all the configured pins
 *                         (lpmod, resetl, intl, mod_sell, mod_prsl)
 *                         pin enable parameter is ignored.
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_modctrl_cfg_qsfp_linecard_get(char* chip_name, bcm_plp_access_t phy_info,
                                         bcm_plp_modctrl_qsfp_io_pins_t *mdcrtl_pins)
{
    return BCM_PM_IF_UNAVAIL;
}

/*! \brief Config get for CFP2/CFP4 modules
 *  Config get the pins of module card for CFP2/CFP4
 *  This API is used to get the configuration of the module controller of line card
 *  This API gets the value of the Pin
 *  Reads from either IN or OUT data register of a GPIO based on Module pin type
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param mdcrtl_pins     [OUT] Module control pin status
 *                         Returns pin values for all the configured pins
 *                         (tx_dis, rx_los, mod_lopwr, mod_abs, glb_alrmn, mod_rstn),
 *                         pin enable parameter is ignored.
 *
 *   @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_modctrl_cfg_cfp_linecard_get(char* chip_name, bcm_plp_access_t phy_info,
                                         bcm_plp_modctrl_cfp_io_pins_t *mdcrtl_pins)
{
    return BCM_PM_IF_UNAVAIL;
}
/*! \brief SHORT channel mode
 *
 *  This API is used to enable/Disable SHORT channel mode
 *
 *  @param chip_name     String representing the chip family name
 *  @param phy_info      Structure for phy access information\n
 *  @param ena_dis       Represents enable/disable\n
 *                        0 - To disable SHORT channel mode
 *                        1 - To enable SHORT channel mode
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_short_channel_mode_set(char* chip_name, bcm_plp_access_t phy_info, unsigned int ena_dis)
{
    return BCM_PM_IF_UNAVAIL;
}

/*! \brief SHORT channel mode
 *
 *  This API is used to get the status of SHORT channel mode
 *
 *  @param chip_name      String representing the chip family name
 *  @param phy_info       Structure for phy access information\n
 *  @param ena_dis        Represents enable/disable\n
 *                         0 - disable SHORT channel mode
 *                         1 - enable SHORT channel mode
 *  @param status        Represents short channel status of enable/disable\n
 *                         0 - disable SHORT channel mode
 *                         1 - enable SHORT channel mode
 *
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_short_channel_mode_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int *ena_dis, unsigned int *status) 
{
    return BCM_PM_IF_UNAVAIL;

}

/*! \brief Config set for GPIO pins
 *  Config set the GPIO pins
 *  This API is used to set the GPIO pins pull up or pull down
 *  This is generic API provided to user to configure GPIOs based on user need
 *  User has to just Pass Register address and direction & Pull function
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param gpio_pin_number GPIO pin number (0-4)
 *  @param cfg_direction   Configuration direction
 *                         1 - in
 *                         0 - out
 *  @param cfg_pull        Configure the pull up or pull down
 *                         1 -  pull up
 *                         0 -  pull down
 *  @param pin_value       Configure the pin value
 *                         1 -  High driven on Pin in o/p mode
 *                         0 -  Low driven on Pin in o/p mode
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_cfg_gpio_pin_set(char* chip_name, bcm_plp_access_t phy_info, unsigned int gpio_pin_number,
                             unsigned int cfg_direction, unsigned int cfg_pull,
                             unsigned int pin_value)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_cfg_gpio_pin_set((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                      gpio_pin_number, cfg_direction, cfg_pull, pin_value);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Config get for GPIO pins
 *  Config get the GPIO pins
 *  This API is used to set the GPIO pins pull up or pull down
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param gpio_pin_number GPIO pin number (0-4)
 *  @param cfg_direction   Configuration direction
 *                          1 - in
 *                          0 - out
 *  @param cfg_pull         [OUT] Configure the pull up or pull down
 *                           0 -  pull down
 *                           1 -  pull up
 *                           2 -  no pull up / no pull down
 *  @param pin_value       pin value
 *                           0 - Logic low
 *                           1 - Logic high
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_cfg_gpio_pin_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int gpio_pin_number,
                             unsigned int *cfg_direction, unsigned int *cfg_pull,
                             unsigned int *pin_value)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_cfg_gpio_pin_get((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                      gpio_pin_number, cfg_direction, cfg_pull, pin_value);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Power Set
 *
 *  This API is used to set the power of a transmitter or receiver of the specified lane.
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param power_rx        Represents Rx power  \n
 *                         0 - PowerOff \n
 *                         1 - PowerOn \n
 *                         2 - PowerOffOn\n
 *                         3 - PowerNoChange,
 *  @param power_tx        Represents Tx power \n
 *                         0 - PowerOff \n
 *                         1 - PowerOn \n
 *                         2 - PowerOffOn\n
 *                         3 - PowerNoChange,
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_power_set(char* chip_name, bcm_plp_access_t phy_info,  unsigned int power_rx, unsigned int power_tx)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_power_set((*(bcm_plp_quadra28_access_t*) (&phy_info)), power_rx, power_tx);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Power get
 *
 *  This API is used to get the power status of a specified lane from
 *  specified interface side
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param power_rx        [OUT] Receiver power status\n
 *                         0 - PowerOff \n
 *                         1 - PowerOn
 *  @param power_tx        [OUT] Transmitter power status\n
 *                         0 - PowerOff \n
 *                         1 - PowerOn
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_power_get(char* chip_name, bcm_plp_access_t phy_info,  unsigned int *power_rx, unsigned int *power_tx)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_power_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), power_rx, power_tx);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Lane config set
 *
 *  This API is used to set lane config parameter such as DFE settings.
 *
 *  @param chip_name    String representing the chip family name
 *  @param phy_info     Structure for phy access information\n
 *  @param firmware_lane_config  Represents lane config parameters\n
 *                        firmware_mode\n
 *                        0 - default mode\n
 *                        1 - dfe mode\n
 *                        2 - osdfe mode\n
 *                        3 - baud rate dfe mode\n
 *                        4 - low power dfe mode\n
 *                        5 - media type sfp dac\n
 *                        6 - media type xlaui\n
 *                        7 - media type optical sr4\n
 *
 *                        ena_dis\n
 *                        0 - disable\n
 *                        1 - enable\n
 *
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_firmware_lane_config_set(char* chip_name, bcm_plp_access_t phy_info, bcm_plp_pm_firmware_lane_config_t* firmware_lane_config)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_firmware_lane_config_set((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                              (bcm_plp_quadra28_pm_firmware_lane_config_t*) firmware_lane_config);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Firmware lane config get
 *
 *  This API is used to get the lane config settings for the specified lane
 *
 *  @param chip_name       String representing the chip family name
 *  @param phy_info        Structure for phy access information\n
 *  @param firmware_lane_config     Represents lane config parameters\n
 *                          firmware_mode\n
 *                          0 - default mode\n
 *                          1 - dfe mode\n
 *                          2 - osdfe mode\n
 *                          3 - baud rate dfe mode\n
 *                          4 - low power dfe mode\n
 *                          5 - media type sfp dac\n
 *                          6 - media type xlaui\n
 *                          7 - media type optical sr4\n
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_firmware_lane_config_get(char* chip_name, bcm_plp_access_t phy_info, bcm_plp_pm_firmware_lane_config_t* firmware_lane_config)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_firmware_lane_config_get((*(bcm_plp_quadra28_access_t*) (&phy_info)),
                                              (bcm_plp_quadra28_pm_firmware_lane_config_t*) firmware_lane_config);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Initialize PHY.
 *
 *  This API initialize the specified PHY-ID by creating software database and
 *  downloading firmware for the specified PHY-ID, This API needs to be called for each PHY-ID.
 *
 *  @param chip_name           String representing the chip family name
 *  @param phy_info            Structure for phy access information\n
 *  @param read                User defined Function pointer for reading register.
 *  @param write               User defined Function pointer for writing register.
 *  @param firmware_load_type  Represents Firmware download method and force download option to be followed
 *                              during initialization. \n
 *                              firmware_load_type.firmware_load_method
 *                              0 - bcmpmFirmwareLoadMethodNone
 *                              1 - bcmpmFirmwareLoadMethodInternal
 *                              2 -bcmpmFirmwareLoadMethodExternal
 *                              3 - bcmpmFirmwareLoadMethodProgEEPROM   \n
 *                              firmware_load_type.force_load_method
 *                              0 - bcmpmFirmwareLoadSkip
 *                              1 - bcmpmFirmwareLoadForce
 *                              2 - bcmpmFirmwareLoadAuto \n
 *  @param broadcast_method   Represents firmware broadcast sequence to be followed in subsequent call to this function \n
 *                              0 - bcmpmFirmwareBroadcastCoreReset    Reset the core for all phy id in mdio bus \n
 *                              1 - bcmpmFirmwareBroadcastEnable         Enable the broadcast for all phy id in mdio bus \n
 *                              2 - bcmpmFirmwareBroadcastFirmwareExecute  Load the FW for only one phy_id, internally it will broadcast firmware
 *                                                                                               of similar type of phys on same mdio bus \n
 *                              3 - bcmpmFirmwareBroadcastFirmwareVerify   FW load verify for all phy id in mdio bus \n
 *                              4 - bcmpmFirmeareBroadcastEnd                  Disable the broadcast for all phy id in mdio bus \n
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_init_fw_bcast(char* chip_name, bcm_plp_access_t phy_info,
                  int (*read)(void* user_acc, unsigned int core_addr, unsigned int reg_addr, unsigned int* val),
                  int (*write)(void* user_acc, unsigned int core_addr, unsigned int reg_addr, unsigned int val),
                  bcm_plp_firmware_load_type_t *firmware_load_type,
                  bcm_plp_firmware_broadcast_method_t broadcast_method)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_init_fw_bcast((*(bcm_plp_quadra28_access_t*) (&phy_info)), read, write,
                                   (bcm_plp_quadra28_firmware_load_type_t*) firmware_load_type, broadcast_method);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Failover mode set
 *
 *    This API is to set failover mode
 *
 *  @param chip_name      String representing the chip family name
 *  @param phy_info       Structure for phy access information\n
 *  @param failover_mode  Represents failovermode to set \n
 *                         0 - bcmFailovermodeNone\n
 *                         1 - bcmFailovermodeEnable\n
 *
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_failover_mode_set(char* chip_name, bcm_plp_access_t phy_info, unsigned int failover_mode)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_failover_mode_set((*(bcm_plp_quadra28_access_t*) (&phy_info)), failover_mode);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief Failover mode get
 *
 *    This API is to get failover mode
 *
 *  @param chip_name      String representing the chip family name
 *  @param phy_info       Structure for phy access information\n
 *  @param failover_mode  [OUT] Represents failover mode \n
 *                         0 - bcmFailovermodeNone\n
 *                         1 - bcmFailovermodeEnable\n

 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_failover_mode_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int *failover_mode)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_failover_mode_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), failover_mode);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}


/*! \brief EDC receiver select
 *
 *    This API is to select EDC receiver
 *
 *  @param chip_name      String representing the chip family name
 *  @param phy_info       Structure for phy access information\n
 *  @param edc_method     EDC receiver configuration method \n
 *                         0 - bcmEdcConfigMethodNone \n
 *                         1 - bcmEdcConfigMethodHardware  EDC mode is set automatically by hardware  \n
 *                         2-  bcmEdcConfigMethodSoftware EDC mode is selected by driver software \n
 *  @param edc_value      Device-specific EDC mode value (valid only when software configuration method is used)
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_edc_config_set(char* chip_name, bcm_plp_access_t phy_info, unsigned int edc_method, unsigned int edc_value)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_edc_config_set((*(bcm_plp_quadra28_access_t*) (&phy_info)), edc_method, edc_value);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

/*! \brief EDC receiver get configuration
 *
 *    This API is to get EDC receiver configuration
 *
 *  @param chip_name      String representing the chip family name
 *  @param phy_info       Structure for phy access information\n
 *  @param edc_method     [OUT] EDC receiver configuration method \n
 *                         0 - bcmEdcConfigMethodNone \n
 *                         1 - bcmEdcConfigMethodHardware  EDC mode is set automatically by hardware  \n
 *                         2-  bcmEdcConfigMethodSoftware EDC mode is selected by driver software \n
 *  @param edc_value     [OUT] Device-specific EDC mode value (valid only when software configuration method is used)
 *  @return SUCCESS(0) for success and corresponding error code on failure
 */
int bcm_plp_edc_config_get(char* chip_name, bcm_plp_access_t phy_info, unsigned int *edc_method, unsigned int *edc_value)
{
    int rv;
    if ( CHIP_IS(chip_name, plp_quadra28) ) {
        rv = bcm_plp_quadra28_edc_config_get((*(bcm_plp_quadra28_access_t*) (&phy_info)), edc_method, edc_value);
    } else {
        rv = BCM_PM_IF_INVALID_PHY;
    }

    return rv;
}

