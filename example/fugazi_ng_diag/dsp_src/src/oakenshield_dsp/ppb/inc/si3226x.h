/* $Id: si3226x.h,v 1.2 2017/07/28 07:58:38 harrchan Exp $
 * $Source: /auto/diag/ngd-linux-rep/cvs/ng_diag/dsp_src/src/oakenshield_dsp/ppb/inc/si3226x.h,v $
 *------------------------------------------------------------------
 * si3226x.h 
 *
 * Oct 2015, Owen Lin for SI32261
 *
 * Copyright (c) 2016-2017 by cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */

/*
** Copyright (c) 2007 by Silicon Laboratories
**
** $Id: si3226x.h,v 1.2 2017/07/28 07:58:38 harrchan Exp $
**
** Author(s): 
** laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
*/

#ifndef SI3226XH_H
#define SI3226XH_H

typedef unsigned char       uInt8   _ADAPTABLE_TYPE_;
typedef unsigned int        uInt32  _ADAPTABLE_TYPE_;   /* 32-bit */


/*
** SI3226X DataTypes/Function Definitions 
*/

#define NUMIRQ 4


#define ramData uInt32

#define SI3226x_PRAM_ADDR (334 + 0x400)
#define SI3226x_PRAM_DATA (335 + 0x400)
#define JMP_TABLE_LOW 82
#define JMP_TABLE_HIGH 1597

/**
* These are the error codes for ProSLIC failures
*/
typedef enum {
    RC_IGNORE = 0,         
    RC_NONE= 0,           /**< Means the function did not encounter an error */
	RC_TEST_PASSED = 0,
	RC_TEST_FAILED = 1,
    RC_COMPLETE_NO_ERR = 1,   /**< A test completed, no error detected */
    RC_POWER_ALARM_Q1,
    RC_POWER_ALARM_Q2,
    RC_POWER_ALARM_Q3,
    RC_POWER_ALARM_Q4,
    RC_POWER_ALARM_Q5,
    RC_POWER_ALARM_Q6,
    RC_SPI_FAIL,         /**< SPI Communications failure */
    RC_POWER_LEAK,
    RC_VBAT_UP_TIMEOUT,
    RC_VBAT_OUT_OF_RANGE,
    RC_VBAT_DOWN_TIMEOUT,
    RC_TG_RG_SHORT,
    RC_CM_CAL_ERR,
    RC_RING_FAIL_INT,
    RC_CAL_TIMEOUT,
    RC_PATCH_ERR,
    RC_BROADCAST_FAIL,           /**< Broadcast unavailable for requested operation */
    RC_UNSUPPORTED_FEATURE,      /**< Feature is not supported by the chipset*/
    RC_CHANNEL_TYPE_ERR,         /**< Channel type does not support called function */
    RC_GAIN_DELTA_TOO_LARGE,     /**< Requested gain delta too large */
    RC_GAIN_OUT_OF_RANGE,        /**< Gain requested exceeds range */
    RC_POWER_ALARM_HVIC,         /**< Power alarm on HVIC */
    RC_POWER_ALARM_OFFLD,        /**< Power alarm on offload transistor */
    RC_THERMAL_ALARM_HVIC,       /**< Thermal alarm detected */
    RC_NO_MEM,                   /**< Out of memory */
    RC_INVALID_GEN_PARAM,        /**< Invalid general parameter */
    RC_LINE_IN_USE,              /**< Line is in use (LCS detected) */
    RC_RING_V_LIMITED,           /**< Ringer voltage limited - signal may be clipped */
    RC_PSTN_CHECK_SINGLE_FAIL,   /**< PSTN detect single current exceeds limit */
    RC_PSTN_CHECK_AVG_FAIL,      /**< PSTN detect average current exceeds limit */
    RC_VDAA_ILOOP_OVLD,          /**< Overload detected */
    RC_UNSUPPORTED_OPTION,       /**< Function parameter is not supported at this time */
    RC_FDT_TIMEOUT,              /**< Timeout waiting for valid frame detect */
    RC_PSTN_OPEN_FEMF,           /**< Detected FEMF, device left in open state */
    RC_VDAA_PAR_HANDSET_DET,     /**< Parallel handset detected */
    RC_VDAA_PAR_HANDSET_NOT_DET, /**< Parallel handset not detected */
    RC_PATCH_RAM_VERIFY_FAIL,    /**< Patch RAM verification failure */
    RC_PATCH_ENTRY_VERIFY_FAIL,  /**< Patch entry table verification failure */ 
    RC_UNSUPPORTED_DEVICE_REV,   /**< Device revision not supported */
	RC_INVALID_PATCH,            /**< No patch for selected options */
	RC_INVALID_PRESET,           /**< Invalid Preset value */
	RC_TEST_DISABLED,            /**< Test Not enabled */
	RC_RING_START_FAIL,          /**< Ringing failed to start */
	RC_MWI_ENABLE_FAIL,          /**< Failed to enable MWI feature */
	RC_MWI_IN_USE,				 /**< MWI active and unable to be modified */
	RC_MWI_NOT_ENABLED,			 /**< MWI not enabled */
    RC_DCDC_SETUP_ERR,           /**< DCDC not properly initialized prior to powerup */
    RC_PLL_FREERUN_ACTIVE,       /**< PLL In Freerun Mode */
    RC_UNSUPPORTED_VDC_RANGE,    /**< VDC Range Unsupported */
    RC_NON_FATAL_INIT_ERR,       /**< Generic error to indicate a non-fatal error during init */
    RC_REINIT_REQUIRED = 255     /**< Soft Reset Required */
} errorCodeType;

/** @addtogroup GAIN_CONTROL
 * @{
* Path Selector
*/
enum {
    TXACGAIN_SEL = 0,
    RXACGAIN_SEL = 1
};

/**
 *  VDC input voltage range option tags - please refer hardware design
*/
typedef enum
{
    VDC_3P0_6P0,
    VDC_4P5_16P0,
    VDC_4P5_27P0,
    VDC_7P0_20P0,
    VDC_8P0_16P0,
    VDC_9P0_16P0,
    VDC_9P0_24P0,
    VDC_10P8_20P0,
    VDC_27P0_42P0,
    VDC_10P8_13P2
} vdcRangeType;

// Linefeed states
enum {
LF_OPEN,
LF_FWD_ACTIVE,
LF_FWD_OHT,
LF_TIP_OPEN,
LF_RINGING,
LF_REV_ACTIVE,
LF_REV_OHT,
LF_RING_OPEN
} ;


/*
** Defines structure for configuring gpio
*/
typedef struct {
	uInt8 outputEn;
	uInt8 analog;
	uInt8 direction;
	uInt8 manual;
	uInt8 polarity;
	uInt8 openDrain;
	uInt8 batselmap;
} Si3226x_GPIO_Cfg;


/**
** DC Feed Preset
*/
typedef struct {
	ramData slope_vlim;
	ramData slope_rfeed;
	ramData slope_ilim;
	ramData delta1;
	ramData delta2;
	ramData v_vlim;
	ramData v_rfeed;
	ramData v_ilim;
	ramData const_rfeed;
	ramData const_ilim;
	ramData i_vlim;
	ramData lcronhk;
	ramData lcroffhk;
	ramData lcrdbi;
	ramData longhith;
	ramData longloth;
	ramData longdbi;
	ramData lcrmask;
	ramData lcrmask_polrev;
	ramData lcrmask_state;
	ramData lcrmask_linecap;
	ramData vcm_oh;
	ramData vov_bat;
	ramData vov_gnd;
} ProSLIC_DCfeed_Cfg;

typedef ProSLIC_DCfeed_Cfg Si3226x_DCfeed_Cfg;
typedef Si3226x_DCfeed_Cfg* Si3226x_DCfeed_Cfg_ptr;


/**
*  BOM Option Tag - refer to hardware design
*/
typedef enum {
	DEFAULT,                      /**< DCDC: Unspecified */
	SI321X_HV,                    /**< DCDC: Si321x high voltage design */
    BO_DCDC_FLYBACK,              /**< DCDC: flyback design */
    BO_DCDC_QCUK,                 /**< DCDC: quasi-cuk design */
    BO_DCDC_BUCK_BOOST,           /**< DCDC: BJT buck-boost design */
    BO_DCDC_LCQCUK,               /**< DCDC: low-cost quasi-cuk design */
	BO_DCDC_P_BUCK_BOOST_5V,      /**< DCDC: PMOS buck-boost 5v design (depricated) */
	BO_DCDC_P_BUCK_BOOST_12V,     /**< DCDC: PMOS buck-boost 12v design (depricated) */
    BO_DCDC_P_BUCK_BOOST_12V_HV,  /**< DCDC: PMOS buck-boost 12v design, high voltage (depricated) */
	BO_DCDC_CUK,                  /**< DCDC: full cuk design */
	BO_DCDC_PMOS_BUCK_BOOST      /**< DCDC: PMOS buck-boost design */
} bomOptionsType;


/**
*  Battery Rail option tags - please refer hardware design
*/
typedef enum 
{
    BO_DCDC_TSS,        /**< Used for Fixed Rail DC-DC supplies */
    BO_DCDC_TRACKING,   /**< Used for Tracking DC-DC supplies */
    BO_DCDC_EXTERNAL,   /**< Used for external fixed rail supplies */
    BO_DCDC_TSS_ISO,    /**< Used for isolated TSS supplies */
    BO_DCDC_QSS,        /**< Used for QSS Designs */
    BO_DCDC_FIXED_RAIL = BO_DCDC_TSS     /**< Fixed rail replaced by TSS */
} batRailType;

/*
** Defines structure for general configuration and the dcdc converter
*/
typedef struct {
    uInt8 device_key;    /* Used to prevent loaded coeffs for another device */
    bomOptionsType bomOpt;
    batRailType  batType;
    ramData bat_hyst;
    ramData vbatr_expect; /* default - this is overwritten by ring preset */
    ramData vbath_expect;  /* default - this is overwritten by dc feed preset */
    ramData pwrsave_timer;
    ramData offhook_thresh; 
    ramData vbat_track_min;
    ramData vbat_track_min_rng;
    ramData pwrsave_dbi;
    ramData dcdc_ana_scale;
    ramData vov_bat_pwrsave_min;
    ramData vov_bat_pwrsave_min_rng;
    ramData therm_dbi;
    ramData cpump_dbi;
    ramData dcdc_verr;
    ramData dcdc_verr_hyst;
    ramData dcdc_oithresh_lo;
    ramData dcdc_oithresh_hi;
    ramData pd_uvlo; 
    ramData pd_ovlo;
    ramData pd_oclo;
    ramData pd_swdrv;
    ramData dcdc_uvhyst; 
    ramData dcdc_uvthresh; 
    ramData dcdc_ovthresh; 
    ramData dcdc_oithresh; 
    ramData dcdc_swdrv_pol; 
    ramData dcdc_uvpol;
    ramData dcdc_vref_man;
    ramData dcdc_vref_ctrl;
    ramData dcdc_rngtype; 
    ramData dcdc_ana_vref;
    ramData dcdc_ana_gain; 
    ramData dcdc_ana_toff; 
    ramData dcdc_ana_tonmin; 
    ramData dcdc_ana_tonmax; 
    ramData dcdc_ana_dshift; 
    ramData dcdc_ana_lpoly; 
    ramData dcdc_aux_invert;
    ramData dcdc_cpump_lp;
    ramData dcdc_cpump_pulldown;
    ramData dcdc_lift_en;
    ramData coef_p_hvic;
    ramData p_th_hvic;
    uInt8 vdc_range;       /* Was cm_clamp  pre 6.5.0 */
    uInt8 autoRegister;
    uInt8 irqen1;
    uInt8 irqen2;
    uInt8 irqen3;
    uInt8 irqen4;
    uInt8 enhance;
    ramData scale_kaudio;
    uInt8 zcal_en;
    ramData lkg_ofhk_offset;
    ramData lkg_lb_offset;
    ramData vbath_delta;
    ramData uvthresh_max;
    ramData uvthresh_scale;
    ramData uvthresh_bias;
} Si3226x_General_Cfg;


/*
** Defines structure for configuring pcm
*/
typedef struct {
    uInt8 pcmFormat;
    uInt8 widebandEn;
    uInt8 pcm_tri;
    uInt8 tx_edge;
    uInt8 alaw_inv;
} Si3226x_PCM_Cfg;

/*
** Defines structure for configuring pulse metering
*/
typedef struct {
	ramData pm_amp_thresh;
	uInt8 pm_freq;
	uInt8 pm_auto;
	ramData pm_active;
	ramData pm_inactive;
} Si3226x_PulseMeter_Cfg;

/*
** Defines structure for configuring FSK generation
*/
typedef struct {
	ramData fsk01;
	ramData fsk10;
	ramData fskamp0;
	ramData fskamp1;
	ramData fskfreq0;
	ramData fskfreq1;
	uInt8 eightBit;
	uInt8 fskdepth;
} Si3226x_FSK_Cfg;

/*
** Defines structure for configuring dtmf decode
*/
typedef struct {
	ramData dtmfdtf_b0_1;
	ramData dtmfdtf_b1_1;
	ramData dtmfdtf_b2_1;
	ramData dtmfdtf_a1_1;
	ramData dtmfdtf_a2_1;
	ramData dtmfdtf_b0_2;
	ramData dtmfdtf_b1_2;
	ramData dtmfdtf_b2_2;
	ramData dtmfdtf_a1_2;
	ramData dtmfdtf_a2_2;
	ramData dtmfdtf_b0_3;
	ramData dtmfdtf_b1_3;
	ramData dtmfdtf_b2_3;
	ramData dtmfdtf_a1_3;
	ramData dtmfdtf_a2_3;
} Si3226x_DTMFDec_Cfg;

/*
** Defines structure for configuring impedence synthesis
*/
typedef struct {
	ramData zsynth_b0;
	ramData zsynth_b1;
	ramData zsynth_b2;
	ramData zsynth_a1;
	ramData zsynth_a2;
	uInt8 ra;
} Si3226x_Zsynth_Cfg;

/*
** Defines structure for configuring hybrid
*/
typedef struct {
	ramData ecfir_c2;
	ramData ecfir_c3;
	ramData ecfir_c4;
	ramData ecfir_c5;
	ramData ecfir_c6;
	ramData ecfir_c7;
	ramData ecfir_c8;
	ramData ecfir_c9;
	ramData ecfir_b0;
	ramData ecfir_b1;
	ramData ecfir_a1;
	ramData ecfir_a2;
} Si3226x_hybrid_Cfg;

/*
** Defines structure for configuring GCI CI bits
*/
typedef struct {
	uInt8 gci_ci;
} Si3226x_CI_Cfg;

/*
** Defines structure for configuring modem tone detect
*/
typedef struct {
	ramData rxmodpwr;
	ramData rxpwr;
	ramData modem_gain;
	ramData txmodpwr;
	ramData txpwr;
} Si3226x_modemDet_Cfg;

/*
** Defines structure for configuring audio eq
*/

typedef struct {
	ramData txaceq_c0;
	ramData txaceq_c1;
	ramData txaceq_c2;
	ramData txaceq_c3;

	ramData rxaceq_c0;
	ramData rxaceq_c1;
	ramData rxaceq_c2;
	ramData rxaceq_c3;
} Si3226x_audioEQ_Cfg;


/*
** Defines structure for configuring audio gain on the fly
*/
typedef struct {
	ramData acgain;
	uInt8 mute;
	ramData aceq_c0;
	ramData aceq_c1;
	ramData aceq_c2;
	ramData aceq_c3;
} ProSLIC_audioGain_Cfg;
/*
** Defines structure for configuring audio gain
*/

typedef ProSLIC_audioGain_Cfg Si3226x_audioGain_Cfg;


typedef struct {
	Si3226x_audioEQ_Cfg audioEQ;
	Si3226x_hybrid_Cfg hybrid;
    Si3226x_Zsynth_Cfg zsynth;
	ramData txgain;
	ramData rxgain;
	ramData rxachpf_b0_1;
	ramData  rxachpf_b1_1;
	ramData  rxachpf_a1_1;
	short txgain_db; /*overall gain associated with this configuration*/
	short rxgain_db;
} Si3226x_Impedance_Cfg;


/** @} PROSLIC_INTERRUPTS */

/** @addtogroup TONE_GEN
 * @{
 */
/**
* Defines structure for configuring 1 oscillator - see your datasheet for specifics or use the configuration
* tool to have this filled in for you.
*/
typedef struct {
	ramData freq;
	ramData amp;
	ramData phas;
	uInt8 talo;
	uInt8 tahi;
	uInt8 tilo;
	uInt8 tihi;
} Oscillator_Cfg;
/*
** Defines structure for configuring tone generator
*/
typedef struct {
	Oscillator_Cfg osc1;
	Oscillator_Cfg osc2;
	uInt8 omode;
} Si3226x_Tone_Cfg; 

/*
** Defines structure for configuring ring generator
*/
typedef struct {
	ramData rtper;
	ramData freq;
	ramData amp;
	ramData phas;
	ramData offset;
	ramData slope_ring;
    ramData iring_lim;
    ramData rtacth;
	ramData rtdcth;
	ramData rtacdb;
	ramData rtdcdb;
	ramData vov_ring_bat;
	ramData vov_ring_gnd;
    ramData vbatr_expect;
	uInt8 talo;
	uInt8 tahi;
	uInt8 tilo;
	uInt8 tihi;
	ramData adap_ring_min_i;
    ramData counter_iring_val;
	ramData counter_vtr_val;
    ramData ar_const28;
    ramData ar_const32;
    ramData ar_const38;
    ramData ar_const46;
	ramData rrd_delay;
	ramData rrd_delay2;
    ramData vbat_track_min_rng;
	uInt8 ringcon;
    uInt8 userstat;
	ramData vcm_ring;
    ramData vcm_ring_fixed;
    ramData delta_vcm;
    ramData dcdc_rngtype;
    ramData vov_dcdc_slope;
    ramData vov_dcdc_os;
    ramData vov_ring_bat_max;
} Si3226x_Ring_Cfg;



/*
** This defines names for the interrupts in the ProSLIC
*/
typedef enum {
/* IRQ1 */
IRQ_OSC1_T1_SI3226X = 0,   
IRQ_OSC1_T2_SI3226X,
IRQ_OSC2_T1_SI3226X,
IRQ_OSC2_T2_SI3226X,
IRQ_RING_T1_SI3226X,
IRQ_RING_T2_SI3226X,
IRQ_FSKBUF_AVAIL_SI3226X,
IRQ_VBAT_SI3226X,
/* IRQ2 */
IRQ_RING_TRIP_SI3226X = 8,
IRQ_LOOP_STAT_SI3226X,
IRQ_LONG_STAT_SI3226X,
IRQ_VOC_TRACK_SI3226X,
IRQ_DTMF_SI3226X,
IRQ_INDIRECT_SI3226X,
IRQ_TXMDM_SI3226X,
IRQ_RXMDM_SI3226X,
/* IRQ3 */
IRQ_P_HVIC_SI3226X = 16,  
IRQ_P_THERM_SI3226X,
IRQ_PQ3_SI3226X,
IRQ_PQ4_SI3226X,
IRQ_PQ5_SI3226X,
IRQ_PQ6_SI3226X,
IRQ_DSP_SI3226X,
IRQ_MADC_FS_SI3226X,
/* IRQ4 */
IRQ_USER_0_SI3226X = 24, 
IRQ_USER_1_SI3226X,
IRQ_USER_2_SI3226X,
IRQ_USER_3_SI3226X,
IRQ_USER_4_SI3226X,
IRQ_USER_5_SI3226X,
IRQ_USER_6_SI3226X,
IRQ_USER_7_SI3226X
}Si3226xProslicInt;

#endif
