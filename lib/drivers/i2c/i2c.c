#include "i2c.h"


/**************************************************************************************************/
/*                            Global I2C State Structure Initialisation                           */
/**************************************************************************************************/

volatile I2C_State_t g_i2c_1 = {
    .instance  = NULL,
    .mode      = I2C_MODE_SLAVE,
    .op        = I2C_OP_TX,
    .tx_buffer = {0},
    .tx_length = 0U,
    .tx_index  = 0U,
    .rx_buffer = {0},
    .rx_length = 0U,
    .rx_index  = 0U
};

volatile I2C_State_t g_i2c_2 = {
    .instance  = NULL,
    .mode      = I2C_MODE_SLAVE,
    .op        = I2C_OP_TX,
    .tx_buffer = {0},
    .tx_length = 0U,
    .tx_index  = 0U,
    .rx_buffer = {0},
    .rx_length = 0U,
    .rx_index  = 0U
};

volatile I2C_State_t g_i2c_3 = {
    .instance  = NULL,
    .mode      = I2C_MODE_SLAVE,
    .op        = I2C_OP_TX,
    .tx_buffer = {0},
    .tx_length = 0U,
    .tx_index  = 0U,
    .rx_buffer = {0},
    .rx_length = 0U,
    .rx_index  = 0U
};


/**************************************************************************************************/
/*                                         Core Functions                                         */
/**************************************************************************************************/

Status I2C_Master_Init(I2C_Master_Config_t *config) {
    CHECK_STATUS(Validate_Ptr(config));
    if ((config->instance != I2C1) && (config->instance != I2C2) && (config->instance != I2C3)) {
        return INVALID_PARAM;
    }

    //enable I2C clock
    if (config->instance == I2C1) {
        RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    } else if (config->instance == I2C2) {
        RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
    } else if (config->instance == I2C3) {
        RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;
    }

    //configure configuration speeds - standard speed or fast speed

    //configure peripheral clock frequency
    if (g_apb1_clk_freq < 2000000U || g_apb1_clk_freq > 42000000U) {
        return ERROR;
    }
    if (config->speed == I2C_SPEED_FM && g_apb1_clk_freq < 4000000U) {
        return ERROR;
    }
    uint32_t apb1_clk_freq_mhz = (g_apb1_clk_freq / 1000000U);
    config->instance->CR2 &= ~(I2C_CR2_FREQ);
    config->instance->CR2 |= apb1_clk_freq_mhz;

    //configure clock control registers


    //configure clock stretching
    config->instance->CR1 &= ~(I2C_CR1_NOSTRETCH);
    config->instance->CR1 |= (((uint32_t) config->clock_stretch) << 7U);

    //configure interrupts
    if (config->event_irq_enable) {
        config->instance->CR2 |= I2C_CR2_ITEVTEN;
    }
    if (config->buffer_irq_enable) {
        config->instance->CR2 |= I2C_CR2_ITBUFEN;
    }
    if (config->error_irq_enable) {
        config->instance->CR2 |= I2C_CR2_ITERREN;
    }

    //configure acknowledge pulse
    config->instance->CR1 &= ~(I2C_CR1_ACK);
    config->instance->CR1 |= (((uint32_t) config->ack) << 10U);

    //enable peripheral
    config->instance->CR1 |= I2C_CR1_PE;

    return SUCCESS;
}


Status I2C_Slave_Init(I2C_Slave_Config_t *config) {
    CHECK_STATUS(Validate_Ptr(config));
    if ((config->instance != I2C1) && (config->instance != I2C2) && (config->instance != I2C3)) {
        return INVALID_PARAM;
    }

    //configure address based on slave communication operation
    config->instance->OAR1 &= ~(I2C_OAR1_ADD);
    if (config->op == I2C_OP_TX) {
        if (((config->slave_tx_addr) % 2U) != 0U) {
            config->instance->OAR1 |= (((uint32_t) config->slave_tx_addr));
        } else {
            return INVALID_PARAM;
        }
    } else {
        if (((config->slave_rx_addr) % 2U) == 0U) {
            config->instance->OAR1 |= (((uint32_t) config->slave_rx_addr));
        } else {
            return INVALID_PARAM;
        }
    }

    //enable peripheral
    config->instance->CR1 |= I2C_CR1_PE;
}


//this function has to be called before the corresponding I2C_Slave_Receive() if master and slave interfaces are in same MCU
Status I2C_Master_Transmit(I2C_Master_Config_t *master_config, I2C_Slave_Config_t *slave_config, uint8_t *tx_buffer, uint16_t *tx_length) {
    CHECK_STATUS(Validate_Ptr(master_config));
    CHECK_STATUS(Validate_Ptr(slave_config));
    if (tx_length <= 0U || tx_length > TX_BUFFER_SIZE) {
        return INVALID_PARAM;
    }

    //ensure that there is no communication currently on the bus
    if (master_config->instance->SR2 & I2C_SR2_BUSY) {
        return ERROR;
    }

    //generate start condition
    master_config->instance->CR1 |= I2C_CR1_START;

    //ensure start condition has been generated
    if (!(master_config->instance->SR1 & I2C_SR1_START_BIT)) {
        return ERROR;
    }

    //write slave address
    if (master_config->op == I2C_OP_TX) {
        master_config->instance->DR = ((uint32_t) slave_config->slave_rx_addr);
    } else {
        master_config->instance->DR = ((uint32_t) slave_config->slave_tx_addr);
    }

    //confirm that master is in transmitter mode
    if (!(master_config->instance->SR2 & I2C_SR2_TRA)) {
        return ERROR;
    }

    //select approriate global state given the I2C instance
    volatile I2C_State_t *current = NULL;
    CHECK_STATUS(I2C_Get_Master_State(master_config, &current));
    current->instance = master_config->instance;

    //initialise the global state
    for (int i = 0; (i < tx_length) && (i < TX_BUFFER_SIZE); i++) {
        current->tx_buffer[i] = tx_buffer[i];
    }
    current->tx_length = tx_length;
    current->tx_index  = 0U;

    //write the first byte
    master_config->instance->DR = tx_buffer[0];
    current->tx_index++;

    return SUCCESS;
}

//this function has to be called before the corresponding I2C_Slave_Transmit() if master and slave interfaces are in same MCU
Status I2C_Master_Receive(I2C_Master_Config_t *master_config, I2C_Slave_Config_t *slave_config, uint8_t *rx_buffer, uint16_t rx_length) {
    CHECK_STATUS(Validate_Ptr(master_config));
    CHECK_STATUS(Validate_Ptr(slave_config));
    if (rx_length <= 0U) {
        return INVALID_PARAM;
    }

    //generate start condition
    master_config->instance->CR1 |= I2C_CR1_START;

    //ensure start condition has been generated
    if (!(master_config->instance->SR1 & I2C_SR1_START_BIT)) {
        return ERROR;
    }

    //write slave address
    if (master_config->op == I2C_OP_TX) {
        master_config->instance->DR = ((uint32_t) slave_config->slave_rx_addr);
    } else {
        master_config->instance->DR = ((uint32_t) slave_config->slave_tx_addr);
    }

    //confirm that master is in receiver mode
    if (master_config->instance->SR2 & I2C_SR2_TRA) {
        return ERROR;
    }

    //select approriate global state given the I2C instance
    volatile I2C_State_t *current = NULL;
    CHECK_STATUS(I2C_Get_Master_State(master_config, &current));
    current->instance = master_config->instance;

    //initialise the global state
    current->rx_buffer = rx_buffer;
    current->rx_length = rx_length;
    current->rx_index  = 0U;

    return SUCCESS;
}


Status I2C_Slave_Transmit(I2C_Slave_Config_t *slave_config, uint8_t *tx_buffer, uint16_t tx_length) {
    CHECK_STATUS(Validate_Ptr(slave_config));
    if (tx_length <= 0U || tx_length > TX_BUFFER_SIZE) {
        return INVALID_PARAM;
    }

    //confirm that slave is in transmitter mode
    if (!(slave_config->instance->SR2 & I2C_SR2_TRA)) {
        return ERROR;
    }

    //select approriate global state given the I2C instance
    volatile I2C_State_t *current = NULL;
    CHECK_STATUS(I2C_Get_Slave_State(slave_config, &current));
    current->instance = slave_config->instance;

    //initialise the global state
    for (int i = 0; (i < tx_length) && (i < TX_BUFFER_SIZE); i++) {
        current->tx_buffer[i] = tx_buffer[i];
    }
    current->tx_length = tx_length;
    current->tx_index  = 0U;

    return SUCCESS;
}


Status I2C_Slave_Receive(I2C_Slave_Config_t *slave_config, uint8_t *rx_buffer, uint16_t rx_length) {
    CHECK_STATUS(Validate_Ptr(slave_config));
    if (rx_length <= 0U) {
        return INVALID_PARAM;
    }

    //confirm that slave is in receiver mode
    if (!(slave_config->instance->SR2 & I2C_SR2_TRA)) {
        return ERROR;
    }

    //select approriate global state given the I2C instance
    volatile I2C_State_t *current = NULL;
    CHECK_STATUS(I2C_Get_Slave_State(slave_config, &current));
    current->instance = slave_config->instance;

    //initialise the global state
    current->rx_buffer = rx_buffer;
    current->rx_length = rx_length;
    current->rx_index  = 0U;

    return SUCCESS;
}


Status I2C_Get_Master_State(I2C_Master_Config_t *master_config, volatile I2C_State_t **g_state) {
    CHECK_STATUS(Validate_Ptr(master_config));
    CHECK_STATUS(Validate_Ptr(g_state));

    //store appropriate global state
    if (master_config->instance == I2C1) {
        *g_state = &g_i2c_1;
    } else if (master_config->instance == I2C2) {
        *g_state = &g_i2c_2;
    } else if (master_config->instance == I2C3) {
        *g_state = &g_i2c_3;
    } else {
        return INVALID_PARAM;
    }

    return SUCCESS;
}


Status I2C_Get_Slave_State(I2C_Slave_Config_t *slave_config, volatile I2C_State_t **g_state) {
    CHECK_STATUS(Validate_Ptr(slave_config));
    CHECK_STATUS(Validate_Ptr(g_state));

    //store appropriate global state
    if (slave_config->instance == I2C1) {
        *g_state = &g_i2c_1;
    } else if (slave_config->instance == I2C2) {
        *g_state = &g_i2c_2;
    } else if (slave_config->instance == I2C3) {
        *g_state = &g_i2c_3;
    } else {
        return INVALID_PARAM;
    }

    return SUCCESS;
}


/**************************************************************************************************/
/*                                     I2C Interrupt Handlers                                     */
/**************************************************************************************************/


static void I2C_EV_Interrupt(volatile I2C_State_t *i2c) {
    CHECK_STATUS(Validate_Ptr(i2c));

    //handle master transmission
    if ((i2c->mode == I2C_MODE_MASTER) && (i2c->op == I2C_OP_TX)) {
        if (i2c->instance->SR1 & I2C_SR1_TXE) {
            if (i2c->tx_index < i2c->tx_length) {
                i2c->instance->DR = i2c->tx_buffer[i2c->tx_index++];
            }
            if (i2c->tx_index == i2c->tx_length) {
                i2c->instance->CR1 |= I2C_CR1_STOP;
            }
        }
    }
    //handle master reception
    else if ((i2c->mode == I2C_MODE_MASTER) && (i2c->op == I2C_OP_RX)) {
        if (i2c->instance->SR1 & I2C_SR1_RXNE) {
            if (i2c->rx_index < i2c->rx_length) {
                i2c->rx_buffer[i2c->rx_index++] = i2c->instance->DR;
            }
            //after transmission of second last byte, send NACK and stop condition
            if ((i2c->rx_index + 1 ) == i2c->rx_length) {
                i2c->instance->CR1 &= ~(I2C_CR1_ACK);
                i2c->instance->CR1 |= I2C_CR1_STOP;
            }
        }
    }
    //handle slave transmission
    else if ((i2c->mode == I2C_MODE_SLAVE) && (i2c->op == I2C_OP_TX)) {
        if ((i2c->instance->SR1 & I2C_SR1_TXE) && (i2c->instance->SR1 & I2C_SR1_BTF)) {
            //clear BTF by reading SR1 and writing to DR
            uint32_t sr1_temp = i2c->instance->SR1;
            (void) sr1_temp;
            if (i2c->tx_index < i2c->tx_length) {
                i2c->instance->DR = i2c->tx_buffer[i2c->tx_index++];
            }
        } else if (i2c->instance->SR1 & I2C_SR1_TXE) {
            if (i2c->tx_index < i2c->tx_length) {
                i2c->instance->DR = i2c->tx_buffer[i2c->tx_index++];
            }
        }
    } 
    //handle slave reception
    else if ((i2c->mode == I2C_MODE_SLAVE) && (i2c->op == I2C_OP_RX)) {
        if (i2c->instance->SR1 & I2C_SR1_RXNE) {
            i2c->rx_buffer[i2c->rx_index++] = i2c->instance->DR;
        }
        if (i2c->instance->SR1 & I2C_SR1_STOPF) {
            //clear stop bit by reading SR1 and writing to CR1
            uint32_t sr1_temp = i2c->instance->SR1;
            (void) sr1_temp;
            uint32_t cr1_temp  = i2c->instance->CR1;
            i2c->instance->CR1 = cr1_temp;
        }  
    }
    //handle general case of set ADDR bit following address transmission 
    else if (i2c->instance->SR1 & I2C_SR1_ADDR) {
        uint32_t sr1_temp = i2c->instance->SR1;
        uint32_t sr2_temp = i2c->instance->SR2;
        (void) sr1_temp;
        (void) sr2_temp;
    }
}


void I2C1_EV_IRQHandler(void) {
    I2C_EV_Interrupt(&g_i2c_1);
}

void I2C2_EV_IRQHandler(void) {
    I2C_EV_Interrupt(&g_i2c_2);
}

void I2C3_EV_IRQHandler(void) {
    I2C_EV_Interrupt(&g_i2c_3);
}
