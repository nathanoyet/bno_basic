/**
 * @file    usart.h
 * @brief   STM32F411 USART Driver
 * @details This header file contains the public interface for the STM32F411 USART driver. It 
 *          includes constants, enumerations, configuration structures and function prototypes for 
 *          USART communication. 
 */


#ifndef __USART_H
#define __USART_H

#ifdef __cplusplus
    extern "C" {
#endif


#include "../../utils/utils.h"
#include "../gpio/gpio.h"


/**************************************************************************************************/
/*                                         Constant Macros                                        */
/**************************************************************************************************/

#define TX_BUFFER_SIZE              512
#define RX_BUFFER_SIZE              512


/**************************************************************************************************/
/*                                          Enumerations                                          */
/**************************************************************************************************/

typedef enum {
    USART1_Idx = 0,
    USART2_Idx,
    USART6_Idx,
    USART_Idx_Error
} USART_Idx;

typedef enum {
    USART_DATA_8 = 0,
    USART_DATA_9
} USART_Word_Length;

typedef enum {
    USART_OVER_16 = 0,
    USART_OVER_8
} USART_Oversampling;

typedef enum {
    USART_STOP_1 = 0,
    USART_STOP_0_5,
    USART_STOP_2,
} USART_Stop_Bits;

typedef enum {
    USART_ONEBIT_3 = 0,
    USART_ONEBIT_1
} USART_One_Bit;

typedef enum {
    USART_PARITY_DIS = 0,
    USART_PARITY_EN
} USART_Parity_Control;

typedef enum {
    USART_EVEN_PARITY = 0,
    USART_ODD_PARITY
} USART_Parity_Selection;

typedef enum {
    USART_IRQ_DISABLED = 0,
    USART_IRQ_ENABLED
} USART_Interrupt;

typedef enum {
    USART_TX_IDLE = 0,
    USART_TX_BUSY
} USART_TX_Status;

typedef enum {
    USART_RX_IDLE,
    USART_RX_BUSY
} USART_RX_Status;

typedef enum {
    USART_RX_ERROR_NONE = 0,
    USART_RX_ERROR_OVERRUN,
    USART_RX_ERROR_FRAMING,
    USART_RX_ERROR_NOISE,
    USART_RX_ERROR_PARITY
} USART_RX_Error;


/**************************************************************************************************/
/*                                    Configuration Structures                                    */
/**************************************************************************************************/

typedef struct {
    /* Required */
    USART_t                *instance;
    uint32_t               baud_rate;
    uint32_t               irq_priority;
    /* Optional */
    USART_One_Bit          one_bit;
    USART_Word_Length      word_length;
    USART_Oversampling     oversampling;
    USART_Stop_Bits        stop_bits;
    USART_Parity_Control   parity_control;
    USART_Parity_Selection parity_selection;
    USART_Interrupt        pe_irq_enable;
    USART_Interrupt        idle_irq_enable;
    USART_Interrupt        cts_irq_enable;
    USART_Interrupt        error_irq_enable;
    USART_Interrupt        lbd_irq_enable;
} USART_Config_t;

typedef struct {
    /* Required */
    USART_t         *tx_instance;
    uint8_t         tx_buffer[TX_BUFFER_SIZE];
    uint16_t        tx_length;
    uint16_t        tx_index;
    USART_TX_Status tx_status;
    USART_t         *rx_instance;
    uint8_t         *rx_buffer;
    uint16_t        rx_length;
    uint16_t        rx_index;
    USART_RX_Status rx_status;
} USART_State_t;

extern volatile USART_State_t g_usart_1;
extern volatile USART_State_t g_usart_2;
extern volatile USART_State_t g_usart_6;


/**************************************************************************************************/
/*                                       Function Prototypes                                      */
/**************************************************************************************************/

Status USART_Init             (USART_Config_t *init_config);
Status USART_Deinit           (USART_t *instance);
Status USART_Transmit_IRQ     (USART_Config_t *init_config, uint8_t *tx_buffer, uint16_t tx_length);
Status USART_Receive_IRQ      (USART_Config_t *init_config, uint8_t *rx_buffer, uint16_t rx_length);
Status USART_Abort_Receive_IRQ(USART_Config_t *init_config);
Status USART_Transmit_Block   (
    USART_Config_t *init_config, 
    uint8_t        *tx_buffer, 
    uint16_t       tx_length, 
    float          timeout_ms
);
Status USART_Receive_Block    (
    USART_Config_t *init_config, 
    uint8_t        *rx_buffer, 
    uint16_t       rx_length, 
    float          timeout_ms
);
Status USART_Calc_Timeout     (
    USART_Config_t *init_config, 
    float          *timeout_ms, 
    float          margin, 
    uint16_t       length
);
Status USART_Get_State        (USART_Config_t *init_config, volatile USART_State_t **global_state);
Status USART_Transmit_Log_Msg (USART_Config_t *init_config, uint8_t *log_msg);
void   USART1_IRQHandler      (void);
void   USART2_IRQHandler      (void);
void   USART6_IRQHandler      (void);




#ifdef __cplusplus
    }
#endif

#endif