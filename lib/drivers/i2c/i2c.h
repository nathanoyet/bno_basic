

#ifndef __I2C_H
#define __I2C_H

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
    I2C_MODE_SLAVE,
    I2C_MODE_MASTER
} I2C_Mode;

typedef enum {
    I2C_SPEED_SM,
    I2C_SPEED_FM
} I2C_Speed;

typedef enum {
    I2C_OP_TX,
    I2C_OP_RX
} I2C_Operation;

typedef enum {
    I2C_SLAVE_TX_ADDR_1 = 1,
    I2C_SLAVE_TX_ADDR_2 = 3,
    I2C_SLAVE_TX_ADDR_3 = 5,
    I2C_SLAVE_TX_ADDR_4 = 7
} I2C_Slave_TX_Addr;

typedef enum {
    I2C_SLAVE_RX_ADDR_1 = 2,
    I2C_SLAVE_RX_ADDR_2 = 4,
    I2C_SLAVE_RX_ADDR_3 = 6,
    I2C_SLAVE_RX_ADDR_4 = 8
} I2C_Slave_RX_Addr;

typedef enum {
    I2C_IRQ_DIS,
    I2C_IRQ_EN
} I2C_Interrupt;

typedef enum {
    I2C_ACK_DIS,
    I2C_ACK_EN
} I2C_ACK;

typedef enum {
    I2C_CLOCK_STRETCH_DIS,
    I2C_CLOCK_STRETCH_EN
} I2C_Clock_Stretch;


/**************************************************************************************************/
/*                                    Configuration Structures                                    */
/**************************************************************************************************/

typedef struct {
    I2C_t               *instance;
    I2C_Operation       op;    
    I2C_ACK             ack;
    I2C_Interrupt       event_irq_enable;
    I2C_Interrupt       buffer_irq_enable;
    I2C_Interrupt       error_irq_enable;
    I2C_Speed           speed;
    I2C_Clock_Stretch   clock_stretch;
} I2C_Master_Config_t;

typedef struct {
    I2C_t               *instance;
    I2C_Operation       op;
    I2C_Slave_TX_Addr   slave_tx_addr;
    I2C_Slave_RX_Addr   slave_rx_addr;
} I2C_Slave_Config_t;

typedef struct {
    I2C_t         *instance;
    I2C_Mode      mode;
    I2C_Operation op;
    uint8_t       tx_buffer[TX_BUFFER_SIZE];
    uint16_t      tx_length;
    uint16_t      tx_index;
    uint8_t       *rx_buffer;
    uint16_t      rx_length;
    uint16_t      rx_index;
} I2C_State_t;

extern volatile I2C_State_t g_i2c_1;
extern volatile I2C_State_t g_i2c_2;
extern volatile I2C_State_t g_i2c_3;


/**************************************************************************************************/
/*                                       Function Prototypes                                      */
/**************************************************************************************************/

Status I2C_Get_Master_State(I2C_Master_Config_t *master_config, volatile I2C_State_t **g_state);
Status I2C_Get_Slave_State (I2C_Slave_Config_t *slave_config, volatile I2C_State_t **g_state);


#ifdef __cplusplus
    }
#endif

#endif
