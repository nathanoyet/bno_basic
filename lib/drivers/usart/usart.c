/**
 * @file    usart.c
 * @brief   STM32F411 USART Driver
 * @details This driver provides an interface for the STM32F411 USART peripheral, including instance
 *          initialisation/deinitialisation, data transmission/reception, and interrupt handling. 
 *          The driver also maintains global state structures for each USART instance to support 
 *          interrupt-driven communication. 
 * 
 * @par     Driver functions:
 *          - USART_Init(): Initialises a USART instance
 *          - USART_Deinit(): Deinitialises USART instance
 *          - USART_Transmit_IRQ(): Transmits an array/string of bytes via USART using interrupts
 *          - USART_Receive_IRQ(): Receives bytes via USART using interrupts
 *          - USART_Abort_Receive_IRQ(): Aborts data reception using interrupts
 *          - USART_Tranmsit_Block(): Transmits an array/string of bytes via USART using blocking
 *          - USART_Receive_Block(): Receives bytes via USART using blocking
 *          - USART_Calc_Timeout(): Calculates an automatic timeout for interrupt-based USART TX/RX
 *          - USART_Get_State(): Stores the address of a specific global USART state in a pointer
 *          - USART_IRQHandler(): Generalised USART interrupt handler based on global USART state
 *          - USART1_IRQHandler(): Handles USART1 interrupts
 *          - USART2_IRQHandler(): Handles USART2 interrupts
 *          - USART6_IRQHandler(): Handles USART6 interrupts
 * 
 * @warning Ensure GPIO pins are configured before calling USART_Init()
 */


#include "usart.h"


/**************************************************************************************************/
/*                           Global USART State Structure Initialisation                          */
/**************************************************************************************************/

/** @brief Initialisation of structure used to store USART1 global state */
volatile USART_State_t g_usart_1 = {
    .tx_instance = NULL,
    .tx_buffer   = {0},
    .tx_length   = 0U,
    .tx_index    = 0U,
    .tx_status   = USART_TX_IDLE,
    .rx_instance = NULL,
    .rx_buffer   = NULL,
    .rx_length   = 0U,
    .rx_index    = 0U,
    .rx_status   = USART_RX_IDLE
};

/** @brief Initialisation of structure used to store USART2 global state */
volatile USART_State_t g_usart_2 = {
    .tx_instance = NULL,
    .tx_buffer   = {0},
    .tx_length   = 0U,
    .tx_index    = 0U,
    .tx_status   = USART_TX_IDLE,
    .rx_instance = NULL,
    .rx_buffer   = NULL,
    .rx_length   = 0U,
    .rx_index    = 0U,
    .rx_status   = USART_RX_IDLE
};

/** @brief Initialisation of structure used to store USART6 global state */
volatile USART_State_t g_usart_6 = {
    .tx_instance = NULL,
    .tx_buffer   = {0},
    .tx_length   = 0U,
    .tx_index    = 0U,
    .tx_status   = USART_TX_IDLE,
    .rx_instance = NULL,
    .rx_buffer   = NULL,
    .rx_length   = 0U,
    .rx_index    = 0U,
    .rx_status   = USART_RX_IDLE
};


/**************************************************************************************************/
/*                                         Core Functions                                         */
/**************************************************************************************************/

/**
 * @brief  Initialises a USART instance
 * @param  init_config: Pointer to a struct containing USART settings
 * @retval Status indicating success or invalid parameters
 * @note   Relevant GPIO pins should be configured prior to USART being initialised 
 */
Status USART_Init(USART_Config_t *init_config) {
    if ((init_config->instance != USART1) && (init_config->instance != USART2)
    &&  (init_config->instance != USART6)) {
        return INVALID_PARAM;
    }

    CHECK_STATUS(Validate_Enum(init_config->word_length, USART_DATA_8, USART_DATA_9));
    CHECK_STATUS(Validate_Enum(init_config->oversampling, USART_OVER_16, USART_OVER_8));
    CHECK_STATUS(Validate_Enum(init_config->stop_bits, USART_STOP_1, USART_STOP_2));
    CHECK_STATUS(Validate_Enum(init_config->one_bit, USART_ONEBIT_3, USART_ONEBIT_1));
    CHECK_STATUS(Validate_Enum(init_config->parity_control, USART_PARITY_DIS, USART_PARITY_EN));
    CHECK_STATUS(Validate_Enum(init_config->parity_selection, USART_EVEN_PARITY, USART_ODD_PARITY));

    CHECK_STATUS(Validate_Priority_IRQ(init_config->irq_priority));

    if (init_config->baud_rate < 0U) {
        return INVALID_PARAM;
    }

    //validate availability of interrupt priority level
    if (irq_priority_tracker[init_config->irq_priority]) {
        return INVALID_PARAM;
    }

    //enable USART clock
    if (init_config->instance == USART1) {
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    } else if (init_config->instance == USART2) {
        RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    } else if (init_config->instance == USART6) {
        RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
    }

    //enable USART
    init_config->instance->CR1 |= USART_CR1_UE;

    //calculate and configure USART_DIV
    uint8_t over;
    if (init_config->oversampling) {
        over = 8U;
    } else {
        over = 16U;
    }
    float usart_div = (((float) g_sys_clk_freq) / ((float) (init_config->baud_rate * over)));
    uint16_t mantissa = ((uint16_t) usart_div);
    if (mantissa > 0xFFFU) {
        return INVALID_PARAM;
    }
    uint8_t fraction = round((usart_div - mantissa) * over);
    mantissa = (mantissa << 4U);

    if (over == 8U && fraction == 8U) {
        mantissa += (0x1U << 4U);
        fraction = 0U;
    } else if (over == 16U && fraction == 16U) {
        mantissa += (0x1U << 4U);
        fraction = 0U;
    }

    uint16_t brr_val = (mantissa + fraction);
    init_config->instance->BRR = brr_val;

    //configure other usart settings
    init_config->instance->CR1 &= ~(USART_CR1_M);
    init_config->instance->CR1 |= (((uint32_t) init_config->word_length) << 12U);

    init_config->instance->CR1 &= ~(USART_CR1_OVER8);
    init_config->instance->CR1 |= (((uint32_t) init_config->oversampling) << 15U);

    init_config->instance->CR2 &= ~(USART_CR2_STOP);
    init_config->instance->CR2 |= (((uint32_t) init_config->stop_bits) << 12U);

    init_config->instance->CR3 &= ~(USART_CR3_ONEBIT);
    init_config->instance->CR3 |= (((uint32_t) init_config->one_bit) << 11U);

    init_config->instance->CR1 &= ~(USART_CR1_PCE);
    init_config->instance->CR1 |= (((uint32_t) init_config->parity_control) << 10U);
    init_config->instance->CR1 &= ~(USART_CR1_PS);
    init_config->instance->CR1 |= (((uint32_t) init_config->parity_selection) << 9U);

    //configure interrupts
    if (init_config->pe_irq_enable) {
        init_config->instance->CR1 |= USART_CR1_PEIE;
    }
    if (init_config->idle_irq_enable) {
        init_config->instance->CR1 |= USART_CR1_IDLEIE;
    }
    if (init_config->cts_irq_enable) {
        init_config->instance->CR3 |= USART_CR3_CTSIE;
    }
    if (init_config->error_irq_enable) {
        init_config->instance->CR3 |= USART_CR3_EIE;
    }
    if (init_config->lbd_irq_enable) {
        init_config->instance->CR2 |= USART_CR2_LBDIE;
    }

    DISABLE_IRQ();
    if (init_config->instance == USART1) {
        NVIC_Enable_IRQ(USART1_IRQn);
        NVIC_Set_Priority(USART1_IRQn, init_config->irq_priority);
    } else if (init_config->instance == USART2) {
        NVIC_Set_Priority(USART2_IRQn, init_config->irq_priority);
        NVIC_Enable_IRQ(USART2_IRQn);
    } else {
        NVIC_Set_Priority(USART6_IRQn, init_config->irq_priority);
        NVIC_Enable_IRQ(USART6_IRQn);
    }
    ENABLE_IRQ();
    
    //record utilised interrupt priority level
    irq_priority_tracker[init_config->irq_priority] = 1U;

    //enable transmitter and receiver
    init_config->instance->CR1 |= USART_CR1_TE;
    init_config->instance->CR1 |= USART_CR1_RE;

    return SUCCESS;
}

/**
 * @brief  Deinitialises USART instance
 * @param  instance: USART instance to be deinitialised
 * @retval Status indicating success, invalid parameters or error
 */
Status USART_Deinit(USART_t *instance) {
    //validate instance
    if (instance != USART1 && instance != USART2 && instance != USART6) {
        return INVALID_PARAM;
    }

    //check that any transmissions conducted are complete
    if (!(instance->SR & USART_SR_TC)) {
        return ERROR;
    }

    //check that any data in RDR has been read
    if (instance->SR & USART_SR_RXNE) {
        return ERROR;
    }
    
    if (instance == USART1) {
        //disable USART1 interrupts
        USART1->CR1 &= ~(
            USART_CR1_PEIE | USART_CR1_TXEIE | USART_CR1_TCIE | USART_CR1_RXNEIE | USART_CR1_IDLEIE
        );
        USART1->CR2 &= ~(USART_CR2_LBDIE);
        USART1->CR3 &= ~(USART_CR3_EIE | USART_CR3_CTSIE);

        //clear pending interrupts and disable USART1 interrupts in NVIC
        NVIC_Clear_Pending_IRQ(USART1_IRQn);
        NVIC_Disable_IRQ(USART1_IRQn);

        //disable USART1
        USART1->CR1 &= ~(USART_CR1_UE);

        //set and clear reset bit
        RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
        RCC->APB2RSTR &= ~(RCC_APB2RSTR_USART1RST);

        //disable USART1 clock
        RCC->APB2ENR &= ~(RCC_APB2ENR_USART1EN);
    } else if (instance == USART2) {
        //disable USART2 interrupts
        USART2->CR1 &= ~(
            USART_CR1_PEIE | USART_CR1_TXEIE | USART_CR1_TCIE | USART_CR1_RXNEIE | USART_CR1_IDLEIE
        );
        USART2->CR2 &= ~(USART_CR2_LBDIE);
        USART2->CR3 &= ~(USART_CR3_EIE | USART_CR3_CTSIE);

        //clear pending interrupts and disable USART2 interrupts in NVIC
        NVIC_Clear_Pending_IRQ(USART2_IRQn);
        NVIC_Disable_IRQ(USART2_IRQn);

        //disable USART2
        USART2->CR1 &= ~(USART_CR1_UE);

        //set and clear reset bit
        RCC->APB1RSTR |= RCC_APB1RSTR_USART2RST;
        RCC->APB1RSTR &= ~(RCC_APB1RSTR_USART2RST);

        //disable USART2 clock
        RCC->APB1ENR &= ~(RCC_APB1ENR_USART2EN);
    } else {
        //disable USART6 interrupts
        USART6->CR1 &= ~(
            USART_CR1_PEIE | USART_CR1_TXEIE | USART_CR1_TCIE | USART_CR1_RXNEIE | USART_CR1_IDLEIE
        );
        USART6->CR2 &= ~(USART_CR2_LBDIE);
        USART6->CR3 &= ~(USART_CR3_EIE | USART_CR3_CTSIE);

        //clear pending interrupts and disable USART6 interrupts in NVIC
        NVIC_Clear_Pending_IRQ(USART6_IRQn);
        NVIC_Disable_IRQ(USART6_IRQn);

        //disable USART6
        USART6->CR1 &= ~(USART_CR1_UE);

        //set and clear reset bit
        RCC->APB2RSTR |= RCC_APB2RSTR_USART6RST;
        RCC->APB2RSTR &= ~(RCC_APB2RSTR_USART6RST);

        //disable USART6 clock
        RCC->APB2ENR &= ~(RCC_APB2ENR_USART6EN);
    }

    return SUCCESS;
}

/**
 * @brief  Transmits an array/string of bytes via USART using interrupts
 * @param  init_config: Pointer to a struct containing USART settings
 * @param  tx_buffer:   Pointer to array/string that contains bytes to be transmitted
 * @param  tx_length:   Number of bytes to be transmitted
 * @retval Status indicating success, invalid parameters or error
 * @note   If tx_buffer is a string, tx_length = strlen((char *) string) + 1. The one is added to
 *         account for \0. 
 * @note   If tx_buffer is an array, tx_length = sizeof(array) / sizeof(array[0])
 * @note   Assumes USART has been initialised via @ref USART_Init
 */
Status USART_Transmit_IRQ(USART_Config_t *init_config, uint8_t *tx_buffer, uint16_t tx_length) {
    CHECK_STATUS(Validate_Ptr(init_config));
    CHECK_STATUS(Validate_Ptr(tx_buffer));
    if (tx_length <= 0U || tx_length > TX_BUFFER_SIZE) {
        return INVALID_PARAM;
    }  

    //select appropriate global state given the USART instance
    volatile USART_State_t *current = NULL;
    CHECK_STATUS(USART_Get_State(init_config, &current));
    current->tx_instance = init_config->instance;

    // check if USART is currently transmitting
    if (current->tx_status == USART_TX_BUSY) {
        return ERROR;
    }

    //initialise the global state
    for (int i = 0; (i < tx_length) && (i < TX_BUFFER_SIZE); i++) {
        current->tx_buffer[i] = tx_buffer[i];
    }
    current->tx_length = tx_length;
    current->tx_index  = 0U;
    current->tx_status = USART_TX_BUSY;

    //enable TXE interrupts
    init_config->instance->CR1 |= USART_CR1_TXEIE;

    return SUCCESS;
}

/**
 * @brief  Receives bytes via USART using interrupts
 * @param  init_config: Pointer to a struct containing USART settings
 * @param  rx_buffer:   Pointer to array that used to store received bytes
 * @param  rx_length:   Number of bytes to be received
 * @retval Status indicating success, invalid parameters or error
 * @note   if the number of bytes to be received is not known, rx_length = RX_BUFFER_SIZE
 * @note   Assumes USART has been initialised via @ref USART_Init
 */
Status USART_Receive_IRQ(USART_Config_t *init_config, uint8_t *rx_buffer, uint16_t rx_length) {
    CHECK_STATUS(Validate_Ptr(init_config));
    CHECK_STATUS(Validate_Ptr(rx_buffer));
    if (rx_length <= 0U) {
        return INVALID_PARAM;
    }

    //select appropriate global state given the USART instance
    volatile USART_State_t *current = NULL;
    CHECK_STATUS(USART_Get_State(init_config, &current));
    current->rx_instance = init_config->instance;

    //check if USART is currently receiving
    if (current->rx_status == USART_RX_BUSY) {
        return ERROR;
    }

    //initialise the global state
    current->rx_buffer = rx_buffer;
    current->rx_length = rx_length;
    current->rx_index  = 0U;
    current->rx_status = USART_RX_BUSY;

    //enable RXNE interrupts
    init_config->instance->CR1 |= USART_CR1_RXNEIE;

    return SUCCESS;
}

/**
 * @brief  Aborts data reception using interrupts
 * @param  init_config: Pointer to a struct containing USART settings
 * @retval Status indicating success or invalid parameters
 */
Status USART_Abort_Receive_IRQ(USART_Config_t *init_config) {
    CHECK_STATUS(Validate_Ptr(init_config));

    //disable RXNE interrupts
    init_config->instance->CR1 &= ~(USART_CR1_RXNEIE);

    //make USART global state default
    volatile USART_State_t *current = NULL;
    CHECK_STATUS(USART_Get_State(init_config, &current));

    //reset global state rx variables
    current->rx_instance = NULL;
    current->rx_buffer = NULL;
    current->rx_length = 0U;
    current->rx_index = 0U;
    current->rx_status = USART_RX_IDLE;

    return SUCCESS;
}

/**
 * @brief  Transmits an array/string of bytes via USART using blocking
 * @param  init_config: Pointer to a struct containing USART settings
 * @param  tx_buffer:   Pointer to array/string that contains bytes to be transmitted
 * @param  tx_length:   Number of bytes to be transmitted
 * @param  timeout_ms:  Timeout in ms
 * @retval Status indicating success, invalid parameters or error
 * @note   If tx_buffer is a string, tx_length = strlen((char *) string) + 1. The one is added to
 *         account for \0. 
 * @note   If tx_buffer is an array, tx_length = sizeof(array) / sizeof(array[0])
 * @note   Assumes USART has been initialised via @ref USART_Init
 * @note   Systick should be configured as the time base with units = ms prior to calling this 
 *         function
 * @note   For the automatic timeout to be used, timeout_ms = 0
 */
Status USART_Transmit_Block(
    USART_Config_t *init_config, 
    uint8_t        *tx_buffer, 
    uint16_t       tx_length, 
    float          timeout_ms
) {
    CHECK_STATUS(Validate_Ptr(init_config));
    CHECK_STATUS(Validate_Ptr(tx_buffer));
    if (tx_length <= 0U || tx_buffer > TX_BUFFER_SIZE) {
        return INVALID_PARAM;
    }

    //initialise start time
    uint32_t start_time = g_systick_time;

    //if timeout has not been specified, calculate an automatic timeout
    if (timeout_ms == 0.0f) {
        float bits_per_tx = 10.0f;
        float timeout_margin = 2.0f;
        float baud_period_ms = ((1.0f / ((float) init_config->baud_rate)) * ((float) SEC_TO_MSEC));
        timeout_ms = (baud_period_ms * bits_per_tx * tx_length) * timeout_margin;
    }

    //shift the bytes into the tx data register
    for (int i = 0; (i < tx_length) && (i < TX_BUFFER_SIZE); i++) {
        while (!(init_config->instance->SR & USART_SR_TXE)) {
            //check for timeout
            if ((start_time + timeout_ms) < g_systick_time) {
                return ERROR;
            }
            NOP();
        }
        init_config->instance->DR = tx_buffer[i];
    }

    //wait for transmission to complete
    while (!(init_config->instance->SR & USART_SR_TC)) {
        //check for timeout
        if ((start_time + timeout_ms) < g_systick_time) {
            return ERROR;
        }
        NOP();
    }
    
    return SUCCESS;
}

/** @warning For some reason, this function leads to an overrun error. Don't use unless fixed */
/**
 * @brief  Receives bytes via USART using blocking
 * @param  init_config: Pointer to a struct containing USART settings
 * @param  rx_buffer:   Pointer to array that used to store received bytes
 * @param  rx_length:   Number of bytes to be received
 * @param  timeout_ms:  Timeout in ms
 * @retval Status indicating success, invalid parameters or error
 * @note   if the number of bytes to be received is not known, rx_length = RX_BUFFER_SIZE
 * @note   Assumes USART has been initialised via @ref USART_Init
 * @note   Systick should be configured as the time base with units = ms prior to this function call
 * @note   For the automatic timeout to be used, timeout_ms = 0
 */
Status USART_Receive_Block(
    USART_Config_t *init_config, 
    uint8_t        *rx_buffer, 
    uint16_t       rx_length, 
    float          timeout_ms
) {
    CHECK_STATUS(Validate_Ptr(init_config));
    CHECK_STATUS(Validate_Ptr(rx_buffer));
    if (rx_length <= 0U) {
        return INVALID_PARAM;
    }

    //initialise start time
    uint32_t start_time = g_systick_time;

    //if timeout has not been specified, calculate an automatic timeout
    if (timeout_ms == 0.0f) {
        float bits_per_rx = 10.0f;
        float timeout_margin = 2.0f;
        float baud_period_ms = ((1.0f / ((float) init_config->baud_rate)) * ((float) SEC_TO_MSEC));
        timeout_ms = (baud_period_ms * bits_per_rx * rx_length) * timeout_margin;
    }

    //store the bytes received on the rx data register and handle errors
    uint16_t rx_index = 0U;
    while (rx_index < rx_length) {
        //check for timeout
        if ((start_time + timeout_ms) < g_systick_time) {
            return ERROR;
        }

        //wait for a byte to be received
        if (init_config->instance->SR & USART_SR_RXNE) {
            //read SR and DR immediately
            uint32_t status_reg = init_config->instance->SR;
            uint8_t data = init_config->instance->DR;

            if (status_reg & (USART_SR_ORE | USART_SR_NF | USART_SR_FE | USART_SR_PE)) {
                return ERROR;
            } else {
                rx_buffer[rx_index++] = data;
            }
        }
    }

    return SUCCESS;
}


/**
 * @brief  Calculates an automatic timeout for interrupt-based USART TX/RX
 * @param  init_config: Pointer to a struct containing USART settings
 * @param  timeout_ms:  A pointer to the float used to store the timeout
 * @param  margin:      Variable used as a multiplier to create a buffer for the timeout
 * @param  length:      Number of data bytes to be transmitted or received
 * @retval Status indicating success or invalid parameter
 * @note   Assumes USART has been initialised via @ref USART_Init
 */
Status USART_Calc_Timeout(
    USART_Config_t *init_config, 
    float          *timeout_ms, 
    float          margin, 
    uint16_t       length
) {
    CHECK_STATUS(Validate_Ptr(init_config));
    CHECK_STATUS(Validate_Ptr(timeout_ms));

    //determine number of bits involved in USART communication
    float std_data_bits = 8.0f;
    float stop_bits;
    if (init_config->stop_bits == USART_STOP_0_5) {
        stop_bits = 0.5f;
    } else if (init_config->stop_bits == USART_STOP_1) {
        stop_bits = 1.0f;
    } else {
        stop_bits = 2.0f;
    }
    float total_bits = (
        std_data_bits + ((float) init_config->word_length) + ((float) init_config->parity_control)
        + stop_bits
    );

    //calculate timeout
    float baud_period_ms = ((1.0f / ((float) init_config->baud_rate)) * ((float) SEC_TO_MSEC));
    *timeout_ms = (baud_period_ms * total_bits * length) * margin;

    return SUCCESS;
}

/**
 * @brief  Stores the address of a specific global USART state in a pointer
 * @param  init_config:  Pointer to a struct containing USART settings
 * @param  global_state: Address of the pointer used to store global USART state
 * @retval Status indicating success or invalid parameters
 */
Status USART_Get_State(USART_Config_t *init_config, volatile USART_State_t **global_state) {
    CHECK_STATUS(Validate_Ptr(init_config));
    CHECK_STATUS(Validate_Ptr(global_state));

    //store appropriate global state
    if (init_config->instance == USART1) {
        *global_state = &g_usart_1;
    } else if (init_config->instance == USART2) {
        *global_state = &g_usart_2;
    } else if (init_config->instance == USART6) {
        *global_state = &g_usart_6;
    } else {
        return INVALID_PARAM;
    }

    return SUCCESS;
}

// !! This function is not complete
Status USART_Transmit_Log_Msg(USART_Config_t *init_config, uint8_t *log_msg) {
    return USART_Transmit_IRQ(init_config, log_msg, strlen((char *) log_msg));
}


/**************************************************************************************************/
/*                                    USART Interrupt Handlers                                    */
/**************************************************************************************************/

/**
 * @brief  Generalised USART interrupt handler based on global USART state
 * @param  usart: Pointer to global USART state
 */
static void USART_IRQHandler(volatile USART_State_t *usart) {
    CHECK_STATUS(Validate_Ptr(usart));

    //handle TXE interrupt
    if (usart->tx_instance && (usart->tx_instance->SR & USART_SR_TXE)) {
        if (usart->tx_index < usart->tx_length) {
            usart->tx_instance->DR = usart->tx_buffer[usart->tx_index++];
        } else {
            usart->tx_instance->CR1 &= ~(USART_CR1_TXEIE);
            usart->tx_instance->CR1 |= USART_CR1_TCIE;
        }
    }

    //handle RXNE interrupt
    if (usart->rx_instance && (usart->rx_instance->SR & USART_SR_RXNE)) {
        uint32_t status_reg = usart->rx_instance->SR;
        uint8_t data = usart->rx_instance->DR;

        if (status_reg & (USART_SR_ORE | USART_SR_NF | USART_SR_FE | USART_SR_PE)) {
            usart->rx_instance->CR1 &= ~(USART_CR1_RXNEIE);
            usart->rx_status = USART_RX_IDLE;
        } else if (usart->rx_status == USART_RX_BUSY) {
            //if no errors have occured and reception is active, store the data
            if (usart->rx_index < usart->rx_length) {
                usart->rx_buffer[usart->rx_index++] = data;

                //end transmission if last byte has been received
                if (usart->rx_index >= usart->rx_length) {
                    usart->rx_instance->CR1 &= ~(USART_CR1_RXNEIE);
                    usart->rx_status = USART_RX_IDLE;
                }
            }
        }
    }

    //handle TC interrupt
    if (usart->tx_instance && (usart->tx_instance->SR & USART_SR_TC)) {
        usart->tx_instance->CR1 &= ~(USART_CR1_TCIE);
        usart->tx_instance->SR &= ~(USART_SR_TC);
        usart->tx_status = USART_TX_IDLE;
    }
}

/** @brief Handles USART1 interrupts */
void USART1_IRQHandler(void) {
    USART_IRQHandler(&g_usart_1);
}

/** @brief Handles USART2 interrupts */
void USART2_IRQHandler(void) {
    USART_IRQHandler(&g_usart_2);
}

/** @brief Handles USART6 interrupts */
void USART6_IRQHandler(void) {
    USART_IRQHandler(&g_usart_6);
}
