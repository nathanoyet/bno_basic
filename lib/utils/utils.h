/**
 * @file    utils.h
 * @brief   STM32F411 Utility Functions and Variables
 * @details This header file contains utility enumerations, global variables, macros, function prototypes 
 *          and inline assembly instructions for STM32F411 applications.
 */


#ifndef __UTILS_H
#define __UTILS_H

#ifdef __cplusplus
    extern "C" {
#endif

#include "../../include/ext_periph_layer.h"
#include "../../include/int_periph_layer.h"
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

// #define LOGGING_ENABLED
#define LOGGING_USART       USART1               


/**************************************************************************************************/
/*                                         Macro Functions                                        */
/**************************************************************************************************/

//figure out how USART_Transmit_Log_Msg() can be passed the appropriate usart_init_config for logging


#define CHECK_STATUS(func_call) \
    do { \
        Status status_ret_val = (func_call); \
        if (status_ret_val != SUCCESS) { \
            return status_ret_val; \
        } \
    } while (0)

/**************************************************************************************************/
/*                                          Enumerations                                          */
/**************************************************************************************************/

typedef enum {
    SUCCESS = 0,
    ERROR,
    INVALID_PARAM
} Status;

typedef enum {
    BIT_SET = 0,
    BIT_RESET,
    BIT_ERROR,
    BIT_INVALID_PARAM
} Bit_State;

typedef enum {
    HSI_CLOCK = 0,
    HSE_CLOCK,
    PLL_CLOCK
} Sys_Clock_Source;

typedef enum {
    AHB_PRESCALER_1   = 1,
    AHB_PRESCALER_2   = 8,
    AHB_PRESCALER_4   = 9,
    AHB_PRESCALER_8   = 10,
    AHB_PRESCALER_16  = 11,
    AHB_PRESCALER_64  = 12,
    AHB_PRESCALER_128 = 13,
    AHB_PRESCALER_256 = 14,
    AHB_PRESCALER_512 = 15,
} AHB_Prescaler;

typedef enum {
    APB_PRESCALER_1  = 1,
    APB_PRESCALER_2  = 4,
    APB_PRESCALER_4  = 5,
    APB_PRESCALER_8  = 6,
    APB_PRESCALER_16 = 7
} APB_Prescaler;

typedef enum {
    PLL_P_DIVISOR_2 = 0,
    PLL_P_DIVISOR_4,
    PLL_P_DIVISOR_6,
    PLL_P_DIVISOR_8
} PLL_P_Divisor;

typedef enum {
    SYSTICK_UNIT_SEC = 0,
    SYSTICK_UNIT_MSEC,
    SYSTICK_UNIT_USEC
} Systick_Base_Unit;


/**************************************************************************************************/
/*                                        Global Constants                                        */
/**************************************************************************************************/

/****************************************** System Clock ******************************************/
static const uint32_t HSI_FREQ_HZ           = 16000000U;
static const uint32_t HSE_FREQ_HZ           = 25000000U;
static const uint32_t LSI_FREQ_HZ           = 32000U;
static const uint32_t LSE_FREQ_HZ           = 32768U;
static const uint32_t VCO_INPUT_FREQ_MIN_HZ = 1000000U;
static const uint32_t VCO_INPUT_FREQ_MAX_HZ = 2000000U;
static const uint32_t PLL_FREQ_MAX_HZ       = 100000000U;

/**************************************** Peripheral Clocks ***************************************/
static const uint32_t AHB_MAX_FREQ_HZ       = 100000000U;
static const uint32_t APB1_MAX_FREQ_HZ      = 50000000U;
static const uint32_t APB2_MAX_FREQ_HZ      = 100000000U;

/******************************************* System Time ******************************************/
static const uint32_t SEC_TO_MSEC           = 1000U;
static const uint32_t SEC_TO_USEC           = 1000000U;
static const uint32_t SEC_TO_NSEC           = 1000000000U;

/********************************************* Timers *********************************************/
static const uint32_t TIM1_CNT_VAL_MAX      = (0xFFFFUL);

/*********************************** Standard Bit Shift Divisors **********************************/
static const uint32_t DIV_BY_2              = 1U;
static const uint32_t DIV_BY_4              = 2U;
static const uint32_t DIV_BY_8              = 3U;
static const uint32_t DIV_BY_16             = 4U;
static const uint32_t DIV_BY_32             = 5U;
   

/**************************************************************************************************/
/*                                    Configuration Structures                                    */
/**************************************************************************************************/

typedef struct {
    Sys_Clock_Source clock_source;
    uint8_t          m_divisor;
    uint16_t         n_multiplier;
    PLL_P_Divisor    p_divisor;
} PLL_Config_t;

typedef struct {
    /* Required */
    Sys_Clock_Source clk_source;
    /* Optional */
    AHB_Prescaler    ahb_prescaler;
    APB_Prescaler    apb1_prescaler;
    APB_Prescaler    apb2_prescaler;
    PLL_Config_t     *pll_config;
} Clock_Config_t;



/**************************************************************************************************/
/*                                         Constant Macros                                        */
/**************************************************************************************************/

/*************************************** Standard Bit Masks ***************************************/
#define SET_ONE                     (0x01UL)
#define SET_TWO                     (0x03UL)
#define SET_THREE                   (0x07UL)
#define SET_FOUR                    (0x0FUL)
#define SET_FIVE                    (0x1FUL)
#define SET_SIX                     (0x3FUL)
#define SET_SEVEN                   (0x7FUL)
#define SET_EIGHT                   (0xFFUL)
#define SET_32                      (0xFFFFFFFFUL)

/********************************************* General ********************************************/
#define NVIC_PRIORITY_BITS          4U
#define CLEAR_REGISTER              0UL
#define WORD_SIZE                   32U


/**************************************************************************************************/
/*                                 Uninitialised Global Variables                                 */
/**************************************************************************************************/

/****************************************** System Clock ******************************************/
Sys_Clock_Source  g_sys_clk_source;
uint32_t          g_sys_clk_freq;
volatile uint32_t g_systick_time;

/**************************************** Peripheral Clocks ***************************************/
uint32_t g_ahb_clk_freq;
uint32_t g_apb1_clk_freq;
uint32_t g_apb2_clk_freq;

/******************************************* Interrupts *******************************************/
extern uint8_t irq_priority_tracker[256];


/**************************************************************************************************/
/*                                       Function Prototypes                                      */
/**************************************************************************************************/

/************************************* Reset and Clock Control ************************************/
void   Peripheral_Reset (void);
Status Clock_Init       (Clock_Config_t *clk_config);
Status PLL_Clock_Init   (PLL_Config_t *pll_config);
Status AHB_Clock_Config (AHB_Prescaler ahb_prescaler);
Status APB1_Clock_Config(APB_Prescaler apb1_prescaler);
Status APB2_Clock_Config(APB_Prescaler apb2_prescaler);
Status Systick_Init     (Systick_Base_Unit unit);
Status Systick_Delay    (uint32_t time_delay);
void   Delay_Loop       (uint32_t delay_duration_ms);

/****************************** Nested Vectored Interrupt Controller ******************************/
Status   NVIC_Enable_IRQ       (IRQn_t IRQn);
Status   NVIC_Disable_IRQ      (IRQn_t IRQn);
uint32_t NVIC_Get_Enable_IRQ   (IRQn_t IRQn);
Status   NVIC_Set_Pending_IRQ  (IRQn_t IRQn);
Status   NVIC_Clear_Pending_IRQ(IRQn_t IRQn);
uint32_t NVIC_Get_Pending_IRQ  (IRQn_t IRQn);
uint32_t NVIC_Get_Active_IRQ   (IRQn_t IRQn);
Status   NVIC_Set_Priority     (IRQn_t IRQn, uint32_t priority);
uint32_t NVIC_Get_Priority     (IRQn_t IRQn);
Status   Validate_Priority_IRQ (uint32_t priority);

void Append_Float_To_String(char *message, size_t message_size, float data);


/**************************************************************************************************/
/*                                   Inline Validation Functions                                  */
/**************************************************************************************************/

/**
 * @brief  Validates that an integer value is an enumerator
 * @param  value: Integer value to be validated
 * @param  min:   The lowest value enumerator in the enumeration
 * @param  max:   The highest value enumerator in the enumeration
 * @retval Status indicating success if the value is in the enum range; otherwise, returns invalid 
 *         param
 */
static inline Status Validate_Enum(const int value, int min, int max) {
    if (value >= min && value <= max) {
        return SUCCESS;
    } else {
        return INVALID_PARAM;
    }
}

/**
 * @brief  Validates a pointer
 * @param  ptr: Pointer to be validated
 * @retval Status indicating success if the pointer is valid; otherwise, returns invalid parameter
 */
static inline Status Validate_Ptr(const void *ptr) {
    if (ptr == NULL) {
        return INVALID_PARAM;
    } else {
        return SUCCESS;
    }
}

/**
 * @brief  Validates that an integer value can be stored in a uint8_t
 * @param  value: Integer value to be validated
 * @retval Status indicating success or invalid parameters
 */
static inline Status Validate_uint8_t(const int value) {
    if (value < 0U || value > 0xFFU) {
        return INVALID_PARAM;
    } else {
        return SUCCESS;
    }
}

/**
 * @brief  Validates that an integer value can be stored in a uint16_t
 * @param  value: Integer value to be validated
 * @retval Status indicating success or invalid parameters
 */
static inline Status Validate_uint16_t(const int value) {
    if (value < 0U || value > 0xFFFFU) {
        return INVALID_PARAM;
    } else {
        return SUCCESS;
    }
}


/**************************************************************************************************/
/*                                    Inline Assembly Functions                                   */
/**************************************************************************************************/

static inline __attribute__((always_inline)) void NOP(void) {
    __asm__ volatile("nop");
}

static inline __attribute__((always_inline)) void WFI(void) {
    __asm__ volatile("wfi");
}

static inline __attribute__((always_inline)) void DSB(void) {
    __asm__ volatile("dsb":::"memory");
}

static inline __attribute__((always_inline)) void ENABLE_IRQ(void) {
    __asm__ volatile("cpsie i":::"memory");
}

static inline __attribute__((always_inline)) void DISABLE_IRQ(void) {
    __asm__ volatile("cpsid i":::"memory");
}




#ifdef __cplusplus
    }
#endif

#endif