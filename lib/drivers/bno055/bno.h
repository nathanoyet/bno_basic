#ifndef __BNO_H
#define __BNO_H

#ifdef __cplusplus
    extern "C" {
#endif


#include <stdint.h>
#include "../usart/usart.h"


/**************************************************************************************************/
/*                                          Enumerations                                          */
/**************************************************************************************************/

/*********************************** General System Enumerations **********************************/
typedef enum {
    BNO_PAGE_0,
    BNO_PAGE_1
} BNO_Page_ID;

typedef enum {
    BNO_ACC,
    BNO_MAG,
    BNO_GYR
} BNO_Sensor;

typedef enum {
    BNO_ACC_CONFIG,
    BNO_MAG_CONFIG,
    BNO_GYR_0_CONFIG,
    BNO_GYR_1_CONFIG
} BNO_Sensor_Config;

typedef enum {
    BNO_PWR_NORMAL_MODE,
    BNO_PWR_LOW_PWR_MODE,
    BNO_PWR_SUSPEND_MODE
} BNO_PWR_Mode;

typedef enum {
    BNO_OPR_CONFIG_MODE,
    BNO_OPR_ACC_ONLY_MODE,
    BNO_OPR_MAG_ONLY_MODE,
    BNO_OPR_GYR_ONLY_MODE,
    BNO_OPR_ACC_MAG_MODE,
    BNO_OPR_ACC_GYR_MODE,
    BNO_OPR_MAG_GYR_MODE,
    BNO_OPR_AMG_MODE,
    BNO_OPR_IMU_MODE,
    BNO_OPR_COMPASS_MODE,
    BNO_OPR_M4G_MODE,
    BNO_OPR_NDOF_FMC_OFF_MODE,
    BNO_OPR_NDOF_MODE
} BNO_OPR_Mode;

typedef enum {
    BNO_UNIT_ACC_MS = 1,
    BNO_UNIT_ACC_MG,
    BNO_UNIT_GYR_DPS,
    BNO_UNIT_GYR_RPS,
    BNO_UNIT_EUL_DEGREES,
    BNO_UNIT_EUL_RADIANS,
    BNO_UNIT_TEMP_CEL,
    BNO_UNIT_TEMP_FAH,
    BNO_UNIT_ORI_WINDOWS,
    BNO_UNIT_ORI_ANDROID
} BNO_Unit;

typedef enum {
    BNO_UNIT_DO_ACC,
    BNO_UNIT_DO_GYR,
    BNO_UNIT_DO_EUL,
    BNO_UNIT_DO_TEMP,
    BNO_UNIT_DO_ORI
} BNO_Unit_DO;

/**************************************** ACC Enumerations ****************************************/
typedef enum {
    BNO_ACC_RANGE_2G,
    BNO_ACC_RANGE_4G,
    BNO_ACC_RANGE_8G,
    BNO_ACC_RANGE_16G
} BNO_ACC_Range;

typedef enum {
    BNO_ACC_BW_7_81_HZ,
    BNO_ACC_BW_15_63_HZ,
    BNO_ACC_BW_31_25_HZ,
    BNO_ACC_BW_62_5_HZ,
    BNO_ACC_BW_125_HZ,
    BNO_ACC_BW_250_HZ,
    BNO_ACC_BW_500_HZ,
    BNO_ACC_BW_1000_HZ
} BNO_ACC_BW;

typedef enum {
    BNO_ACC_PWR_MODE_NORMAL,
    BNO_ACC_PWR_MODE_SUSPEND,
    BNO_ACC_PWR_MODE_LOW_POWER_1,
    BNO_ACC_PWR_MODE_STANDBY,
    BNO_ACC_PWR_MODE_LOW_POWER_2,
    BNO_ACC_PWR_MODE_DEEP_SUSPEND
} BNO_ACC_PWR_Mode;

/**************************************** MAG Enumerations ****************************************/
typedef enum {
    BNO_MAG_DOR_2_HZ,
    BNO_MAG_DOR_6_HZ,
    BNO_MAG_DOR_8_HZ,
    BNO_MAG_DOR_10_HZ,
    BNO_MAG_DOR_15_HZ,
    BNO_MAG_DOR_20_HZ,
    BNO_MAG_DOR_25_HZ,
    BNO_MAG_DOR_30_HZ
} BNO_MAG_DOR;

typedef enum {
    BNO_MAG_OPR_MODE_LOW_PWR,
    BNO_MAG_OPR_MODE_RGLR,
    BNO_MAG_OPR_MODE_EN_RGLR,
    BNO_MAG_OPR_MODE_HI_ACC
} BNO_MAG_OPR_Mode;

typedef enum {
    BNO_MAG_PWR_MODE_NORMAL,
    BNO_MAG_PWR_MODE_SLEEP,
    BNO_MAG_PWR_MODE_SUSPEND,
    BNO_MAG_PWR_MODE_FORCE
} BNO_MAG_PWR_Mode;

/**************************************** GYR Enumerations ****************************************/
typedef enum {
    BNO_GYR_RANGE_2000_DPS,
    BNO_GYR_RANGE_1000_DPS,
    BNO_GYR_RANGE_500_DPS,
    BNO_GYR_RANGE_250_DPS,
    BNO_GYR_RANGE_125_DPS
} BNO_GYR_Range;

typedef enum {
    BNO_GYR_BW_523_HZ,
    BNO_GYR_BW_230_HZ,
    BNO_GYR_BW_116_HZ,
    BNO_GYR_BW_47_HZ,
    BNO_GYR_BW_23_HZ,
    BNO_GYR_BW_12_HZ,
    BNO_GYR_BW_64_HZ,
    BNO_GYR_BW_32_HZ
} BNO_GYR_BW;

typedef enum {
    BNO_GYR_PWR_MODE_NORMAL,
    BNO_GYR_PWR_MODE_FAST_PWR_UP,
    BNO_GYR_PWR_MODE_DEEP_SUSPEND,
    BNO_GYR_PWR_MODE_SUSPEND,
    BNO_GYR_PWR_MODE_ADV_PWRSAVE
} BNO_GYR_PWR_Mode;

/******************************** Sleep Configuration Enumerations ********************************/
typedef enum {
    BNO_ACC_SLP_EVENT_MODE,
    BNO_ACC_SLP_SAMPLING_MODE,
} BNO_ACC_Slp_Mode;

typedef enum {
    BNO_ACC_SLP_DUR_0_5_MS = 5,
    BNO_ACC_SLP_DUR_1_MS,
    BNO_ACC_SLP_DUR_2_MS,
    BNO_ACC_SLP_DUR_4_MS,
    BNO_ACC_SLP_DUR_6_MS,
    BNO_ACC_SLP_DUR_10_MS,
    BNO_ACC_SLP_DUR_25_MS,
    BNO_ACC_SLP_DUR_50_MS,
    BNO_ACC_SLP_DUR_100_MS,
    BNO_ACC_SLP_DUR_500_MS,
    BNO_ACC_SLP_DUR_1000_MS
} BNO_ACC_Slp_Dur;

typedef enum {
    BNO_GYR_SLP_DUR_2_MS,
    BNO_GYR_SLP_DUR_4_MS,
    BNO_GYR_SLP_DUR_5_MS,
    BNO_GYR_SLP_DUR_8_MS,
    BNO_GYR_SLP_DUR_10_MS,
    BNO_GYR_SLP_DUR_15_MS,
    BNO_GYR_SLP_DUR_18_MS,
    BNO_GYR_SLP_DUR_20_MS
} BNO_GYR_Slp_Dur;

typedef enum {
    BNO_GYR_SLP_AUTO_DUR_4_MS = 1,
    BNO_GYR_SLP_AUTO_DUR_5_MS,
    BNO_GYR_SLP_AUTO_DUR_8_MS,
    BNO_GYR_SLP_AUTO_DUR_10_MS,
    BNO_GYR_SLP_AUTO_DUR_15_MS,
    BNO_GYR_SLP_AUTO_DUR_20_MS,
    BNO_GYR_SLP_AUTO_DUR_40_MS
} BNO_GYR_Slp_Auto_Dur;

/*********************************** Sensor Output Enumerations ***********************************/
typedef enum {
    BNO_ODR_ACC,
    BNO_ODR_MAG,
    BNO_ODR_GYR,
    BNO_ODR_EUL,
    BNO_ODR_LIA,
    BNO_ODR_GRV
} BNO_ODR;

typedef enum {
    BNO_ODR_X,
    BNO_ODR_Y,
    BNO_ODR_Z
} BNO_ODR_Axis;

typedef enum {
    BNO_EUL_HEADING,
    BNO_EUL_ROLL,
    BNO_EUL_PITCH
} BNO_EUL_Angle;

typedef enum {
    BNO_QUA_W,
    BNO_QUA_X,
    BNO_QUA_Y,
    BNO_QUA_Z
} BNO_QUA_Value;

/************************************* Axis Remap Enumerations ************************************/
typedef enum {
    BNO_AXIS_X,
    BNO_AXIS_Y,
    BNO_AXIS_Z
} BNO_Axis;

typedef enum {
    BNO_POSITIVE_SIGN,
    BNO_NEGATIVE_SIGN
} BNO_Axis_Sign;

/************************************* Interrupt Enumerations *************************************/
typedef enum {
    BNO_IRQ_ACC_BSX_DRDY,
    BNO_IRQ_MAG_DRDY,
    BNO_IRQ_GYR_AM,
    BNO_IRQ_GYR_HIGH_RATE,
    BNO_IRQ_GYR_DRDY,
    BNO_IRQ_ACC_HIGH_G,
    BNO_IRQ_ACC_AM,
    BNO_IRQ_ACC_NM
} BNO_IRQ;

typedef enum {
    BNO_IRQ_AXIS_DISABLED,
    BNO_IRQ_AXIS_ENABLED
} BNO_IRQ_Axis_State;

typedef enum {
    BNO_SM_NM_SLOW_MOTION,
    BNO_SM_NM_NO_MOTION
} BNO_SM_NM_Det_Type;

typedef enum {
    BNO_GYR_FILTERED,
    BNO_GYR_UNFILTERED,
} BNO_GYR_Filter;

typedef enum {
    BNO_GYR_AWAKE_DUR_8_SAMPS,
    BNO_GYR_AWAKE_DUR_16_SAMPS,
    BNO_GYR_AWAKE_DUR_32_SAMPS,
    BNO_GYR_AWAKE_DUR_64_SAMPS
} BNO_GYR_Awake_Dur;


/**************************************************************************************************/
/*                                           Structures                                           */
/**************************************************************************************************/

/************************************ Configurations Structures ***********************************/
typedef struct {
    BNO_PWR_Mode pwr_mode;
    BNO_OPR_Mode opr_mode;
} BNO_Config_t;

typedef struct {
    BNO_ACC_Range    acc_range;
    BNO_ACC_BW       acc_bw;
    BNO_ACC_PWR_Mode acc_pwr_mode;
} BNO_ACC_Config_t;

typedef struct {
    BNO_MAG_DOR      mag_dor;
    BNO_MAG_OPR_Mode mag_opr_mode;
    BNO_MAG_PWR_Mode mag_pwr_mode;
} BNO_MAG_Config_t;

typedef struct {
    BNO_GYR_Range    gyr_range;
    BNO_GYR_BW       gyr_bw;
    BNO_GYR_PWR_Mode gyr_pwr_mode;
} BNO_GYR_Config_t;

/********************************* Sleep Configuration Structures *********************************/
typedef struct {
    BNO_ACC_Slp_Mode slp_mode;
    BNO_ACC_Slp_Dur  slp_dur;
} BNO_ACC_Slp_Config_t;

typedef struct {
    BNO_GYR_Slp_Dur      slp_dur;
    BNO_GYR_Slp_Auto_Dur auto_dur;
} BNO_GYR_Slp_Config_t;

/************************************* Calibration Structures *************************************/
typedef struct {
    int16_t offset_x;
    int16_t offset_y;
    int16_t offset_z;
} BNO_Offset_t;

typedef struct {
    int8_t radius_lsb;
    int8_t radius_msb;
} BNO_Radius_t;

typedef struct {
    BNO_Offset_t *acc_offset;
    BNO_Offset_t *mag_offset;
    BNO_Offset_t *gyr_offset;
    BNO_Radius_t *acc_radius;
    BNO_Radius_t *mag_radius;
} BNO_Calib_Profile_t;

/********************************* Sensor Data Storage Structures *********************************/
typedef struct {
    int16_t x_raw;
    int16_t y_raw;
    int16_t z_raw;
} BNO_ODR_Raw_t;

typedef struct {
    int16_t w_raw;
    int16_t x_raw;
    int16_t y_raw;
    int16_t z_raw;
} BNO_QUA_Raw_t;

typedef struct {
    float x_float;
    float y_float;
    float z_float;
} BNO_ODR_Float_t;

typedef struct {
    float w_float;
    float x_float;
    float y_float;
    float z_float;
} BNO_QUA_Float_t;

/************************************** Interrupt Structures **************************************/
typedef struct {
    BNO_SM_NM_Det_Type det_type;
    float              thres;
    uint8_t            slope_points;
    uint16_t           delay_s;
    BNO_IRQ_Axis_State x_axis;
    BNO_IRQ_Axis_State y_axis;
    BNO_IRQ_Axis_State z_axis;
} BNO_ACC_SM_NM_Config_t;

typedef struct {
    float              thres;
    uint8_t            slope_points;
    BNO_IRQ_Axis_State x_axis;
    BNO_IRQ_Axis_State y_axis;
    BNO_IRQ_Axis_State z_axis;
} BNO_ACC_AM_Config_t;

typedef struct {
    float              thres;
    uint16_t           dur_ms;
    BNO_IRQ_Axis_State x_axis;
    BNO_IRQ_Axis_State y_axis;
    BNO_IRQ_Axis_State z_axis;
} BNO_ACC_HG_Config_t;

typedef struct {
    BNO_IRQ_Axis_State x_axis;
    BNO_IRQ_Axis_State y_axis;
    BNO_IRQ_Axis_State z_axis;
    BNO_GYR_Filter     filter;
    float              x_set_thres;
    float              x_set_hyst;
    uint16_t           x_set_dur_ms;
    float              y_set_thres;
    float              y_set_hyst;
    uint16_t           y_set_dur_ms;
    float              z_set_thres;
    float              z_set_hyst;
    uint16_t           z_set_dur_ms;
} BNO_GYR_HR_Config_t;

typedef struct {
    float              thres;
    uint8_t            samples;
    BNO_GYR_Awake_Dur  awake_dur;
    BNO_GYR_Filter     filter;
    BNO_IRQ_Axis_State x_axis;
    BNO_IRQ_Axis_State y_axis;
    BNO_IRQ_Axis_State z_axis;
} BNO_GYR_AM_Config_t;

     
/**************************************************************************************************/
/*                                 Register Map Address Definition                                */
/**************************************************************************************************/

/********************************* Page 0 register map definition *********************************/
#define BNO_CHIP_ID_REG             (0x00)
#define BNO_ACC_ID_REG              (0x01)
#define BNO_MAG_ID_REG              (0x02)
#define BNO_GYR_ID_REG              (0x03)

#define BNO_SW_REV_ID_LSB_REG       (0x04)
#define BNO_SW_REV_ID_MSB_REG       (0x05)
#define BNO_BL_REV_ID_REG           (0x06)
#define BNO_PAGE_ID_REG             (0x07)

#define BNO_ACC_DATA_X_LSB_REG      (0x08)
#define BNO_ACC_DATA_X_MSB_REG      (0x09)
#define BNO_ACC_DATA_Y_LSB_REG      (0x0A)
#define BNO_ACC_DATA_Y_MSB_REG      (0x0B)
#define BNO_ACC_DATA_Z_LSB_REG      (0x0C)
#define BNO_ACC_DATA_Z_MSB_REG      (0x0D)

#define BNO_MAG_DATA_X_LSB_REG      (0x0E)
#define BNO_MAG_DATA_X_MSB_REG      (0x0F)
#define BNO_MAG_DATA_Y_LSB_REG      (0x10)
#define BNO_MAG_DATA_Y_MSB_REG      (0x11)
#define BNO_MAG_DATA_Z_LSB_REG      (0x12)
#define BNO_MAG_DATA_Z_MSB_REG      (0x13)

#define BNO_GYR_DATA_X_LSB_REG      (0x14)
#define BNO_GYR_DATA_X_MSB_REG      (0x15)
#define BNO_GYR_DATA_Y_LSB_REG      (0x16)
#define BNO_GYR_DATA_Y_MSB_REG      (0x17)
#define BNO_GYR_DATA_Z_LSB_REG      (0x18)
#define BNO_GYR_DATA_Z_MSB_REG      (0x19)

#define BNO_EUL_HEADING_LSB_REG     (0x1A)
#define BNO_EUL_HEADING_MSB_REG     (0x1B)
#define BNO_EUL_ROLL_LSB_REG        (0x1C)
#define BNO_EUL_ROLL_MSB_REG        (0x1D)
#define BNO_EUL_PITCH_LSB_REG       (0x1E)
#define BNO_EUL_PITCH_MSB_REG       (0x1F)

#define BNO_QUA_DATA_W_LSB_REG      (0x20)
#define BNO_QUA_DATA_W_MSB_REG      (0x21)
#define BNO_QUA_DATA_X_LSB_REG      (0x22)
#define BNO_QUA_DATA_X_MSB_REG      (0x23)
#define BNO_QUA_DATA_Y_LSB_REG      (0x24)
#define BNO_QUA_DATA_Y_MSB_REG      (0x25)
#define BNO_QUA_DATA_Z_LSB_REG      (0x26)
#define BNO_QUA_DATA_Z_MSB_REG      (0x27)

#define BNO_LIA_DATA_X_LSB_REG      (0x28)
#define BNO_LIA_DATA_X_MSB_REG      (0x29)
#define BNO_LIA_DATA_Y_LSB_REG      (0x2A)
#define BNO_LIA_DATA_Y_MSB_REG      (0x2B)
#define BNO_LIA_DATA_Z_LSB_REG      (0x2C)
#define BNO_LIA_DATA_Z_MSB_REG      (0x2D)

#define BNO_GRV_DATA_X_LSB_REG      (0x2E)
#define BNO_GRV_DATA_X_MSB_REG      (0x2F)
#define BNO_GRV_DATA_Y_LSB_REG      (0x30)
#define BNO_GRV_DATA_Y_MSB_REG      (0x31)
#define BNO_GRV_DATA_Z_LSB_REG      (0x32)
#define BNO_GRV_DATA_Z_MSB_REG      (0x33)

#define BNO_TEMP_REG                (0x34)
#define BNO_CALIB_STAT_REG          (0x35)
#define BNO_ST_RESULT_REG           (0x36)
#define BNO_INT_STA_REG             (0x37)
#define BNO_SYS_CLK_STATUS_REG      (0x38)
#define BNO_SYS_STATUS_REG          (0x39)
#define BNO_SYS_ERR_REG             (0x3A)
#define BNO_UNIT_SEL_REG            (0x3B)
#define BNO_OPR_MODE_REG            (0x3D)
#define BNO_PWR_MODE_REG            (0x3E)
#define BNO_SYS_TRIGGER_REG         (0x3F)
#define BNO_SYS_TEMP_SOURCE_REG     (0x40)
#define BNO_AXIS_MAP_CONFIG_REG     (0x41)
#define BNO_AXIS_MAP_SIGN_REG       (0x42)

#define BNO_SIC_MATRIX_LSB0_REG     (0x43)
#define BNO_SIC_MATRIX_MSB0_REG     (0x44)
#define BNO_SIC_MATRIX_LSB1_REG     (0x45)
#define BNO_SIC_MATRIX_MSB1_REG     (0x46)
#define BNO_SIC_MATRIX_LSB2_REG     (0x47)
#define BNO_SIC_MATRIX_MSB2_REG     (0x48)
#define BNO_SIC_MATRIX_LSB3_REG     (0x49)
#define BNO_SIC_MATRIX_MSB3_REG     (0x4A)
#define BNO_SIC_MATRIX_LSB4_REG     (0x4B)
#define BNO_SIC_MATRIX_MSB4_REG     (0x4C)
#define BNO_SIC_MATRIX_LSB5_REG     (0x4D)
#define BNO_SIC_MATRIX_MSB5_REG     (0x4E)
#define BNO_SIC_MATRIX_LSB6_REG     (0x4F)
#define BNO_SIC_MATRIX_MSB6_REG     (0x50)
#define BNO_SIC_MATRIX_LSB7_REG     (0x51)
#define BNO_SIC_MATRIX_MSB7_REG     (0x52)
#define BNO_SIC_MATRIX_LSB8_REG     (0x53)
#define BNO_SIC_MATRIX_MSB8_REG     (0x54)

#define BNO_ACC_OFFSET_X_LSB_REG    (0x55)
#define BNO_ACC_OFFSET_X_MSB_REG    (0x56)
#define BNO_ACC_OFFSET_Y_LSB_REG    (0x57)
#define BNO_ACC_OFFSET_Y_MSB_REG    (0x58)
#define BNO_ACC_OFFSET_Z_LSB_REG    (0x59)
#define BNO_ACC_OFFSET_Z_MSB_REG    (0x5A)

#define BNO_MAG_OFFSET_X_LSB_REG    (0x5B)
#define BNO_MAG_OFFSET_X_MSB_REG    (0x5C)
#define BNO_MAG_OFFSET_Y_LSB_REG    (0x5D)
#define BNO_MAG_OFFSET_Y_MSB_REG    (0x5E)
#define BNO_MAG_OFFSET_Z_LSB_REG    (0x5F)
#define BNO_MAG_OFFSET_Z_MSB_REG    (0x60)

#define BNO_GYR_OFFSET_X_LSB_REG    (0x61)
#define BNO_GYR_OFFSET_X_MSB_REG    (0x62)
#define BNO_GYR_OFFSET_Y_LSB_REG    (0x63)
#define BNO_GYR_OFFSET_Y_MSB_REG    (0x64)
#define BNO_GYR_OFFSET_Z_LSB_REG    (0x65)
#define BNO_GYR_OFFSET_Z_MSB_REG    (0x66)

#define BNO_ACC_RADIUS_LSB_REG      (0x67)
#define BNO_ACC_RADIUS_MSB_REG      (0x68)
#define BNO_MAG_RADIUS_LSB_REG      (0x69)
#define BNO_MAG_RADIUS_MSB_REG      (0x6A)

/********************************* Page 1 register map definition *********************************/
#define BNO_ACC_CONFIG_REG          (0x08)
#define BNO_MAG_CONFIG_REG          (0x09)
#define BNO_GYR_CONFIG_0_REG        (0x0A)
#define BNO_GYR_CONFIG_1_REG        (0x0B)
#define BNO_ACC_SLEEP_CONFIG_REG    (0x0C)
#define BNO_GYR_SLEEP_CONFIG_REG    (0x0D)
#define BNO_INT_MSK_REG             (0x0F)
#define BNO_INT_EN_REG              (0x10)
#define BNO_ACC_AM_THRES_REG        (0x11)
#define BNO_ACC_INT_SETTINGS_REG    (0x12)
#define BNO_ACC_HG_DURATION_REG     (0x13)
#define BNO_ACC_HG_THRES_REG        (0x14)
#define BNO_ACC_NM_THRES_REG        (0x15)
#define BNO_ACC_NM_SET_REG          (0x16)
#define BNO_GYR_INT_SETTINGS_REG    (0x17)
#define BNO_GYR_HR_X_SET_REG        (0x18)
#define BNO_GYR_DUR_X_REG           (0x19)
#define BNO_GYR_HR_Y_SET_REG        (0x1A)
#define BNO_GYR_DUR_Y_REG           (0x1B)
#define BNO_GYR_HR_Z_SET_REG        (0x1C)
#define BNO_GYR_DUR_Z_REG           (0x1D)
#define BNO_GYR_AM_THRES_REG        (0x1E)
#define BNO_GYR_AM_SET_REG          (0x1F)          


/**************************************************************************************************/
/*                              Common Access Register Address Bases                              */
/**************************************************************************************************/

#define BNO_ACC_BASE_REG            BNO_ACC_DATA_X_LSB_REG
#define BNO_MAG_BASE_REG            BNO_MAG_DATA_X_LSB_REG
#define BNO_GYR_BASE_REG            BNO_GYR_DATA_X_LSB_REG
#define BNO_EUL_BASE_REG            BNO_EUL_HEADING_LSB_REG
#define BNO_QUA_BASE_REG            BNO_QUA_DATA_W_LSB_REG
#define BNO_LIA_BASE_REG            BNO_LIA_DATA_X_LSB_REG
#define BNO_GRV_BASE_REG            BNO_GRV_DATA_X_LSB_REG


/**************************************************************************************************/
/*                                        General Constants                                       */
/**************************************************************************************************/

/************************************* Read/Write data lengths ************************************/
#define BNO_GENERIC_RW_LENGTH       ((uint8_t) 1U) 
#define BNO_LSB_MSB_LENGTH          ((uint8_t) 2U)
#define BNO_ACC_DATA_LENGTH         ((uint8_t) 6U)
#define BNO_MAG_DATA_LENGTH         ((uint8_t) 6U)
#define BNO_GYR_DATA_LENGTH         ((uint8_t) 6U)
#define BNO_AMG_DATA_LENGTH         ((uint8_t) 6U)
#define BNO_QUA_DATA_LENGTH         ((uint8_t) 8U)
#define BNO_RESPONSE_HEADER_LENGTH  ((uint8_t) 2U)

/****************************************** Unit settings *****************************************/
#define BNO_ACC_MS                  (100.0f)
#define BNO_ACC_MG                  (1.0f)
#define BNO_MAG_UT                  (16.0f)
#define BNO_GYR_DPS                 (16.0f)
#define BNO_GYR_RPS                 (900.0f)
#define BNO_EUL_DEGREES             (16.0f)
#define BNO_EUL_RADIANS             (900.0f)
#define BNO_QUA_QUATERNIONS         (16384.0f)
#define BNO_LIA_MS                  BNO_ACC_MS
#define BNO_LIA_MG                  BNO_ACC_MG
#define BNO_GRV_MS                  BNO_ACC_MS
#define BNO_GRV_MG                  BNO_ACC_MG
#define BNO_TEMP_CEL                (1.0f)
#define BNO_TEMP_FAH                (0.5f)



/**************************************************************************************************/
/*                                Page 0 Registers Bits Definition                                */
/**************************************************************************************************/

/***************************** Bits definition for CALIB_STAT register ****************************/
#define BNO_CALIB_STAT_MAG_Pos      (0U)
#define BNO_CALIB_STAT_MAG_Msk      (0x03U << BNO_CALIB_STAT_MAG_Pos)
#define BNO_CALIB_STAT_MAG          BNO_CALIB_STAT_MAG_Msk

#define BNO_CALIB_STAT_ACC_Pos      (2U)
#define BNO_CALIB_STAT_ACC_Msk      (0x03U << BNO_CALIB_STAT_ACC_Pos)
#define BNO_CALIB_STAT_ACC          BNO_CALIB_STAT_ACC_Msk

#define BNO_CALIB_STAT_GYR_Pos      (4U)
#define BNO_CALIB_STAT_GYR_Msk      (0x03U << BNO_CALIB_STAT_GYR_Pos)
#define BNO_CALIB_STAT_GYR          BNO_CALIB_STAT_GYR_Msk

#define BNO_CALIB_STAT_SYS_Pos      (6U)
#define BNO_CALIB_STAT_SYS_Msk      (0x03U << BNO_CALIB_STAT_SYS_Pos)
#define BNO_CALIB_STAT_SYS          BNO_CALIB_STAT_SYS_Msk

/***************************** Bits definition for ST_RESULT register *****************************/
#define BNO_ST_RESULT_ACC_Pos       (0U)
#define BNO_ST_RESULT_ACC_Msk       (0x01U << BNO_ST_RESULT_ACC_Pos)
#define BNO_ST_RESULT_ACC           BNO_ST_RESULT_ACC_Msk

#define BNO_ST_RESULT_MAG_Pos       (1U)
#define BNO_ST_RESULT_MAG_Msk       (0x01U << BNO_ST_RESULT_MAG_Pos)
#define BNO_ST_RESULT_MAG           BNO_ST_RESULT_MAG_Msk

#define BNO_ST_RESULT_GYR_Pos       (2U)
#define BNO_ST_RESULT_GYR_Msk       (0x01U << BNO_ST_RESULT_GYR_Pos)
#define BNO_ST_RESULT_GYR           BNO_ST_RESULT_GYR_Msk

#define BNO_ST_RESULT_MCU_Pos       (3U)
#define BNO_ST_RESULT_MCU_Msk       (0x01U << BNO_ST_RESULT_MCU_Pos)
#define BNO_ST_RESULT_MCU           BNO_ST_RESULT_MCU_Msk

/****************************** Bits definition for INT_STA register ******************************/
#define BNO_INT_STA_ACC_BSX_DRDY_Pos    (0U)
#define BNO_INT_STA_ACC_BSX_DRDY_Msk    (0x01U << BNO_INT_STA_ACC_BSX_DRDY_Pos)
#define BNO_INT_STA_ACC_BSX_DRDY        BNO_INT_STA_ACC_BSX_DRDY_Msk

#define BNO_INT_STA_MAG_DRDY_Pos        (1U)
#define BNO_INT_STA_MAG_DRDY_Msk        (0x01U << BNO_INT_STA_MAG_DRDY_Pos)
#define BNO_INT_STA_MAG_DRDY            BNO_INT_STA_MAG_DRDY_Msk

#define BNO_INT_STA_GYR_AM_Pos          (2U)
#define BNO_INT_STA_GYR_AM_Msk          (0x01U << BNO_INT_STA_GYR_AM_Pos)
#define BNO_INT_STA_GYR_AM              BNO_INT_STA_GYR_AM_Msk

#define BNO_INT_STA_GYR_HIGH_RATE_Pos   (3U)
#define BNO_INT_STA_GYR_HIGH_RATE_Msk   (0x01U << BNO_INT_STA_GYR_HIGH_RATE_Pos)
#define BNO_INT_STA_GYR_HIGH_RATE       BNO_INT_STA_GYR_HIGH_RATE_Msk

#define BNO_INT_STA_GYR_DRDY_Pos        (4U)
#define BNO_INT_STA_GYR_DRDY_Msk        (0x01U << BNO_INT_STA_GYR_DRDY_Pos)
#define BNO_INT_STA_GYR_DRDY            BNO_INT_STA_GYR_DRDY_Msk

#define BNO_INT_STA_ACC_HIGH_G_Pos      (5U)
#define BNO_INT_STA_ACC_HIGH_G_Msk      (0x01U << BNO_INT_STA_ACC_HIGH_G_Pos)
#define BNO_INT_STA_ACC_HIGH_G          BNO_INT_STA_ACC_HIGH_G_Msk

#define BNO_INT_STA_ACC_AM_Pos          (6U)
#define BNO_INT_STA_ACC_AM_Msk          (0x01U << BNO_INT_STA_ACC_AM_Pos)
#define BNO_INT_STA_ACC_AM              BNO_INT_STA_ACC_AM_Msk

#define BNO_INT_STA_ACC_NM_Pos          (7U)
#define BNO_INT_STA_ACC_NM_Msk          (0x01U << BNO_INT_STA_ACC_NM_Pos)
#define BNO_INT_STA_ACC_NM              BNO_INT_STA_ACC_NM_Msk

/*************************** Bits definition for SYS_CLK_STATUS register **************************/
#define BNO_SYS_CLK_STA_MAIN_CLK_Msk    (0x01U)
#define BNO_SYS_CLK_STA_MAIN_CLK        BNO_SYS_CLK_STA_MAIN_CLK_Msk

/***************************** Bits definition for SYS_STATUS register ****************************/
#define BNO_SYS_STATUS_Msk              (0xFFU)
#define BNO_SYS_STATUS                  BNO_SYS_STATUS_Msk
#define BNO_SYS_STATUS_SYS_ERR          (0x01U)
#define BNO_SYS_STATUS_PERIPH_INIT      (0x02U)
#define BNO_SYS_STATUS_SYS_INIT         (0x03U)
#define BNO_SYS_STATUS_EXECUTE_ST       (0x04U)
#define BNO_SYS_STATUS_SEN_FUSION_ON    (0x05U)
#define BNO_SYS_STATUS_SEN_FUSION_OFF   (0x06U)

/****************************** Bits definition for SYS_ERR register ******************************/
#define BNO_SYS_ERR_Msk                 (0xFFU)
#define BNO_SYS_ERR                     BNO_SYS_ERR_Msk
#define BNO_SYS_ERR_PERIPH_INIT_ERR     (0x01U)
#define BNO_SYS_ERR_SYS_INIT_ERR        (0x02U)
#define BNO_SYS_ERR_ST_FAILED           (0x03U)
#define BNO_SYS_ERR_REG_MAP_VAL_ORR     (0x04U)
#define BNO_SYS_ERR_REG_MAP_ADDR_ORR    (0x05U)
#define BNO_SYS_ERR_REG_MAP_WRITE_ERR   (0x06U)
#define BNO_SYS_ERR_LOW_PWR_MODE_NA     (0x07U)
#define BNO_SYS_ERR_ACC_PWR_MODE_NA     (0x08U)
#define BNO_SYS_ERR_FUSION_CONFIG_ERR   (0x09U)
#define BNO_SYS_ERR_SENSOR_CONFIG_ERR   (0x0AU)

/****************************** Bits definition for UNIT_SEL register *****************************/
#define BNO_UNIT_SEL_ACC_UNIT_Pos       (0U)
#define BNO_UNIT_SEL_ACC_UNIT_Msk       (0x01U << BNO_UNIT_SEL_ACC_UNIT_Pos)
#define BNO_UNIT_SEL_ACC_UNIT           BNO_UNIT_SEL_ACC_UNIT_Msk

#define BNO_UNIT_SEL_GYR_UNIT_Pos       (1U)
#define BNO_UNIT_SEL_GYR_UNIT_Msk       (0x01U << BNO_UNIT_SEL_GYR_UNIT_Pos)
#define BNO_UNIT_SEL_GYR_UNIT           BNO_UNIT_SEL_GYR_UNIT_Msk

#define BNO_UNIT_SEL_EUL_UNIT_Pos       (2U)
#define BNO_UNIT_SEL_EUL_UNIT_Msk       (0x01U << BNO_UNIT_SEL_EUL_UNIT_Pos)
#define BNO_UNIT_SEL_EUL_UNIT           BNO_UNIT_SEL_EUL_UNIT_Msk

#define BNO_UNIT_SEL_TEMP_UNIT_Pos      (4U)
#define BNO_UNIT_SEL_TEMP_UNIT_Msk      (0x01U << BNO_UNIT_SEL_TEMP_UNIT_Pos)
#define BNO_UNIT_SEL_TEMP_UNIT          BNO_UNIT_SEL_TEMP_UNIT_Msk

#define BNO_UNIT_SEL_ORI_UNIT_Pos       (7U)
#define BNO_UNIT_SEL_ORI_UNIT_Msk       (0x01U << BNO_UNIT_SEL_ORI_UNIT_Pos)
#define BNO_UNIT_SEL_ORI_UNIT           BNO_UNIT_SEL_ORI_UNIT_Msk

/****************************** Bits definition for OPR_MODE register *****************************/
#define BNO_OPR_MODE                    (0x0FU)
#define BNO_OPR_MODE_CONFIG_MODE        (0x00U)
#define BNO_OPR_MODE_NFM_ACC_ONLY       (0x01U)
#define BNO_OPR_MODE_NFM_MAG_ONLY       (0x02U)
#define BNO_OPR_MODE_NFM_GYR_ONLY       (0x03U)
#define BNO_OPR_MODE_NFM_ACC_MAG        (0x04U)
#define BNO_OPR_MODE_NFM_ACC_GYR        (0x05U)
#define BNO_OPR_MODE_NFM_MAG_GYR        (0x06U)
#define BNO_OPR_MODE_NFM_AMG            (0x07U)

#define BNO_OPR_MODE_FM_IMU             (0x08U)
#define BNO_OPR_MODE_FM_COMPASS         (0x09U)
#define BNO_OPR_MODE_FM_M4G             (0x0AU)
#define BNO_OPR_MODE_FM_NDOF_FMC_OFF    (0x0BU)
#define BNO_OPR_MODE_FM_NDOF            (0x0CU)

/****************************** Bits definition for PWR_MODE register *****************************/
#define BNO_PWR_MODE                    (0x03U)
#define BNO_PWR_MODE_NORMAL             (0x00U)
#define BNO_PWR_MODE_LOW_PWR            (0x01U)
#define BNO_PWR_MODE_SUSPEND            (0x02U)

/**************************** Bits definition for SYS_TRIGGER register ****************************/
#define BNO_SYS_TRIGGER_SELF_TEST_Pos   (0U)
#define BNO_SYS_TRIGGER_SELF_TEST_Msk   (0x01U << BNO_SYS_TRIGGER_SELF_TEST_Pos)
#define BNO_SYS_TRIGGER_SELF_TEST       BNO_SYS_TRIGGER_SELF_TEST_Msk

#define BNO_SYS_TRIGGER_RST_SYS_Pos     (5U)
#define BNO_SYS_TRIGGER_RST_SYS_Msk     (0x01U << BNO_SYS_TRIGGER_RST_SYS_Pos)
#define BNO_SYS_TRIGGER_RST_SYSCFG      BNO_SYS_TRIGGER_RST_SYS_Msk

#define BNO_SYS_TRIGGER_RST_INT_Pos     (6U)
#define BNO_SYS_TRIGGER_RST_INT_Msk     (0x01U << BNO_SYS_TRIGGER_RST_INT_Pos)
#define BNO_SYS_TRIGGER_RST_INT         BNO_SYS_TRIGGER_RST_INT_Msk

#define BNO_SYS_TRIGGER_CLK_SEL_Pos     (7U)
#define BNO_SYS_TRIGGER_CLK_SEL_Msk     (0x01U << BNO_SYS_TRIGGER_CLK_SEL_Pos)
#define BNO_SYS_TRIGGER_CLK_SEL         BNO_SYS_TRIGGER_CLK_SEL_Msk

/**************************** Bits definition for TEMP_SOURCE register ****************************/
#define BNO_TEMP_SOURCE_Msk             (0x03U)
#define BNO_TEMP_SOURCE                 BNO_TEMP_SOURCE_Msk
#define BNO_TEMP_SOURCE_ACC             (0x00U)
#define BNO_TEMP_SOURCE_GYR             (0x01U)

/************************** Bits definition for AXIS_MAP_CONFIG register **************************/
#define BNO_AXIS_MAP_CONFIG_X_REMAP_Pos (0U)
#define BNO_AXIS_MAP_CONFIG_X_REMAP_Msk (0x03U << BNO_AXIS_MAP_CONFIG_X_REMAP_Pos)
#define BNO_AXIS_MAP_CONFIG_X_REMAP     BNO_AXIS_MAP_CONFIG_X_REMAP_Msk
#define BNO_AXIS_MAP_CONFIG_X_REMAP_X   (0x00U << BNO_AXIS_MAP_CONFIG_X_REMAP_Pos)
#define BNO_AXIS_MAP_CONFIG_X_REMAP_Y   (0x01U << BNO_AXIS_MAP_CONFIG_X_REMAP_Pos)
#define BNO_AXIS_MAP_CONFIG_X_REMAP_Z   (0x02U << BNO_AXIS_MAP_CONFIG_X_REMAP_Pos)

#define BNO_AXIS_MAP_CONFIG_Y_REMAP_Pos (2U)
#define BNO_AXIS_MAP_CONFIG_Y_REMAP_Msk (0x03U << BNO_AXIS_MAP_CONFIG_Y_REMAP_Pos)
#define BNO_AXIS_MAP_CONFIG_Y_REMAP     BNO_AXIS_MAP_CONFIG_Y_REMAP_Msk
#define BNO_AXIS_MAP_CONFIG_Y_REMAP_X   (0x00U << BNO_AXIS_MAP_CONFIG_Y_REMAP_Pos)
#define BNO_AXIS_MAP_CONFIG_Y_REMAP_Y   (0x01U << BNO_AXIS_MAP_CONFIG_Y_REMAP_Pos)
#define BNO_AXIS_MAP_CONFIG_Y_REMAP_Z   (0x02U << BNO_AXIS_MAP_CONFIG_Y_REMAP_Pos)

#define BNO_AXIS_MAP_CONFIG_Z_REMAP_Pos (4U)
#define BNO_AXIS_MAP_CONFIG_Z_REMAP_Msk (0x03U << BNO_AXIS_MAP_CONFIG_Z_REMAP_Pos)
#define BNO_AXIS_MAP_CONFIG_Z_REMAP     BNO_AXIS_MAP_CONFIG_Z_REMAP_Msk
#define BNO_AXIS_MAP_CONFIG_Z_REMAP_X   (0x00U << BNO_AXIS_MAP_CONFIG_Z_REMAP_Pos)
#define BNO_AXIS_MAP_CONFIG_Z_REMAP_Y   (0x01U << BNO_AXIS_MAP_CONFIG_Z_REMAP_Pos)
#define BNO_AXIS_MAP_CONFIG_Z_REMAP_Z   (0x02U << BNO_AXIS_MAP_CONFIG_Z_REMAP_Pos)

/******************* Bits definition for AXIS_MAP_SIGN register *******************/
#define BNO_AXIS_MAP_SIGN_REMAP_Pos     (0U)
#define BNO_AXIS_MAP_SIGN_REMAP_Msk     (0x03U << BNO_AXIS_MAP_SIGN_REMAP_Pos)
#define BNO_AXIS_MAP_SIGN_REMAP         BNO_AXIS_MAP_SIGN_REMAP_Msk
#define BNO_AXIS_MAP_SIGN_REMAP_Z_POS   (0x00U)
#define BNO_AXIS_MAP_SIGN_REMAP_Z_NEG   (0x01U)
#define BNO_AXIS_MAP_SIGN_REMAP_Y_POS   (0x00U)
#define BNO_AXIS_MAP_SIGN_REMAP_Y_NEG   (0x02U)
#define BNO_AXIS_MAP_SIGN_REMAP_X_POS   (0x00U)
#define BNO_AXIS_MAP_SIGN_REMAP_X_NEG   (0x10U)


/**************************************************************************************************/
/*                                Page 1 Registers Bits Definition                                */
/**************************************************************************************************/

/***************************** Bits definition for ACC_CONFIG register ****************************/
#define BNO_ACC_CONFIG_RANGE_Pos        (0U)
#define BNO_ACC_CONFIG_RANGE_Msk        (0x03U << BNO_ACC_CONFIG_RANGE_Pos)
#define BNO_ACC_CONFIG_RANGE            BNO_ACC_CONFIG_RANGE_Msk
#define BNO_ACC_CONFIG_RANGE_2G         (0x00U << BNO_ACC_CONFIG_RANGE_Pos)
#define BNO_ACC_CONFIG_RANGE_4G         (0x01U << BNO_ACC_CONFIG_RANGE_Pos)
#define BNO_ACC_CONFIG_RANGE_8G         (0x02U << BNO_ACC_CONFIG_RANGE_Pos)
#define BNO_ACC_CONFIG_RANGE_16G        (0x03U << BNO_ACC_CONFIG_RANGE_Pos)

#define BNO_ACC_CONFIG_BW_Pos           (2U)
#define BNO_ACC_CONFIG_BW_Msk           (0x03U << BNO_ACC_CONFIG_BW_Pos)
#define BNO_ACC_CONFIG_BW               BNO_ACC_CONFIG_BW_Msk
#define BNO_ACC_CONFIG_BW_7_81HZ        (0x00U << BNO_ACC_CONFIG_BW_Pos)
#define BNO_ACC_CONFIG_BW_15_63HZ       (0x01U << BNO_ACC_CONFIG_BW_Pos)
#define BNO_ACC_CONFIG_BW_31_25HZ       (0x02U << BNO_ACC_CONFIG_BW_Pos)
#define BNO_ACC_CONFIG_BW_62_5HZ        (0x03U << BNO_ACC_CONFIG_BW_Pos)
#define BNO_ACC_CONFIG_BW_125HZ         (0x04U << BNO_ACC_CONFIG_BW_Pos)
#define BNO_ACC_CONFIG_BW_250HZ         (0x05U << BNO_ACC_CONFIG_BW_Pos)
#define BNO_ACC_CONFIG_BW_500HZ         (0x06U << BNO_ACC_CONFIG_BW_Pos)
#define BNO_ACC_CONFIG_BW_1000HZ        (0x07U << BNO_ACC_CONFIG_BW_Pos)

#define BNO_ACC_CONFIG_PWR_MODE_Pos     (5U)
#define BNO_ACC_CONFIG_PWR_MODE_Msk     (0x03U << BNO_ACC_CONFIG_PWR_MODE_Pos)
#define BNO_ACC_CONFIG_PWR_MODE         BNO_ACC_CONFIG_PWR_MODE_Msk
#define BNO_ACC_CONFIG_PWR_MODE_NORMAL  (0x00U << BNO_ACC_CONFIG_PWR_MODE_Pos)
#define BNO_ACC_CONFIG_PWR_MODE_SUSPEND (0x01U << BNO_ACC_CONFIG_PWR_MODE_Pos)
#define BNO_ACC_CONFIG_PWR_MODE_L_PWR_1 (0x02U << BNO_ACC_CONFIG_PWR_MODE_Pos)
#define BNO_ACC_CONFIG_PWR_MODE_STANDBY (0x03U << BNO_ACC_CONFIG_PWR_MODE_Pos)
#define BNO_ACC_CONFIG_PWR_MODE_L_PWR_2 (0x04U << BNO_ACC_CONFIG_PWR_MODE_Pos)
#define BNO_ACC_CONFIG_PWR_MODE_DEEP_S  (0x05U << BNO_ACC_CONFIG_PWR_MODE_Pos)

/***************************** Bits definition for MAG_CONFIG register ****************************/
#define BNO_MAG_CONFIG_DOR_Pos          (0U)
#define BNO_MAG_CONFIG_DOR_Msk          (0x07U << BNO_MAG_CONFIG_DOR_Pos)
#define BNO_MAG_CONFIG_DOR              BNO_MAG_CONFIG_DOR_Msk
#define BNO_MAG_CONFIG_DOR_2_HZ         (0x00U << BNO_MAG_CONFIG_DOR_Pos)
#define BNO_MAG_CONFIG_DOR_6_HZ         (0x01U << BNO_MAG_CONFIG_DOR_Pos)
#define BNO_MAG_CONFIG_DOR_8_HZ         (0x02U << BNO_MAG_CONFIG_DOR_Pos)
#define BNO_MAG_CONFIG_DOR_10_HZ        (0x03U << BNO_MAG_CONFIG_DOR_Pos)
#define BNO_MAG_CONFIG_DOR_15_HZ        (0x04U << BNO_MAG_CONFIG_DOR_Pos)
#define BNO_MAG_CONFIG_DOR_20_HZ        (0x05U << BNO_MAG_CONFIG_DOR_Pos)
#define BNO_MAG_CONFIG_DOR_25_HZ        (0x06U << BNO_MAG_CONFIG_DOR_Pos)
#define BNO_MAG_CONFIG_DOR_30_HZ        (0x07U << BNO_MAG_CONFIG_DOR_Pos)

#define BNO_MAG_CONFIG_OPR_MODE_Pos     (3U)
#define BNO_MAG_CONFIG_OPR_MODE_Msk     (0x03U << BNO_MAG_CONFIG_OPR_MODE_Pos)
#define BNO_MAG_CONFIG_OPR_MODE         BNO_MAG_CONFIG_OPR_MODE_Msk
#define BNO_MAG_CONFIG_OPR_MODE_LOW_PWR (0x00U << BNO_MAG_CONFIG_OPR_MODE_Pos)
#define BNO_MAG_CONFIG_OPR_MODE_RGLR    (0x01U << BNO_MAG_CONFIG_OPR_MODE_Pos)
#define BNO_MAG_CONFIG_OPR_MODE_EN_RGLR (0x02U << BNO_MAG_CONFIG_OPR_MODE_Pos)
#define BNO_MAG_CONFIG_OPR_MODE_HI_ACC  (0x03U << BNO_MAG_CONFIG_OPR_MODE_Pos)

#define BNO_MAG_CONFIG_PWR_MODE_Pos     (5U)
#define BNO_MAG_CONFIG_PWR_MODE_Msk     (0x03U << BNO_MAG_CONFIG_PWR_MODE_Pos)
#define BNO_MAG_CONFIG_PWR_MODE         BNO_MAG_CONFIG_PWR_MODE_Msk
#define BNO_MAG_CONFIG_PWR_MODE_NORMAL  (0x00U << BNO_MAG_CONFIG_PWR_MODE_Pos)
#define BNO_MAG_CONFIG_PWR_MODE_SLEEP   (0x01U << BNO_MAG_CONFIG_PWR_MODE_Pos)
#define BNO_MAG_CONFIG_PWR_MODE_SUSPEND (0x02U << BNO_MAG_CONFIG_PWR_MODE_Pos)
#define BNO_MAG_CONFIG_PWR_MODE_FORCE   (0x03U << BNO_MAG_CONFIG_PWR_MODE_Pos)

/***************************** Bits definition for GYR_CONFIG register ****************************/
#define BNO_GYR_CONFIG_0_RANGE_Pos      (0U)
#define BNO_GYR_CONFIG_0_RANGE_Msk      (0x07U << BNO_GYR_CONFIG_0_RANGE_Pos)
#define BNO_GYR_CONFIG_0_RANGE          BNO_GYR_CONFIG_0_RANGE_Msk
#define BNO_GYR_CONFIG_0_RANGE_2000_DPS (0x00U << BNO_GYR_CONFIG_0_RANGE_Pos)
#define BNO_GYR_CONFIG_0_RANGE_1000_DPS (0x01U << BNO_GYR_CONFIG_0_RANGE_Pos)
#define BNO_GYR_CONFIG_0_RANGE_500_DPS  (0x02U << BNO_GYR_CONFIG_0_RANGE_Pos)
#define BNO_GYR_CONFIG_0_RANGE_250_DPS  (0x03U << BNO_GYR_CONFIG_0_RANGE_Pos)
#define BNO_GYR_CONFIG_0_RANGE_125_DPS  (0x04U << BNO_GYR_CONFIG_0_RANGE_Pos)

#define BNO_GYR_CONFIG_0_BW_Pos         (3U)
#define BNO_GYR_CONFIG_0_BW_Msk         (0x07U << BNO_GYR_CONFIG_0_BW_Pos)
#define BNO_GYR_CONFIG_0_BW             BNO_GYR_CONFIG_0_BW_Msk
#define BNO_GYR_CONFIG_0_BW_523_HZ      (0x00U << BNO_GYR_CONFIG_0_BW_Pos)
#define BNO_GYR_CONFIG_0_BW_230_HZ      (0x01U << BNO_GYR_CONFIG_0_BW_Pos)
#define BNO_GYR_CONFIG_0_BW_116_HZ      (0x02U << BNO_GYR_CONFIG_0_BW_Pos)
#define BNO_GYR_CONFIG_0_BW_47_HZ       (0x03U << BNO_GYR_CONFIG_0_BW_Pos)
#define BNO_GYR_CONFIG_0_BW_23_HZ       (0x04U << BNO_GYR_CONFIG_0_BW_Pos)
#define BNO_GYR_CONFIG_0_BW_12_HZ       (0x05U << BNO_GYR_CONFIG_0_BW_Pos)
#define BNO_GYR_CONFIG_0_BW_64_HZ       (0x06U << BNO_GYR_CONFIG_0_BW_Pos)
#define BNO_GYR_CONFIG_0_BW_32_HZ       (0x07U << BNO_GYR_CONFIG_0_BW_Pos)

#define BNO_GYR_CONFIG_1_PWR_MODE_Pos         (0U)
#define BNO_GYR_CONFIG_1_PWR_MODE_Msk         (0x07U << BNO_GYR_CONFIG_1_PWR_MODE_Pos)
#define BNO_GYR_CONFIG_1_PWR_MODE             BNO_GYR_CONFIG_1_PWR_MODE_Msk
#define BNO_GYR_CONFIG_1_PWR_MODE_NORMAL      (0x00U << BNO_GYR_CONFIG_1_PWR_MODE_Pos)
#define BNO_GYR_CONFIG_1_PWR_MODE_FAST_PWR_UP (0x01U << BNO_GYR_CONFIG_1_PWR_MODE_Pos)
#define BNO_GYR_CONFIG_1_PWR_MODE_DEEP_S      (0x02U << BNO_GYR_CONFIG_1_PWR_MODE_Pos)
#define BNO_GYR_CONFIG_1_PWR_MODE_SUSPEND     (0x03U << BNO_GYR_CONFIG_1_PWR_MODE_Pos)
#define BNO_GYR_CONFIG_1_PWR_MODE_ADV_PWRSAVE (0x04U << BNO_GYR_CONFIG_1_PWR_MODE_Pos)

/************************** Bits definition for ACC_SLEEP_CONFIG register *************************/
#define BNO_ACC_SLEEP_CONFIG_SLP_MODE_Pos   (0U)
#define BNO_ACC_SLEEP_CONFIG_SLP_MODE_Msk   (0x01U << BNO_ACC_SLEEP_CONFIG_SLP_MODE_Pos)
#define BNO_ACC_SLEEP_CONFIG_SLP_MODE       BNO_ACC_SLEEP_CONFIG_SLP_MODE_Msk

#define BNO_ACC_SLEEP_CONFIG_SLP_DUR_Pos    (1U)
#define BNO_ACC_SLEEP_CONFIG_SLP_DUR_Msk    (0x0FU << BNO_ACC_SLEEP_CONFIG_SLP_DUR_Pos)
#define BNO_ACC_SLEEP_CONFIG_SLP_DUR        BNO_ACC_SLEEP_CONFIG_SLP_DUR_Msk

/************************** Bits definition for GYR_SLEEP_CONFIG register *************************/
#define BNO_GYR_SLEEP_CONFIG_SLP_DUR_Pos    (0U)
#define BNO_GYR_SLEEP_CONFIG_SLP_DUR_Msk    (0x07U << BNO_GYR_SLEEP_CONFIG_SLP_DUR_Pos)
#define BNO_GYR_SLEEP_CONFIG_SLP_DUR        BNO_GYR_SLEEP_CONFIG_SLP_DUR_Msk

#define BNO_GYR_SLEEP_CONFIG_AUTO_DUR_Pos   (3U)
#define BNO_GYR_SLEEP_CONFIG_AUTO_DUR_Msk   (0x07U << BNO_GYR_SLEEP_CONFIG_AUTO_DUR_Pos)
#define BNO_GYR_SLEEP_CONFIG_AUTO_DUR       BNO_GYR_SLEEP_CONFIG_AUTO_DUR_Msk

/****************************** Bits definition for INT_MSK register ******************************/
#define BNO_INT_MSK_ACC_BSX_DRDY_Pos    (0U)
#define BNO_INT_MSK_ACC_BSX_DRDY_Msk    (0x01U << BNO_INT_MSK_ACC_BSX_DRDY_Pos)
#define BNO_INT_MSK_ACC_BSX_DRDY        BNO_INT_MSK_ACC_BSX_DRDY_Msk

#define BNO_INT_MSK_MAG_DRDY_Pos        (1U)
#define BNO_INT_MSK_MAG_DRDY_Msk        (0x01U << BNO_INT_MSK_MAG_DRDY_Pos)
#define BNO_INT_MSK_MAG_DRDY            BNO_INT_MSK_MAG_DRDY_Msk

#define BNO_INT_MSK_GYR_AM_Pos          (2U)
#define BNO_INT_MSK_GYR_AM_Msk          (0x01U << BNO_INT_MSK_GYR_AM_Pos)
#define BNO_INT_MSK_GYR_AM              BNO_INT_MSK_GYR_AM_Msk

#define BNO_INT_MSK_GYR_HIGH_RATE_Pos   (3U)
#define BNO_INT_MSK_GYR_HIGH_RATE_Msk   (0x01U << BNO_INT_MSK_GYR_HIGH_RATE_Pos)
#define BNO_INT_MSK_GYR_HIGH_RATE       BNO_INT_MSK_GYR_HIGH_RATE_Msk

#define BNO_INT_MSK_GYR_DRDY_Pos        (4U)
#define BNO_INT_MSK_GYR_DRDY_Msk        (0x01U << BNO_INT_MSK_GYR_DRDY_Pos)
#define BNO_INT_MSK_GYR_DRDY            BNO_INT_MSK_GYR_DRDY_Msk

#define BNO_INT_MSK_ACC_HIGH_G_Pos      (5U)
#define BNO_INT_MSK_ACC_HIGH_G_Msk      (0x01U << BNO_INT_MSK_ACC_HIGH_G_Pos)
#define BNO_INT_MSK_ACC_HIGH_G          BNO_INT_MSK_ACC_HIGH_G_Msk

#define BNO_INT_MSK_ACC_AM_Pos          (6U)
#define BNO_INT_MSK_ACC_AM_Msk          (0x01U << BNO_INT_MSK_ACC_AM_Pos)
#define BNO_INT_MSK_ACC_AM              BNO_INT_MSK_ACC_AM_Msk

#define BNO_INT_MSK_ACC_NM_Pos          (7U)
#define BNO_INT_MSK_ACC_NM_Msk          (0x01U << BNO_INT_MSK_ACC_NM_Pos)
#define BNO_INT_MSK_ACC_NM              BNO_INT_MSK_ACC_NM_Msk

/******************************* Bits definition for INT_EN register ******************************/
#define BNO_INT_EN_ACC_BSX_DRDY_Pos     (0U)
#define BNO_INT_EN_ACC_BSX_DRDY_Msk     (0x01U << BNO_INT_EN_ACC_BSX_DRDY_Pos)
#define BNO_INT_EN_ACC_BSX_DRDY         BNO_INT_EN_ACC_BSX_DRDY_Msk

#define BNO_INT_EN_MAG_DRDY_Pos         (1U)
#define BNO_INT_EN_MAG_DRDY_Msk         (0x01U << BNO_INT_EN_MAG_DRDY_Pos)
#define BNO_INT_EN_MAG_DRDY             BNO_INT_EN_MAG_DRDY_Msk

#define BNO_INT_EN_GYR_AM_Pos           (2U)
#define BNO_INT_EN_GYR_AM_Msk           (0x01U << BNO_INT_EN_GYR_AM_Pos)
#define BNO_INT_EN_GYR_AM               BNO_INT_EN_GYR_AM_Msk

#define BNO_INT_EN_GYR_HIGH_RATE_Pos    (3U)
#define BNO_INT_EN_GYR_HIGH_RATE_Msk    (0x01U << BNO_INT_EN_GYR_HIGH_RATE_Pos)
#define BNO_INT_EN_GYR_HIGH_RATE        BNO_INT_EN_GYR_HIGH_RATE_Msk

#define BNO_INT_EN_GYR_DRDY_Pos         (4U)
#define BNO_INT_EN_GYR_DRDY_Msk         (0x01U << BNO_INT_EN_GYR_DRDY_Pos)
#define BNO_INT_EN_GYR_DRDY             BNO_INT_EN_GYR_DRDY_Msk

#define BNO_INT_EN_ACC_HIGH_G_Pos       (5U)
#define BNO_INT_EN_ACC_HIGH_G_Msk       (0x01U << BNO_INT_EN_ACC_HIGH_G_Pos)
#define BNO_INT_EN_ACC_HIGH_G           BNO_INT_EN_ACC_HIGH_G_Msk

#define BNO_INT_EN_ACC_AM_Pos           (6U)
#define BNO_INT_EN_ACC_AM_Msk           (0x01U << BNO_INT_EN_ACC_AM_Pos)
#define BNO_INT_EN_ACC_AM               BNO_INT_EN_ACC_AM_Msk

#define BNO_INT_EN_ACC_NM_Pos           (7U)
#define BNO_INT_EN_ACC_NM_Msk           (0x01U << BNO_INT_EN_ACC_NM_Pos)
#define BNO_INT_EN_ACC_NM               BNO_INT_EN_ACC_NM_Msk

/**************************** Bits definition for ACC_AM_THRES register ***************************/
#define BNO_ACC_AM_THRES_Pos            (0U)
#define BNO_ACC_AM_THRES_Msk            (0xFFU << BNO_ACC_AM_THRES_Pos)
#define BNO_ACC_AM_THRES                BNO_ACC_AM_THRES_Msk

/************************** Bits definition for ACC_INT_SETTINGS register *************************/
#define BNO_ACC_INT_SETTINGS_AM_DUR_Pos         (0U)
#define BNO_ACC_INT_SETTINGS_AM_DUR_Msk         (0x03U << BNO_ACC_INT_SETTINGS_AM_DUR_Pos)
#define BNO_ACC_INT_SETTINGS_AM_DUR             BNO_ACC_INT_SETTINGS_AM_DUR_Msk

#define BNO_ACC_INT_SETTINGS_AM_NM_X_AXIS_Pos   (2U)
#define BNO_ACC_INT_SETTINGS_AM_NM_X_AXIS_Msk   (0x01U << BNO_ACC_INT_SETTINGS_AM_NM_X_AXIS_Pos)
#define BNO_ACC_INT_SETTINGS_AM_NM_X_AXIS       BNO_ACC_INT_SETTINGS_AM_NM_X_AXIS_Msk

#define BNO_ACC_INT_SETTINGS_AM_NM_Y_AXIS_Pos   (3U)
#define BNO_ACC_INT_SETTINGS_AM_NM_Y_AXIS_Msk   (0x01U << BNO_ACC_INT_SETTINGS_AM_NM_Y_AXIS_Pos)
#define BNO_ACC_INT_SETTINGS_AM_NM_Y_AXIS       BNO_ACC_INT_SETTINGS_AM_NM_Y_AXIS_Msk

#define BNO_ACC_INT_SETTINGS_AM_NM_Z_AXIS_Pos   (4U)
#define BNO_ACC_INT_SETTINGS_AM_NM_Z_AXIS_Msk   (0x01U << BNO_ACC_INT_SETTINGS_AM_NM_Z_AXIS_Pos)
#define BNO_ACC_INT_SETTINGS_AM_NM_Z_AXIS       BNO_ACC_INT_SETTINGS_AM_NM_Z_AXIS_Msk

#define BNO_ACC_INT_SETTINGS_HG_X_AXIS_Pos      (5U)
#define BNO_ACC_INT_SETTINGS_HG_X_AXIS_Msk      (0x01U << BNO_ACC_INT_SETTINGS_HG_X_AXIS_Pos)
#define BNO_ACC_INT_SETTINGS_HG_X_AXIS          BNO_ACC_INT_SETTINGS_HG_X_AXIS_Msk

#define BNO_ACC_INT_SETTINGS_HG_Y_AXIS_Pos      (6U)
#define BNO_ACC_INT_SETTINGS_HG_Y_AXIS_Msk      (0x01U << BNO_ACC_INT_SETTINGS_HG_Y_AXIS_Pos)
#define BNO_ACC_INT_SETTINGS_HG_Y_AXIS          BNO_ACC_INT_SETTINGS_HG_Y_AXIS_Msk

#define BNO_ACC_INT_SETTINGS_HG_Z_AXIS_Pos      (7U)
#define BNO_ACC_INT_SETTINGS_HG_Z_AXIS_Msk      (0x01U << BNO_ACC_INT_SETTINGS_HG_Z_AXIS_Pos)
#define BNO_ACC_INT_SETTINGS_HG_Z_AXIS          BNO_ACC_INT_SETTINGS_HG_Z_AXIS_Msk

/************************** Bits definition for ACC_HG_DURATION register **************************/
#define BNO_ACC_HG_DURATION_Pos         (0U)
#define BNO_ACC_HG_DURATION_Msk         (0xFFU << BNO_ACC_HG_DURATION_Pos)
#define BNO_ACC_HG_DURATION             BNO_ACC_HG_DURATION_Msk

/**************************** Bits definition for ACC_HG_THRES register ***************************/
#define BNO_ACC_HG_THRES_Pos            (0U)
#define BNO_ACC_HG_THRES_Msk            (0xFFU << BNO_ACC_HG_THRES_Pos)
#define BNO_ACC_HG_THRES                BNO_ACC_HG_THRES_Msk

/**************************** Bits definition for ACC_NM_THRES register ***************************/
#define BNO_ACC_NM_THRES_Pos            (0U)
#define BNO_ACC_NM_THRES_Msk            (0xFFU << BNO_ACC_NM_THRES_Pos)
#define BNO_ACC_NM_THRES                BNO_ACC_NM_THRES_Msk

/***************************** Bits definition for ACC_NM_SET register ****************************/
#define BNO_ACC_NM_SET_SM_NM_Pos        (0U)
#define BNO_ACC_NM_SET_SM_NM_Msk        (0x01U << BNO_ACC_NM_SET_SM_NM_Pos)
#define BNO_ACC_NM_SET_SM_NM            BNO_ACC_NM_SET_SM_NM_Msk

#define BNO_ACC_NM_SET_SM_NM_DUR_Pos    (1U)
#define BNO_ACC_NM_SET_SM_NM_DUR_Msk    (0x3FU << BNO_ACC_NM_SET_SM_NM_DUR_Pos)
#define BNO_ACC_NM_SET_SM_NM_DUR        BNO_ACC_NM_SET_SM_NM_DUR_Msk
#define BNO_ACC_NM_SET_SM_ONLY_DUR      (0x03U << BNO_ACC_NM_SET_SM_NM_DUR_Pos)

/************************** Bits definition for GYR_INT_SETTINGS register *************************/
#define BNO_GYR_INT_SETTINGS_AM_X_AXIS_Pos  (0U)
#define BNO_GYR_INT_SETTINGS_AM_X_AXIS_Msk  (0x01U << BNO_GYR_INT_SETTINGS_AM_X_AXIS_Pos)
#define BNO_GYR_INT_SETTINGS_AM_X_AXIS      BNO_GYR_INT_SETTINGS_AM_X_AXIS_Msk

#define BNO_GYR_INT_SETTINGS_AM_Y_AXIS_Pos  (1U)
#define BNO_GYR_INT_SETTINGS_AM_Y_AXIS_Msk  (0x01U << BNO_GYR_INT_SETTINGS_AM_Y_AXIS_Pos)
#define BNO_GYR_INT_SETTINGS_AM_Y_AXIS      BNO_GYR_INT_SETTINGS_AM_Y_AXIS_Msk

#define BNO_GYR_INT_SETTINGS_AM_Z_AXIS_Pos  (2U)
#define BNO_GYR_INT_SETTINGS_AM_Z_AXIS_Msk  (0x01U << BNO_GYR_INT_SETTINGS_AM_Z_AXIS_Pos)
#define BNO_GYR_INT_SETTINGS_AM_Z_AXIS      BNO_GYR_INT_SETTINGS_AM_Z_AXIS_Msk

#define BNO_GYR_INT_SETTINGS_HR_X_AXIS_Pos  (3U)
#define BNO_GYR_INT_SETTINGS_HR_X_AXIS_Msk  (0x01U << BNO_GYR_INT_SETTINGS_HR_X_AXIS_Pos)
#define BNO_GYR_INT_SETTINGS_HR_X_AXIS      BNO_GYR_INT_SETTINGS_HR_X_AXIS_Msk

#define BNO_GYR_INT_SETTINGS_HR_Y_AXIS_Pos  (4U)
#define BNO_GYR_INT_SETTINGS_HR_Y_AXIS_Msk  (0x01U << BNO_GYR_INT_SETTINGS_HR_Y_AXIS_Pos)
#define BNO_GYR_INT_SETTINGS_HR_Y_AXIS      BNO_GYR_INT_SETTINGS_HR_Y_AXIS_Msk

#define BNO_GYR_INT_SETTINGS_HR_Z_AXIS_Pos  (5U)
#define BNO_GYR_INT_SETTINGS_HR_Z_AXIS_Msk  (0x01U << BNO_GYR_INT_SETTINGS_HR_Z_AXIS_Pos)
#define BNO_GYR_INT_SETTINGS_HR_Z_AXIS      BNO_GYR_INT_SETTINGS_HR_Z_AXIS_Msk

#define BNO_GYR_INT_SETTINGS_AM_FILTER_Pos  (6U)
#define BNO_GYR_INT_SETTINGS_AM_FILTER_Msk  (0x01U << BNO_GYR_INT_SETTINGS_AM_FILTER_Pos)
#define BNO_GYR_INT_SETTINGS_AM_FILTER      BNO_GYR_INT_SETTINGS_AM_FILTER_Msk

#define BNO_GYR_INT_SETTINGS_HR_FILTER_Pos  (7U)
#define BNO_GYR_INT_SETTINGS_HR_FILTER_Msk  (0x01U << BNO_GYR_INT_SETTINGS_HR_FILTER_Pos)
#define BNO_GYR_INT_SETTINGS_HR_FILTER      BNO_GYR_INT_SETTINGS_HR_FILTER_Msk

/**************************** Bits definition for GYR_HR_X_SET register ***************************/
#define BNO_GYR_HR_X_SET_THRES_Pos      (0U)
#define BNO_GYR_HR_X_SET_THRES_Msk      (0x1FU << BNO_GYR_HR_X_SET_THRES_Pos)
#define BNO_GYR_HR_X_SET_THRES          BNO_GYR_HR_X_SET_THRES_Msk

#define BNO_GYR_HR_X_SET_HYST_Pos       (5U)
#define BNO_GYR_HR_X_SET_HYST_Msk       (0x03U << BNO_GYR_HR_X_SET_HYST_Pos)
#define BNO_GYR_HR_X_SET_HYST           BNO_GYR_HR_X_SET_HYST_Msk

/***************************** Bits definition for GYR_DUR_X register *****************************/
#define BNO_GYR_DUR_X_Pos               (0U)
#define BNO_GYR_DUR_X_Msk               (0xFFU << BNO_GYR_DUR_X_Pos)
#define BNO_GYR_DUR_X                   BNO_GYR_DUR_X_Msk

/**************************** Bits definition for GYR_HR_Y_SET register ***************************/
#define BNO_GYR_HR_Y_SET_THRES_Pos      (0U)
#define BNO_GYR_HR_Y_SET_THRES_Msk      (0x1FU << BNO_GYR_HR_Y_SET_THRES_Pos)
#define BNO_GYR_HR_Y_SET_THRES          BNO_GYR_HR_Y_SET_THRES_Msk

#define BNO_GYR_HR_Y_SET_HYST_Pos       (5U)
#define BNO_GYR_HR_Y_SET_HYST_Msk       (0x03U << BNO_GYR_HR_Y_SET_HYST_Pos)
#define BNO_GYR_HR_Y_SET_HYST           BNO_GYR_HR_Y_SET_HYST_Msk

/***************************** Bits definition for GYR_DUR_Y register *****************************/
#define BNO_GYR_DUR_Y_Pos               (0U)
#define BNO_GYR_DUR_Y_Msk               (0xFFU << BNO_GYR_DUR_Y_Pos)
#define BNO_GYR_DUR_Y                   BNO_GYR_DUR_Y_Msk

/**************************** Bits definition for GYR_HR_Z_SET register ***************************/
#define BNO_GYR_HR_Z_SET_THRES_Pos      (0U)
#define BNO_GYR_HR_Z_SET_THRES_Msk      (0x01FU << BNO_GYR_HR_Z_SET_THRES_Pos)
#define BNO_GYR_HR_Z_SET_THRES          BNO_GYR_HR_Z_SET_THRES_Msk

#define BNO_GYR_HR_Z_SET_HYST_Pos       (5U)
#define BNO_GYR_HR_Z_SET_HYST_Msk       (0x03U << BNO_GYR_HR_Z_SET_HYST_Pos)
#define BNO_GYR_HR_Z_SET_HYST           BNO_GYR_HR_Z_SET_HYST_Msk

/***************************** Bits definition for GYR_DUR_Z register *****************************/
#define BNO_GYR_DUR_Z_Pos               (0U)
#define BNO_GYR_DUR_Z_Msk               (0xFFU << BNO_GYR_DUR_Z_Pos)
#define BNO_GYR_DUR_Z                   BNO_GYR_DUR_Z_Msk

/**************************** Bits definition for GYR_AM_THRES register ***************************/
#define BNO_GYR_AM_THRES_Pos            (0U)
#define BNO_GYR_AM_THRES_Msk            (0x7FU << BNO_GYR_AM_THRES_Pos)
#define BNO_GYR_AM_THRES                BNO_GYR_AM_THRES_Msk

/***************************** Bits definition for GYR_AM_SET register ****************************/
#define BNO_GYR_AM_SET_SLPE_SAMPLES_Pos (0U)
#define BNO_GYR_AM_SET_SLPE_SAMPLES_Msk (0x03U << BNO_GYR_AM_SET_SLPE_SAMPLES_Pos)
#define BNO_GYR_AM_SET_SLPE_SAMPLES     BNO_GYR_AM_SET_SLPE_SAMPLES_Msk

#define BNO_GYR_AM_SET_AWAKE_DUR_Pos    (2U)
#define BNO_GYR_AM_SET_AWAKE_DUR_Msk    (0x03U << BNO_GYR_AM_SET_AWAKE_DUR_Pos)
#define BNO_GYR_AM_SET_AWAKE_DUR        BNO_GYR_AM_SET_AWAKE_DUR_Msk


/**************************************************************************************************/
/*                                       Function Prototypes                                      */
/**************************************************************************************************/

/********************************** Register Read/Write Functions *********************************/
Status BNO_Read_Reg (USART_Config_t *usart, uint8_t reg, uint16_t length, uint8_t *data);
Status BNO_Write_Reg(USART_Config_t *usart, uint8_t reg, uint16_t length, uint8_t *data);

/*************************** Sensor and System Initialisation Functions ***************************/
Status BNO_Init    (USART_Config_t *usart, BNO_Config_t *bno_config);
Status BNO_ACC_Init(USART_Config_t *usart, BNO_ACC_Config_t *acc_config);
Status BNO_MAG_Init(USART_Config_t *usart, BNO_MAG_Config_t *mag_config);
Status BNO_GYR_Init(USART_Config_t *usart, BNO_GYR_Config_t *gyr_config);

Status BNO_Get_PWR_Mode(USART_Config_t *usart, uint8_t *current_pwr_mode);
Status BNO_Set_PWR_Mode(USART_Config_t *usart, BNO_PWR_Mode pwr_mode);
Status BNO_Get_OPR_Mode(USART_Config_t *usart, uint8_t *current_opr_mode);
Status BNO_Set_OPR_Mode(USART_Config_t *usart, BNO_OPR_Mode opr_mode);

/***************************** Sensor Settings Configuration Functions ****************************/
Status BNO_Set_ACC_Range   (USART_Config_t *usart, BNO_ACC_Range acc_range);
Status BNO_Get_ACC_Range   (USART_Config_t *usart, uint8_t *acc_range);
Status BNO_Set_ACC_BW      (USART_Config_t *usart, BNO_ACC_BW acc_bw);
Status BNO_Get_ACC_BW      (USART_Config_t *usart, uint8_t *acc_bw);
Status BNO_Set_ACC_PWR_Mode(USART_Config_t *usart, BNO_ACC_PWR_Mode acc_pwr_mode);
Status BNO_Get_ACC_PWR_Mode(USART_Config_t *usart, uint8_t *acc_pwr_mode);

Status BNO_Set_MAG_DOR     (USART_Config_t *usart, BNO_MAG_DOR mag_dor);
Status BNO_Get_MAG_DOR     (USART_Config_t *usart, uint8_t *mag_dor);
Status BNO_Set_MAG_OPR_Mode(USART_Config_t *usart, BNO_MAG_OPR_Mode mag_opr_mode);
Status BNO_Get_MAG_OPR_Mode(USART_Config_t *usart, uint8_t *mag_opr_mode);
Status BNO_Set_MAG_PWR_Mode(USART_Config_t *usart, BNO_MAG_PWR_Mode mag_pwr_mode);
Status BNO_Get_MAG_PWR_Mode(USART_Config_t *usart, uint8_t *mag_pwr_mode);

Status BNO_Set_GYR_Range   (USART_Config_t *usart, BNO_GYR_Range gyr_range);
Status BNO_Get_GYR_Range   (USART_Config_t *usart, uint8_t *gyr_range);
Status BNO_Set_GYR_BW      (USART_Config_t *usart, BNO_GYR_BW gyr_bw);
Status BNO_Get_GYR_BW      (USART_Config_t *usart, uint8_t *gyr_bw);
Status BNO_Set_GYR_PWR_Mode(USART_Config_t *usart, BNO_GYR_PWR_Mode gyr_pwr_mode);
Status BNO_Get_GYR_PWR_Mode(USART_Config_t *usart, uint8_t *gyr_pwr_mode);

/********************************* Sleep Configuration Structures *********************************/
Status BNO_ACC_Slp_Config  (USART_Config_t *usart, BNO_ACC_Slp_Config_t *slp_config);
Status BNO_Set_ACC_Slp_Mode(USART_Config_t *usart, BNO_ACC_Slp_Mode slp_mode);
Status BNO_Get_ACC_Slp_Mode(USART_Config_t *usart, uint8_t *slp_mode);
Status BNO_Set_ACC_Slp_Dur (USART_Config_t *usart, BNO_ACC_Slp_Dur slp_dur);
Status BNO_Get_ACC_Slp_Dur (USART_Config_t *usart, uint8_t *slp_dur);

Status BNO_GYR_Slp_Config      (USART_Config_t *usart, BNO_GYR_Slp_Config_t *slp_config);
Status BNO_Set_GYR_Slp_Dur     (USART_Config_t *usart, BNO_GYR_Slp_Dur slp_dur);
Status BNO_Get_GYR_Slp_Dur     (USART_Config_t *usart, uint8_t *slp_dur);
Status BNO_Set_GYR_Slp_Auto_Dur(USART_Config_t *usart, BNO_GYR_Slp_Auto_Dur auto_dur);
Status BNO_Get_GYR_Slp_Auto_Dur(USART_Config_t *usart, uint8_t *auto_dur);

/*************************************** Self-Test Functions **************************************/
Status BNO_Get_MCU_POST_Result(USART_Config_t *usart, uint8_t *result);
Status BNO_Get_ACC_POST_Result(USART_Config_t *usart, uint8_t *result);
Status BNO_Get_MAG_POST_Result(USART_Config_t *usart, uint8_t *result);
Status BNO_Get_GYR_POST_Result(USART_Config_t *usart, uint8_t *result);
Status BNO_Run_BIST           (USART_Config_t *usart, uint8_t *result);

/********************************** Sensor Calibration Functions **********************************/
Status BNO_Get_ACC_Offset(USART_Config_t *usart, BNO_Offset_t *acc_offset);
Status BNO_Set_ACC_Offset(USART_Config_t *usart, BNO_Offset_t *acc_offset);

Status BNO_Get_MAG_Offset(USART_Config_t *usart, BNO_Offset_t *mag_offset);
Status BNO_Set_MAG_Offset(USART_Config_t *usart, BNO_Offset_t *mag_offset);

Status BNO_Get_GYR_Offset(USART_Config_t *usart, BNO_Offset_t *gyr_offset);
Status BNO_Set_GYR_Offset(USART_Config_t *usart, BNO_Offset_t *gyr_offset);

Status BNO_Get_ACC_Radius(USART_Config_t *usart, BNO_Radius_t *acc_radius);
Status BNO_Set_MAG_Radius(USART_Config_t *usart, BNO_Radius_t *mag_radius);

Status BNO_Get_MAG_Radius(USART_Config_t *usart, BNO_Radius_t *mag_radius);
Status BNO_Set_ACC_Radius(USART_Config_t *usart, BNO_Radius_t *acc_radius);

Status BNO_Get_Calib_Profile     (USART_Config_t *usart, BNO_Calib_Profile_t *profile);
Status BNO_Set_Calib_Profile     (USART_Config_t *usart, BNO_Calib_Profile_t *profile);
Status BNO_Transmit_Calib_Profile(USART_Config_t *usart_term_config, BNO_Calib_Profile_t *profile);

Status BNO_Get_Sys_Calib_Status(USART_Config_t *usart, uint8_t *sys_calib_status);
Status BNO_Get_ACC_Calib_Status(USART_Config_t *usart, uint8_t *acc_calib_status);
Status BNO_Get_MAG_Calib_Status(USART_Config_t *usart, uint8_t *mag_calib_status);
Status BNO_Get_GYR_Calib_Status(USART_Config_t *usart, uint8_t *gyr_calib_status);

/************************** Sensor and Fusion Output Conversion Functions *************************/
Status BNO_Get_ACC_XYZ(USART_Config_t *usart, BNO_ODR_Float_t *acc_xyz_float);
Status BNO_Get_ACC_X  (USART_Config_t *usart, float *acc_x_float);
Status BNO_Get_ACC_Y  (USART_Config_t *usart, float *acc_y_float);
Status BNO_Get_ACC_Z  (USART_Config_t *usart, float *acc_z_float);

Status BNO_Get_MAG_XYZ(USART_Config_t *usart, BNO_ODR_Float_t *mag_xyz_float);
Status BNO_Get_MAG_X  (USART_Config_t *usart, float *mag_x_float);
Status BNO_Get_MAG_Y  (USART_Config_t *usart, float *mag_y_float);
Status BNO_Get_MAG_Z  (USART_Config_t *usart, float *mag_z_float);

Status BNO_Get_GYR_XYZ(USART_Config_t *usart, BNO_ODR_Float_t *gyr_xyz_float);
Status BNO_Get_GYR_X  (USART_Config_t *usart, float *gyr_x_float);
Status BNO_Get_GYR_Y  (USART_Config_t *usart, float *gyr_y_float);
Status BNO_Get_GYR_Z  (USART_Config_t *usart, float *gyr_z_float);

Status BNO_Get_EUL_HRP    (USART_Config_t *usart, BNO_ODR_Float_t *eul_hrp_float);
Status BNO_Get_EUL_Heading(USART_Config_t *usart, float *eul_heading_float);
Status BNO_Get_EUL_Roll   (USART_Config_t *usart, float *eul_roll_float);
Status BNO_Get_EUL_Pitch  (USART_Config_t *usart, float *eul_pitch_float);

Status BNO_Get_QUA_WXYZ(USART_Config_t *usart, BNO_QUA_Float_t *qua_wxyz_float);
Status BNO_Get_QUA_W   (USART_Config_t *usart, float *qua_w_float);
Status BNO_Get_QUA_X   (USART_Config_t *usart, float *qua_x_float);
Status BNO_Get_QUA_Y   (USART_Config_t *usart, float *qua_y_float);
Status BNO_Get_QUA_Z   (USART_Config_t *usart, float *qua_z_float);

Status BNO_Get_LIA_XYZ(USART_Config_t *usart, BNO_ODR_Float_t *lia_xyz_float);
Status BNO_Get_LIA_X  (USART_Config_t *usart, float *lia_x_float);
Status BNO_Get_LIA_Y  (USART_Config_t *usart, float *lia_y_float);
Status BNO_Get_LIA_Z  (USART_Config_t *usart, float *lia_z_float);

Status BNO_Get_GRV_XYZ(USART_Config_t *usart, BNO_ODR_Float_t *grv_xyz_float);
Status BNO_Get_GRV_X  (USART_Config_t *usart, float *grv_x_float);
Status BNO_Get_GRV_Y  (USART_Config_t *usart, float *grv_y_float);
Status BNO_Get_GRV_Z  (USART_Config_t *usart, float *grv_z_float);

Status BNO_Get_TEMP   (USART_Config_t *usart, float *temp_float);

/************************************ Unit Selection Functions ************************************/
Status BNO_Set_ACC_Unit (USART_Config_t *usart, BNO_Unit acc_unit);
Status BNO_Get_ACC_Unit (USART_Config_t *usart, uint8_t *acc_unit);

Status BNO_Set_GYR_Unit (USART_Config_t *usart, BNO_Unit gyr_unit);
Status BNO_Get_GYR_Unit (USART_Config_t *usart, uint8_t *gyr_unit);

Status BNO_Set_EUL_Unit (USART_Config_t *usart, BNO_Unit eul_unit);
Status BNO_Get_EUL_Unit (USART_Config_t *usart, uint8_t *eul_unit);

Status BNO_Set_TEMP_Unit(USART_Config_t *usart, BNO_Unit temp_unit);
Status BNO_Get_TEMP_Unit(USART_Config_t *usart, uint8_t *temp_unit);

Status BNO_Set_ORI_Unit (USART_Config_t *usart, BNO_Unit ori_unit);
Status BNO_Get_ORI_Unit (USART_Config_t *usart, uint8_t *ori_unit);

/************************************ Axis Sign Remap Functions ***********************************/
Status BNO_Axis_Remap     (USART_Config_t *usart, BNO_Axis target, BNO_Axis new_axis);
Status BNO_Axis_Sign_Remap(USART_Config_t *usart, BNO_Axis axis, BNO_Axis_Sign sign);

/*************************************** Interrupt Functions **************************************/
Status BNO_Enable_IRQ     (USART_Config_t *usart, BNO_IRQ irq);
Status BNO_Disable_IRQ    (USART_Config_t *usart, BNO_IRQ irq);
Status BNO_Reset_IRQ      (USART_Config_t *usart);
Status BNO_Get_IRQ_Status (USART_Config_t *usart, BNO_IRQ irq, uint8_t *status);
Status BNO_Enable_IRQ_Msk (USART_Config_t *usart, BNO_IRQ irq);
Status BNO_Disable_IRQ_Msk(USART_Config_t *usart, BNO_IRQ irq);

Status BNO_Set_ACC_SM_NM_Det_Type  (USART_Config_t *usart, BNO_SM_NM_Det_Type det_type);
Status BNO_Get_ACC_SM_NM_Det_Type  (USART_Config_t *usart, uint8_t *det_type);
Status BNO_Set_ACC_SM_NM_Thres     (USART_Config_t *usart, float thres_mg);
Status BNO_Get_ACC_SM_NM_Thres     (USART_Config_t *usart, float *thres_mg);
Status BNO_Set_ACC_SM_Slope_Points (USART_Config_t *usart, uint8_t slope_points);
Status BNO_Get_ACC_SM_Slope_Points (USART_Config_t *usart, uint8_t *slope_points);
Status BNO_Set_ACC_NM_Delay        (USART_Config_t *usart, uint16_t delay_s);
Status BNO_Get_ACC_NM_Delay        (USART_Config_t *usart, uint16_t *delay_s);
Status BNO_Set_ACC_SM_NM_Axis_State(USART_Config_t *usart, BNO_Axis axis, BNO_IRQ_Axis_State state);
Status BNO_Get_ACC_SM_NM_Axis_State(USART_Config_t *usart, BNO_Axis axis, uint8_t *state);

Status BNO_ACC_AM_Config          (USART_Config_t *usart, BNO_ACC_AM_Config_t *am_config);
Status BNO_Set_ACC_AM_Thres       (USART_Config_t *usart, float thres_mg);
Status BNO_Get_ACC_AM_Thres       (USART_Config_t *usart, float *thres_mg);
Status BNO_Set_ACC_AM_Slope_Points(USART_Config_t *usart, uint8_t slope_points);
Status BNO_Get_ACC_AM_Slope_Points(USART_Config_t *usart, uint8_t *slope_points);
Status BNO_Set_ACC_AM_Axis_State  (USART_Config_t *usart, BNO_Axis axis, BNO_IRQ_Axis_State state);
Status BNO_Get_ACC_AM_Axis_State  (USART_Config_t *usart, BNO_Axis axis, uint8_t *state);

Status BNO_ACC_HG_Config        (USART_Config_t *usart, BNO_ACC_HG_Config_t *hg_config);
Status BNO_Set_ACC_HG_Thres     (USART_Config_t *usart, float thres_mg);
Status BNO_Get_ACC_HG_Thres     (USART_Config_t *usart, float *thres_mg);
Status BNO_Set_ACC_HG_Dur       (USART_Config_t *usart, uint16_t delay_ms);
Status BNO_Get_ACC_HG_Dur       (USART_Config_t *usart, uint16_t *delay_ms);
Status BNO_Get_ACC_HG_Axis_State(USART_Config_t *usart, BNO_Axis axis, uint8_t *state);
Status BNO_Set_ACC_HG_Axis_State(USART_Config_t *usart, BNO_Axis axis, BNO_IRQ_Axis_State state);

Status BNO_GYR_HR_Config        (USART_Config_t *usart, BNO_GYR_HR_Config_t *hr_config);
Status BNO_Set_GYR_HR_Thres     (USART_Config_t *usart, BNO_Axis axis, float thres_dps);
Status BNO_Get_GYR_HR_Thres     (USART_Config_t *usart, BNO_Axis axis, float *thres_dps);
Status BNO_Set_GYR_HR_Hyst      (USART_Config_t *usart, BNO_Axis axis, float hyst_dps);
Status BNO_Get_GYR_HR_Hyst      (USART_Config_t *usart, BNO_Axis axis, float *hyst_dps);
Status BNO_Set_GYR_HR_Dur       (USART_Config_t *usart, BNO_Axis axis, uint16_t dur_ms);
Status BNO_Get_GYR_HR_Dur       (USART_Config_t *usart, BNO_Axis axis, uint16_t *dur_ms);
Status BNO_Set_GYR_HR_Filter    (USART_Config_t *usart, BNO_GYR_Filter filter);
Status BNO_Get_GYR_HR_Filter    (USART_Config_t *usart, uint8_t *filter);
Status BNO_Set_GYR_HR_Axis_State(USART_Config_t *usart, BNO_Axis axis, BNO_IRQ_Axis_State state);
Status BNO_Get_GYR_HR_Axis_State(USART_Config_t *usart, BNO_Axis axis, uint8_t *state);

Status BNO_GYR_AM_Config        (USART_Config_t *usart, BNO_GYR_AM_Config_t *am_config);
Status BNO_Set_GYR_AM_Thres     (USART_Config_t *usart, float thres_dps);
Status BNO_Get_GYR_AM_Thres     (USART_Config_t *usart, float *thres_dps);
Status BNO_Set_GYR_AM_Slpe_Samps(USART_Config_t *usart, uint8_t samples);
Status BNO_Get_GYR_AM_Slpe_Samps(USART_Config_t *usart, uint8_t *samples);
Status BNO_Set_GYR_AM_Awake_Dur (USART_Config_t *usart, BNO_GYR_Awake_Dur awake_dur);
Status BNO_Get_GYR_AM_Awake_Dur (USART_Config_t *usart, uint8_t *awake_dur);
Status BNO_Set_GYR_AM_Filter    (USART_Config_t *usart, BNO_GYR_Filter filter);
Status BNO_Get_GYR_AM_Filter    (USART_Config_t *usart, uint8_t *filter);
Status BNO_Set_GYR_AM_Axis_State(USART_Config_t *usart, BNO_Axis axis, BNO_IRQ_Axis_State state);
Status BNO_Get_GYR_AM_Axis_State(USART_Config_t *usart, BNO_Axis axis, uint8_t *state);







#ifdef __cplusplus
        }
#endif

#endif
