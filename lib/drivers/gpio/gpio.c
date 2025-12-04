/**
 * @file    gpio.c
 * @brief   STM32F411 GPIO Driver
 * @details This driver provides an interface for the STM32F411 GPIO peripheral. It supports
 *          initialisation/deinitialisation, and control of pin state. 
 * 
 * @par     Driver functions:
 *          - GPIO_Init(): Initialises a GPIO pin
 *          - GPIO_Deinit(): Deinitialises a GPIO pin
 *          - GPIO_Read_Pin(): Reads a GPIO pin to determine bit state
 *          - GPIO_Set_Pin(): Sets a GPIO pin
 *          - GPIO_Reset_Pin(): Resets a GPIO pin
 *          - GPIO_Toggle_Pin(): Toggles the bit state of a GPIO pin
 *          - GPIO_Lock_Pin(): Locks the configuration of a GPIO pin
 */


#include "gpio.h"


/**************************************************************************************************/
/*                                  GPIO Initialisation Functions                                 */
/**************************************************************************************************/

/**
 * @brief  Initialises a GPIO pin
 * @param  gpio_config: Pointer to GPIO_Config structure containing GPIO settings
 * @retval Status indicating success or invalid parameters
 */
Status GPIO_Init(GPIO_Config_t *gpio_config) {
    CHECK_STATUS(Validate_Ptr(gpio_config));

    CHECK_STATUS(Validate_Enum(gpio_config->pin, GPIO_PIN_0, GPIO_PIN_15));
    CHECK_STATUS(Validate_Enum(gpio_config->mode, GPIO_MODE_INPUT, GPIO_MODE_ANALOG));
    CHECK_STATUS(Validate_Enum(gpio_config->output_type, GPIO_OTYPE_PUSH_PULL, GPIO_OTYPE_OPEN_DRAIN));
    CHECK_STATUS(Validate_Enum(gpio_config->output_speed, GPIO_OSPEED_LOW, GPIO_OSPEED_HIGH));
    CHECK_STATUS(Validate_Enum(gpio_config->alt_function, GPIO_AF_0, GPIO_AF_15));
    CHECK_STATUS(Validate_Enum(gpio_config->pupd, GPIO_PUPD_NO, GPIO_PUPD_PULLDOWN));

    //enable clock
    if (gpio_config->port == GPIOA) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    } else if (gpio_config->port == GPIOB) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    } else if (gpio_config->port == GPIOC) {
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    } else {
        return INVALID_PARAM;
    }

    //configure mode
    gpio_config->port->MODER &= ~(SET_TWO << (gpio_config->pin * 2U));
    gpio_config->port->MODER |= (((uint32_t) gpio_config->mode) << (gpio_config->pin * 2U));

    //configure alternate function
    if (gpio_config->mode == GPIO_MODE_AF) {
        uint8_t afr = (gpio_config->pin <= 7) ? 0 : 1;
        if (afr == 0) {
            gpio_config->port->AFR[0] &= ~(SET_FOUR << (gpio_config->pin * 4U));
            gpio_config->port->AFR[0] |= (
                ((uint32_t) gpio_config->alt_function) << (gpio_config->pin * 4U)
            );
        } else {
            gpio_config->port->AFR[1] &= ~(SET_FOUR << ((gpio_config->pin - 8U) * 4U));
            gpio_config->port->AFR[1] |= (
                ((uint32_t) gpio_config->alt_function) << ((gpio_config->pin - 8U) * 4U)
            );
        }
    }

    //configure output type and speed
    if (gpio_config->mode == GPIO_MODE_OUTPUT || gpio_config->mode == GPIO_MODE_AF) {
        gpio_config->port->OTYPER |= (((uint32_t) gpio_config->output_type) << gpio_config->pin);
        gpio_config->port->OSPEEDR &= ~(SET_TWO << (gpio_config->pin * 2));
        gpio_config->port->OSPEEDR |= (
            ((uint32_t) gpio_config->output_speed) << (gpio_config->pin * 2)
        );
    }

    //configure pull-up/pull-down resistors
    gpio_config->port->PUPDR &= ~(SET_TWO << (gpio_config->pin * 2U));
    gpio_config->port->PUPDR |= (((uint32_t) gpio_config->pupd) << (gpio_config->pin * 2U));

    return SUCCESS;
}

/**
* @brief  Deinitialises a GPIO pin
* @param  port: Pointer to GPIO_t structure containing the GPIO port
* @param  pin:  Number of the pin to be deinitialised
* @retval Status indicating success or invalid parameters
*/
Status GPIO_Deinit(GPIO_t *port, GPIO_Pin pin) {
    CHECK_STATUS(Validate_Ptr(port));
    CHECK_STATUS(Validate_Enum(pin, GPIO_PIN_0, GPIO_PIN_15));

    //reset mode
    port->MODER &= ~(SET_TWO << (pin * 2U));

    //reset output type and speed
    port->OTYPER  &= ~(SET_ONE << pin);
    port->OSPEEDR &= ~(SET_TWO << (pin * 2U));

    //reset pull-up/pull-down resistors
    port->PUPDR &= ~(SET_TWO << (pin * 2U));

    //reset alternate function
    uint8_t afr = (pin <= 7) ? 0 : 1;
    if (afr == 0) {
        port->AFR[0] &= ~(SET_FOUR << (pin * 4U));
    } else {
        port->AFR[1] &= ~(SET_FOUR << ((pin - 8U) * 4U));
    }

    return SUCCESS;
}


/**************************************************************************************************/
/*                                     GPIO Modifier Functions                                    */
/**************************************************************************************************/

/**
 * @brief  Reads a GPIO pin to determine bit state
 * @param  port: Pointer to GPIO_t structure containing the GPIO port
 * @param  pin:  Number of the pin to be read
 * @retval Bit_State indicating whether a bit is set or reset
 */
Bit_State GPIO_Read_Pin(GPIO_t *port, GPIO_Pin pin) {
    CHECK_STATUS(Validate_Ptr(port));
    CHECK_STATUS(Validate_Enum(pin, GPIO_PIN_0, GPIO_PIN_15));

    //read the pin's input data register
    if (port->IDR & (SET_ONE << pin)) {
        return BIT_SET;
    } else {
        return BIT_RESET;
    }
}

/**
 * @brief  Sets a GPIO pin
 * @param  port: Pointer to GPIO_t structure containing the GPIO port
 * @param  pin:  Number of the pin to be set
 * @retval Status indicating success or invalid parameters
 */
Status GPIO_Set_Pin(GPIO_t *port, GPIO_Pin pin) {
    CHECK_STATUS(Validate_Ptr(port));
    CHECK_STATUS(Validate_Enum(pin, GPIO_PIN_0, GPIO_PIN_15));

    //set corresponding BSRR bit
    port->BSRR |= (SET_ONE << pin);

    return SUCCESS;
}

/**
 * @brief  Resets a GPIO pin
 * @param  port: Pointer to GPIO_t structure containing the GPIO port
 * @param  pin:  Number of the pin to be reset
 * @retval Status indicating success or invalid parameters
 */
Status GPIO_Reset_Pin(GPIO_t *port, GPIO_Pin pin) {
    CHECK_STATUS(Validate_Ptr(port));
    CHECK_STATUS(Validate_Enum(pin, GPIO_PIN_0, GPIO_PIN_15));

    //reset corresponding BSRR bit
    port->BSRR |= (SET_ONE << (pin + 16U));

    return SUCCESS;
}

/**
* @brief  Toggles the bit state of a GPIO pin
* @param  port: Pointer to GPIO_t structure containing the GPIO port
* @param  pin:  Number of the pin whose bit state will be toggled
* @retval Status indicating success, error or invalid parameters
*/
Status GPIO_Toggle_Pin(GPIO_t *port, GPIO_Pin pin) {
    CHECK_STATUS(Validate_Ptr(port));
    CHECK_STATUS(Validate_Enum(pin, GPIO_PIN_0, GPIO_PIN_15));

    //toggle the pin's output data register
    if (port->ODR & (SET_ONE << pin)) {
        GPIO_Reset_Pin(port, pin);
        return SUCCESS;
    } else {
        GPIO_Set_Pin(port, pin);
        return SUCCESS;
    }

    return ERROR;
}

/**
* @brief  Locks the configuration of a GPIO pin
* @param  port: Pointer to GPIO_t structure containing the GPIO port
* @param  pin:  Number of the pin to be locked
* @retval Status indicating success, error or invalid parameters
*/
Status GPIO_Lock_Pin(GPIO_t *port, GPIO_Pin pin) {
    CHECK_STATUS(Validate_Ptr(port));
    CHECK_STATUS(Validate_Enum(pin, GPIO_PIN_0, GPIO_PIN_15));

    //execute lock sequence
    port->LCKR |= (GPIO_LCKR_LCKK | (SET_ONE << pin));
    port->LCKR = (SET_ONE << pin);
    port->LCKR |= (GPIO_LCKR_LCKK | (SET_ONE << pin));
    uint32_t temp = port->LCKR;
    (void)temp;

    //validate lock
    if (!(port->LCKR & GPIO_LCKR_LCKK)) {
        return ERROR;
    }

    return SUCCESS;
}


/**************************************************************************************************/
/*                                     GPIO Interrupt Handlers                                    */
/**************************************************************************************************/

//insert any interrupt handlers linked to EXTI lines