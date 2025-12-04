/**
 * @file    utils.c
 * @brief   STM32F411 Utility Functions and Variables
 * @details This source file contains utility functions used across STM32F411 applications. This
 *          including system initialisation, timing utilities, parameter validation and NVIC
 *          interrupt management functions. 
 * 
 * @par     Functions include:
 *          - Peripheral_Reset(): Resets all peripherals
 *          - Sys_Clock_Init(): Initialises the system clock
 *          - Delay_Loop(): 
 *          - Systick_Init(): Initialises Systick as a time base
 *          - Systick_Delay(): Delays program execution
 *          - Validate_uint8(): Validates that an integer value can be stored in a uint8_t
 *          - Validate_uint16(): Validates that an integer value can be stored in a uint16_t
 *          - Validate_Enum_Param(): Validates that an integer value is an enumerator
 *          - NVIC_Enable_IRQ(): Enables a specific interrupt via the NVIC
 *          - NVIC_Disable_IRQ(): Disables a specific interrupt via the NVIC
 *          - NVIC_Get_Enable_IRQ(): Gets the enable status of an interrupt
 *          - NVIC_Set_Pending_IRQ(): Sets a specific interrupt pending
 *          - NVIC_Clear_Pending_IRQ(): Clears the pending status of a specific interrupt
 *          - NVIC_Get_Pending_IRQ(): Gets the pending status of an interrupt
 *          - NVIC_Get_Active_IRQ(): Gets the active status of an interrupt
 *          - NVIC_Set_Priority_IRQ(): Sets the priority of a specific interrupt
 *          - NVIC_Get_Priority(): Gets the priority of a specific interrupt
 *          - Validate_Priority(): Validates an interrupt priority
 *          - SysTick_Handler(): Handles Systick interrupts
 */


#include "utils.h"


/**************************************************************************************************/
/*                                          RCC Functions                                         */
/**************************************************************************************************/

/*** @brief Resets all peripherals */
void Peripheral_Reset(void) {
    //set reset bits
    RCC->AHB1RSTR |= SET_32;
    RCC->AHB2RSTR |= SET_32;
    RCC->APB1RSTR |= SET_32;
    RCC->APB2RSTR |= SET_32;  

    //clear reset bits
    RCC->AHB1RSTR = CLEAR_REGISTER;
    RCC->AHB2RSTR = CLEAR_REGISTER;
    RCC->APB1RSTR = CLEAR_REGISTER;
    RCC->APB2RSTR = CLEAR_REGISTER;
}

/**
 * @brief  Initialises the system clock
 * @param  sys_clk_config: Pointer to a struct containing clock settings
 * @retval Status indicating success, invalid parameters or error
 */
Status Clock_Init(Clock_Config_t *clk_config) {
    //configure system clock
    switch(clk_config->clk_source) {
        case HSI_CLOCK: {
            RCC->CR |= RCC_CR_HSION;
            do {
                //wait for HSI to stabilise
            } while (!(RCC->CR & RCC_CR_HSIRDY));
            RCC->CFGR &= ~(RCC_CFGR_SW);
            g_sys_clk_source = HSI_CLOCK;
            g_sys_clk_freq = HSI_FREQ_HZ;
        }
        case HSE_CLOCK: {
            RCC->CR |= RCC_CR_HSEON;
            do {
                //wait for HSE to stabilise
            } while (!(RCC->CR & RCC_CR_HSERDY));
            RCC->CFGR |= RCC_CFGR_SW_0;
            g_sys_clk_source = HSE_CLOCK;
            g_sys_clk_freq = HSE_FREQ_HZ;
        } case PLL_CLOCK: {
            CHECK_STATUS(PLL_Clock_Init(clk_config->pll_config));
        }
        default: return INVALID_PARAM;
    }

    //configure peripheral clocks
    CHECK_STATUS(AHB_Clock_Config(clk_config->ahb_prescaler));
    CHECK_STATUS(APB1_Clock_Config(clk_config->apb1_prescaler));
    CHECK_STATUS(APB2_Clock_Config(clk_config->apb2_prescaler));

    return SUCCESS;
}

/**
 * @brief  Initialises the PLL clock
 * @param  pll_config: Pointer to a struct containing PLL clock settings
 * @retval Status indicating success, invalid parameters or error
 */
Status PLL_Clock_Init(PLL_Config_t *pll_config) {
    CHECK_STATUS(Validate_Ptr(pll_config));
    CHECK_STATUS(Validate_Enum(pll_config->clock_source, HSI_CLOCK, HSE_CLOCK));
    CHECK_STATUS(Validate_Enum(pll_config->p_divisor, PLL_P_DIVISOR_2, PLL_P_DIVISOR_8));
    if ((pll_config->m_divisor < 2U) || (pll_config->m_divisor > 63U)) {
        return INVALID_PARAM;
    }
    if ((pll_config->n_multiplier < 50U) || (pll_config->n_multiplier > 432U)) {
        return INVALID_PARAM;
    }

    //disable PLL
    RCC->CR &= ~(RCC_CR_PLLON);

    //configure clock source
    RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLSRC);
    RCC->PLLCFGR |= ((uint32_t) (pll_config->clock_source << 22U));
    uint32_t pll_input_clk_freq = 0U;
    if (pll_config->clock_source == HSI_CLOCK) {
        pll_input_clk_freq = HSI_FREQ_HZ;
    } else {
        pll_input_clk_freq = HSE_FREQ_HZ;
    }

    //configure M divisor
    uint32_t vco_input_freq = (pll_input_clk_freq / ((uint32_t) pll_config->m_divisor));
    if (vco_input_freq < VCO_INPUT_FREQ_MIN_HZ || vco_input_freq > VCO_INPUT_FREQ_MAX_HZ) {
        return ERROR;
    } else {
        RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLM);
        RCC->PLLCFGR |= ((uint32_t) pll_config->m_divisor);
    }

    //configure N multiplier
    uint32_t vco_clk_freq = (vco_input_freq * ((uint32_t) pll_config->n_multiplier));
    RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLN);
    RCC->PLLCFGR |= ((uint32_t) pll_config->n_multiplier);

    //configure P divisor
    uint32_t pll_clk_freq = (vco_clk_freq / ((uint32_t) ((pll_config->p_divisor * 2U) + 2U)));
    if (pll_clk_freq > PLL_FREQ_MAX_HZ) {
        return ERROR;
    }

    //enable PLL
    RCC->CR |= RCC_CR_PLLON;
    do {
        //wait for PLL to lock
    } while (!(RCC->CR & RCC_CR_PLLRDY));
    g_sys_clk_freq = pll_clk_freq;
    g_sys_clk_freq = PLL_CLOCK;

    return SUCCESS;
}

/**
 * @brief  Configures the AHB clock
 * @param  ahb_prescaler: AHB prescaler value
 * @retval Status indicating success or invalid parameters
 */
Status AHB_Clock_Config(AHB_Prescaler ahb_prescaler) {
    CHECK_STATUS(Validate_Enum(ahb_prescaler, AHB_PRESCALER_1, AHB_PRESCALER_512));

    //configure AHB prescaler
    RCC->CFGR &= ~(RCC_CFGR_HPRE);
    RCC->CFGR |= ((uint32_t) (ahb_prescaler << 4U));

    //ensure AHB clock frequency <= 10 MHz
    g_ahb_clk_freq = (g_sys_clk_freq / ((uint32_t) ahb_prescaler));
    if (g_ahb_clk_freq > AHB_MAX_FREQ_HZ) {
        g_ahb_clk_freq = AHB_MAX_FREQ_HZ;
    }

    return SUCCESS;
}

/**
 * @brief  Configures the APB1 clock
 * @param  apb1_prescaler: APB1 prescaler value
 * @retval Status indicating success or invalid parameters
 */
Status APB1_Clock_Config(APB_Prescaler apb1_prescaler) {
    CHECK_STATUS(Validate_Enum(apb1_prescaler, APB_PRESCALER_1, APB_PRESCALER_16));

    //configure APB1 prescaler
    RCC->CFGR &= ~(RCC_CFGR_PPRE1);
    RCC->CFGR |= ((uint32_t) (apb1_prescaler << 10U));

    //ensure APB1 clock frequency <= 50 MHz
    g_apb1_clk_freq = (g_ahb_clk_freq / ((uint32_t) apb1_prescaler));
    if (g_apb1_clk_freq > APB1_MAX_FREQ_HZ) {
        g_apb1_clk_freq = APB1_MAX_FREQ_HZ;
    }

    return SUCCESS;
}

/**
 * @brief  Configures the APB2 clock
 * @param  apb2_prescaler: APB2 prescaler value
 * @retval Status indicating success or invalid parameters
 */
Status APB2_Clock_Config(APB_Prescaler apb2_prescaler) {
    CHECK_STATUS(Validate_Enum(apb2_prescaler, APB_PRESCALER_1, APB_PRESCALER_16));

    //configure APB2 prescaler
    RCC->CFGR &= ~(RCC_CFGR_PPRE2);
    RCC->CFGR |= ((uint32_t) (apb2_prescaler << 13U));

    //ensure APB2 clock frequency <= 10 MHz
    g_apb2_clk_freq = (g_ahb_clk_freq / ((uint32_t) apb2_prescaler));
    if (g_apb2_clk_freq > APB2_MAX_FREQ_HZ) {
        g_apb2_clk_freq = APB2_MAX_FREQ_HZ;
    }
}


/**************************************************************************************************/
/*                                         SYSTICK Functions                                      */
/**************************************************************************************************/

/**
 * @brief  Delays program execution using busy waiting with Systick timer
 * @param  delay_ms: The desired time delay in ms
 * @note   Assumes Systick has been configured as a time base unit via @ref Systick_Init
 */
void Delay_Loop(uint32_t delay_ms) {
    if (delay_ms <= 0U) {
        return;
    }
    uint32_t ticks_per_ms       = (g_sys_clk_freq / SEC_TO_MSEC);
    uint32_t systick_reload_val = ((ticks_per_ms * delay_ms)  - 1U);
    SYSTICK->CTRL &= ~(SYSTICK_CTRL_ENABLE);

    while (systick_reload_val > 0U) {
        if (systick_reload_val <= SYSTICK_LOAD_RELOAD) {
            if (delay_ms <= 1000) {
                systick_reload_val = ((systick_reload_val * 55U) / 100U);
            }
            SYSTICK->LOAD  = systick_reload_val;
            SYSTICK->VAL   = CLEAR_REGISTER;
            SYSTICK->CTRL |= SYSTICK_CTRL_CLKSOURCE | SYSTICK_CTRL_ENABLE;
            while (!(SYSTICK->CTRL & SYSTICK_CTRL_COUNTFLAG)) {
                NOP();
            }
            systick_reload_val = 0U;
            return;
        } else {
            SYSTICK->LOAD  = SYSTICK_LOAD_RELOAD;
            SYSTICK->VAL   = CLEAR_REGISTER;
            SYSTICK->CTRL |= SYSTICK_CTRL_CLKSOURCE | SYSTICK_CTRL_ENABLE;
            while (!(SYSTICK->CTRL & SYSTICK_CTRL_COUNTFLAG)) {
                NOP();
            }
            systick_reload_val -= SYSTICK_LOAD_RELOAD;
        }
        SYSTICK->CTRL &= ~(SYSTICK_CTRL_ENABLE);
    }
}

/**
 * @brief  Initialises Systick as a time base
 * @param  unit: Time unit for the time base. Limited to seconds, milliseconds and microseconds
 * @retval Status indicating success or invalid parameters
 */
Status Systick_Init(Systick_Base_Unit unit) {
    CHECK_STATUS(Validate_Enum(unit, SYSTICK_UNIT_SEC, SYSTICK_UNIT_USEC));

    //initialise global systick time base
    g_systick_time = 0U;

    //calculate reload value
    uint32_t ticks_per_unit = 0U;
    switch (unit) {
        case SYSTICK_UNIT_SEC:  ticks_per_unit = g_sys_clk_freq; break;
        case SYSTICK_UNIT_MSEC: ticks_per_unit = (g_sys_clk_freq / SEC_TO_MSEC); break;
        case SYSTICK_UNIT_USEC: ticks_per_unit = (g_sys_clk_freq / SEC_TO_USEC); break;
        default: return INVALID_PARAM;
    }
    uint32_t reload_val = (ticks_per_unit - 1UL);

    //cap reload value
    if (reload_val > 0xFFFFFF) {
        reload_val = 0xFFFFFF;
    }

    //configure systick
    SYSTICK->CTRL &= ~(SYSTICK_CTRL_ENABLE);
    SYSTICK->LOAD  = reload_val;
    SYSTICK->VAL   = CLEAR_REGISTER;
    SYSTICK->CTRL |= SYSTICK_CTRL_ENABLE | SYSTICK_CTRL_CLKSOURCE | SYSTICK_CTRL_TICKINT;

    return SUCCESS;
}

/**
 * @brief  Delays program execution using an interrupt-driven Systick timer
 * @param  time_delay: The desired time delay
 * @retval Status indicating success or invalid parameters
 * @note   Assumes Systick has been configured as a time base unit via @ref Systick_Init
 * @note   Ensure that the implied units of time_delay match the units of the Systick time base
 */
Status Systick_Delay(uint32_t time_delay) {
    if (time_delay <= 0U) {
        return INVALID_PARAM;
    }

    //wait for time delay to elapse
    uint32_t previous_sys_time = g_systick_time;
    while ((g_systick_time - previous_sys_time) < time_delay) {
        NOP();
    }

    return SUCCESS;
}


/**************************************************************************************************/
/*                                          NVIC Functions                                        */
/**************************************************************************************************/

/**
 * @brief  Enables a specific interrupt via the NVIC
 * @param  IRQn: Interrupt number
 * @retval Status indicating success or error
 */
Status NVIC_Enable_IRQ(IRQn_t IRQn) {
    if ((int32_t) IRQn >= 0) {
        uint8_t iser_index = (((uint32_t) IRQn) >> DIV_BY_32);
        uint8_t bit_index = (((uint32_t) IRQn) & 0x1FUL);
        NVIC->ISER[iser_index] = ((uint32_t) (1UL << bit_index));
        return SUCCESS;       
    }
    return ERROR;
}

/**
 * @brief  Disables a specific interrupt via the NVIC
 * @param  IRQn: Interrupt number
 * @retval Status indicating success or error
 */
Status NVIC_Disable_IRQ(IRQn_t IRQn) {
    if ((int32_t) IRQn >= 0) {
        uint8_t icer_index = (((uint32_t) IRQn) >> DIV_BY_32);
        uint8_t bit_index = (((uint32_t) IRQn) & 0x1FUL);
        NVIC->ICER[icer_index] = ((uint32_t) (1UL << bit_index));
        return SUCCESS;
    }
    return ERROR;
}

/**
 * @brief  Gets the enable status of an interrupt
 * @param  IRQn: interrupt number
 * @retval 1U (interrupt enabled) or 0U (interrupt disabled)
 */
uint32_t NVIC_Get_Enable_IRQ(IRQn_t IRQn) {
    if ((int32_t) IRQn >= 0) {
        uint8_t iser_index = (((uint32_t) IRQn) >> DIV_BY_32);
        uint8_t bit_index = (((uint32_t) IRQn) & 0x1FUL);
        uint32_t enable_status = (NVIC->ISER[iser_index]) & ((uint32_t) (1UL << bit_index));
        if (enable_status == 0U) {
            return 0U;
        } else {
            return 1U;
        }
    } else {
        return 0U;
    }
}

/**
 * @brief  Sets a specific interrupt pending
 * @param  IRQn: Interrupt number
 * @retval Status indicating success or error
 */
Status NVIC_Set_Pending_IRQ(IRQn_t IRQn) {
    if ((int32_t) IRQn >= 0) {
        uint8_t ispr_index = (((uint32_t) IRQn) >> DIV_BY_32);
        uint8_t bit_index = (((uint32_t) IRQn) & 0x1FUL);
        NVIC->ISPR[ispr_index] = ((uint32_t) (1UL << bit_index));
        return SUCCESS;
    }
    return ERROR;
}

/**
 * @brief  Clears the pending status of a specific interrupt
 * @param  IRQn: Interrupt number
 * @retval Status indicating success or error
 */
Status NVIC_Clear_Pending_IRQ(IRQn_t IRQn) {
    if ((int32_t) IRQn >= 0) {
        uint8_t icpr_index = (((uint32_t) IRQn) >> DIV_BY_32);
        uint8_t bit_index = (((uint32_t) IRQn) & 0x1FUL);
        NVIC->ICPR[icpr_index] = ((uint32_t) (1UL << bit_index));
        return SUCCESS;
    }
    return ERROR;
}

/**
 * @brief  Gets the pending status of an interrupt
 * @param  IRQn: interrupt number
 * @retval 1U (pending) or 0U (not pending)
 */
uint32_t NVIC_Get_Pending_IRQ(IRQn_t IRQn) {
    if ((int32_t) IRQn >= 0) {
        uint8_t ispr_index = (((uint32_t) IRQn) >> DIV_BY_32);
        uint8_t bit_index = (((uint32_t) IRQn) & 0x1FUL);
        uint32_t pending_status = (NVIC->ISPR[ispr_index]) & ((uint32_t) (1UL << bit_index));
        if (pending_status == 0U) {
            return 0U;
        } else {
            return 1U;
        }
    } else {
        return 0U;
    }
}

/**
 * @brief  Gets the active status of an interrupt
 * @param  IRQn: interrupt number
 * @retval 1U (active) or 0U (not active)
 */
uint32_t NVIC_Get_Active_IRQ(IRQn_t IRQn) {
    if ((int32_t) IRQn >= 0) {
        uint8_t iabr_index = (((uint32_t) IRQn) >> DIV_BY_32);
        uint8_t bit_index = (((uint32_t) IRQn) & 0x1FUL);
        uint32_t active_status = (NVIC->IABR[iabr_index]) & ((uint32_t) (1UL << bit_index));
        if (active_status == 0U) {
            return 0U;
        } else {
            return 1U;
        }
    } else {
        return 0U;
    }
}

/**
 * @brief  Sets the priority of a specific interrupt
 * @param  IRQn:     Interrupt number
 * @param  priority: Priority to set
 * @retval Status indicating success or error
 */
Status NVIC_Set_Priority(IRQn_t IRQn, uint32_t priority) {
    if ((int32_t) IRQn >= 0) {
        NVIC->IPR[(uint32_t) IRQn] = ((uint8_t) ((priority << NVIC_PRIORITY_BITS) & 0xFFUL));
        return SUCCESS;
    } else {
        uint8_t shpr_index = ((uint8_t) IRQn) + ((uint8_t) 0xC);
        if (shpr_index < 0U) {
            return ERROR;
        } else {
            SCB->SHPR[shpr_index] = ((uint8_t) ((priority << NVIC_PRIORITY_BITS) & 0xFFUL));
            return SUCCESS;
        }
    }      
}

/**
 * @brief  Gets the priority of a specific interrupt
 * @param  IRQn: Interrupt number
 * @retval Interrupt priority
 */
uint32_t NVIC_Get_Priority(IRQn_t IRQn) {
    if ((int32_t) IRQn >= 0) {
        uint32_t priority_status = (NVIC->IPR[(uint32_t) IRQn] >> NVIC_PRIORITY_BITS);
        return priority_status;
    } else {
        uint8_t shpr_index = ((uint8_t) IRQn) + ((uint8_t) 0xC);
        if (shpr_index < 0U) {
            return (uint32_t) ERROR;
        } else {
            uint32_t priority_status = (SCB->SHPR[shpr_index] >> NVIC_PRIORITY_BITS);
            return priority_status;
        }
    }
}

/**
 * @brief  Validates an interrupt priority
 * @param  priority: priority value to be validated
 * @retval Status indicating success or invalid parameters
 */
Status Validate_Priority_IRQ(uint32_t priority) {
    if (priority < 0U || priority > 0xFFU) {
        return INVALID_PARAM;
    } else {
        return SUCCESS;
    }
}

/** @brief A global array used to track assigned and available interrupt priorities */
uint8_t irq_priority_tracker[256];


/**************************************************************************************************/
/*                                       Interrupt Handlers                                       */
/**************************************************************************************************/

/** @brief Handles Systick interrupts */
void SysTick_Handler(void) {
    g_systick_time++;
}

// !! This function is under development
//if message is a uint8_t array, it should be passed as ((char *) message)
void Append_Float_To_String(char *message, size_t message_size, float data) {
    char temp[32];
    snprintf(temp, sizeof(temp), "%f", data);
    size_t used = strlen(message);
    size_t remaining = message_size - used;
    strncat(message, temp, remaining - 1);
}

