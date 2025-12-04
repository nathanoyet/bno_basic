/**
 * @file    tim1.c
 * @brief   STM32F411 TIM1 Advanced Timer Driver
 * @details This driver provides an interface for the STM32F411 TIM1 advanced control timer. It
 *          supports counter mode, time base operations, input capture, PWM input/output, output 
 *          compare, and servo motor control.
 * 
 *@par      Driver functions:
 *          - TIM1_CNT_Init(): Initialises TIM1 in counter mode
 *          - TIM1_IC_Init(): Initialises TIM1 in input capture mode
 *          - TIM1_PWM_Input_Init(): Initialises TIM1 in PWM input mode
 *          - TIM1_OC_Init(): Initialises TIM1 in output compare mode
 *          - TIM1_PWM_Output_Init(): Initialises TIM1 in PWM output mode
 *          - TIM1_PWM_Set_Duty_Cycle(): Sets the PWM duty cycle for a particular TIM1 channel
 *          - TIM1_Deinit(): Deinitialises TIM1
 *          - TIM1_Validate_Channel(): Validates TIM1 channel
 *          - TIM1_Servo_Init(): Initialises TIM1 in PWM output mode to drive a servo motor
 *          - TIM1_Servo_Set_Position(): Sets the angle for a servo driven by a TIM1 channel
 *          - TIM1_MS_Base_Init(): Initialises TIM1 as a time base in milli-seconds
 *          - TIM1_MS_Delay(): Delays program execution by a specified number of milli-seconds
 *          - TIM1_UP_TIM10_IRQHandler(): Handles TIM1 update and TIM10 global interrupts
 *          - TIM1_CC_IRQHandler(): Handles TIM1 capture and compare interrupts
 * 
 * @warning Ensure GPIO pins are configured before calling TIM1 init functions
 */


 #include "tim1.h"


/**************************************************************************************************/
/*                               TIM1 Core Initialisation Functions                               */
/**************************************************************************************************/

/**
 * @brief  Initialises TIM1 in counter mode
 * @param  cnt_config: Pointer to TIM1_CNT_Config structure containing counter settings
 * @retval Status indicating success or invalid parameters
 */
Status TIM1_CNT_Init(TIM1_CNT_Config_t *cnt_config) {
    CHECK_STATUS(Validate_Ptr(cnt_config));

    CHECK_STATUS(Validate_uint16_t(cnt_config->prescaler));
    CHECK_STATUS(Validate_uint16_t(cnt_config->auto_reload));
    CHECK_STATUS(Validate_uint8_t(cnt_config->repetition));
    CHECK_STATUS(Validate_Priority_IRQ(cnt_config->interrupt_priority));

    cnt_config->prescaler   = (uint16_t) cnt_config->prescaler;
    cnt_config->auto_reload = (uint16_t) cnt_config->auto_reload;
    cnt_config->repetition  = (uint8_t) cnt_config->repetition;

    //validate availability of interrupt priority level
    if (cnt_config->interrupt_enable) {
        if (irq_priority_tracker[cnt_config->interrupt_priority]) {
            return INVALID_PARAM;
        }
    }

    //enable TIM1 clock
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    //configure the counter in edge-aligned or centre-aligned mode
    if (cnt_config->centre_aligned_mode) {
        TIM1->CR1 &= ~(TIM_CR1_CMS);
        switch (cnt_config->centre_aligned_mode) {
            case TIM1_CENTRE_MODE_UP:   TIM1->CR1   |= TIM_CR1_CMS_UP; break;
            case TIM1_CENTRE_MODE_DOWN: TIM1->CR1 |= TIM_CR1_CMS_DOWN; break;
            case TIM1_CENTRE_MODE_BOTH: TIM1->CR1 |= TIM_CR1_CMS_BOTH; break;
            default: return INVALID_PARAM; 
        }
    } else {
        switch (cnt_config->direction) {
            case TIM1_DIR_UP: TIM1->CR1   &= ~(TIM_CR1_DIR); break;
            case TIM1_DIR_DOWN: TIM1->CR1 |= TIM_CR1_DIR; break;
            default: return INVALID_PARAM;
        }
    }

    //configure auto-reload, prescaler, and repetition
    if (cnt_config->auto_reload) {
        TIM1->CR1 |= TIM_CR1_ARPE;
        TIM1->ARR = (cnt_config->auto_reload - 1UL);
    }
    TIM1->PSC = (cnt_config->prescaler - 1UL);
    TIM1->RCR = cnt_config->repetition;

    //configure interrupts
    switch (cnt_config->interrupt_enable) {
        case TIM1_INTERRUPT_ENABLED: {
            TIM1->DIER |= TIM_DIER_UIE;
            DISABLE_IRQ();
            NVIC_Set_Priority(TIM1_UP_TIM10_IRQn, cnt_config->interrupt_priority);
            NVIC_Enable_IRQ(TIM1_UP_TIM10_IRQn);
            ENABLE_IRQ();
            break;
        }
        case TIM1_INTERRUPT_DISABLED: TIM1->DIER &= ~(TIM_DIER_UIE); break;
        default: return INVALID_PARAM;
    }

    //configure DMA
    switch (cnt_config->dma_enable) {
        case TIM1_DMA_ENABLED: TIM1->DIER  |= TIM_DIER_UDE; break;
        case TIM1_DMA_DISABLED: TIM1->DIER &= ~(TIM_DIER_UDE); break;
        default: return INVALID_PARAM;
    }

    //configure update event
    switch (cnt_config->update_event) {
        case TIM1_UPDATE_EVENT_ENABLED: TIM1->CR1  &= ~(TIM_CR1_UDIS); break;
        case TIM1_UPDATE_EVENT_DISABLED: TIM1->CR1 |= TIM_CR1_UDIS; break;
        default: return INVALID_PARAM; 
    }

    //configure update request
    switch (cnt_config->update_request) {
        case TIM1_UPDATE_REQ_ALL: TIM1->CR1  &= ~(TIM_CR1_URS); break;
        case TIM1_UPDATE_REQ_FLOW: TIM1->CR1 |= TIM_CR1_URS; break;
        default: return INVALID_PARAM;
    }

    //enable the counter
    TIM1->CR1 |= TIM_CR1_CEN;

    //record utilised interrupt priority level
    if (cnt_config->interrupt_enable) {
        irq_priority_tracker[cnt_config->interrupt_priority] = 1U;
    }
    
    DSB();
    return SUCCESS;
}

/**
 * @brief  Initialises TIM1 in input capture mode
 * @param  ic_config: Pointer to TIM1_IC_Config structure containing input capture settings
 * @retval Status indicating success or invalid parameters
 * @note   Can be called independent of counter initialisation via @ref TIM1_CNT_Init
 */
Status TIM1_IC_Init(TIM1_IC_Config_t *ic_config) {
    CHECK_STATUS(Validate_Ptr(ic_config));
    CHECK_STATUS(TIM1_Validate_Channel(ic_config->channel));
    CHECK_STATUS(Validate_Priority_IRQ(ic_config->interrupt_priority));
    CHECK_STATUS(Validate_Enum(ic_config->selection, TIM1_CC_INPUT_MAP_EQ, TIM1_CC_INPUT_MAP_TRC));

    //validate availability of interrupt priority level
    if (ic_config->interrupt_enable) {
        if (irq_priority_tracker[ic_config->interrupt_priority]) {
            return INVALID_PARAM;
        }
    }

    //enable TIM1 clock
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    //disable capture
    TIM1->CCER &= ~(SET_ONE << ((ic_config->channel - 1U) * 4U));

    //configure TIM1 channel as input
    uint8_t ccmr_shift = (ic_config->channel % 2) ? 0 : 8;
    uint8_t ccmr_reg   = (ic_config->channel <= 2) ? 0 : 1;
    if (ccmr_reg == 0) {
        //configure input mapping
        TIM1->CCMR1 &= ~(SET_THREE << ccmr_shift);
        TIM1->CCMR1 |= (((uint32_t) ic_config->selection) << ccmr_shift);
        //configure input prescaler
        TIM1->CCMR1 &= ~(SET_TWO << (ccmr_shift + 2U));
        TIM1->CCMR1 |= (((uint32_t) ic_config->prescaler) << (ccmr_shift + 2U));
        //configure input filter
        TIM1->CCMR1 &= ~(SET_FOUR << (ccmr_shift + 4U));
        TIM1->CCMR1 |= (((uint32_t) ic_config->filter) << (ccmr_shift + 4U));
    } else {
        //configure input mapping
        TIM1->CCMR2 &= ~(SET_THREE << ccmr_shift);
        TIM1->CCMR2 |= (((uint32_t) ic_config->selection) << ccmr_shift);
        //configure input prescaler
        TIM1->CCMR2 &= ~(SET_TWO << (ccmr_shift + 2U));
        TIM1->CCMR2 |= (((uint32_t) ic_config->prescaler) << (ccmr_shift + 2U));
        //configure input filter
        TIM1->CCMR2 &= ~(SET_FOUR << (ccmr_shift + 4U));
        TIM1->CCMR2 |= (((uint32_t) ic_config->filter) << (ccmr_shift + 4U));
    }

    //configure polarity
    TIM1->CCER |= (((uint32_t) ic_config->polarity) << (1U + ((ic_config->channel - 1) * 4U)));
    switch (ic_config->polarity) {
        case TIM1_CC_NON_INV_RISING: {
            TIM1->CCER &= ~(SET_ONE << (1U + ((ic_config->channel - 1) * 4U)));
            TIM1->CCER &= ~(SET_ONE << (3U + ((ic_config->channel - 1) * 4U)));
        }
        break;
        case TIM1_CC_INV_FALLING: {
            TIM1->CCER |= (SET_ONE << (1U + ((ic_config->channel - 1) * 4U)));
            TIM1->CCER &= ~(SET_ONE << (3U + ((ic_config->channel - 1) * 4U)));
        }
        break;
        case TIM1_CC_NON_INV_BOTH: {
            TIM1->CCER |= (SET_ONE << (1U + ((ic_config->channel - 1) * 4U)));
            TIM1->CCER |= (SET_ONE << (3U + ((ic_config->channel - 1) * 4U)));
        }
        break;
        default: return INVALID_PARAM;
    }
    
    //configure interrupts
    switch (ic_config->interrupt_enable) {
        case TIM1_CC_INTERRUPT_ENABLED: {
            TIM1->DIER |= (SET_ONE << ic_config->channel);
            DISABLE_IRQ();
            NVIC_Set_Priority(TIM1_CC_IRQn, ic_config->interrupt_priority);
            NVIC_Enable_IRQ(TIM1_CC_IRQn);
            ENABLE_IRQ();
            break;
        }
        case TIM1_CC_INTERRUPT_DISABLED: TIM1->DIER &= ~(SET_ONE << ic_config->channel); break;
        default: return INVALID_PARAM;
    }

    //configure DMA
    switch (ic_config->dma_enable) {
        case TIM1_CC_DMA_ENABLED: TIM1->DIER  |= (SET_ONE << (ic_config->channel + 8U)); break;
        case TIM1_CC_DMA_DISABLED: TIM1->DIER &= ~(SET_ONE << (ic_config->channel + 8U)); break;
        default: return INVALID_PARAM;
    }
        
    //enable capture
    TIM1->CCER |= (SET_ONE << ((ic_config->channel - 1U) * 4U));

    //enable counter
    if (!(TIM1->CR1 & TIM_CR1_CEN)) {
        TIM1->CR1 |= TIM_CR1_CEN;
    }

    //record utilised interrupt priority level
    if (ic_config->interrupt_enable) {
        irq_priority_tracker[ic_config->interrupt_priority] = 1U;
    }

    DSB();
    return SUCCESS;
}

/**
 * @brief  Initialises TIM1 in PWM input mode
 * @param  pwm_input_config: Pointer to TIM1_PWM_Input_Config structure containing PWM input 
 *         settings
 * @retval Status indicating success or invalid parameters
 * @note   Can be called independent of counter initialisation via @ref TIM1_CNT_Init
 */
Status TIM1_PWM_Input_Init(TIM1_PWM_Input_Config_t *pwm_input_config) {
    CHECK_STATUS(Validate_Ptr(pwm_input_config));

    //validate channel pair
    if (!((pwm_input_config->channel_1  == TIM1_CHANNEL_1 
        && pwm_input_config->channel_2  == TIM1_CHANNEL_2)
        || 
        (pwm_input_config->channel_1   == TIM1_CHANNEL_2 
        && pwm_input_config->channel_2 == TIM1_CHANNEL_1))) {
        return INVALID_PARAM;
    }

    //configure channel 1
    TIM1_IC_Config_t input_channel_1 = {
        .channel            = pwm_input_config->channel_1,
        .selection          = pwm_input_config->selection_1,
        .prescaler          = pwm_input_config->prescaler_1,
        .filter             = pwm_input_config->filter_1,
        .polarity           = pwm_input_config->polarity_1,
        .interrupt_enable   = pwm_input_config->interrupt_enable_1,
        .interrupt_priority = pwm_input_config->interrupt_priority_1,
        .dma_enable         = pwm_input_config->dma_enable_1
    };

    //configure channel 2
    TIM1_IC_Config_t input_channel_2 = {
        .channel            = pwm_input_config->channel_2,
        .selection          = pwm_input_config->selection_2,
        .prescaler          = pwm_input_config->prescaler_2,
        .filter             = pwm_input_config->filter_2,
        .polarity           = pwm_input_config->polarity_2,
        .interrupt_enable   = pwm_input_config->interrupt_enable_2,
        .interrupt_priority = pwm_input_config->interrupt_priority_2,
        .dma_enable         = pwm_input_config->dma_enable_2
    };

    //configure trigger input
    TIM1->SMCR &= ~(TIM_SMCR_TS);
    switch (pwm_input_config->trigger_selection) {
        case TIM1_FILTERED_TI1: TIM1->SMCR |= TIM_SMCR_TS_TI1FP1; break;
        case TIM1_FILTERED_TI2: TIM1->SMCR |= TIM_SMCR_TS_TI2FP2; break;
        default: return INVALID_PARAM;  
    }

    //configure slave mode controller in reset mode
    TIM1->SMCR &= ~(TIM_SMCR_SMS);
    TIM1->SMCR |= TIM_SMCR_SMS_RESET;

    //initialise channel 1 and 2
    CHECK_STATUS(TIM1_IC_Init(&input_channel_1));
    CHECK_STATUS(TIM1_IC_Init(&input_channel_2));

    DSB();
    return SUCCESS;
}

/**
 * @brief  Initialises TIM1 in output compare mode
 * @param  oc_config: Pointer to TIM1_OC_Config structure containing output compare settings
 * @retval Status indicating success or invalid parameters
 * @note   Assumes TIM1 has been configured in counter mode via @ref TIM1_CNT_Init
 */
Status TIM1_OC_Init(TIM1_OC_Config_t *oc_config) {
    CHECK_STATUS(Validate_Ptr(oc_config));
    CHECK_STATUS(TIM1_Validate_Channel(oc_config->channel));
    CHECK_STATUS(Validate_Enum_Param(oc_config->oc_mode, TIM1_OCM_FROZEN, TIM1_OCM_PWM_2));

    CHECK_STATUS(Validate_uint16_t(oc_config->compare_value));
    CHECK_STATUS(Validate_uint16_t(oc_config->auto_reload));
    CHECK_STATUS(Validate_uint16_t(oc_config->prescaler));

    oc_config->compare_value = (uint16_t) oc_config->compare_value;
    oc_config->auto_reload   = (uint16_t) oc_config->auto_reload;
    oc_config->prescaler     = (uint16_t) oc_config->prescaler;

    //validate availability of interrupt priority level
    if (oc_config->interrupt_enable) {
        if (irq_priority_tracker[oc_config->interrupt_priority]) {
            return INVALID_PARAM;
        }
    }

    //disable compare
    TIM1->CCER &= ~(SET_ONE << ((oc_config->channel - 1U) * 4U));

    //write values to ARR, PSC and CCRx
    TIM1->ARR = (oc_config->auto_reload - 1U);
    TIM1->PSC = (oc_config->prescaler - 1U);
    switch (oc_config->channel) {
        case TIM1_CHANNEL_1: TIM1->CCR1 = oc_config->compare_value; break;
        case TIM1_CHANNEL_2: TIM1->CCR2 = oc_config->compare_value; break;
        case TIM1_CHANNEL_3: TIM1->CCR3 = oc_config->compare_value; break;
        case TIM1_CHANNEL_4: TIM1->CCR4 = oc_config->compare_value; break;
        default: return INVALID_PARAM;
    }

    //configure TIM1 channel as output
    uint8_t ccmr_shift = (oc_config->channel % 2) ? 0 : 8;
    uint8_t ccmr_reg   = (oc_config->channel <= 2) ? 0 : 2;
    if (ccmr_reg == 0) {
        //configure channel as ouput
        TIM1->CCMR1 &= ~(SET_TWO << ccmr_shift);
        //configure output compare mode
        TIM1->CCMR1 &= ~(SET_THREE << (ccmr_shift + 4U));
        TIM1->CCMR1 |= (((uint32_t) oc_config->oc_mode) << (ccmr_shift + 4U));
        //configure preload
        TIM1->CCMR1 |= (((uint32_t) oc_config->preload) << (ccmr_shift + 3U));
        //configure fast enable
        TIM1->CCMR1 |= (((uint32_t) oc_config->fast_enable) << (ccmr_shift + 2U));
    } else {
        //configure channel as output
        TIM1->CCMR2 &= ~(SET_TWO << ccmr_shift);
        //configure output compare mode
        TIM1->CCMR2 &= ~(SET_THREE << (ccmr_shift + 4U));
        TIM1->CCMR2 |= (((uint32_t) oc_config->oc_mode) << (ccmr_shift + 4U));
        //configure preload
        TIM1->CCMR2 |= (((uint32_t) oc_config->preload) << (ccmr_shift + 3U));
        //configure fast enable
        TIM1->CCMR2 |= (((uint32_t) oc_config->fast_enable) << (ccmr_shift + 2U));
    }

    //configure polarity
    TIM1->CCER &= ~(SET_ONE << (1U + ((oc_config->channel - 1U) * 4U)));
    TIM1->CCER |= (((uint32_t) oc_config->polarity) << (1U + ((oc_config->channel - 1U) * 4U)));

    //configure interrupts
    switch (oc_config->interrupt_enable) {
        case TIM1_CC_INTERRUPT_ENABLED: {
            TIM1->DIER |= (SET_ONE << oc_config->channel);
            DISABLE_IRQ();
            NVIC_Set_Priority(TIM1_CC_IRQn, oc_config->interrupt_priority);
            NVIC_Enable_IRQ(TIM1_CC_IRQn);
            ENABLE_IRQ();
            break;
        }
        case TIM1_CC_INTERRUPT_DISABLED: TIM1->DIER &= ~(SET_ONE << oc_config->channel); break;
        default: return INVALID_PARAM;
    }

    //configure dma
    switch (oc_config->dma_enable) {
        case TIM1_CC_DMA_ENABLED: TIM1->DIER  |= (SET_ONE << (oc_config->channel + 8U)); break;
        case TIM1_CC_DMA_DISABLED: TIM1->DIER &= ~(SET_ONE << (oc_config->channel + 8U)); break;
        default: return INVALID_PARAM;
    }

    //enable compare
    TIM1->CCER |= (SET_ONE << ((oc_config->channel - 1U) * 4U));

    //enable main output
    TIM1->BDTR |= TIM_BDTR_MOE;

    //enable counter
    if (!(TIM1->CR1 & TIM_CR1_CEN)) {
        TIM1->CR1 |= TIM_CR1_CEN;
    }

    //record utilised interrupt priority level
    if (oc_config->interrupt_enable) {
        irq_priority_tracker[oc_config->interrupt_priority] = 1U;
    }
    
    DSB();
    return SUCCESS;
}

/**
 * @brief  Initialises TIM1 in PWM output mode
 * @param  pwm_output_config: Pointer to TIM1_PWM_Output_Config structure containing PWM output 
 *         settings
 * @retval Status indicating success or invalid parameters
 * @note   Assumes TIM1 has been configured in counter mode via @ref TIM1_CNT_Init
 */
Status TIM1_PWM_Output_Init(TIM1_PWM_Output_Config_t *pwm_output_config) {
    CHECK_STATUS(Validate_Ptr(pwm_output_config));
    CHECK_STATUS(TIM1_Validate_Channel(pwm_output_config->channel));

    if (pwm_output_config->duty_cycle < 0.0f || pwm_output_config->duty_cycle > 1.0f) {
        return INVALID_PARAM;
    }

    //configure channel as PWM output
    TIM1_OC_Config_t pwm_channel = {
        .channel            = pwm_output_config->channel,
        .auto_reload        = pwm_output_config->auto_reload,
        .prescaler          = pwm_output_config->prescaler,
        .compare_value      = (uint16_t)(((float) pwm_output_config->auto_reload) 
                              * pwm_output_config->duty_cycle),
        .oc_mode            = pwm_output_config->oc_mode,
        .preload            = pwm_output_config->preload,
        .polarity           = pwm_output_config->polarity,
        .fast_enable        = pwm_output_config->fast_enable,
        .interrupt_enable   = pwm_output_config->interrupt_enable,
        .interrupt_priority = pwm_output_config->interrupt_priority,
        .dma_enable         = pwm_output_config->dma_enable
    };

    //initialise PWM channel
    CHECK_STATUS(TIM1_OC_Init(&pwm_channel));

    DSB();
    return SUCCESS;
}


/**************************************************************************************************/
/*                                      TIM1 Other Functions                                      */
/**************************************************************************************************/

/**
 * @brief  Sets the PWM duty cycle for a particular TIM1 channel
 * @param  channel:    TIM1 channel whose duty cycle will be set
 * @param  duty_cycle: Duty cycle as a decimal (0.0 - 1.0)
 * @retval Status indicating success or invalid parameters
 * @note   Assumes TIM1 has been configured in PWM output mode via @ref TIM1_PWM_Output_Init
 */
Status TIM1_PWM_Set_Duty_Cycle(TIM1_Channel channel, float duty_cycle) {
    CHECK_STATUS(TIM1_Validate_Channel(channel));
    if (duty_cycle < 0.0f || duty_cycle > 1.0f) {
        return INVALID_PARAM;
    }

    //calculate and update compare value
    uint16_t compare_value = (uint16_t)(((float) TIM1->ARR) * duty_cycle);
    switch (channel) {
        case TIM1_CHANNEL_1: TIM1->CCR1 = compare_value; break;
        case TIM1_CHANNEL_2: TIM1->CCR2 = compare_value; break;
        case TIM1_CHANNEL_3: TIM1->CCR3 = compare_value; break;
        case TIM1_CHANNEL_4: TIM1->CCR4 = compare_value; break;
        default: return INVALID_PARAM;
    }

    DSB();
    return SUCCESS;
}

/**
 * @brief  Deinitialises TIM1
 * @retval Status indicating success
 */
Status TIM1_Deinit(void) {
    //disable TIM1 interrupts and DMA requests
    TIM1->DIER = CLEAR_REGISTER;

    //clear pending interrupts and disable TIM1 interrupts in NVIC
    NVIC_Clear_Pending_IRQ(TIM1_CC_IRQn);
    NVIC_Disable_IRQ(TIM1_CC_IRQn);

    //disable TIM1
    TIM1->CR1 &= ~(TIM_CR1_CEN);

    //set and clear reset bit
    RCC->APB2RSTR |= RCC_APB2RSTR_TIM1RST;
    RCC->APB2RSTR &= ~(RCC_APB2RSTR_TIM1RST);

    //disable TIM1 clock
    RCC->APB2ENR &= ~(RCC_APB2ENR_TIM1EN);

    return SUCCESS;
}

/**
 * @brief  Validates TIM1 channel
 * @param  channel: TIM1 channel whose duty cycle will be set
 * @retval Status indicating success or invalid parameters
 */
Status TIM1_Validate_Channel(TIM1_Channel channel) {
    CHECK_STATUS(Validate_Enum_Param(channel, TIM1_CHANNEL_1, TIM1_CHANNEL_4));
    
    return SUCCESS;
}


/**************************************************************************************************/
/*                                   TIM1 Servo Motor Functions                                   */
/**************************************************************************************************/

/**
 * @brief  Initialises TIM1 in PWM output mode to drive a servo motor
 * @param  channel: TIM1 channel to be used to drive the servo motor
 * @retval Status indicating success or invalid parameters
 * @note   Assumes TIM1 has been configured in counter mode via @ref TIM1_CNT_Init
 * @note   The default duty cycle set of 2.5% sets the servo position to 0 degrees
 */
Status TIM1_Servo_Init(TIM1_Channel channel) {
    //set prescaler value based on system clock source
    uint16_t prescaler_val = 1UL;
    if (g_sys_clk_source == HSI_CLOCK) {
        prescaler_val = 16UL;
    } else if (g_sys_clk_source == HSE_CLOCK) {
        prescaler_val = 25UL;
    }

    //configure PWM output
    TIM1_PWM_Output_Config_t config = {
        .channel     = channel,
        .auto_reload = (20000UL - 1UL),
        .prescaler   = prescaler_val,
        .duty_cycle  = 0.025f,
        .oc_mode     = TIM1_OCM_PWM_1,
        .polarity    = TIM1_CC_ACTIVE_HIGH,
        .preload     = TIM1_OC_PRELOAD_ENABLED
    };

    //initialise PWM output
    CHECK_STATUS(TIM1_PWM_Output_Init(&config));

    return SUCCESS;
}

/**
 * @brief  Sets the angle for a servo driven by a TIM1 channel
 * @param  channel: TIM1 channel to be used to drive the servo motor
 * @param  degrees: Position in degrees to set servo to
 * @retval Status indicating success or invalid parameters
 * @note   The duty cycle formula used in this function is specific to the FS5109M servo
 */
Status TIM1_Servo_Set_Position(TIM1_Channel channel, float degrees) {
    if (degrees < 0.0f || degrees > 180.0f) {
        return INVALID_PARAM;
    }

    //calculate and set duty cycle
    float duty_cycle = (0.025f + ((degrees / 180.0f) * 0.10f));
    CHECK_STATUS(TIM1_PWM_Set_Duty_Cycle(channel, duty_cycle));

    return SUCCESS;
}


/**************************************************************************************************/
/*                                    TIM1 Time Base Functions                                    */
/**************************************************************************************************/

/**
 * @brief  Initialises TIM1 as a time base in milli-seconds
 * @retval Status indicating success or invalid parameters
 * @note   This function is called independent of counter initialisation via @ref TIM1_CNT_Init
 *         If configured as a time base, TIM1 should not be used for any other functionality
 */
Status TIM1_MS_Base_Init(void) {
    //set global tim1 time to 0
    g_tim1_time = 0U;

    //calculate appropriate prescaler value based on system clock source
    uint16_t prescaler_val = 1UL;
    if (g_sys_clk_source == HSI_CLOCK) {
        prescaler_val = 16UL;
    } else if (g_sys_clk_source == HSE_CLOCK) {
        prescaler_val = 25UL;
    }

    //configure settings for time base
    TIM1_CNT_Config_t base_config = {
        .auto_reload = 1000UL,
        .prescaler   = prescaler_val,
        .interrupt_enable = TIM1_INTERRUPT_ENABLED
    };
    
    return TIM1_CNT_Init(&base_config);
}

/**
 * @brief  Delays program execution by a specified number of milli-seconds
 * @param  time_delay: The desired time delay in milli-seconds
 * @retval Status indicating success or invalid parameters
 * @note   Assumes TIM1 has been configured as a time base unit via @ref TIM1_Base_Init
 */
Status TIM1_MS_Delay(uint32_t time_delay) {
    if (time_delay <= 0) {
        return INVALID_PARAM;
    }

    //save current tim1 time and wait for time delay to elapse
    uint32_t prev_tim1_time = g_tim1_time;
    while ((g_tim1_time - prev_tim1_time) < time_delay) {
        NOP();
    }

    return SUCCESS;
}


/**************************************************************************************************/
/*                                     TIM1 Interrupt Handlers                                    */
/**************************************************************************************************/

/** @brief Handles TIM1 update and TIM10 global interrupts */
void TIM1_UP_TIM10_IRQHandler(void) {
    if (TIM1->SR & TIM_SR_UIF) {
        TIM1->SR &= ~(TIM_SR_UIF);
        g_tim1_time++;
    }
}

/** @brief Handles TIM1 capture and compare interrupts */
void TIM1_CC_IRQHandler(void) {
    if (TIM1->SR & TIM_SR_CC1IF) {
        TIM1->SR &= ~(TIM_SR_CC1IF);
        g_prev_cc1 = g_curr_cc1;
        g_curr_cc1 = TIM1->CCR1;
        if (g_prev_cc1 > 0) {
            if (g_curr_cc1 > g_prev_cc1) {
                g_pwm_input_period = (g_curr_cc1 - g_prev_cc1) * g_tim1_tick_time;
                g_pwm_input_duty_cycle = (g_pwm_input_pulse_width / g_pwm_input_period);
            } else {
                g_pwm_input_period = (
                    ((TIM1_CNT_VAL_MAX + g_curr_cc1 + 1) - g_prev_cc1) * g_tim1_tick_time
                );
                g_pwm_input_duty_cycle = (g_pwm_input_pulse_width / g_pwm_input_period);
            }
        }          
    } else if (TIM1->SR & TIM_SR_CC2IF) {
        TIM1->SR &= ~(TIM_SR_CC2IF);
        uint32_t cc2_value = TIM1->CCR2;
        if (cc2_value > g_curr_cc1) {
            g_pwm_input_pulse_width = (cc2_value - g_curr_cc1) * g_tim1_tick_time;
        } else {
            g_pwm_input_pulse_width = (
                ((TIM1_CNT_VAL_MAX + cc2_value + 1) - g_curr_cc1) * g_tim1_tick_time
            );
        }
    }
}

