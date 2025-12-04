#include "bno.h"



/**************************************************************************************************/
/*                              Static Function Forward Declarations                              */
/**************************************************************************************************/

static Status BNO_Read_Reg_Retry(
    USART_Config_t *usart, 
    uint8_t         reg, 
    uint16_t        length, 
    uint8_t        *data
);

static Status BNO_Write_Reg_Retry(
    USART_Config_t *usart, 
    uint8_t         reg, 
    uint16_t        length, 
    uint8_t        *data
);

/**************************************************************************************************/
/*                                      Core Helper Functions                                     */
/**************************************************************************************************/

/**
 * @brief  Send a read command to the BNO055 via USART
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  reg:    Address of the register to be read 
 * @param  length: Number of bytes to be read
 * @param  data:   Pointer to an array that will be used to store the retrieved read values
 * @retval Status indicating success, invalid parameters or error
 * @note   The data array should be initialised as data[BNO_RESPONSE_HEADER_LENGTH + length] 
 */
Status BNO_Read_Reg(USART_Config_t *usart, uint8_t reg, uint16_t length, uint8_t *data) {
    //validate data pointer and length
    CHECK_STATUS(Validate_Ptr(data));
    if (length <= 0U) {
        return INVALID_PARAM;
    }

    //get current global USART state
    volatile USART_State_t *current_state = NULL;
    CHECK_STATUS(USART_Get_State(usart, &current_state));

    //compose and transmit the read command
    uint8_t read_cmd[] = {0xAAU, 0x01U, reg, length};
    CHECK_STATUS(USART_Transmit_IRQ(usart, read_cmd, 4U));

    //wait for tx to complete
    while (current_state->tx_status == USART_TX_BUSY) {};

    //start reception
    CHECK_STATUS(USART_Receive_IRQ(usart, data, BNO_RESPONSE_HEADER_LENGTH + length));

    Delay_Loop(10);

    //calculate timeout
    float timeout_ms = 0.0f;
    float rx_length = BNO_RESPONSE_HEADER_LENGTH + length;
    CHECK_STATUS(USART_Calc_Timeout(usart, &timeout_ms, 5.0f, rx_length));

    //wait for rx to complete or timeout
    uint32_t start_time = g_systick_time;
    while (current_state->rx_status == USART_RX_BUSY) {
        if ((start_time + timeout_ms) < g_systick_time) {
            CHECK_STATUS(USART_Abort_Receive_IRQ(usart));
            break;
        }
    };

    //retry if an error occured
    uint8_t max_retry = 2U;
    Status ret_val = 0;
    if (data[0] == 0xEEU) {
        for (int i = 0; i < max_retry; i++) {
            Delay_Loop(10);
            ret_val = BNO_Read_Reg_Retry(usart, reg, length, data);
            if (ret_val == ERROR) {
                continue;
            } else {
                break;
            }
        }
        return ret_val;
    }

    return SUCCESS;
}

/**
 * @brief  Retry sending a read command to the BNO055 via USART
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  reg:    Address of the register to be read 
 * @param  length: Number of bytes to be read
 * @param  data:   Pointer to an array that will be used to store the retrieved read values
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Read_Reg_Retry(
    USART_Config_t *usart, 
    uint8_t         reg, 
    uint16_t        length, 
    uint8_t        *data
) {
    //validate data pointer and length
    CHECK_STATUS(Validate_Ptr(data));
    if (length <= 0U) {
        return INVALID_PARAM;
    }

    //get current global USART state
    volatile USART_State_t *current_state = NULL;
    CHECK_STATUS(USART_Get_State(usart, &current_state));

    //compose and transmit the read command
    uint8_t read_cmd[] = {0xAAU, 0x01U, reg, length};
    CHECK_STATUS(USART_Transmit_IRQ(usart, read_cmd, 4U));

    //wait for tx to complete
    while (current_state->tx_status == USART_TX_BUSY) {};

    //start reception
    CHECK_STATUS(USART_Receive_IRQ(usart, data, BNO_RESPONSE_HEADER_LENGTH + length));

    Delay_Loop(15);

    //calculate timeout
    float timeout_ms = 0.0f;
    float rx_length = BNO_RESPONSE_HEADER_LENGTH + length;
    CHECK_STATUS(USART_Calc_Timeout(usart, &timeout_ms, 5.0f, rx_length));

    //wait for rx to complete or timeout
    uint32_t start_time = g_systick_time;
    while (current_state->rx_status == USART_RX_BUSY) {
        if ((start_time + timeout_ms) < g_systick_time) {
            CHECK_STATUS(USART_Abort_Receive_IRQ(usart));
            break;
        }
    };

    //check if an error occured
    if (data[0] == 0xEEU) {
        return ERROR;
    }

    return SUCCESS;
}

/**
 * @brief  Send a write command to the BNO055 via USART
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  reg:    Address of the register to be written to 
 * @param  length: Number of bytes to be written
 * @param  data:   Pointer to an array that contains the bytes to be written
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Write_Reg(USART_Config_t *usart, uint8_t reg, uint16_t length, uint8_t *data) {
    //validate data pointer and length
    CHECK_STATUS(Validate_Ptr(data));
    if (length <= 0U) {
        return INVALID_PARAM;
    }

    //get current global USART state
    volatile USART_State_t *current_state = NULL;
    CHECK_STATUS(USART_Get_State(usart, &current_state));

    //compose write command
    uint8_t write_cmd[4 + length];
    write_cmd[0] = 0xAAU;
    write_cmd[1] = 0x00U;
    write_cmd[2] = reg;
    write_cmd[3] = length;
    for (int i = 0; i < length; i++) {
        write_cmd[4 + i] = data[i];
    }

    //transmit write command
    uint8_t cmd_length = 4U + length;
    CHECK_STATUS(USART_Transmit_IRQ(usart, write_cmd, cmd_length));

    //wait for tx to complete
    while (current_state->tx_status == USART_TX_BUSY) {};

    //start reception
    uint8_t write_response[2] = {0};
    CHECK_STATUS(USART_Receive_IRQ(usart, write_response, 2U));

    Delay_Loop(10);

    //calculate timeout
    float timeout_ms = 0.0f;
    float rx_length = BNO_RESPONSE_HEADER_LENGTH;
    CHECK_STATUS(USART_Calc_Timeout(usart, &timeout_ms, 5.0f, rx_length));

    //wait for rx to complete or timeout
    uint32_t start_time = g_systick_time;
    while (current_state->rx_status == USART_RX_BUSY) {
        if ((start_time + timeout_ms) < g_systick_time) {
            CHECK_STATUS(USART_Abort_Receive_IRQ(usart));
            break;
        }
    };

    //retry if an error occured
    uint8_t max_retry = 2U;
    Status ret_val = 0;
    if (write_response[0] == 0xEE && write_response[1] != 0x01) {
        for (int i = 0; i < max_retry; i++) {
            Delay_Loop(10);
            ret_val = BNO_Write_Reg_Retry(usart, reg, length, data);
            if (ret_val == ERROR) {
                continue;
            } else {
                break;
            }
        }
        return ret_val;
    }

    return SUCCESS;
}

/**
 * @brief  Retry sending a write command to the BNO055 via USART
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  reg:    Address of the register to be written to 
 * @param  length: Number of bytes to be written
 * @param  data:   Pointer to an array that contains the bytes to be written
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Write_Reg_Retry(
    USART_Config_t *usart, 
    uint8_t         reg, 
    uint16_t        length, 
    uint8_t        *data
) {
    //validate data pointer and length
    CHECK_STATUS(Validate_Ptr(data));
    if (length <= 0U) {
        return INVALID_PARAM;
    }

    //get current global USART state
    volatile USART_State_t *current_state = NULL;
    CHECK_STATUS(USART_Get_State(usart, &current_state));

    //compose write command
    uint8_t write_cmd[4 + length];
    write_cmd[0] = 0xAAU;
    write_cmd[1] = 0x00U;
    write_cmd[2] = reg;
    write_cmd[3] = length;
    for (int i = 0; i < length; i++) {
        write_cmd[4 + i] = data[i];
    }

    //transmit write command
    uint8_t cmd_length = 4U + length;
    CHECK_STATUS(USART_Transmit_IRQ(usart, write_cmd, cmd_length));

    //wait for tx to complete
    while (current_state->tx_status == USART_TX_BUSY) {};

    //start reception
    uint8_t write_response[2] = {0};
    CHECK_STATUS(USART_Receive_IRQ(usart, write_response, 2));

    Delay_Loop(15);

    //calculate timeout
    float timeout_ms = 0.0f;
    float rx_length = BNO_RESPONSE_HEADER_LENGTH;
    CHECK_STATUS(USART_Calc_Timeout(usart, &timeout_ms, 5.0f, rx_length));

    //wait for rx to complete or timeout
    uint32_t start_time = g_systick_time;
    while (current_state->rx_status == USART_RX_BUSY) {
        if ((start_time + timeout_ms) < g_systick_time) {
            USART_Abort_Receive_IRQ(usart);
            break;
        }
    };

    //check if an error has occured
    if (write_response[0] == 0xEE && write_response[1] != 0x01) {
        return ERROR;
    }
    
    return SUCCESS;
}

/**
 * @brief  Selects either page 0 or page 1 on the register map
 * @param  usart:   Pointer to a struct containing USART settings
 * @param  page_id: Page ID of the page to be selected
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Select_Page(USART_Config_t *usart, BNO_Page_ID page_id) {
    CHECK_STATUS(Validate_Enum(page_id, BNO_PAGE_0, BNO_PAGE_1));

    //get the current page selection
    uint8_t current_page[BNO_RESPONSE_HEADER_LENGTH + BNO_GENERIC_RW_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, BNO_PAGE_ID_REG, 1U, current_page));

    //based on the current page selection, change the page id or return early
    if (current_page[2] != page_id) {
        uint8_t page_val[1];
        if (page_id == BNO_PAGE_0) {
            page_val[0] = 0x00U;
        } else if (page_id == BNO_PAGE_1) {
            page_val[0] = 0x01U;
        }
        Delay_Loop(2);
        return BNO_Write_Reg(usart, BNO_PAGE_ID_REG, 1U, page_val);
    } else {
        return SUCCESS;
    }
}

/**
 * @brief  Configures the BNO055 in CONFIG_MODE
 * @param  usart:            Pointer to a struct containing USART settings
 * @param  current_opr_mode: Pointer to a variable used to store the retrieved current operating mode
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Set_Config_Mode(USART_Config_t *usart, uint8_t *current_opr_mode) {
    CHECK_STATUS(Validate_Ptr(current_opr_mode));

    //retrieve and store current operating mode
    CHECK_STATUS(BNO_Get_OPR_Mode(usart, current_opr_mode));

    //switch to CONFIG_MODE
    if (*current_opr_mode != BNO_OPR_CONFIG_MODE) {
        CHECK_STATUS(BNO_Set_OPR_Mode(usart, BNO_OPR_CONFIG_MODE));
    }

    return SUCCESS;
}


/**************************************************************************************************/
/*                                  Sensor Availability Functions                                 */
/**************************************************************************************************/

/**
 * @brief  Validates the availability of the acceloremeter based on operating mode
 * @param  usart: Pointer to a struct containing USART settings
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Validate_ACC_Avail(USART_Config_t *usart) {
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Get_OPR_Mode(usart, &current_opr_mode));

    if (current_opr_mode == BNO_OPR_CONFIG_MODE    || current_opr_mode  == BNO_OPR_MAG_ONLY_MODE 
    ||  current_opr_mode == BNO_OPR_GYR_ONLY_MODE  || current_opr_mode  == BNO_OPR_MAG_GYR_MODE) {
        return ERROR;
    } 

    return SUCCESS;
}

/**
 * @brief  Validates the availability of the mag based on operating mode
 * @param  usart: Pointer to a struct containing USART settings
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Validate_MAG_Avail(USART_Config_t *usart) {
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Get_OPR_Mode(usart, &current_opr_mode));

    if (current_opr_mode == BNO_OPR_CONFIG_MODE   ||  current_opr_mode == BNO_OPR_ACC_ONLY_MODE 
    ||  current_opr_mode == BNO_OPR_GYR_ONLY_MODE ||  current_opr_mode == BNO_OPR_ACC_GYR_MODE 
    ||  current_opr_mode == BNO_OPR_IMU_MODE) {
        return ERROR;
    }

    return SUCCESS;
}

/**
 * @brief  Validates the availability of the gyr based on operating mode
 * @param  usart: Pointer to a struct containing USART settings
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Validate_GYR_Avail(USART_Config_t *usart) {
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Get_OPR_Mode(usart, &current_opr_mode));

    if (current_opr_mode == BNO_OPR_CONFIG_MODE   || current_opr_mode  == BNO_OPR_ACC_ONLY_MODE 
    ||  current_opr_mode == BNO_OPR_MAG_ONLY_MODE ||  current_opr_mode == BNO_OPR_ACC_MAG_MODE 
    ||  current_opr_mode == BNO_OPR_COMPASS_MODE  || current_opr_mode  == BNO_OPR_M4G_MODE) {
        return ERROR;
    }

    return SUCCESS;
}


/**************************************************************************************************/
/*                           System and Sensor Initialisation Functions                           */
/**************************************************************************************************/

/**
 * @brief  Writes a setting
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  reg:         Register address of setting
 * @param  mask:        Bit mask of setting
 * @param  setting_val: Value of setting
 * @retval Status indicating success, invalid parameters or error
 * @note   The appropriate page should be selected before this function is called 
 * @note   If no bits will be cleared, mask = 0x00U
 */
static Status BNO_Set_Setting(
    USART_Config_t *usart, 
    uint8_t        reg, 
    uint8_t        mask, 
    uint8_t        setting_val
) {
    //read register value and clear relevant bits
    uint8_t reg_val_og[BNO_RESPONSE_HEADER_LENGTH + BNO_GENERIC_RW_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, reg, 1U, reg_val_og));
    uint8_t reg_val_clear = (reg_val_og[2] & ~(mask));

    //modify and write the register value back
    uint8_t reg_val_mod = (reg_val_clear | setting_val);
    CHECK_STATUS(BNO_Write_Reg(usart, reg, 1U, &reg_val_mod));

    return SUCCESS;
}

/**
 * @brief  Reads a setting
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  reg:         Register address of setting
 * @param  mask:        Bit mask of setting
 * @param  setting_val: Pointer to variable used to store bit value of setting
 * @retval Status indicating success, invalid parameters or error
 * @note   The appropriate page should be selected before this function is called  
 */
static Status BNO_Get_Setting(
    USART_Config_t *usart, 
    uint8_t        reg, 
    uint8_t        mask, 
    uint8_t        *setting_val
) {
    CHECK_STATUS(Validate_Ptr(setting_val));

    //read register value and extract setting value
    uint8_t reg_val_og[BNO_RESPONSE_HEADER_LENGTH + BNO_GENERIC_RW_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, reg, 1U, reg_val_og));
    *setting_val = (reg_val_og[2] & mask);

    return SUCCESS;
}

/**
 * @brief  Initialises the BNO055 sensor
 * @param  usart:      Pointer to a struct containing USART settings
 * @param  bno_config: Pointer to a struct containing BNO055 init settings
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Init(USART_Config_t *usart, BNO_Config_t *bno_config) {
    CHECK_STATUS(Validate_Ptr(bno_config));
    CHECK_STATUS(Validate_Enum(bno_config->pwr_mode, BNO_PWR_NORMAL_MODE, BNO_PWR_SUSPEND_MODE));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //configure power mode
    uint8_t clear_pwr_mode_val[] = {(~((uint8_t) BNO_PWR_MODE))};
    CHECK_STATUS(BNO_Write_Reg(usart, BNO_PWR_MODE_REG, 1U, clear_pwr_mode_val));

    uint8_t pwr_mode_val[1];
    if (bno_config->pwr_mode == BNO_PWR_NORMAL_MODE) {
        pwr_mode_val[0] = BNO_PWR_MODE_NORMAL;
    } else if (bno_config->pwr_mode == BNO_PWR_LOW_PWR_MODE) {
        pwr_mode_val[0] = BNO_PWR_MODE_LOW_PWR;
    } else if (bno_config->pwr_mode == BNO_PWR_SUSPEND_MODE) {
        pwr_mode_val[0] = BNO_PWR_MODE_SUSPEND;
    }
    CHECK_STATUS(BNO_Write_Reg(usart, BNO_PWR_MODE_REG, 1U, pwr_mode_val));

    //configure operating mode
    uint8_t clear_opr_mode_val[] = {(~((uint8_t) BNO_OPR_MODE))};
    CHECK_STATUS(BNO_Write_Reg(usart, BNO_OPR_MODE_REG, 1U, clear_opr_mode_val));

    //delay by the max time required to switch operating modes i.e. any other mode to CONFIG_MODE
    Delay_Loop(20);

    CHECK_STATUS(Validate_Enum(bno_config->opr_mode, BNO_OPR_CONFIG_MODE, BNO_OPR_NDOF_MODE));
    uint8_t opr_mode_val[] = {((uint8_t) (bno_config->opr_mode))};
    CHECK_STATUS(BNO_Write_Reg(usart, BNO_OPR_MODE_REG, 1U, opr_mode_val));

    //delay by the max time required to switch operating modes i.e. any other mode to CONFIGMODE
    Delay_Loop(20);

    return SUCCESS;
}

/**
 * @brief  Initialises the acc
 * @param  usart:      Pointer to a struct containing USART settings
 * @param  acc_config: Pointer to a struct containing acc init settings 
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_ACC_Init(USART_Config_t *usart, BNO_ACC_Config_t *acc_config) {
    CHECK_STATUS(BNO_Validate_ACC_Avail(usart));
    CHECK_STATUS(Validate_Ptr(acc_config));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //configure range
    CHECK_STATUS(Validate_Enum(acc_config->acc_range, BNO_ACC_RANGE_2G, BNO_ACC_RANGE_16G));
    CHECK_STATUS(BNO_Set_Setting(
        usart, 
        BNO_ACC_CONFIG_REG, 
        BNO_ACC_CONFIG_RANGE, 
        (uint8_t) acc_config->acc_range
    ));

    //configure bandwidth
    CHECK_STATUS(Validate_Enum(acc_config->acc_bw, BNO_ACC_BW_7_81_HZ, BNO_ACC_BW_1000_HZ));
    CHECK_STATUS(BNO_Set_Setting(
        usart, 
        BNO_ACC_CONFIG_REG, 
        BNO_ACC_CONFIG_BW, 
        (uint8_t) acc_config->acc_bw
    ));

    //configure power mode
    CHECK_STATUS(Validate_Enum(
        acc_config->acc_pwr_mode, 
        BNO_ACC_PWR_MODE_NORMAL, 
        BNO_ACC_PWR_MODE_DEEP_SUSPEND
    ));
    CHECK_STATUS(BNO_Set_Setting(
        usart, 
        BNO_ACC_CONFIG_REG, 
        BNO_ACC_CONFIG_PWR_MODE, 
        (uint8_t) acc_config->acc_pwr_mode
    ));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Initialises the mag
 * @param  usart:      Pointer to a struct containing USART settings
 * @param  mag_config: Pointer to a struct containing mag init settings
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_MAG_Init(USART_Config_t *usart, BNO_MAG_Config_t *mag_config) {
    CHECK_STATUS(BNO_Validate_MAG_Avail(usart));
    CHECK_STATUS(Validate_Ptr(mag_config));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //configure data output rate
    CHECK_STATUS(Validate_Enum(mag_config->mag_dor, BNO_MAG_DOR_2_HZ, BNO_MAG_DOR_30_HZ));
    CHECK_STATUS(BNO_Set_Setting(
        usart, 
        BNO_MAG_CONFIG_REG, 
        BNO_MAG_CONFIG_DOR, 
        mag_config->mag_dor
    ));

    //configure operation mode
    CHECK_STATUS(Validate_Enum(
        mag_config->mag_opr_mode, 
        BNO_MAG_OPR_MODE_LOW_PWR, 
        BNO_MAG_OPR_MODE_HI_ACC
    ));
    CHECK_STATUS(BNO_Set_Setting(
        usart, 
        BNO_MAG_CONFIG_REG, 
        BNO_MAG_CONFIG_OPR_MODE, 
        mag_config->mag_opr_mode
    ));

    //configure power mode
    CHECK_STATUS(Validate_Enum(
        mag_config->mag_pwr_mode, 
        BNO_MAG_PWR_MODE_NORMAL, 
        BNO_MAG_PWR_MODE_FORCE
    ));
    CHECK_STATUS(BNO_Set_Setting(
        usart, 
        BNO_MAG_CONFIG_REG, 
        BNO_MAG_CONFIG_PWR_MODE, 
        mag_config->mag_pwr_mode
    ));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Initialises the mag
 * @param  usart:      Pointer to a struct containing USART settings
 * @param  gyr_config: Pointer to a struct containing gyr init settings
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_GYR_Init(USART_Config_t *usart, BNO_GYR_Config_t *gyr_config) {
    CHECK_STATUS(BNO_Validate_GYR_Avail(usart));
    CHECK_STATUS(Validate_Ptr(gyr_config));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //configure range
    CHECK_STATUS(Validate_Enum(gyr_config->gyr_range, BNO_GYR_RANGE_2000_DPS, BNO_GYR_RANGE_125_DPS));
    CHECK_STATUS(BNO_Set_Setting(
        usart, 
        BNO_GYR_CONFIG_0_REG, 
        BNO_GYR_CONFIG_0_RANGE, 
        (uint8_t) gyr_config->gyr_range
    ));

    //configure bandwidth
    CHECK_STATUS(Validate_Enum(
        gyr_config->gyr_bw, 
        BNO_GYR_BW_523_HZ, 
        BNO_GYR_BW_32_HZ
    ));
    CHECK_STATUS(BNO_Set_Setting(
        usart, 
        BNO_GYR_CONFIG_0_REG, 
        BNO_GYR_CONFIG_0_BW, 
        (uint8_t) gyr_config->gyr_bw
    ));

    //configure power mode
    CHECK_STATUS(Validate_Enum(
        gyr_config->gyr_pwr_mode, 
        BNO_GYR_PWR_MODE_NORMAL, 
        BNO_GYR_PWR_MODE_ADV_PWRSAVE
    ));
    CHECK_STATUS(BNO_Set_Setting(
        usart, 
        BNO_GYR_CONFIG_1_REG, 
        BNO_GYR_CONFIG_1_PWR_MODE, 
        (uint8_t) gyr_config->gyr_pwr_mode
    ));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Sets the power mode
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  pwr_mode: New power mode to be configured
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_PWR_Mode(USART_Config_t *usart, BNO_PWR_Mode pwr_mode) {
    CHECK_STATUS(Validate_Enum(pwr_mode, BNO_PWR_NORMAL_MODE, BNO_PWR_SUSPEND_MODE));
    
    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //save operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //write power mode selection
    uint8_t setting_val = ((uint8_t) pwr_mode);
    CHECK_STATUS(BNO_Set_Setting(usart, BNO_PWR_MODE_REG, BNO_PWR_MODE, setting_val));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the current power mode
 * @param  usart:            Pointer to a struct containing USART settings
 * @param  current_pwr_mode: Pointer to a variable used to store the retrieved current power mode
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_PWR_Mode(USART_Config_t *usart, uint8_t *current_pwr_mode) {
    CHECK_STATUS(Validate_Ptr(current_pwr_mode));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //transmit read command
    uint8_t data[BNO_RESPONSE_HEADER_LENGTH + BNO_GENERIC_RW_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, BNO_PWR_MODE_REG, 1U, data));

    //extract and store current power mode
    *current_pwr_mode = data[2];

    return SUCCESS;
}

/**
 * @brief  Sets the operating mode
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  opr_mode: New operating mode to be configured
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_OPR_Mode(USART_Config_t *usart, BNO_OPR_Mode opr_mode) {
    CHECK_STATUS(Validate_Enum(opr_mode, BNO_OPR_CONFIG_MODE, BNO_OPR_NDOF_MODE));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //read and store current operating mode
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Get_OPR_Mode(usart, &current_opr_mode));

    //write operating mode selection
    uint8_t setting_val = ((uint8_t) opr_mode);
    CHECK_STATUS(BNO_Set_Setting(usart, BNO_OPR_MODE_REG, BNO_OPR_MODE, setting_val));

    //delay by operating mode switching time if switching to/from CONFIG_MODE
    if (opr_mode == BNO_OPR_CONFIG_MODE && current_opr_mode != BNO_OPR_CONFIG_MODE) {
        Delay_Loop(19);
    } else if (current_opr_mode == BNO_OPR_CONFIG_MODE && opr_mode != BNO_OPR_CONFIG_MODE) {
        Delay_Loop(7);
    }

    return SUCCESS;
}

/**
 * @brief  Gets the current operating mode
 * @param  usart:            Pointer to a struct containing USART settings
 * @param  current_opr_mode: Pointer to a variable used to store the retrieved current operating mode
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_OPR_Mode(USART_Config_t *usart, uint8_t *current_opr_mode) {
    CHECK_STATUS(Validate_Ptr(current_opr_mode));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //transmit read command
    uint8_t data[BNO_RESPONSE_HEADER_LENGTH + BNO_GENERIC_RW_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, BNO_OPR_MODE_REG, 1U, data));

    //extract and store current operating mode
    *current_opr_mode = data[2];

    return SUCCESS;
}


/**************************************************************************************************/
/*                           Sensor Settings Configuration Functions                              */
/**************************************************************************************************/

/**
 * @brief  Writes a sensor setting
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  sensor:      Sensor
 * @param  mask:        Bit mask of the sensor setting
 * @param  setting_val: New value of sensor setting
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Set_Sensor_Setting(
    USART_Config_t    *usart,
    BNO_Sensor_Config sensor,
    uint8_t           mask,
    uint8_t           setting_val
) {
    CHECK_STATUS(Validate_Enum(sensor, BNO_ACC_CONFIG, BNO_GYR_1_CONFIG));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //save operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //write sensor setting to appropriate sensor base address
    uint8_t sensor_base_adr = 0U;
    if (sensor == BNO_ACC_CONFIG) {
        sensor_base_adr = BNO_ACC_CONFIG_REG;
    } else if (sensor == BNO_MAG_CONFIG) {
        sensor_base_adr = BNO_MAG_CONFIG_REG;
    } else if (sensor == BNO_GYR_0_CONFIG) {
        sensor_base_adr = BNO_GYR_CONFIG_0_REG;
    } else if (sensor == BNO_GYR_1_CONFIG) {
        sensor_base_adr = BNO_GYR_CONFIG_1_REG;
    }
    CHECK_STATUS(BNO_Set_Setting(usart, sensor_base_adr, mask, setting_val));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Reads a sensor setting
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  sensor:      Sensor
 * @param  mask:        Bit mask of sensor setting
 * @param  setting_val: Bit value of sensor setting
 * @retval Status indicating success, invalid parameters or error 
 */
static Status BNO_Get_Sensor_Setting(
    USART_Config_t    *usart, 
    BNO_Sensor_Config sensor, 
    uint8_t           mask, 
    uint8_t           *setting_val
) {
    CHECK_STATUS(Validate_Enum(sensor, BNO_ACC_CONFIG, BNO_GYR_1_CONFIG));
    CHECK_STATUS(Validate_Ptr(setting_val));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read sensor setting at appropriate sensor base address
    uint8_t sensor_base_adr = 0U;
    if (sensor == BNO_ACC_CONFIG) {
        sensor_base_adr = BNO_ACC_CONFIG_REG;
    } else if (sensor == BNO_MAG_CONFIG) {
        sensor_base_adr = BNO_MAG_CONFIG_REG;
    } else if (sensor == BNO_GYR_0_CONFIG) {
        sensor_base_adr = BNO_GYR_CONFIG_0_REG;
    } else if (sensor == BNO_GYR_1_CONFIG) {
        sensor_base_adr = BNO_GYR_CONFIG_1_REG;
    }
    CHECK_STATUS(BNO_Get_Setting(usart, sensor_base_adr, mask, setting_val));

    return SUCCESS;
}

/**
 * @brief  Sets acc range
 * @param  usart:     Pointer to a struct containing USART settings
 * @param  acc_range: Value of acc range
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_ACC_Range(USART_Config_t *usart, BNO_ACC_Range acc_range) {
    CHECK_STATUS(Validate_Enum(acc_range, BNO_ACC_RANGE_2G, BNO_ACC_RANGE_16G));

    return BNO_Set_Sensor_Setting(usart, BNO_ACC_CONFIG, BNO_ACC_CONFIG_RANGE, acc_range);
}

/**
 * @brief  Gets acc range
 * @param  usart:     Pointer to a struct containing USART settings
 * @param  acc_range: Pointer to a variable used to store acc range
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_ACC_Range(USART_Config_t *usart, uint8_t *acc_range) {
    CHECK_STATUS(Validate_Ptr(acc_range));

    CHECK_STATUS(BNO_Get_Sensor_Setting(
        usart, 
        BNO_ACC_CONFIG, 
        BNO_ACC_CONFIG_RANGE, 
        acc_range
    ));

    return SUCCESS;
}

/**
 * @brief  Sets acc bandwidth
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  acc_bw: Value of acc bandwidth
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_ACC_BW(USART_Config_t *usart, BNO_ACC_BW acc_bw) {
    CHECK_STATUS(Validate_Enum(acc_bw, BNO_ACC_BW_7_81_HZ, BNO_ACC_BW_1000_HZ));

    return BNO_Set_Sensor_Setting(usart, BNO_ACC_CONFIG, BNO_ACC_CONFIG_BW, acc_bw);
}

/**
 * @brief  Gets acc bandwidth
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  acc_bw: Pointer to a variable used to store acc bandwidth
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_ACC_BW(USART_Config_t *usart, uint8_t *acc_bw) {
    CHECK_STATUS(Validate_Ptr(acc_bw));

    CHECK_STATUS(BNO_Get_Sensor_Setting(usart, BNO_ACC_CONFIG, BNO_ACC_CONFIG_BW, acc_bw));
    *acc_bw = (*acc_bw >> BNO_ACC_CONFIG_BW_Pos);

    return SUCCESS;
}

/**
 * @brief  Sets acc power mode
 * @param  usart:        Pointer to a struct containing USART settings
 * @param  acc_pwr_mode: Value of acc power mode
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_ACC_PWR_Mode(USART_Config_t *usart, BNO_ACC_PWR_Mode acc_pwr_mode) {
    CHECK_STATUS(Validate_Enum(
        acc_pwr_mode, 
        BNO_ACC_PWR_MODE_NORMAL, 
        BNO_ACC_PWR_MODE_DEEP_SUSPEND
    ));

    return BNO_Set_Sensor_Setting(
        usart, 
        BNO_ACC_CONFIG, 
        BNO_ACC_CONFIG_PWR_MODE, 
        acc_pwr_mode
    );
}

/**
 * @brief  Gets acc power mode
 * @param  usart:        Pointer to a struct containing USART settings
 * @param  acc_pwr_mode: Pointer to a variable used to store acc power mode
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_ACC_PWR_Mode(USART_Config_t *usart, uint8_t *acc_pwr_mode) {
    CHECK_STATUS(Validate_Ptr(acc_pwr_mode));

    CHECK_STATUS(BNO_Get_Sensor_Setting(
        usart, 
        BNO_ACC_CONFIG, 
        BNO_ACC_CONFIG_PWR_MODE, 
        acc_pwr_mode
    ));
    *acc_pwr_mode = (*acc_pwr_mode >> BNO_ACC_CONFIG_PWR_MODE_Pos);

    return SUCCESS;
}

/**
 * @brief  Sets mag data output rate
 * @param  usart:   Pointer to a struct containing USART settings
 * @param  mag_dor: Value of mag data output rate
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_MAG_DOR(USART_Config_t *usart, BNO_MAG_DOR mag_dor) {
    CHECK_STATUS(Validate_Enum(mag_dor, BNO_MAG_DOR_2_HZ, BNO_MAG_DOR_30_HZ));

    return BNO_Set_Sensor_Setting(usart, BNO_MAG_CONFIG, BNO_MAG_CONFIG_DOR, mag_dor);
}

/**
 * @brief  Gets mag data output rate
 * @param  usart:   Pointer to a struct containing USART settings
 * @param  mag_dor: Pointer to a variable used to store mag data output rate
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_MAG_DOR(USART_Config_t *usart, uint8_t *mag_dor) {
    CHECK_STATUS(Validate_Ptr(mag_dor));

    CHECK_STATUS(BNO_Get_Sensor_Setting(
        usart, BNO_MAG_CONFIG, 
        BNO_MAG_CONFIG_DOR, 
        mag_dor
    ));

    return SUCCESS;
}

/**
 * @brief  Sets mag operating mode
 * @param  usart:        Pointer to a struct containing USART settings
 * @param  mag_opr_mode: Value of mag operating mode
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_MAG_OPR_Mode(USART_Config_t *usart, BNO_MAG_OPR_Mode mag_opr_mode) {
    CHECK_STATUS(Validate_Enum(mag_opr_mode, BNO_MAG_OPR_MODE_LOW_PWR, BNO_MAG_OPR_MODE_HI_ACC));

    return BNO_Set_Sensor_Setting(
        usart, 
        BNO_MAG_CONFIG, 
        BNO_MAG_CONFIG_OPR_MODE, 
        mag_opr_mode
    );
}

/**
 * @brief  Gets mag operating mode
 * @param  usart:        Pointer to a struct containing USART settings
 * @param  mag_opr_mode: Pointer to a variable used to store mag operating mode
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_MAG_OPR_Mode(USART_Config_t *usart, uint8_t *mag_opr_mode) {
    CHECK_STATUS(Validate_Ptr(mag_opr_mode));

    CHECK_STATUS(BNO_Get_Sensor_Setting(
        usart, 
        BNO_MAG_CONFIG, 
        BNO_MAG_CONFIG_OPR_MODE, 
        mag_opr_mode
    ));
    *mag_opr_mode = (*mag_opr_mode >> BNO_MAG_CONFIG_OPR_MODE_Pos);

    return SUCCESS;
}

/**
 * @brief  Sets mag power mode
 * @param  usart:        Pointer to a struct containing USART settings
 * @param  mag_pwr_mode: Value of mag power mode
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_MAG_PWR_Mode(USART_Config_t *usart, BNO_MAG_PWR_Mode mag_pwr_mode) {
    CHECK_STATUS(Validate_Enum(mag_pwr_mode, BNO_MAG_PWR_MODE_NORMAL, BNO_MAG_PWR_MODE_FORCE));

    return BNO_Set_Sensor_Setting(
        usart, 
        BNO_MAG_CONFIG, 
        BNO_MAG_CONFIG_PWR_MODE, 
        mag_pwr_mode
    );
}

/**
 * @brief  Gets mag power mode
 * @param  usart:        Pointer to a struct containing USART settings
 * @param  mag_pwr_mode: Pointer to a variable used to store mag power mode
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_MAG_PWR_Mode(USART_Config_t *usart, uint8_t *mag_pwr_mode) {
    CHECK_STATUS(Validate_Ptr(mag_pwr_mode));

    CHECK_STATUS(BNO_Get_Sensor_Setting(
        usart, 
        BNO_MAG_CONFIG, 
        BNO_MAG_CONFIG_PWR_MODE, 
        mag_pwr_mode
    ));
    *mag_pwr_mode = (*mag_pwr_mode >> BNO_MAG_CONFIG_PWR_MODE_Pos);

    return SUCCESS;
}

/**
 * @brief  Sets gyr range
 * @param  usart:     Pointer to a struct containing USART settings
 * @param  gyr_range: Value of gyr range
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_GYR_Range(USART_Config_t *usart, BNO_GYR_Range gyr_range) {
    CHECK_STATUS(Validate_Enum(gyr_range, BNO_GYR_RANGE_2000_DPS, BNO_GYR_RANGE_125_DPS));

    return BNO_Set_Sensor_Setting(usart, BNO_GYR_0_CONFIG, BNO_GYR_CONFIG_0_REG, gyr_range);
}

/**
 * @brief  Gets gyr range
 * @param  usart:     Pointer to a struct containing USART settings
 * @param  gyr_range: Pointer to a variable used to store gyr range
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GYR_Range(USART_Config_t *usart, uint8_t *gyr_range) {
    CHECK_STATUS(Validate_Ptr(gyr_range));

    CHECK_STATUS(BNO_Get_Sensor_Setting(
        usart, 
        BNO_GYR_0_CONFIG, 
        BNO_GYR_CONFIG_0_REG, 
        gyr_range
    ));

    return SUCCESS;
}

/**
 * @brief  Sets gyr bandwidth
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  gyr_bw: Value of gyr bandwidth
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_GYR_BW(USART_Config_t *usart, BNO_GYR_BW gyr_bw) {
    CHECK_STATUS(Validate_Enum(gyr_bw, BNO_GYR_BW_523_HZ, BNO_GYR_BW_32_HZ));

    return BNO_Set_Sensor_Setting(usart, BNO_GYR_0_CONFIG, BNO_GYR_CONFIG_0_REG, gyr_bw);
}

/**
 * @brief  Gets gyr bandwidth
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  gyr_bw: Pointer to a variable used to store gyr bandwidth
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GYR_BW(USART_Config_t *usart, uint8_t *gyr_bw) {
    CHECK_STATUS(Validate_Ptr(gyr_bw));

    CHECK_STATUS(BNO_Get_Sensor_Setting(
        usart, 
        BNO_GYR_0_CONFIG, 
        BNO_GYR_CONFIG_0_REG, 
        gyr_bw
    ));
    *gyr_bw = (*gyr_bw >> BNO_GYR_CONFIG_0_BW_Pos);

    return SUCCESS;
}

/**
 * @brief  Sets gyr power mode
 * @param  usart:        Pointer to a struct containing USART settings
 * @param  gyr_pwr_mode: Value of gyr power mode
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_GYR_PWR_Mode(USART_Config_t *usart, BNO_GYR_PWR_Mode gyr_pwr_mode) {
    CHECK_STATUS(Validate_Enum(
        gyr_pwr_mode, 
        BNO_GYR_PWR_MODE_NORMAL, 
        BNO_GYR_PWR_MODE_ADV_PWRSAVE
    ));
    return BNO_Set_Sensor_Setting(
        usart, 
        BNO_GYR_1_CONFIG, 
        BNO_GYR_CONFIG_1_REG, 
        gyr_pwr_mode
    );
}

/**
 * @brief  Gets gyr power mode
 * @param  usart:        Pointer to a struct containing USART settings
 * @param  gyr_pwr_mode: Pointer to a variable used to store gyr power mode
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GYR_PWR_Mode(USART_Config_t *usart, uint8_t *gyr_pwr_mode) {
    CHECK_STATUS(Validate_Ptr(gyr_pwr_mode));

    CHECK_STATUS(BNO_Get_Sensor_Setting(
        usart, 
        BNO_GYR_1_CONFIG, 
        BNO_GYR_CONFIG_1_REG, 
        gyr_pwr_mode
    ));
    *gyr_pwr_mode = (*gyr_pwr_mode >> BNO_GYR_CONFIG_1_PWR_MODE_Pos);

    return SUCCESS;
}


/**************************************************************************************************/
/*                             Low Power Sleep Configuration Functions                            */
/**************************************************************************************************/

/**
 * @brief  Configures acc sleep settings
 * @param  usart:      Pointer to a struct containing USART settings
 * @param  slp_config: Pointer to structure containing acc sleep config settings
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_ACC_Slp_Config(USART_Config_t *usart, BNO_ACC_Slp_Config_t *slp_config) {
    CHECK_STATUS(Validate_Ptr(slp_config));

    //configure sleep mode and sleep duration (if required)
    CHECK_STATUS(BNO_Set_ACC_Slp_Mode(usart, slp_config->slp_mode));
    if (!(slp_config->slp_mode)) {
        CHECK_STATUS(BNO_Set_ACC_Slp_Dur(usart, slp_config->slp_dur));
    }

    return SUCCESS;
}

/**
 * @brief  Sets acc sleep mode
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  slp_mode: Sleep mode setting
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_ACC_Slp_Mode(USART_Config_t *usart, BNO_ACC_Slp_Mode slp_mode) {
    CHECK_STATUS(Validate_Enum(slp_mode, BNO_ACC_SLP_EVENT_MODE, BNO_ACC_SLP_SAMPLING_MODE));

    //validate acc is in low-power mode
    uint8_t acc_pwr_mode = 0U;
    CHECK_STATUS(BNO_Get_ACC_PWR_Mode(usart, &acc_pwr_mode));
    if (acc_pwr_mode != BNO_ACC_PWR_MODE_LOW_POWER_1 
    &&  acc_pwr_mode != BNO_ACC_PWR_MODE_LOW_POWER_2) {
        return ERROR;
    }

    //validate sensor is in a non-fusion operating mode
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Get_OPR_Mode(usart, &current_opr_mode));
    if (Validate_Enum(current_opr_mode, BNO_OPR_CONFIG_MODE, BNO_OPR_AMG_MODE) != SUCCESS) {
        return ERROR;
    }

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //configure sleep mode
    uint8_t mask        = BNO_ACC_SLEEP_CONFIG_SLP_MODE;
    uint8_t setting_val = (uint8_t) (slp_mode);
    CHECK_STATUS(BNO_Set_Setting(usart, BNO_ACC_SLEEP_CONFIG_REG, mask, setting_val));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets acc sleep mode
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  slp_mode: Pointer to a variable used to store acc sleep mode
 * @retval Status indicating success, invalid parameters or error
 * @note   If slp_mode = 0, event driven mode is selected; otherwise, equidistant sampling mode is 
 *         selected
 */
Status BNO_Get_ACC_Slp_Mode(USART_Config_t *usart, uint8_t *slp_mode) {
    CHECK_STATUS(Validate_Ptr(slp_mode));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read sleep mode
    CHECK_STATUS(BNO_Get_Setting(
        usart,
        BNO_ACC_SLEEP_CONFIG_REG,
        BNO_ACC_SLEEP_CONFIG_SLP_MODE,
        slp_mode
    ));

    return SUCCESS;
}

/**
 * @brief  Sets acc sleep duration
 * @param  usart:   Pointer to a struct containing USART settings
 * @param  slp_dur: Sleep duration setting
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_ACC_Slp_Dur(USART_Config_t *usart, BNO_ACC_Slp_Dur slp_dur) {
    CHECK_STATUS(Validate_Enum(slp_dur, BNO_ACC_SLP_DUR_0_5_MS, BNO_ACC_SLP_DUR_1000_MS));

    //validate acc is in low-power mode
    uint8_t acc_pwr_mode = 0U;
    CHECK_STATUS(BNO_Get_ACC_PWR_Mode(usart, &acc_pwr_mode));
    if (acc_pwr_mode != BNO_ACC_PWR_MODE_LOW_POWER_1 
    &&  acc_pwr_mode != BNO_ACC_PWR_MODE_LOW_POWER_2) {
        return ERROR;
    }

    //validate sensor is in a non-fusion operating mode
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Get_OPR_Mode(usart, &current_opr_mode));
    if (Validate_Enum(current_opr_mode, BNO_OPR_CONFIG_MODE, BNO_OPR_AMG_MODE) != SUCCESS) {
        return ERROR;
    }

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //configure sleep duration
    uint8_t setting_val = (((uint8_t) slp_dur) << BNO_ACC_SLEEP_CONFIG_SLP_DUR_Pos);
    CHECK_STATUS(BNO_Set_Setting(
        usart,
        BNO_ACC_SLEEP_CONFIG_REG,
        BNO_ACC_SLEEP_CONFIG_SLP_DUR,
        setting_val
    ));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets acc sleep duration
 * @param  usart:   Pointer to a struct containing USART settings
 * @param  slp_dur: Pointer to a variable used to store acc sleep duration
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_ACC_Slp_Dur(USART_Config_t *usart, uint8_t *slp_dur) {
    CHECK_STATUS(Validate_Ptr(slp_dur));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read and store sleep duration
    CHECK_STATUS(BNO_Get_Setting(
        usart,
        BNO_ACC_SLEEP_CONFIG_REG,
        BNO_ACC_SLEEP_CONFIG_SLP_DUR,
        slp_dur
    ));
    *slp_dur = (*slp_dur >> BNO_ACC_SLEEP_CONFIG_SLP_DUR_Pos);

    return SUCCESS;
}

/**
 * @brief  Configures gyr sleep settings
 * @param  usart:      Pointer to a struct containing USART settings
 * @param  slp_config: Pointer to structure containing gyr sleep config settings
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_GYR_Slp_Config(USART_Config_t *usart, BNO_GYR_Slp_Config_t *slp_config) {
    CHECK_STATUS(Validate_Ptr(slp_config));

    //configure sleep duration and auto sleep duration
    CHECK_STATUS(BNO_Set_GYR_Slp_Dur(usart, slp_config->slp_dur));
    CHECK_STATUS(BNO_Set_GYR_Slp_Auto_Dur(usart, slp_config->auto_dur));

    return SUCCESS;
}

/**
 * @brief  Sets gyr sleep duration
 * @param  usart:   Pointer to a struct containing USART settings
 * @param  slp_dur: Sleep duration setting
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_GYR_Slp_Dur(USART_Config_t *usart, BNO_GYR_Slp_Dur slp_dur) {
    CHECK_STATUS(Validate_Enum(slp_dur, BNO_GYR_SLP_DUR_2_MS, BNO_GYR_SLP_DUR_20_MS));

    //validate gyr is in advanced power mode
    uint8_t gyr_pwr_mode = 0U;
    CHECK_STATUS(BNO_Get_GYR_PWR_Mode(usart, &gyr_pwr_mode));
    if (gyr_pwr_mode != BNO_GYR_PWR_MODE_ADV_PWRSAVE) {
        return ERROR;
    }

    //validate sensor is in a non-fusion operating mode
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Get_OPR_Mode(usart, &current_opr_mode));
    if (Validate_Enum(current_opr_mode, BNO_OPR_CONFIG_MODE, BNO_OPR_AMG_MODE) != SUCCESS) {
        return ERROR;
    }

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));
    
    //store current operating mode and switch to CONFIG_MODE
    current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //configure sleep duration
    uint8_t setting_val = (((uint8_t) slp_dur) << BNO_GYR_SLEEP_CONFIG_SLP_DUR_Pos);
    CHECK_STATUS(BNO_Set_Setting(
        usart,
        BNO_GYR_SLEEP_CONFIG_REG,
        BNO_GYR_SLEEP_CONFIG_SLP_DUR,
        setting_val
    ));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode)); 

    return SUCCESS;
}

/**
 * @brief  Gets gyr sleep duration
 * @param  usart:   Pointer to a struct containing USART settings
 * @param  slp_dur: Pointer to a variable used to store gyr sleep duration
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GYR_Slp_Dur(USART_Config_t *usart, uint8_t *slp_dur) {
    CHECK_STATUS(Validate_Ptr(slp_dur));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read and store sleep duration
    CHECK_STATUS(BNO_Get_Setting(
        usart,
        BNO_GYR_SLEEP_CONFIG_REG,
        BNO_GYR_SLEEP_CONFIG_SLP_DUR,
        slp_dur
    ));

    return SUCCESS;
}

/**
 * @brief  Sets gyr auto sleep duration
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  auto_dur: Auto sleep duration setting
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_GYR_Slp_Auto_Dur(USART_Config_t *usart, BNO_GYR_Slp_Auto_Dur auto_dur) {
    CHECK_STATUS(Validate_Enum(auto_dur, BNO_GYR_SLP_AUTO_DUR_4_MS, BNO_GYR_SLP_AUTO_DUR_40_MS));

    //validate auto sleep duration based on configured bandwidth
    uint8_t min_auto_dur_vals[] = {4U, 4U, 4U, 5U, 10U, 20U, 10U, 20U};
    uint8_t gyr_bw = 0U;
    CHECK_STATUS(BNO_Get_GYR_BW(usart, &gyr_bw));
    uint8_t min_auto_dur = min_auto_dur_vals[gyr_bw];
    if (((uint8_t) auto_dur) < min_auto_dur) {
        return INVALID_PARAM;
    }

    //validate gyr is in advanced power mode
    uint8_t gyr_pwr_mode = 0U;
    CHECK_STATUS(BNO_Get_GYR_PWR_Mode(usart, &gyr_pwr_mode));
    if (gyr_pwr_mode != BNO_GYR_PWR_MODE_ADV_PWRSAVE) {
        return ERROR;
    }

    //validate sensor is in a non-fusion operating mode
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Get_OPR_Mode(usart, &current_opr_mode));
    if (Validate_Enum(current_opr_mode, BNO_OPR_CONFIG_MODE, BNO_OPR_AMG_MODE) != SUCCESS) {
        return ERROR;
    }

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));
    
    //store current operating mode and switch to CONFIG_MODE
    current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //configure auto sleep duration
    uint8_t setting_val = (((uint8_t) auto_dur) << BNO_GYR_SLEEP_CONFIG_AUTO_DUR_Pos);
    CHECK_STATUS(BNO_Set_Setting(
        usart,
        BNO_GYR_SLEEP_CONFIG_REG,
        BNO_GYR_SLEEP_CONFIG_AUTO_DUR,
        setting_val
    ));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode)); 

    return SUCCESS;
}

/**
 * @brief  Gets gyr auto sleep duration
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  auto_dur: Pointer to a variable used to store gyr auto sleep duration
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GYR_Slp_Auto_Dur(USART_Config_t *usart, uint8_t *auto_dur) {
    CHECK_STATUS(Validate_Ptr(auto_dur));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read and store auto sleep duration
    CHECK_STATUS(BNO_Get_Setting(
        usart,
        BNO_GYR_SLEEP_CONFIG_REG,
        BNO_GYR_SLEEP_CONFIG_AUTO_DUR,
        auto_dur
    ));
    *auto_dur = (*auto_dur >> BNO_GYR_SLEEP_CONFIG_AUTO_DUR_Pos);

    return SUCCESS;
}


/**************************************************************************************************/
/*                                       Self-Test Functions                                      */
/**************************************************************************************************/

/**
 * @brief  Gets the power on self-test result for the MCU
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  result: Pointer to a variable used to store power on self-test result
 * @retval Status indicating success, invalid parameters or error
 * @note   If result != 0, the power on self-test has passed
 */
Status BNO_Get_MCU_POST_Result(USART_Config_t *usart, uint8_t *result) {
    CHECK_STATUS(Validate_Ptr(result));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //read, extract and store POST result
    uint8_t data[BNO_RESPONSE_HEADER_LENGTH + BNO_GENERIC_RW_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, BNO_ST_RESULT_REG, 1U, data));
    *result = (uint8_t) (data[2] & BNO_ST_RESULT_MCU);

    return SUCCESS;
}

/**
 * @brief  Gets the power on self-test result for a particular sensor
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  sensor: Sensor (ACC/MAG/GYR)
 * @param  result: Pointer to a variable used to store power on self-test result
 * @retval Status indicating success, invalid parameters or error
 * @note   If result != 0, the power on self-test has passed
 */
static Status BNO_Get_Sensor_POST_Result(
    USART_Config_t *usart, 
    BNO_Sensor     sensor, 
    uint8_t        *result
) {
    CHECK_STATUS(Validate_Enum(sensor, BNO_ACC, BNO_GYR));
    CHECK_STATUS(Validate_Ptr(result));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //determine bit mask based on sensor
    uint8_t bit_mask;
    if (sensor == BNO_ACC) {
        bit_mask = BNO_ST_RESULT_ACC;
    } else if (sensor == BNO_MAG) {
        bit_mask = BNO_ST_RESULT_MAG;
    } else {
        bit_mask = BNO_ST_RESULT_GYR;
    }

    //read, extract and store POST result
    uint8_t data[BNO_RESPONSE_HEADER_LENGTH + BNO_GENERIC_RW_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, BNO_ST_RESULT_REG, 1U, data));
    *result = (uint8_t) (data[2] & bit_mask);

    return SUCCESS;
}

/**
 * @brief  Gets the power on self-test result for the acc
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  result: Pointer to a variable used to store power on self-test result
 * @retval Status indicating success, invalid parameters or error
 * @note   If result != 0, the power on self-test has passed
 */
Status BNO_Get_ACC_POST_Result(USART_Config_t *usart, uint8_t *result) {
    return BNO_Get_Sensor_POST_Result(usart, BNO_ACC, result);
}

/**
 * @brief  Gets the power on self-test result for the mag
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  result: Pointer to a variable used to store power on self-test result
 * @retval Status indicating success, invalid parameters or error
 * @note   If result != 0, the power on self-test has passed
 */
Status BNO_Get_MAG_POST_Result(USART_Config_t *usart, uint8_t *result) {
    return BNO_Get_Sensor_POST_Result(usart, BNO_MAG, result);
}

/**
 * @brief  Gets the power on self-test result for the gyr
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  result: Pointer to a variable used to store power on self-test result
 * @retval Status indicating success, invalid parameters or error
 * @note   If result != 0, the power on self-test has passed
 */
Status BNO_Get_GYR_POST_Result(USART_Config_t *usart, uint8_t *result) {
    return BNO_Get_Sensor_POST_Result(usart, BNO_GYR, result);
}

/**
 * @brief  Runs the built-in self-test
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  result: Pointer to a variable used to store built-in self-test result
 * @retval Status indicating success, invalid parameters or error
 * @note   If result == 0, the built-in self-test has passed
 */
Status BNO_Run_BIST(USART_Config_t *usart, uint8_t *result) {
    CHECK_STATUS(Validate_Ptr(result));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //save operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //trigger self-test and wait for it to run
    uint8_t mask        = 0x00U;
    uint8_t setting_val = BNO_SYS_TRIGGER_SELF_TEST;
    CHECK_STATUS(BNO_Set_Setting(usart, BNO_SYS_TRIGGER_REG, mask, setting_val));
    uint8_t delay_margin = 2U;
    Delay_Loop(400 * delay_margin);

    //read, extract and store self-test result
    uint8_t data[BNO_RESPONSE_HEADER_LENGTH + BNO_GENERIC_RW_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, BNO_SYS_ERR_REG, 1U, data));
    *result = (uint8_t) data[2];

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}


/**************************************************************************************************/
/*                                  Sensor Calibration Functions                                  */
/**************************************************************************************************/
//note: sensors should be fully calibrated before offset bytes are read
/**
 * @brief  Gets the offset of a particular sensor
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  sensor: Sensor (ACC/MAG/GYR)
 * @param  offset: Pointer to a struct used to store a sensor's offset data
 * @retval Status indicating success, invalid parameters or error
 * @note   All sensors (the whole system) should be fully calibrated before offset data is read
 */
static Status BNO_Get_Offset(USART_Config_t *usart, BNO_Sensor sensor, BNO_Offset_t *offset) {
    CHECK_STATUS(Validate_Enum(sensor, BNO_ACC, BNO_GYR));
    CHECK_STATUS(Validate_Ptr(offset));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //save operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //select appropriate offset register base address
    uint8_t offset_base = 0U;
    if (sensor == BNO_ACC) {
        offset_base = BNO_ACC_OFFSET_X_LSB_REG;
    } else if (sensor == BNO_MAG) {
        offset_base = BNO_MAG_OFFSET_X_LSB_REG;
    } else {
        offset_base = BNO_GYR_OFFSET_X_LSB_REG;
    }

    //read, extract and store the sensor offset
    uint8_t data[BNO_RESPONSE_HEADER_LENGTH + BNO_AMG_DATA_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, offset_base, BNO_AMG_DATA_LENGTH, data));
    offset->offset_x = (int16_t) (data[2] | (data[3] << 8U));
    offset->offset_y = (int16_t) (data[4] | (data[5] << 8U));
    offset->offset_z = (int16_t) (data[6] | (data[7] << 8U));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Sets the offset of a particular sensor
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  sensor: Sensor (ACC/MAG/GYR)
 * @param  offset: Pointer to a struct containing a sensor's offset data
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Set_Offset(USART_Config_t *usart, BNO_Sensor sensor, BNO_Offset_t *offset) {
    CHECK_STATUS(Validate_Enum(sensor, BNO_ACC, BNO_GYR));
    CHECK_STATUS(Validate_Ptr(offset));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //save operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));
    
    //select appropriate offset register base address
    uint8_t offset_base = 0U;
    if (sensor == BNO_ACC) {
        offset_base = BNO_ACC_OFFSET_X_LSB_REG;
    } else if (sensor == BNO_MAG) {
        offset_base = BNO_MAG_OFFSET_X_LSB_REG;
    } else {
        offset_base = BNO_GYR_OFFSET_X_LSB_REG;
    }

    //compose and write offset data
    uint8_t data[BNO_AMG_DATA_LENGTH] = {0};
    data[0] = (uint8_t) (offset->offset_x & 0xFF);
    data[1] = (uint8_t) ((offset->offset_x >> 8U) & 0xFF);
    data[2] = (uint8_t) (offset->offset_y & 0xFF);
    data[3] = (uint8_t) ((offset->offset_y >> 8U) & 0xFF);
    data[4] = (uint8_t) (offset->offset_z & 0xFF);
    data[5] = (uint8_t) ((offset->offset_z >> 8U) & 0xFF);

    CHECK_STATUS(BNO_Write_Reg(usart, offset_base, BNO_AMG_DATA_LENGTH, data));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the acc offset
 * @param  usart:      Pointer to a struct containing USART settings
 * @param  acc_offset: Pointer to a struct used to store acc offset data
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_ACC_Offset(USART_Config_t *usart, BNO_Offset_t *acc_offset) {
    return BNO_Get_Offset(usart, BNO_ACC, acc_offset);
}

/**
 * @brief  Sets the acc offset
 * @param  usart:      Pointer to a struct containing USART settings
 * @param  acc_offset: Pointer to a struct containing acc offset data
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_ACC_Offset(USART_Config_t *usart, BNO_Offset_t *acc_offset) {
    return BNO_Set_Offset(usart, BNO_ACC, acc_offset);
}

/**
 * @brief  Gets the mag offset
 * @param  usart:      Pointer to a struct containing USART settings
 * @param  mag_offset: Pointer to a struct used to store mag offset data
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_MAG_Offset(USART_Config_t *usart, BNO_Offset_t *mag_offset) {
    return BNO_Get_Offset(usart, BNO_MAG, mag_offset);
}

/**
 * @brief  Sets the mag offset
 * @param  usart:      Pointer to a struct containing USART settings
 * @param  mag_offset: Pointer to a struct containing mag offset data
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_MAG_Offset(USART_Config_t *usart, BNO_Offset_t *mag_offset) {
    return BNO_Set_Offset(usart, BNO_MAG, mag_offset);
}

/**
 * @brief  Gets the gyr offset
 * @param  usart:      Pointer to a struct containing USART settings
 * @param  gyr_offset: Pointer to a struct used to store gyr offset data
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GYR_Offset(USART_Config_t *usart, BNO_Offset_t *gyr_offset) {
    return BNO_Get_Offset(usart, BNO_GYR, gyr_offset);
}

/**
 * @brief  Sets the gyr offset
 * @param  usart:      Pointer to a struct containing USART settings
 * @param  gyr_offset: Pointer to a struct containing gyr offset data
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_GYR_Offset(USART_Config_t *usart, BNO_Offset_t *gyr_offset) {
    return BNO_Set_Offset(usart, BNO_GYR, gyr_offset);
}

/**
 * @brief  Gets the radius of a particular sensor
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  sensor: Sensor (ACC/MAG)
 * @param  radius: Pointer to a struct used to store a sensor's radius data
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Get_Radius(USART_Config_t *usart, BNO_Sensor sensor, BNO_Radius_t *radius) {
    CHECK_STATUS(Validate_Enum(sensor, BNO_ACC, BNO_MAG));
    CHECK_STATUS(Validate_Ptr(radius));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //save operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //select appropriate offset register base address
    uint8_t offset_base = 0U;
    if (sensor == BNO_ACC) {
        offset_base = BNO_ACC_RADIUS_LSB_REG;
    } else {
        offset_base = BNO_MAG_RADIUS_LSB_REG;
    }

    //read, extract and store the sensor offset
    uint8_t data[BNO_RESPONSE_HEADER_LENGTH + BNO_LSB_MSB_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, offset_base, BNO_LSB_MSB_LENGTH, data));

    radius->radius_lsb = (int8_t) data[2];
    radius->radius_msb = (int8_t) data[3];

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Sets the radius of a particular sensor
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  sensor: Sensor (ACC/MAG)
 * @param  radius: Pointer to a struct containing a sensor's radius data
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Set_Radius(USART_Config_t *usart, BNO_Sensor sensor, BNO_Radius_t *radius) {
    CHECK_STATUS(Validate_Enum(sensor, BNO_ACC, BNO_MAG));
    CHECK_STATUS(Validate_Ptr(radius));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //save operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //select appropriate offset register base address
    uint8_t offset_base = 0U;
    if (sensor == BNO_ACC) {
        offset_base = BNO_ACC_RADIUS_LSB_REG;
    } else {
        offset_base = BNO_MAG_RADIUS_LSB_REG;
    }

    //compose and write offset data
    uint8_t data[BNO_LSB_MSB_LENGTH] = {0};
    data[0] = (uint8_t) (radius->radius_lsb);
    data[1] = (uint8_t) (radius->radius_msb);
    CHECK_STATUS(BNO_Write_Reg(usart, offset_base, BNO_LSB_MSB_LENGTH, data));    

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the acc radius
 * @param  usart:      Pointer to a struct containing USART settings
 * @param  acc_radius: Pointer to a struct used to store acc radius data
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_ACC_Radius(USART_Config_t *usart, BNO_Radius_t *acc_radius) {
    return BNO_Get_Radius(usart, BNO_ACC, acc_radius);
}

/**
 * @brief  Sets the acc radius
 * @param  usart:      Pointer to a struct containing USART settings
 * @param  acc_radius: Pointer to a struct containing acc radius data
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_ACC_Radius(USART_Config_t *usart, BNO_Radius_t *acc_radius) {
    return BNO_Set_Radius(usart, BNO_ACC, acc_radius);
}

/**
 * @brief  Gets the mag radius
 * @param  usart:      Pointer to a struct containing USART settings
 * @param  mag_radius: Pointer to a struct used to store mag radius data
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_MAG_Radius(USART_Config_t *usart, BNO_Radius_t *mag_radius) {
    return BNO_Get_Radius(usart, BNO_MAG, mag_radius);
}

/**
 * @brief  Sets the mag radius
 * @param  usart:      Pointer to a struct containing USART settings
 * @param  mag_radius: Pointer to a struct containing mag radius data
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_MAG_Radius(USART_Config_t *usart, BNO_Radius_t *mag_radius) {
    return BNO_Set_Radius(usart, BNO_MAG, mag_radius);
}

/**
 * @brief  Gets the calibration profile
 * @param  usart:   Pointer to a struct containing USART settings
 * @param  profile: Pointer to a struct used to store the retrieved calibration
 *                  profile
 * @retval Status indicating success, invalid parameters or error
 * @note   When initialising the profile struct, each .member = NULL
 */
Status BNO_Get_Calib_Profile(USART_Config_t *usart, BNO_Calib_Profile_t *profile) {
    CHECK_STATUS(Validate_Ptr(profile));

    //read offset and radius values
    CHECK_STATUS(BNO_Get_ACC_Offset(usart, profile->acc_offset));
    CHECK_STATUS(BNO_Get_MAG_Offset(usart, profile->mag_offset));
    CHECK_STATUS(BNO_Get_GYR_Offset(usart, profile->gyr_offset));
    CHECK_STATUS(BNO_Get_ACC_Radius(usart, profile->acc_radius));
    CHECK_STATUS(BNO_Get_MAG_Radius(usart, profile->mag_radius));

    return SUCCESS;
}

/**
 * @brief  Sets the calibration profile
 * @param  usart:   Pointer to a struct containing USART settings
 * @param  profile: Pointer to a struct containing the calibration profile
 * @retval Status indicating success, invalid parameters or error
 * @note   When initialising the profile struct, each .member = &offset_struct
 * @note   This function should be called close to the BNO init functions before any data reads are
 *         performed
 */
Status BNO_Set_Calib_Profile(USART_Config_t *usart, BNO_Calib_Profile_t *profile) {
    CHECK_STATUS(Validate_Ptr(profile));

    //restore offset and radius values
    CHECK_STATUS(BNO_Set_ACC_Offset(usart, profile->acc_offset));
    CHECK_STATUS(BNO_Set_MAG_Offset(usart, profile->mag_offset));
    CHECK_STATUS(BNO_Set_GYR_Offset(usart, profile->gyr_offset));
    CHECK_STATUS(BNO_Set_ACC_Radius(usart, profile->acc_radius));
    CHECK_STATUS(BNO_Set_MAG_Radius(usart, profile->mag_radius));

    return SUCCESS;
}

/**
 * @brief  Transmits the calibration profile to a terminal
 * @param  usart_term_config: Pointer to a struct containing settings of the USART
 *                            instance used to communicate with the terminal
 * @param  profile:           Pointer to a struct containing the calibration profile
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Transmit_Calib_Profile(USART_Config_t *usart_term_config, BNO_Calib_Profile_t *profile) {
    CHECK_STATUS(Validate_Ptr(profile));

    //get global USART state
    volatile USART_State_t *current_state = NULL;
    CHECK_STATUS(USART_Get_State(usart_term_config, &current_state));

    //compose and transmit message
    uint8_t profile_msg[TX_BUFFER_SIZE] = {0};
    snprintf(
        (char *) profile_msg, sizeof(profile_msg), 
        "ACC Offset Values\n\r"
        "x-axis -> %6i\n\r"
        "y-axis -> %6i\n\r"
        "z-axis -> %6i\n\r"
        "MAG Offset Values\n\r"
        "x-axis -> %6i\n\r"
        "y-axis -> %6i\n\r"
        "z-axis -> %6i\n\r"
        "GYR Offset Values\n\r"
        "x-axis -> %6i\n\r"
        "y-axis -> %6i\n\r"
        "z-axis -> %6i\n\r"
        "ACC Radius Values\n\r"
        "lsb    -> %6i\n\r"
        "msb    -> %6i\n\r"
        "MAG Radius Values\n\r"
        "lsb    -> %6i\n\r"
        "msb    -> %6i\n\n\n\r",
        profile->acc_offset->offset_x, profile->acc_offset->offset_y, profile->acc_offset->offset_z,
        profile->mag_offset->offset_x, profile->mag_offset->offset_y, profile->mag_offset->offset_z,
        profile->gyr_offset->offset_x, profile->gyr_offset->offset_y, profile->gyr_offset->offset_z,
        profile->acc_radius->radius_lsb, profile->acc_radius->radius_msb,
        profile->mag_radius->radius_lsb, profile->mag_radius->radius_msb
    );
    CHECK_STATUS(USART_Transmit_IRQ(usart_term_config, profile_msg, strlen((char *) profile_msg)));

    //wait for tx to complete
    while (current_state->tx_status == USART_TX_BUSY) {}

    return SUCCESS;
}

/**
 * @brief  Gets the system calibration status
 * @param  usart:            Pointer to a struct containing USART settings
 * @param  sys_calib_status: Pointer to a variable used to store system calibration status
 * @retval Status indicating success, invalid parameters or error
 * @note   If sys_calib_status != 0, the system has been fully calibrated
 * @note   The whole system must be calibrated before program execution proceeds
 */
Status BNO_Get_Sys_Calib_Status(USART_Config_t *usart, uint8_t *sys_calib_status) {
    CHECK_STATUS(Validate_Ptr(sys_calib_status));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //read, extract and store calibration status
    uint8_t data[BNO_RESPONSE_HEADER_LENGTH + BNO_GENERIC_RW_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, BNO_CALIB_STAT_REG, 1U, data));
    *sys_calib_status = (data[2] & BNO_CALIB_STAT_SYS);

    return SUCCESS;
}

/**
 * @brief  Gets the the calibration status of a particular sensor
 * @param  usart:        Pointer to a struct containing USART settings
 * @param  sensor:       Sensor (ACC/MAG/GYR)
 * @param  calib_status: Pointer to a variable used to store a sensor's calibration status
 * @retval Status indicating success, invalid parameters or error
 * @note   If calib_status != 0, the sensor has been fully calibrated
 */
static Status BNO_Get_Sensor_Calib_Status(
    USART_Config_t *usart, 
    BNO_Sensor     sensor,
    uint8_t        *calib_status
) {
    CHECK_STATUS(Validate_Enum(sensor, BNO_ACC, BNO_GYR));
    CHECK_STATUS(Validate_Ptr(calib_status));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //determine bit mask based on sensor
    uint8_t bit_mask = 0U;
    if (sensor == BNO_ACC) {
        bit_mask = BNO_CALIB_STAT_ACC;
    } else if (sensor == BNO_MAG) {
        bit_mask = BNO_CALIB_STAT_MAG;
    } else {
        bit_mask = BNO_CALIB_STAT_GYR;
    }

    //read, extract and store calibration status
    uint8_t data[BNO_RESPONSE_HEADER_LENGTH + BNO_GENERIC_RW_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, BNO_CALIB_STAT_REG, 1U, data));
    *calib_status = (data[2] & bit_mask);

    return SUCCESS;
}

/**
 * @brief  Gets the calibration status of the acc
 * @param  usart:            Pointer to a struct containing USART settings
 * @param  acc_calib_status: Pointer to a variable used to store the retrieved acc calibration 
 *                           status
 * @retval Status indicating success, invalid parameters or error
 * @note   If acc_calib_status != 0, the acc has been fully calibrated
 */
Status BNO_Get_ACC_Calib_Status(USART_Config_t *usart, uint8_t *acc_calib_status) {
    return BNO_Get_Sensor_Calib_Status(usart, BNO_ACC, acc_calib_status);
}

/**
 * @brief  Gets the calibration status of the mag
 * @param  usart:            Pointer to a struct containing USART settings
 * @param  mag_calib_status: Pointer to a variable used to store the retrieved mag calibration 
 *                           status
 * @retval Status indicating success, invalid parameters or error
 * @note   If mag_calib_status != 0, the mag has been fully calibrated
 */
Status BNO_Get_MAG_Calib_Status(USART_Config_t *usart, uint8_t *mag_calib_status) {
    return BNO_Get_Sensor_Calib_Status(usart, BNO_MAG, mag_calib_status);
}

/**
 * @brief  Gets the calibration status of the gyr
 * @param  usart:            Pointer to a struct containing USART settings
 * @param  gyr_calib_status: Pointer to a variable used to store the retrieved gyr calibration 
 *                           status
 * @retval Status indicating success, invalid parameters or error
 * @note   If gyr_calib_status != 0, the gyr has been fully calibrated
 */
Status BNO_Get_GYR_Calib_Status(USART_Config_t *usart, uint8_t *gyr_calib_status) {
    return BNO_Get_Sensor_Calib_Status(usart, BNO_GYR, gyr_calib_status);
}


/**************************************************************************************************/
/*                               Sensor and Fusion Output Functions                               */
/**************************************************************************************************/

/**
 * @brief  Gets a output data register base address
 * @param  odr: Sensor output selection
 * @retval Register base address of sensor
 */
static uint8_t BNO_Get_ODR_Base(BNO_ODR odr) {
    if (odr == BNO_ODR_ACC) {
        return BNO_ACC_BASE_REG;
    } else if (odr == BNO_ODR_MAG) {
        return BNO_MAG_BASE_REG;
    } else if (odr == BNO_ODR_GYR) {
        return BNO_GYR_BASE_REG;
    } else if (odr == BNO_ODR_EUL) {
        return BNO_EUL_BASE_REG;
    } else if (odr == BNO_ODR_LIA) {
        return BNO_LIA_BASE_REG;
    } else {
        return BNO_GRV_BASE_REG;
    }
}

/**
 * @brief  Reads X, Y and Z values from a set of output data registers
 * @param  usart:   Pointer to a struct containing USART settings
 * @param  odr_raw: Pointer to a struct used to store raw output data
 * @param  odr:     Sensor output selection
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Read_ODR_All(USART_Config_t *usart, BNO_ODR_Raw_t *odr_raw, BNO_ODR odr) {
    CHECK_STATUS(Validate_Ptr(odr_raw));
    CHECK_STATUS(Validate_Enum(odr, BNO_ODR_ACC, BNO_ODR_GRV));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //select appropriate odr base address
    uint8_t odr_base_adr = BNO_Get_ODR_Base(odr);

    //transmit read command
    uint8_t data[BNO_RESPONSE_HEADER_LENGTH + BNO_AMG_DATA_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, odr_base_adr, BNO_AMG_DATA_LENGTH, data));

    //extract and store sensor data
    odr_raw->x_raw = (int16_t) (data[2] | (data[3] << 8U));
    odr_raw->y_raw = (int16_t) (data[4] | (data[5] << 8U));
    odr_raw->z_raw = (int16_t) (data[6] | (data[7] << 8U));

    return SUCCESS;
}

/**
 * @brief  Reads an axis value from an output data register
 * @param  usart:   Pointer to a struct containing USART settings
 * @param  odr_raw: Pointer to int16_t variable used to store raw axis output data
 * @param  odr:     Sensor output selection
 * @param  axis:    Sensor axis to be read
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Read_ODR_Axis(
    USART_Config_t *usart, 
    int16_t        *odr_raw, 
    BNO_ODR        odr, 
    BNO_ODR_Axis   axis
) {
    CHECK_STATUS(Validate_Ptr(odr_raw));
    CHECK_STATUS(Validate_Enum(axis, BNO_ODR_X, BNO_ODR_Z));
    CHECK_STATUS(Validate_Enum(odr, BNO_ODR_ACC, BNO_ODR_GRV));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //determine appropriate sensor base address
    uint8_t axis_offset  = (axis * 2U);
    uint8_t odr_base_adr = BNO_Get_ODR_Base(odr);
    odr_base_adr        += axis_offset;

    //transmit read command
    uint8_t data[BNO_RESPONSE_HEADER_LENGTH + BNO_LSB_MSB_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, odr_base_adr, BNO_LSB_MSB_LENGTH, data));

    //extract and store axis data
    *odr_raw = (int16_t) (data[2] | (data[3] << 8U));

    return SUCCESS;
}

/**
 * @brief  Reads a euler angle value from an output data register
 * @param  usart:     Pointer to a struct containing USART settings
 * @param  angle_raw: Pointer to int16_t variable used to store raw euler angle data
 * @param  angle:     Euler angle to be read
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Read_EUL_Angle(USART_Config_t *usart, int16_t *angle_raw, BNO_EUL_Angle angle) {
    CHECK_STATUS(Validate_Ptr(angle_raw));

    return BNO_Read_ODR_Axis(usart, angle_raw, BNO_ODR_EUL, angle);
}

/**
 * @brief  Reads W, X, Y and Z quaternion values from a set of output data registers
 * @param  usart:   Pointer to a struct containing USART settings
 * @param  qua_raw: Pointer to a struct used to store raw quaternion data
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Read_QUA_All(USART_Config_t *usart, BNO_QUA_Raw_t *qua_raw) {
    CHECK_STATUS(Validate_Ptr(qua_raw));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //transmit read command
    uint8_t data[BNO_RESPONSE_HEADER_LENGTH + BNO_QUA_DATA_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, BNO_QUA_BASE_REG, BNO_QUA_DATA_LENGTH, data));

    //extract and store quaternion data
    qua_raw->w_raw = (int16_t) (data[2] | (data[3] << 8U));
    qua_raw->x_raw = (int16_t) (data[4] | (data[5] << 8U));
    qua_raw->y_raw = (int16_t) (data[6] | (data[7] << 8U));
    qua_raw->z_raw = (int16_t) (data[8] | (data[9] << 8U));

    return SUCCESS;
}

/**
 * @brief  Reads a quaternion value from an output data register
 * @param  usart:   Pointer to a struct containing USART settings
 * @param  qua_raw: Pointer to int16_t variable used to store a raw quaternion value
 * @param  value:   Quaternion value to be read
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Read_QUA_Value(USART_Config_t *usart, int16_t *qua_raw, BNO_QUA_Value value) {
    CHECK_STATUS(Validate_Ptr(qua_raw));
    CHECK_STATUS(Validate_Enum(value, BNO_QUA_W, BNO_QUA_Z));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //determine appropriate base address
    uint8_t axis_offset = (value * 2U);
    uint8_t qua_base_adr = BNO_QUA_BASE_REG + axis_offset;

    //transmit read command
    uint8_t data[BNO_RESPONSE_HEADER_LENGTH + BNO_LSB_MSB_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, qua_base_adr, BNO_LSB_MSB_LENGTH, data));

    //extract and store axis data
    *qua_raw = (int16_t) (data[2] | (data[3] << 8U));

    return SUCCESS;
}


/**************************************************************************************************/
/*                          Sensor and Fusion Output Conversion Functions                         */
/**************************************************************************************************/

/**
 * @brief  Gets the acc raw data conversion factor
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  conv_factor: Pointer to float variable used to store the retrieved conversion factor
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Get_ACC_Conv_Factor(USART_Config_t *usart, float *conv_factor) {
    CHECK_STATUS(Validate_Ptr(conv_factor));

    //read acc unit selection
    uint8_t acc_unit_state = 0U;
    CHECK_STATUS(BNO_Get_ACC_Unit(usart, &acc_unit_state));

    //select appropriate conversion factor
    if (acc_unit_state == 0U) {
        *conv_factor = BNO_ACC_MS;
    } else {
        *conv_factor = BNO_ACC_MG;
    }

    return SUCCESS;
}

/**
 * @brief  Gets the gyr raw data conversion factor
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  conv_factor: Pointer to float variable used to store the retrieved conversion factor
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Get_GYR_Conv_Factor(USART_Config_t *usart, float *conv_factor) {
    CHECK_STATUS(Validate_Ptr(conv_factor));

    //read gyr unit selection
    uint8_t gyr_unit_state = 0U;
    CHECK_STATUS(BNO_Get_GYR_Unit(usart, &gyr_unit_state));

    //select appropriate conversion factor
    if (gyr_unit_state == 0U) {
        *conv_factor = BNO_GYR_DPS;
    } else {
        *conv_factor = BNO_GYR_RPS;
    }

    return SUCCESS;
}

/**
 * @brief  Gets the euler raw data conversion factor
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  conv_factor: Pointer to float variable used to store the retrieved conversion factor
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Get_EUL_Conv_Factor(USART_Config_t *usart, float *conv_factor) {
    CHECK_STATUS(Validate_Ptr(conv_factor));

    //read eul unit selection
    uint8_t eul_unit_state = 0U;
    CHECK_STATUS(BNO_Get_EUL_Unit(usart, &eul_unit_state));

    //select appropriate conversion factor
    if (eul_unit_state == 0U) {
        *conv_factor = BNO_EUL_DEGREES;
    } else {
        *conv_factor = BNO_EUL_RADIANS;
    }

    return SUCCESS;
}

/**
 * @brief  Gets the temperature raw data conversion factor
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  conv_factor: Pointer to float variable used to store the retrieved conversion factor
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Get_TEMP_Conv_Factor(USART_Config_t *usart, float *conv_factor) {
    CHECK_STATUS(Validate_Ptr(conv_factor));

    //read temp unit selection
    uint8_t temp_unit_state = 0;
    CHECK_STATUS(BNO_Get_TEMP_Unit(usart, &temp_unit_state));

    //select appropriate conversion factor
    if (temp_unit_state == 0U) {
        *conv_factor = BNO_TEMP_CEL;
    } else {
        *conv_factor = BNO_TEMP_FAH;
    }

    return SUCCESS;
}

/**
 * @brief  Reads x, y and z-axis acc values
 * @param  usart:         Pointer to a struct containing USART settings
 * @param  acc_xyz_float: Pointer to a struct used to store acc data
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_ACC_XYZ(USART_Config_t *usart, BNO_ODR_Float_t *acc_xyz_float) {
    CHECK_STATUS(Validate_Ptr(acc_xyz_float));

    //read raw values
    BNO_ODR_Raw_t acc_xyz_raw = {0.0f, 0.0f, 0.0f};
    CHECK_STATUS(BNO_Read_ODR_All(usart, &acc_xyz_raw, BNO_ODR_ACC));

    //get conversion factor
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_ACC_Conv_Factor(usart, &conv_factor));

    //perform conversion and store float values
    acc_xyz_float->x_float = (((float) acc_xyz_raw.x_raw) / conv_factor);
    acc_xyz_float->y_float = (((float) acc_xyz_raw.y_raw) / conv_factor);
    acc_xyz_float->z_float = (((float) acc_xyz_raw.z_raw) / conv_factor);

    return SUCCESS;
}

/**
 * @brief  Reads x-axis acc value
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  acc_x_float: Pointer to float variable used to store x-axis acc value
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_ACC_X(USART_Config_t *usart, float *acc_x_float) {
    CHECK_STATUS(Validate_Ptr(acc_x_float));

    //read raw value
    int16_t acc_x_raw = 0;
    CHECK_STATUS(BNO_Read_ODR_Axis(usart, &acc_x_raw, BNO_ODR_ACC, BNO_ODR_X));

    //get conversion factor, perform conversion and store float value
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_ACC_Conv_Factor(usart, &conv_factor));
    *acc_x_float = (((float) acc_x_raw) / conv_factor);

    return SUCCESS;
}

/**
 * @brief  Reads y-axis acc value
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  acc_y_float: Pointer to float variable used to store y-axis acc value
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_ACC_Y(USART_Config_t *usart, float *acc_y_float) {
    CHECK_STATUS(Validate_Ptr(acc_y_float));

    //read raw value
    int16_t acc_y_raw = 0;
    CHECK_STATUS(BNO_Read_ODR_Axis(usart, &acc_y_raw, BNO_ODR_ACC, BNO_ODR_Y));

    //get conversion factor, perform conversion and store float value
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_ACC_Conv_Factor(usart, &conv_factor));
    *acc_y_float = (((float) acc_y_raw) / conv_factor);

    return SUCCESS;
}

/**
 * @brief  Reads z-axis acc value
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  acc_z_float: Pointer to float variable used to store z-axis acc value
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_ACC_Z(USART_Config_t *usart, float *acc_z_float) {
    CHECK_STATUS(Validate_Ptr(acc_z_float));

    //read raw value
    int16_t acc_z_raw = 0;
    CHECK_STATUS(BNO_Read_ODR_Axis(usart, &acc_z_raw, BNO_ODR_ACC, BNO_ODR_Z));

    //get conversion factor, perform conversion and store float value
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_ACC_Conv_Factor(usart, &conv_factor));
    *acc_z_float = (((float) acc_z_raw) / conv_factor);
    
    return SUCCESS;
}

/**
 * @brief  Reads x, y and z-axis mag values
 * @param  usart:         Pointer to a struct containing USART settings
 * @param  mag_xyz_float: Pointer to a struct used to store mag data
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_MAG_XYZ(USART_Config_t *usart, BNO_ODR_Float_t *mag_xyz_float) {
    CHECK_STATUS(Validate_Ptr(mag_xyz_float));

    //read raw values
    BNO_ODR_Raw_t mag_xyz_raw = {0.0f, 0.0f, 0.0f};
    CHECK_STATUS(BNO_Read_ODR_All(usart, &mag_xyz_raw, BNO_ODR_MAG));

    //perform conversion and store float values
    mag_xyz_float->x_float = (((float) mag_xyz_raw.x_raw) / BNO_MAG_UT);
    mag_xyz_float->y_float = (((float) mag_xyz_raw.y_raw) / BNO_MAG_UT);
    mag_xyz_float->z_float = (((float) mag_xyz_raw.z_raw) / BNO_MAG_UT);

    return SUCCESS;
}

/**
 * @brief  Reads x-axis mag value
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  mag_x_float: Pointer to float variable used to store x-axis mag value
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_MAG_X(USART_Config_t *usart, float *mag_x_float) {
    CHECK_STATUS(Validate_Ptr(mag_x_float));

    //read raw value, perform conversion and store float value
    int16_t mag_x_raw = 0;
    CHECK_STATUS(BNO_Read_ODR_Axis(usart, &mag_x_raw, BNO_ODR_MAG, BNO_ODR_X));
    *mag_x_float = (((float) mag_x_raw) / BNO_MAG_UT);

    return SUCCESS;
}

/**
 * @brief  Reads y-axis mag value
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  mag_y_float: Pointer to float variable used to store y-axis mag value
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_MAG_Y(USART_Config_t *usart, float *mag_y_float) {
    CHECK_STATUS(Validate_Ptr(mag_y_float));

    //read raw value, perform conversion and store float value
    int16_t mag_y_raw = 0;
    CHECK_STATUS(BNO_Read_ODR_Axis(usart, &mag_y_raw, BNO_ODR_MAG, BNO_ODR_Y));
    *mag_y_float = (((float) mag_y_raw) / BNO_MAG_UT);

    return SUCCESS;
}

/**
 * @brief  Reads z-axis mag value
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  mag_z_float: Pointer to float variable used to store z-axis mag value
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_MAG_Z(USART_Config_t *usart, float *mag_z_float) {
    CHECK_STATUS(Validate_Ptr(mag_z_float));

    //read raw value, perform conversion and store float value
    int16_t mag_z_raw = 0;
    CHECK_STATUS(BNO_Read_ODR_Axis(usart, &mag_z_raw, BNO_ODR_MAG, BNO_ODR_Z));
    *mag_z_float = (((float) mag_z_raw) / BNO_MAG_UT);

    return SUCCESS;
}

/**
 * @brief  Reads x, y and z-axis gyr values
 * @param  usart:         Pointer to a struct containing USART settings
 * @param  gyr_xyz_float: Pointer to a struct used to store gyr data
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GYR_XYZ(USART_Config_t *usart, BNO_ODR_Float_t *gyr_xyz_float) {
    CHECK_STATUS(Validate_Ptr(gyr_xyz_float));

    //read raw values
    BNO_ODR_Raw_t gyr_xyz_raw = {0.0f, 0.0f, 0.0f};
    CHECK_STATUS(BNO_Read_ODR_All(usart, &gyr_xyz_raw, BNO_ODR_GYR));

    //get conversion factor
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_GYR_Conv_Factor(usart, &conv_factor));

    //perform conversion and store float values
    gyr_xyz_float->x_float = (((float) gyr_xyz_raw.x_raw) / conv_factor);
    gyr_xyz_float->y_float = (((float) gyr_xyz_raw.y_raw) / conv_factor);
    gyr_xyz_float->z_float = (((float) gyr_xyz_raw.z_raw) / conv_factor);

    return SUCCESS;
}

/**
 * @brief  Reads x-axis gyr value
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  gyr_x_float: Pointer to float variable used to store x-axis gyr value
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GYR_X(USART_Config_t *usart, float *gyr_x_float) {
    CHECK_STATUS(Validate_Ptr(gyr_x_float));

    //read raw value
    int16_t gyr_x_raw = 0;
    CHECK_STATUS(BNO_Read_ODR_Axis(usart, &gyr_x_raw, BNO_ODR_GYR, BNO_ODR_X));

    //get conversion factor, perform conversion and store float value
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_GYR_Conv_Factor(usart, &conv_factor));
    *gyr_x_float = (((float) gyr_x_raw) / conv_factor);

    return SUCCESS;
}

/**
 * @brief  Reads y-axis gyr value
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  gyr_y_float: Pointer to float variable used to store y-axis gyr value
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GYR_Y(USART_Config_t *usart, float *gyr_y_float) {
    CHECK_STATUS(Validate_Ptr(gyr_y_float));

    //read raw value
    int16_t gyr_y_raw = 0;
    CHECK_STATUS(BNO_Read_ODR_Axis(usart, &gyr_y_raw, BNO_ODR_GYR, BNO_ODR_Y));

    //get conversion factor, perform conversion and store float value
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_GYR_Conv_Factor(usart, &conv_factor));
    *gyr_y_float = (((float) gyr_y_raw) / conv_factor);

    return SUCCESS;
}

/**
 * @brief  Reads z-axis gyr value
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  gyr_z_float: Pointer to float variable used to store z-axis gyr value
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GYR_Z(USART_Config_t *usart, float *gyr_z_float) {
    CHECK_STATUS(Validate_Ptr(gyr_z_float));

    //read raw value
    int16_t gyr_z_raw = 0;
    CHECK_STATUS(BNO_Read_ODR_Axis(usart, &gyr_z_raw, BNO_ODR_GYR, BNO_ODR_Z));

    //get conversion factor, perform conversion and store float value
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_GYR_Conv_Factor(usart, &conv_factor));
    *gyr_z_float = (((float) gyr_z_raw) / conv_factor);

    return SUCCESS;
}

/**
 * @brief  Reads heading, roll and pitch euler values
 * @param  usart:         Pointer to a struct containing USART settings
 * @param  eul_hrp_float: Pointer to a struct used to store euler angles
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_EUL_HRP(USART_Config_t *usart, BNO_ODR_Float_t *eul_hrp_float) {
    CHECK_STATUS(Validate_Ptr(eul_hrp_float));

    //read raw values
    BNO_ODR_Raw_t eul_hrp_raw = {0.0f, 0.0f, 0.0f};
    CHECK_STATUS(BNO_Read_ODR_All(usart, &eul_hrp_raw, BNO_ODR_EUL));

    //get conversion factor
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_EUL_Conv_Factor(usart, &conv_factor));

    //perform conversion and store float values
    eul_hrp_float->x_float = (((float) eul_hrp_raw.x_raw) / conv_factor);
    eul_hrp_float->y_float = (((float) eul_hrp_raw.y_raw) / conv_factor);
    eul_hrp_float->z_float = (((float) eul_hrp_raw.z_raw) / conv_factor);

    return SUCCESS;
}

/**
 * @brief  Reads heading euler value
 * @param  usart:             Pointer to a struct containing USART settings
 * @param  eul_heading_float: Pointer to float variable used to store heading euler value
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_EUL_Heading(USART_Config_t *usart, float *eul_heading_float) {
    CHECK_STATUS(Validate_Ptr(eul_heading_float));

    //read raw value
    int16_t eul_heading_raw = 0;
    CHECK_STATUS(BNO_Read_EUL_Angle(usart, &eul_heading_raw, BNO_EUL_HEADING));

    //get conversion factor, perform conversion and store float value
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_EUL_Conv_Factor(usart, &conv_factor));
    *eul_heading_float = (((float) eul_heading_raw) / conv_factor);

    return SUCCESS;
}

/**
 * @brief  Reads roll euler value
 * @param  usart:          Pointer to a struct containing USART settings
 * @param  eul_roll_float: Pointer to float variable used to store roll euler value
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_EUL_Roll(USART_Config_t *usart, float *eul_roll_float) {
    CHECK_STATUS(Validate_Ptr(eul_roll_float));

    //read raw value
    int16_t eul_roll_raw = 0;
    CHECK_STATUS(BNO_Read_EUL_Angle(usart, &eul_roll_raw, BNO_EUL_ROLL));

    //get conversion factor, perform conversion and store float value
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_EUL_Conv_Factor(usart, &conv_factor));
    *eul_roll_float = (((float) eul_roll_raw) / conv_factor);

    return SUCCESS;
}

/**
 * @brief  Reads pitch euler value
 * @param  usart:           Pointer to a struct containing USART settings
 * @param  eul_pitch_float: Pointer to float variable used to store pitch euler value
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_EUL_Pitch(USART_Config_t *usart, float *eul_pitch_float) {
    CHECK_STATUS(Validate_Ptr(eul_pitch_float));

    //read raw value
    int16_t eul_pitch_raw = 0;
    CHECK_STATUS(BNO_Read_EUL_Angle(usart, &eul_pitch_raw, BNO_EUL_PITCH));

    //get conversion factor, perform conversion and store float value
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_EUL_Conv_Factor(usart, &conv_factor));
    *eul_pitch_float = (((float) eul_pitch_raw) / conv_factor);

    return SUCCESS;
}

/**
 * @brief  Reads w, x, y and z quaternion values
 * @param  usart:          Pointer to a struct containing USART settings
 * @param  qua_wxyz_float: Pointer to a struct used to store quaternion values
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_QUA_WXYZ(USART_Config_t *usart, BNO_QUA_Float_t *qua_wxyz_float) {
    CHECK_STATUS(Validate_Ptr(qua_wxyz_float));

    //read raw values
    BNO_QUA_Raw_t qua_wxyz_raw = {0.0f, 0.0f, 0.0f, 0.0f};
    CHECK_STATUS(BNO_Read_QUA_All(usart, &qua_wxyz_raw));

    //perform conversion and store float values
    qua_wxyz_float->w_float = (((float) qua_wxyz_raw.w_raw) / BNO_QUA_QUATERNIONS);
    qua_wxyz_float->x_float = (((float) qua_wxyz_raw.x_raw) / BNO_QUA_QUATERNIONS);
    qua_wxyz_float->y_float = (((float) qua_wxyz_raw.y_raw) / BNO_QUA_QUATERNIONS);
    qua_wxyz_float->z_float = (((float) qua_wxyz_raw.z_raw) / BNO_QUA_QUATERNIONS);

    return SUCCESS;
}

/**
 * @brief  Reads w quaternion value
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  qua_w_float: Pointer to float variable used to store w quaternion value
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_QUA_W(USART_Config_t *usart, float *qua_w_float) {
    CHECK_STATUS(Validate_Ptr(qua_w_float));

    //read raw value, perform conversion and store float value
    int16_t qua_w_raw = 0;
    CHECK_STATUS(BNO_Read_QUA_Value(usart, &qua_w_raw, BNO_QUA_W));
    *qua_w_float = (((float) qua_w_raw) / BNO_QUA_QUATERNIONS);

    return SUCCESS;
}

/**
 * @brief  Reads x quaternion value
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  qua_x_float: Pointer to float variable used to store x quaternion value
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_QUA_X(USART_Config_t *usart, float *qua_x_float) {
    CHECK_STATUS(Validate_Ptr(qua_x_float));

    //read raw value, perform conversion and store float value
    int16_t qua_x_raw = 0;
    CHECK_STATUS(BNO_Read_QUA_Value(usart, &qua_x_raw, BNO_QUA_X));
    *qua_x_float = (((float) qua_x_raw) / BNO_QUA_QUATERNIONS);

    return SUCCESS;
}

/**
 * @brief  Reads y quaternion value
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  qua_y_float: Pointer to float variable used to store y quaternion value
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_QUA_Y(USART_Config_t *usart, float *qua_y_float) {
    CHECK_STATUS(Validate_Ptr(qua_y_float));

    //read raw value, perform conversion and store float value
    int16_t qua_y_raw = 0;
    CHECK_STATUS(BNO_Read_QUA_Value(usart, &qua_y_raw, BNO_QUA_Y));
    *qua_y_float = (((float) qua_y_raw) / BNO_QUA_QUATERNIONS);

    return SUCCESS;
}

/**
 * @brief  Reads z quaternion value
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  qua_z_float: Pointer to float variable used to store z quaternion value
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_QUA_Z(USART_Config_t *usart, float *qua_z_float) {
    CHECK_STATUS(Validate_Ptr(qua_z_float));

    //read raw value, perform conversion and store float value
    int16_t qua_z_raw = 0;
    CHECK_STATUS(BNO_Read_QUA_Value(usart, &qua_z_raw, BNO_QUA_Z));
    *qua_z_float = (((float) qua_z_raw) / BNO_QUA_QUATERNIONS);

    return SUCCESS;
}

/**
 * @brief  Reads x, y and z-axis linear acceleration values
 * @param  usart:         Pointer to a struct containing USART settings
 * @param  lia_xyz_float: Pointer to a struct used to store linear acceleration data
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_LIA_XYZ(USART_Config_t *usart, BNO_ODR_Float_t *lia_xyz_float) {
    CHECK_STATUS(Validate_Ptr(lia_xyz_float));

    //read raw value
    BNO_ODR_Raw_t lia_xyz_raw = {0.0f, 0.0f, 0.0f};
    CHECK_STATUS(BNO_Read_ODR_All(usart, &lia_xyz_raw, BNO_ODR_LIA));

    //get conversion factor
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_ACC_Conv_Factor(usart, &conv_factor));

    //perform conversion and store float values
    lia_xyz_float->x_float = (((float) lia_xyz_raw.x_raw) / conv_factor);
    lia_xyz_float->y_float = (((float) lia_xyz_raw.y_raw) / conv_factor);
    lia_xyz_float->z_float = (((float) lia_xyz_raw.z_raw) / conv_factor);

    return SUCCESS;
}

/**
 * @brief  Reads x-axis linear acceleration value
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  lia_x_float: Pointer to float variable used to store x-axis linear acceleration
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_LIA_X(USART_Config_t *usart, float *lia_x_float) {
    CHECK_STATUS(Validate_Ptr(lia_x_float));

    //read raw value
    int16_t lia_x_raw = 0;
    CHECK_STATUS(BNO_Read_ODR_Axis(usart, &lia_x_raw, BNO_ODR_LIA, BNO_ODR_X));

    //get conversion factor, perform conversion and store float value
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_ACC_Conv_Factor(usart, &conv_factor));
    *lia_x_float = (((float) lia_x_raw) / conv_factor);

    return SUCCESS;
}

/**
 * @brief  Reads y-axis linear acceleration value
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  lia_y_float: Pointer to float variable used to store y-axis linear acceleration
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_LIA_Y(USART_Config_t *usart, float *lia_y_float) {
    CHECK_STATUS(Validate_Ptr(lia_y_float));

    //read raw value
    int16_t lia_y_raw = 0;
    CHECK_STATUS(BNO_Read_ODR_Axis(usart, &lia_y_raw, BNO_ODR_LIA, BNO_ODR_Y));

    //get conversion factor, perform conversion and store float value
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_ACC_Conv_Factor(usart, &conv_factor));
    *lia_y_float = (((float) lia_y_raw) / conv_factor);

    return SUCCESS;
}

/**
 * @brief  Reads z-axis linear acceleration value
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  lia_z_float: Pointer to float variable used to store z-axis linear acceleration
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_LIA_Z(USART_Config_t *usart, float *lia_z_float) {
    CHECK_STATUS(Validate_Ptr(lia_z_float));

    //read raw value
    int16_t lia_z_raw = 0;
    CHECK_STATUS(BNO_Read_ODR_Axis(usart, &lia_z_raw, BNO_ODR_LIA, BNO_ODR_Z));

    //get conversion factor, perform conversion and store float value
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_ACC_Conv_Factor(usart, &conv_factor));
    *lia_z_float = (((float) lia_z_raw) / conv_factor);

    return SUCCESS;
}

/**
 * @brief  Reads x, y and z axis gravity vector values
 * @param  usart:         Pointer to a struct containing USART settings
 * @param  grv_xyz_float: Pointer to a struct used to store gravity vector data 
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GRV_XYZ(USART_Config_t *usart, BNO_ODR_Float_t *grv_xyz_float) {
    CHECK_STATUS(Validate_Ptr(grv_xyz_float));

    //read raw value
    BNO_ODR_Raw_t grv_xyz_raw = {0.0f, 0.0f, 0.0f};
    CHECK_STATUS(BNO_Read_ODR_All(usart, &grv_xyz_raw, BNO_ODR_GRV));

    //get conversion factor
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_ACC_Conv_Factor(usart, &conv_factor));

    //perform conversion and store float values
    grv_xyz_float->x_float = (((float) grv_xyz_raw.x_raw) / conv_factor);
    grv_xyz_float->y_float = (((float) grv_xyz_raw.y_raw) / conv_factor);
    grv_xyz_float->z_float = (((float) grv_xyz_raw.z_raw) / conv_factor);

    return SUCCESS;
}

/**
 * @brief  Reads x-axis gravity vector value
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  grv_x_float: Pointer to float variable used to store x-axis gravity vector
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GRV_X(USART_Config_t *usart, float *grv_x_float) {
    CHECK_STATUS(Validate_Ptr(grv_x_float));

    //read raw value
    int16_t grv_x_raw = 0;
    CHECK_STATUS(BNO_Read_ODR_Axis(usart, &grv_x_raw, BNO_ODR_GRV, BNO_ODR_X));

    //get conversion factor, perform conversion and store float value
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_ACC_Conv_Factor(usart, &conv_factor));
    *grv_x_float = (((float) grv_x_raw) / conv_factor);

    return SUCCESS;
}

/**
 * @brief  Reads y-axis gravity vector value
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  grv_y_float: Pointer to float variable used to store y-axis gravity vector
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GRV_Y(USART_Config_t *usart, float *grv_y_float) {
    CHECK_STATUS(Validate_Ptr(grv_y_float));

    //read raw value
    int16_t grv_y_raw = 0;
    CHECK_STATUS(BNO_Read_ODR_Axis(usart, &grv_y_raw, BNO_ODR_GRV, BNO_ODR_Y));

    //get conversion factor, perform conversion and store float value
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_ACC_Conv_Factor(usart, &conv_factor));
    *grv_y_float = (((float) grv_y_raw) / conv_factor);

    return SUCCESS;
}

/**
 * @brief  Reads z-axis gravity vector value
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  grv_z_float: Pointer to float variable used to store z-axis gravity vector
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GRV_Z(USART_Config_t *usart, float *grv_z_float) {
    CHECK_STATUS(Validate_Ptr(grv_z_float));

    //read raw value
    int16_t grv_z_raw = 0;
    CHECK_STATUS(BNO_Read_ODR_Axis(usart, &grv_z_raw, BNO_ODR_GRV, BNO_ODR_Z));

    //get conversion factor, perform conversion and store float value
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_ACC_Conv_Factor(usart, &conv_factor));
    *grv_z_float = (((float) grv_z_raw) / conv_factor);

    return SUCCESS;
}

/**
 * @brief  Reads temperature value
 * @param  usart:      Pointer to a struct containing USART settings
 * @param  temp_float: Pointer to float variable used to store temperature
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_TEMP(USART_Config_t *usart, float *temp_float) {
    CHECK_STATUS(Validate_Ptr(temp_float));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));
    
    //read raw value
    uint8_t data[BNO_RESPONSE_HEADER_LENGTH + BNO_GENERIC_RW_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, BNO_TEMP_REG, BNO_GENERIC_RW_LENGTH, data));

    //get conversion factor, perform conversion and store float value
    float conv_factor = 0.0f;
    CHECK_STATUS(BNO_Get_TEMP_Conv_Factor(usart, &conv_factor));
    *temp_float = (((float) data[2]) / conv_factor);

    return SUCCESS;
}


/**************************************************************************************************/
/*                                    Unit Selection Functions                                    */
/**************************************************************************************************/

/**
 * @brief  Sets the units for various data outputs
 * @param  usart: Pointer to a struct containing USART settings
 * @param  unit:  Units selection
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Set_Unit(USART_Config_t *usart, BNO_Unit unit) {
    CHECK_STATUS(Validate_Enum(unit, BNO_UNIT_ACC_MS, BNO_UNIT_ORI_ANDROID));

    //select appropriate offset
    uint8_t unit_offset = 0U;
    if (unit == BNO_UNIT_ACC_MS || unit == BNO_UNIT_ACC_MG) {
        unit_offset = 0U;
    } else if (unit == BNO_UNIT_GYR_DPS || unit == BNO_UNIT_GYR_RPS) {
        unit_offset = 1U;
    } else if (unit == BNO_UNIT_EUL_DEGREES || unit == BNO_UNIT_EUL_RADIANS) {
        unit_offset = 2U;
    } else if (unit == BNO_UNIT_TEMP_CEL || unit == BNO_UNIT_TEMP_FAH) {
        unit_offset = 4U;
    } else {
        unit_offset = 7U;
    }

    //select write value
    uint8_t write_val = 0U;
    if ((unit % 2) == 0) {
        write_val = 1U;
    } else {
        write_val = 0U;
    }

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //save operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //write unit setting
    uint8_t mask        = (0x01U << unit_offset);
    uint8_t setting_val = (write_val << unit_offset);
    CHECK_STATUS(BNO_Set_Setting(usart, BNO_UNIT_SEL_REG, mask, setting_val));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the units for various data outputs
 * @param  usart:       Pointer to a struct containing USART settings
 * @param  data_output: Data output
 * @param  unit:        Pointer to a variable used to store the retrieved units selection
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Get_Unit(USART_Config_t *usart, BNO_Unit_DO data_output, uint8_t *unit) {
    CHECK_STATUS(Validate_Ptr(unit));
    CHECK_STATUS(Validate_Enum(data_output, BNO_UNIT_DO_ACC, BNO_UNIT_DO_ORI));

    //select appropriate offset
    uint8_t unit_offset = 0U;
    if (data_output == BNO_UNIT_DO_ACC) {
        unit_offset = 0U;
    } else if (data_output == BNO_UNIT_DO_GYR) {
        unit_offset = 1U;
    } else if (data_output == BNO_UNIT_DO_EUL) {
        unit_offset = 2U;
    } else if (data_output == BNO_UNIT_DO_TEMP) {
        unit_offset = 4U;
    } else {
        unit_offset = 7U;
    }

    //read UNIT_SEL value and extract relevant bit
    uint8_t reg_val_og[BNO_RESPONSE_HEADER_LENGTH + BNO_GENERIC_RW_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, BNO_UNIT_SEL_REG, 1U, reg_val_og));
    uint8_t reg_val_ex = (reg_val_og[2] & (1U << unit_offset));

    //store the extracted bit
    if (reg_val_ex == 0U) {
        *unit = 0U;
    } else {
        *unit = 1U;
    }

    return SUCCESS;
}

/**
 * @brief  Sets the units for the acc
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  acc_unit: Accelerometer units selection
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_ACC_Unit(USART_Config_t *usart, BNO_Unit acc_unit) {
    CHECK_STATUS(Validate_Enum(acc_unit, BNO_UNIT_ACC_MS, BNO_UNIT_ACC_MG));

    CHECK_STATUS(BNO_Set_Unit(usart, acc_unit));

    return SUCCESS;
}

/**
 * @brief  Gets the units for the acc
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  acc_unit: Pointer to a variable used to store the retrieved acc units
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_ACC_Unit(USART_Config_t *usart, uint8_t *acc_unit) {
    CHECK_STATUS(Validate_Ptr(acc_unit));

    CHECK_STATUS(BNO_Get_Unit(usart, BNO_UNIT_DO_ACC, acc_unit));

    return SUCCESS;
}

/**
 * @brief  Sets the units for the gyr
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  gyr_unit: Gyroscope units selection
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_GYR_Unit(USART_Config_t *usart, BNO_Unit gyr_unit) {
    CHECK_STATUS(Validate_Enum(gyr_unit, BNO_UNIT_GYR_DPS, BNO_UNIT_GYR_RPS));

    CHECK_STATUS(BNO_Set_Unit(usart, gyr_unit));

    return SUCCESS;
}

/**
 * @brief  Gets the units for the gyr
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  gyr_unit: Pointer to a variable used to store the retrieved gyr units
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GYR_Unit(USART_Config_t *usart, uint8_t *gyr_unit) {
    CHECK_STATUS(Validate_Ptr(gyr_unit));

    CHECK_STATUS(BNO_Get_Unit(usart, BNO_UNIT_DO_GYR, gyr_unit));

    return SUCCESS;
}

/**
 * @brief  Sets the units for the euler angles
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  eul_unit: Euler angle units selection
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_EUL_Unit(USART_Config_t *usart, BNO_Unit eul_unit) {
    CHECK_STATUS(Validate_Enum(eul_unit, BNO_UNIT_EUL_DEGREES, BNO_UNIT_EUL_RADIANS));

    CHECK_STATUS(BNO_Set_Unit(usart, eul_unit));

    return SUCCESS;
}

/**
 * @brief  Gets the units for the euler angles
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  eul_unit: Pointer to a variable used to store the retrieved euler angle units
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_EUL_Unit(USART_Config_t *usart, uint8_t *eul_unit) {
    CHECK_STATUS(Validate_Ptr(eul_unit));

    CHECK_STATUS(BNO_Get_Unit(usart, BNO_UNIT_DO_EUL, eul_unit));

    return SUCCESS;
}

/**
 * @brief  Sets the units for the temperature
 * @param  usart:     Pointer to a struct containing USART settings
 * @param  temp_unit: Temperature units selection
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_TEMP_Unit(USART_Config_t *usart, BNO_Unit temp_unit) {
    CHECK_STATUS(Validate_Enum(temp_unit, BNO_UNIT_TEMP_CEL, BNO_UNIT_TEMP_FAH));

    CHECK_STATUS(BNO_Set_Unit(usart, temp_unit));

    return SUCCESS;
}

/**
 * @brief  Gets the units for the temperature
 * @param  usart:     Pointer to a struct containing USART settings
 * @param  temp_unit: Pointer to a variable used to store the retrieved temperature units
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_TEMP_Unit(USART_Config_t *usart, uint8_t *temp_unit) {
    CHECK_STATUS(Validate_Ptr(temp_unit));

    CHECK_STATUS(BNO_Get_Unit(usart, BNO_UNIT_DO_TEMP, temp_unit));

    return SUCCESS;
}

/**
 * @brief  Sets the operating system-based orientation
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  ori_unit: Operating system-based orientation
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_ORI_Unit(USART_Config_t *usart, BNO_Unit ori_unit) {
    CHECK_STATUS(Validate_Enum(ori_unit, BNO_UNIT_ORI_WINDOWS, BNO_UNIT_ORI_ANDROID));

    CHECK_STATUS(BNO_Set_Unit(usart, ori_unit));

    return SUCCESS;
}

/**
 * @brief  Gets the operating system-based orientation 
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  ori_unit: Pointer to a variable used to store the retrieved operating system-based 
 *                   orientation
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_ORI_Unit(USART_Config_t *usart, uint8_t *ori_unit) {
    CHECK_STATUS(Validate_Ptr(ori_unit));

    CHECK_STATUS(BNO_Get_Unit(usart, BNO_UNIT_DO_ORI, ori_unit));

    return SUCCESS;
}


/**************************************************************************************************/
/*                                      Axis Remap Functions                                      */
/**************************************************************************************************/

/**
 * @brief  Remaps an axis to a new reference axis
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  target_axis: Axis to be remaped
 * @param  new_axis:    New reference axis
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Axis_Remap(USART_Config_t *usart, BNO_Axis target_axis, BNO_Axis new_axis) {
    CHECK_STATUS(Validate_Enum(target_axis, BNO_AXIS_X, BNO_AXIS_Z));
    CHECK_STATUS(Validate_Enum(new_axis, BNO_AXIS_X, BNO_AXIS_Z));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //configure axis remap if necessary
    if (target_axis == new_axis) {
        return SUCCESS;
    } else {
        //store current operating mode and switch to CONFIG_MODE
        uint8_t current_opr_mode = 0U;
        CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

        //write axis remap
        uint8_t mask        = (0x03U << (target_axis * 2U));
        uint8_t setting_val = (new_axis << (target_axis * 2U));
        CHECK_STATUS(BNO_Set_Setting(usart, BNO_AXIS_MAP_CONFIG_REG, mask, setting_val));

        //restore previous operating mode
        CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));
    }

    return SUCCESS;
}

/**
 * @brief  Remaps an axis' sign
 * @param  usart: Pointer to a struct containing USART settings
 * @param  axis:  Axis whose sign will be remapped
 * @param  sign:  New reference sign
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Axis_Sign_Remap(USART_Config_t *usart, BNO_Axis axis, BNO_Axis_Sign sign) {
    CHECK_STATUS(Validate_Enum(axis, BNO_AXIS_X, BNO_AXIS_Z));
    CHECK_STATUS(Validate_Enum(sign, BNO_POSITIVE_SIGN, BNO_NEGATIVE_SIGN));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //determine offset
    uint8_t offset = 0U;
    if (axis == BNO_AXIS_X) {
        offset = 2U;
    } else if (axis == BNO_AXIS_Y) {
        offset = 1U;
    } else {
        offset = 0U;
    }

    //read AXIS_MAP_SIGN and check if the current sign = remapped sign
    uint8_t reg_val_og[BNO_RESPONSE_HEADER_LENGTH + BNO_GENERIC_RW_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, BNO_AXIS_MAP_SIGN_REG, 1U, reg_val_og));
    if ((reg_val_og[2] & (1U << offset)) == sign) {
        return SUCCESS;
    }

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //modify and write register value back
    uint8_t reg_val_mod = 0U;
    if (sign == BNO_POSITIVE_SIGN) {
        reg_val_mod = (reg_val_og[2] & ~(1U << offset));
    } else {
        reg_val_mod = (reg_val_og[2] | (1U << offset));
    }
    CHECK_STATUS(BNO_Write_Reg(usart, BNO_AXIS_MAP_SIGN_REG, 1U, &reg_val_mod));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}


/**************************************************************************************************/
/*                                       Interrupt Functions                                      */
/**************************************************************************************************/

/**
 * @brief  Enables a particular interrupt
 * @param  usart: Pointer to a struct containing USART settings
 * @param  irq:   Interrupt
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Enable_IRQ(USART_Config_t *usart, BNO_IRQ irq) {
    CHECK_STATUS(Validate_Enum(irq, BNO_IRQ_ACC_BSX_DRDY, BNO_IRQ_ACC_NM));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read INT_EN and check if interrupts are already enabled
    uint8_t reg_val_og[BNO_RESPONSE_HEADER_LENGTH + BNO_GENERIC_RW_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, BNO_INT_EN_REG, 1U, reg_val_og));
    if (reg_val_og[2] & (1U << irq)) {
        return SUCCESS;
    }

    //modify and write register value back
    uint8_t reg_val_mod = (reg_val_og[2] | (1U << irq));
    CHECK_STATUS(BNO_Write_Reg(usart, BNO_INT_EN_REG, 1U, &reg_val_mod));

    return SUCCESS;
}

/**
 * @brief  Disables a particular interrupt
 * @param  usart: Pointer to a struct containing USART settings
 * @param  irq:   Interrupt
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Disable_IRQ(USART_Config_t *usart, BNO_IRQ irq) {
    CHECK_STATUS(Validate_Enum(irq, BNO_IRQ_ACC_BSX_DRDY, BNO_IRQ_ACC_NM));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read INT_EN and check if interrupts are already disabled
    uint8_t reg_val_og[BNO_RESPONSE_HEADER_LENGTH + BNO_GENERIC_RW_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, BNO_INT_EN_REG, 1U, reg_val_og));
    if (!(reg_val_og[2] | ~(1U << irq))) {
        return SUCCESS;
    }

    //modify and write register value back
    uint8_t reg_val_mod = (reg_val_og[2] & ~(1U << irq));
    CHECK_STATUS(BNO_Write_Reg(usart, BNO_INT_EN_REG, 1U, &reg_val_mod));

    return SUCCESS;
}

/**
 * @brief  Resets all interrupts
 * @param  usart: Pointer to a struct containing USART settings
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Reset_IRQ(USART_Config_t *usart) {
    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //set reset interrupt bit
    uint8_t mask        = (0x00U);
    uint8_t setting_val = BNO_SYS_TRIGGER_RST_INT;
    CHECK_STATUS(BNO_Set_Setting(usart, BNO_SYS_TRIGGER_REG, mask, setting_val));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the status of a particular BNO055 interrupt
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  irq:    BNO055 Interrupt
 * @param  status: Pointer to a variable used to store interrupt status
 * @retval Status indicating success, invalid parameters or error
 * @note   If status != 0, the interrupt has been triggered
 */
Status BNO_Get_IRQ_Status(USART_Config_t *usart, BNO_IRQ irq, uint8_t *status) {
    CHECK_STATUS(Validate_Enum(irq, BNO_IRQ_ACC_BSX_DRDY, BNO_IRQ_ACC_NM));
    CHECK_STATUS(Validate_Ptr(status));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_0));

    //read INT_STA, then extract and store interrupt status
    uint8_t data[BNO_RESPONSE_HEADER_LENGTH + BNO_GENERIC_RW_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, BNO_INT_STA_REG, 1U, data));
    *status = (uint8_t) (data[2] & (1U << irq));

    return SUCCESS;
}

/**
 * @brief  Enables masking for a specified interrupt, allowing it to trigger the INT pin
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  irq:    BNO055 Interrupt
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Enable_IRQ_Msk(USART_Config_t *usart, BNO_IRQ irq) {
    CHECK_STATUS(Validate_Enum(irq, BNO_IRQ_ACC_BSX_DRDY, BNO_IRQ_ACC_NM));
    
    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read INT_MSK and check if the mask is already enabled
    uint8_t reg_val_og[BNO_RESPONSE_HEADER_LENGTH + BNO_GENERIC_RW_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, BNO_INT_MSK_REG, 1U, reg_val_og));
    if (reg_val_og[2] & (1U << irq)) {
        return SUCCESS;
    }

    //modify and write register value back
    uint8_t reg_val_mod = (reg_val_og[2] | (1U << irq));
    CHECK_STATUS(BNO_Write_Reg(usart, BNO_INT_MSK_REG, 1U, &reg_val_mod));

    return SUCCESS;
}

/**
 * @brief  Disables masking for a specified interrupt, preventing it from triggering the INT pin
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  irq:    BNO055 Interrupt
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Disable_IRQ_Msk(USART_Config_t *usart, BNO_IRQ irq) {
    CHECK_STATUS(Validate_Enum(irq, BNO_IRQ_ACC_BSX_DRDY, BNO_IRQ_ACC_NM));
    
    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read INT_MSK and check if the mask is already disabled 
    uint8_t reg_val_og[BNO_RESPONSE_HEADER_LENGTH + BNO_GENERIC_RW_LENGTH] = {0};
    CHECK_STATUS(BNO_Read_Reg(usart, BNO_INT_MSK_REG, 1U, reg_val_og));
    if (!(reg_val_og[2] | ~(1U << irq))) {
        return SUCCESS;
    }

    //modify and write register value back
    uint8_t reg_val_mod = (reg_val_og[2] & ~(1U << irq));
    CHECK_STATUS(BNO_Write_Reg(usart, BNO_INT_MSK_REG, 1U, &reg_val_mod));

    return SUCCESS;
}

/**
 * @brief  Sets the enable state of a specified axis monitored for a given interrupt source
 * @param  usart:   Pointer to a struct containing USART settings
 * @param  axis:    Target axis to configure (X/Y/Z)
 * @param  irq_adr: Register address of the target interrupt
 * @param  mask:    Bit mask used to isolate the target axis bits in the interrupt register
 * @param  state:   Value to write to @p irq_adr to set the axis state
 * @retval Status indicating success, invalid parameters or error
 */
static Status BNO_Set_Axis_State(
    USART_Config_t     *usart,
    BNO_Axis           axis,
    uint8_t            irq_adr,
    uint8_t            mask,
    uint8_t            state
) {
    CHECK_STATUS(Validate_Enum(axis, BNO_AXIS_X, BNO_AXIS_Y));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //write axis state
    CHECK_STATUS(BNO_Set_Setting(usart, irq_adr, mask, state));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the enable state of a specified axis monitored for a given interrupt source
 * @param  usart:   Pointer to a struct containing USART settings
 * @param  axis:    Target axis to query (X/Y/Z)
 * @param  irq_adr: Register address of the target interrupt
 * @param  mask:    Bit mask used to isolate the target axis bits in the interrupt register
 * @param  state:   Pointer to a variable used to store the retrieved axis state
 * @retval Status indicating success, invalid parameters or error
 * @note   If state = 0, axis is disabled; otherwise axis is enabled
 */
static Status BNO_Get_Axis_State(
    USART_Config_t *usart,
    BNO_Axis       axis, 
    uint8_t        irq_adr, 
    uint8_t        mask, 
    uint8_t        *state
) {
    CHECK_STATUS(Validate_Enum(axis, BNO_AXIS_X, BNO_AXIS_Y));
    CHECK_STATUS(Validate_Ptr(state));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read axis state
    CHECK_STATUS(BNO_Get_Setting(usart, irq_adr, mask, state));

    return SUCCESS;
}

/**
 * @brief  Configures the acc slow/no motion interrupt
 * @param  usart:        Pointer to a struct containing USART settings
 * @param  sm_nm_config: Pointer to a struct containing config settings
 * @note   In the initialisation of the config settings, if no motion mode is selected, 
 *         slope_points = 0U. If slow motion mode is selected, delay_s = 0U
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_ACC_SM_NM_Config(USART_Config_t *usart, BNO_ACC_SM_NM_Config_t *sm_nm_config) {
    CHECK_STATUS(Validate_Ptr(sm_nm_config));

    //enable acc slow/no motion interrupts
    CHECK_STATUS(BNO_Enable_IRQ(usart, BNO_IRQ_ACC_NM));

    //configure detection type, threshold, and slope points/delay appropriately
    CHECK_STATUS(BNO_Set_ACC_SM_NM_Det_Type(usart, sm_nm_config->det_type));
    CHECK_STATUS(BNO_Set_ACC_SM_NM_Thres(usart, sm_nm_config->thres));
    if (sm_nm_config->det_type == BNO_SM_NM_NO_MOTION) {
        CHECK_STATUS(BNO_Set_ACC_NM_Delay(usart, sm_nm_config->delay_s));
    } else {
        CHECK_STATUS(BNO_Set_ACC_SM_Slope_Points(usart, sm_nm_config->slope_points));
    }

    //enable selected axis/axes
    if (sm_nm_config->x_axis) {
        CHECK_STATUS(BNO_Set_ACC_SM_NM_Axis_State(usart, BNO_AXIS_X, BNO_IRQ_AXIS_ENABLED));
    }
    if (sm_nm_config->y_axis) {
        CHECK_STATUS(BNO_Set_ACC_SM_NM_Axis_State(usart, BNO_AXIS_Y, BNO_IRQ_AXIS_ENABLED));
    }
    if (sm_nm_config->z_axis) {
        CHECK_STATUS(BNO_Set_ACC_SM_NM_Axis_State(usart, BNO_AXIS_Z, BNO_IRQ_AXIS_ENABLED));
    }

    return SUCCESS;
}

/**
 * @brief  Sets the acc slow/no motion interrupt detection type
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  det_type: Detection type to configure
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_ACC_SM_NM_Det_Type(USART_Config_t *usart, BNO_SM_NM_Det_Type det_type) {
    CHECK_STATUS(Validate_Enum(det_type, BNO_SM_NM_SLOW_MOTION, BNO_SM_NM_NO_MOTION));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //configure detection type
    uint8_t mask        = BNO_ACC_NM_SET_SM_NM;
    uint8_t setting_val = (((uint8_t) det_type) << BNO_ACC_NM_SET_SM_NM_Pos);
    CHECK_STATUS(BNO_Set_Setting(usart, BNO_ACC_NM_SET_REG, mask, setting_val));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the acc slow/no motion interrupt detection type
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  det_type: Pointer to a variable used to store the retrieved detection type
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_ACC_SM_NM_Det_Type(USART_Config_t *usart, uint8_t *det_type) {
    CHECK_STATUS(Validate_Ptr(det_type));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read detection type
    CHECK_STATUS(BNO_Get_Setting(usart, BNO_ACC_NM_SET_REG, BNO_ACC_NM_SET_SM_NM, det_type));
    *det_type = (*det_type >> BNO_ACC_NM_SET_SM_NM_Pos);

    return SUCCESS;
}

/**
 * @brief  Sets the acc slow/no motion interrupt threshold
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  thres_mg: Threshold (in mg) to configure
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_ACC_SM_NM_Thres(USART_Config_t *usart, float thres_mg) {
    //validate thres_mg based on configured acc range
    uint8_t acc_range = 0U;
    CHECK_STATUS(BNO_Get_ACC_Range(usart, &acc_range));
    float max_thres_mg[] = {996.0f, 1990.0f, 3980.0f, 7970.0f};
    if (thres_mg > max_thres_mg[acc_range] || thres_mg < 0.0f) {
        return INVALID_PARAM;
    }

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //configure threshold
    float acc_sm_nm_lsb_vals[] = {3.91f, 7.81f, 15.6f, 31.3f};
    float acc_sm_nm_lsb_sel    = acc_sm_nm_lsb_vals[acc_range];
    uint8_t thres = (uint8_t) (thres_mg / acc_sm_nm_lsb_sel);
    if (thres > 255U) {
        thres = 255U;
    }
    CHECK_STATUS(BNO_Write_Reg(usart, BNO_ACC_NM_THRES_REG, 1U, &thres));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the acc slow/no motion interrupt threshold
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  thres_mg: Pointer to a float variable used to store the retrieved threshold (in mg)
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_ACC_SM_NM_Thres(USART_Config_t *usart, float *thres_mg) {
    CHECK_STATUS(Validate_Ptr(thres_mg));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read raw sm/nm threshold
    uint8_t raw_thres = 0U;
    CHECK_STATUS(BNO_Get_Setting(usart, BNO_ACC_NM_THRES_REG, BNO_ACC_NM_THRES, &raw_thres));
    raw_thres = (raw_thres >> BNO_ACC_NM_THRES_Pos);

    //convert and store sm/nm threshold
    uint8_t acc_range = 0U;
    CHECK_STATUS(BNO_Get_ACC_Range(usart, &acc_range));
    float acc_sm_nm_lsb_vals[] = {3.91f, 7.81f, 15.6f, 31.3f};
    float acc_sm_nm_lsb_sel    = acc_sm_nm_lsb_vals[acc_range];
    *thres_mg = (((float) raw_thres) * acc_sm_nm_lsb_sel);

    return SUCCESS;
}

/**
 * @brief  Sets the acc slow motion interrupt slope points
 * @param  usart:        Pointer to a struct containing USART settings
 * @param  slope_points: Slope points to configure
 * @retval Status indicating success, invalid parameters or error
 * @note   This function should only be used in slow motion detection mode
 */
Status BNO_Set_ACC_SM_Slope_Points(USART_Config_t *usart, uint8_t slope_points) {
    //validate that slow motion detection type is selected
    uint8_t det_type = 0U;
    CHECK_STATUS(BNO_Get_ACC_SM_NM_Det_Type(usart, &det_type));
    if (det_type != BNO_SM_NM_SLOW_MOTION) {
        return INVALID_PARAM;
    }

    //validate slope points
    if (slope_points < 0U || slope_points > 4U) {
        return INVALID_PARAM;
    }

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //configure slope points
    uint8_t mask        = BNO_ACC_NM_SET_SM_ONLY_DUR;
    uint8_t setting_val = ((slope_points - 1U));
    CHECK_STATUS(BNO_Set_Setting(usart, BNO_ACC_NM_SET_REG, mask, setting_val));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the acc slow motion interrupt slope points
 * @param  usart:        Pointer to a struct containing USART settings
 * @param  slope_points: Pointer to a variable used to store the retrieved slope points
 * @retval Status indicating success, invalid parameters or error
 * @note   This function should only be used in slow motion detection mode
 */
Status BNO_Get_ACC_SM_Slope_Points(USART_Config_t *usart, uint8_t *slope_points) {
    CHECK_STATUS(Validate_Ptr(slope_points));

    //validate that slow motion detection type is selected
    uint8_t det_type = 0U;
    CHECK_STATUS(BNO_Get_ACC_SM_NM_Det_Type(usart, &det_type));
    if (det_type != BNO_SM_NM_SLOW_MOTION) {
        return INVALID_PARAM;
    }

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));
    
    //read raw slope points, then convert and store slope data points
    uint8_t raw_n_slope_points = 0U;
    CHECK_STATUS(BNO_Get_Setting(
        usart,
        BNO_ACC_NM_SET_REG,
        BNO_ACC_NM_SET_SM_ONLY_DUR,
        &raw_n_slope_points
    ));
    *slope_points = ((raw_n_slope_points >> BNO_ACC_NM_SET_SM_NM_DUR_Pos) + 1U);

    return SUCCESS;
}

/**
 * @brief  Sets the acc no motion interrupt delay
 * @param  usart:   Pointer to a struct containing USART settings
 * @param  delay_s: Delay (in seconds) to configure
 * @retval Status indicating success, invalid parameters or error
 * @note   This function should only be used in no motion detection mode
 * @note   delay_s can only be in the range of 1s to 336s. If delay_s > 16s, it must be divisible
 *         by 8
 */
Status BNO_Set_ACC_NM_Delay(USART_Config_t *usart, uint16_t delay_s) {
    //validate that no motion detection type is selected
    uint8_t det_type = 0U;
    CHECK_STATUS(BNO_Get_ACC_SM_NM_Det_Type(usart, &det_type));
    if (det_type != BNO_SM_NM_NO_MOTION) {
        return INVALID_PARAM;
    }

    //validate delay
    if (delay_s >= 1U && delay_s <= 336U) {
        if (delay_s > 16U) {
            if ((delay_s % 8U) != 0) {
                return INVALID_PARAM;
            }
        }
    } else {
        return INVALID_PARAM;
    }

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //determine setting bit value
    uint8_t setting_val = 0U;
    if (delay_s >= 1U && delay_s <= 15U) {
        setting_val = (delay_s - 1U);
    } else if (delay_s >= 40U && delay_s <= 80U) {
        setting_val = (((delay_s - 40U) / 8U) + 16U);
    } else {
        setting_val = (((delay_s - 88U) / 8U) + 32U);
    }
    setting_val = (setting_val << BNO_ACC_NM_SET_SM_NM_DUR_Pos);

    //configure delay
    uint8_t mask = BNO_ACC_NM_SET_SM_NM_DUR;
    CHECK_STATUS(BNO_Set_Setting(usart, BNO_ACC_NM_SET_REG, mask, setting_val));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the acc no motion interrupt delay
 * @param  usart:   Pointer to a struct containing USART settings
 * @param  delay_s: Pointer to a uint16_t variable used to store the retrieved delay (in seconds)
 * @retval Status indicating success, invalid parameters or error
 * @note   This function should only be used in no motion detection mode
 */
Status BNO_Get_ACC_NM_Delay(USART_Config_t *usart, uint16_t *delay_s) {
    CHECK_STATUS(Validate_Ptr(delay_s));

    //validate that no motion detection type is selected
    uint8_t det_type = 0U;
    CHECK_STATUS(BNO_Get_ACC_SM_NM_Det_Type(usart, &det_type));
    if (det_type != BNO_SM_NM_NO_MOTION) {
        return INVALID_PARAM;
    }

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read raw delay
    uint8_t raw_delay = 0U;
    CHECK_STATUS(BNO_Get_Setting(
        usart,
        BNO_ACC_NM_SET_REG,
        BNO_ACC_NM_SET_SM_NM_DUR,
        &raw_delay
    ));
    raw_delay = (raw_delay >> BNO_ACC_NM_SET_SM_NM_DUR_Pos);

    //extract 2 MSBs
    uint8_t two_msbs = (raw_delay & (0x03U << 4U));

    //convert and store no motion delay
    if (two_msbs == 0U) {
        uint8_t four_lsbs = (raw_delay & 0x0FU);
        *delay_s = (four_lsbs + 1U);
    } else if (two_msbs == 1U) {
        uint8_t four_lsbs = (raw_delay & 0x0FU);
        *delay_s = ((four_lsbs * 8U) + 40U);
    } else {
        uint8_t five_lsbs = (raw_delay & 0x1FU);
        *delay_s = ((five_lsbs * 8U) + 88U);
    }

    return SUCCESS;
}

/**
 * @brief  Sets the enable state of a specified axis monitored for an acc slow/any motion interrupt
 * @param  usart: Pointer to a struct containing USART settings
 * @param  axis:  Target axis to configure (X/Y/Z)
 * @param  state: Enable state to configure
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_ACC_SM_NM_Axis_State(USART_Config_t *usart, BNO_Axis axis, BNO_IRQ_Axis_State state) {
    CHECK_STATUS(Validate_Enum(state, BNO_IRQ_AXIS_DISABLED, BNO_IRQ_AXIS_ENABLED));

    uint8_t mask     = (0x01U << (BNO_ACC_INT_SETTINGS_AM_NM_X_AXIS_Pos + axis));
    uint8_t axis_val = (((uint8_t) state) << (BNO_ACC_INT_SETTINGS_AM_NM_X_AXIS_Pos + axis));
    CHECK_STATUS(BNO_Set_Axis_State(usart, axis, BNO_ACC_INT_SETTINGS_REG, mask, axis_val));

    return SUCCESS;
}

/**
 * @brief  Gets the enable state of a specified axis monitored for an acc slow/any motion interrupt
 * @param  usart: Pointer to a struct containing USART settings
 * @param  axis:  Target axis to query (X/Y/Z)
 * @param  state: Pointer to a variable used to store the retrieved axis state
 * @retval Status indicating success, invalid parameters or error
 * @note   If state = 0, axis is disabled; otherwise axis is enabled
 */
Status BNO_Get_ACC_SM_NM_Axis_State(USART_Config_t *usart, BNO_Axis axis, uint8_t *state) {
    uint8_t mask = (0x01U << (BNO_ACC_INT_SETTINGS_AM_NM_X_AXIS_Pos + axis));
    CHECK_STATUS(BNO_Get_Axis_State(usart, axis, BNO_ACC_INT_SETTINGS_REG, mask, state));

    return SUCCESS;
}

/**
 * @brief  Configures the acc any motion interrupt
 * @param  usart:     Pointer to a struct containing USART settings
 * @param  am_config: Pointer to a struct containing config settings
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_ACC_AM_Config(USART_Config_t *usart, BNO_ACC_AM_Config_t *am_config) {
    CHECK_STATUS(Validate_Ptr(am_config));

    //enable acc any motion interrupts
    CHECK_STATUS(BNO_Enable_IRQ(usart, BNO_IRQ_ACC_AM));

    //configure threshold and slope points
    CHECK_STATUS(BNO_Set_ACC_AM_Thres(usart, am_config->thres));
    CHECK_STATUS(BNO_Set_ACC_AM_Slope_Points(usart, am_config->slope_points));

    //enable selected axis/axes
    if (am_config->x_axis) {
        CHECK_STATUS(BNO_Set_ACC_AM_Axis_State(usart, BNO_AXIS_X, BNO_IRQ_AXIS_ENABLED));
    }
    if (am_config->y_axis) {
        CHECK_STATUS(BNO_Set_ACC_AM_Axis_State(usart, BNO_AXIS_Y, BNO_IRQ_AXIS_ENABLED));
    }
    if (am_config->z_axis) {
        CHECK_STATUS(BNO_Set_ACC_AM_Axis_State(usart, BNO_AXIS_Z, BNO_IRQ_AXIS_ENABLED));
    }

    return SUCCESS;
}

/**
 * @brief  Sets the acc any motion interrupt threshold
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  thres_mg: Threshold (in mg) to configure
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_ACC_AM_Thres(USART_Config_t *usart, float thres_mg) {
    //validate thres_mg based on configured acc range
    uint8_t acc_range = 0U;
    CHECK_STATUS(BNO_Get_ACC_Range(usart, &acc_range));
    float max_thres_mg[] = {996.0f, 1990.0f, 3980.0f, 7970.0f};
    if (thres_mg > max_thres_mg[acc_range] || thres_mg < 0.0f) {
        return INVALID_PARAM;
    }

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //configure threshold
    float acc_am_lsb_vals[] = {3.91f, 7.81f, 15.6f, 31.3f};
    float acc_am_lsb_sel    = acc_am_lsb_vals[acc_range];
    uint8_t thres = (uint8_t) (thres_mg / acc_am_lsb_sel);
    if (thres > 255U) {
        thres = 255U;
    }
    CHECK_STATUS(BNO_Write_Reg(usart, BNO_ACC_AM_THRES_REG, 1U, &thres));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the acc any motion interrupt threshold
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  thres_mg: Pointer to a variable used to store the retrieved threshold (in mg)
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_ACC_AM_Thres(USART_Config_t *usart, float *thres_mg) {
    CHECK_STATUS(Validate_Ptr(thres_mg));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read raw any motion threshold
    uint8_t raw_thres = 0U;
    CHECK_STATUS(BNO_Get_Setting(usart, BNO_ACC_AM_THRES_REG, BNO_ACC_AM_THRES, &raw_thres));

    //convert and store any motion threshold
    uint8_t acc_range = 0U;
    CHECK_STATUS(BNO_Get_ACC_Range(usart, &acc_range));
    float acc_am_lsb_vals[] = {3.91f, 7.81f, 15.6f, 31.3f};
    float acc_am_lsb_sel    = acc_am_lsb_vals[acc_range];
    *thres_mg = (((float) raw_thres) * acc_am_lsb_sel);

    return SUCCESS;
}

/**
 * @brief  Sets the acc any motion interrupt slope points
 * @param  usart:        Pointer to a struct containing USART settings
 * @param  slope_points: Slope points to configure
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_ACC_AM_Slope_Points(USART_Config_t *usart, uint8_t slope_points) {
    //validate slope points 
    if (slope_points < 0U || slope_points > 4U) {
        return INVALID_PARAM;
    }

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //configure slope points
    uint8_t mask        = BNO_ACC_INT_SETTINGS_AM_DUR;
    uint8_t setting_val = ((slope_points - 1U));
    CHECK_STATUS(BNO_Set_Setting(usart, BNO_ACC_INT_SETTINGS_REG, mask, setting_val));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the acc any motion interrupt slope points
 * @param  usart:        Pointer to a struct containing USART settings
 * @param  slope_points: Pointer to a variable used to store the retrieved slope points
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_ACC_AM_Slope_Points(USART_Config_t *usart, uint8_t *slope_points) {
    CHECK_STATUS(Validate_Ptr(slope_points));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read raw slope points, then convert and store slope data points
    uint8_t raw_n_slope_points = 0U;
    CHECK_STATUS(BNO_Get_Setting(
        usart,
        BNO_ACC_INT_SETTINGS_REG,
        BNO_ACC_INT_SETTINGS_AM_DUR,
        &raw_n_slope_points
    ));
    *slope_points = (raw_n_slope_points + 1U);

    return SUCCESS;
}

/**
 * @brief  Sets the enable state of a specified axis monitored for an acc any motion interrupt
 * @param  usart: Pointer to a struct containing USART settings
 * @param  axis:  Target axis to configure (X/Y/Z)
 * @param  state: Enable state to configure
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_ACC_AM_Axis_State(USART_Config_t *usart, BNO_Axis axis, BNO_IRQ_Axis_State state) {
    CHECK_STATUS(Validate_Enum(state, BNO_IRQ_AXIS_DISABLED, BNO_IRQ_AXIS_ENABLED));

    uint8_t mask     = (0x01U << (BNO_ACC_INT_SETTINGS_AM_NM_X_AXIS_Pos + axis));
    uint8_t axis_val = (((uint8_t) state) << (BNO_ACC_INT_SETTINGS_AM_NM_X_AXIS_Pos + axis));
    CHECK_STATUS(BNO_Set_Axis_State(usart, axis, BNO_ACC_INT_SETTINGS_REG, mask, axis_val));

    return SUCCESS;
}

/**
 * @brief  Gets the enable state of a specified axis monitored for an acc any motion interrupt
 * @param  usart: Pointer to a struct containing USART settings
 * @param  axis:  Target axis to query (X/Y/Z)
 * @param  state: Pointer to a variable used to store the retrieved axis state
 * @retval Status indicating success, invalid parameters or error
 * @note   If state = 0, axis is disabled; otherwise axis is enabled
 */
Status BNO_Get_ACC_AM_Axis_State(USART_Config_t *usart, BNO_Axis axis, uint8_t *state) {
    uint8_t mask = (0x01U << (BNO_ACC_INT_SETTINGS_AM_NM_X_AXIS_Pos + axis));
    CHECK_STATUS(BNO_Get_Axis_State(usart, axis, BNO_ACC_INT_SETTINGS_REG, mask, state));

    return SUCCESS;
}

/**
 * @brief  Configures the acc high-g interrupt
 * @param  usart:     Pointer to a struct containing USART settings
 * @param  hg_config: Pointer to a struct containing config settings
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_ACC_HG_Config(USART_Config_t *usart, BNO_ACC_HG_Config_t *hg_config) {
    CHECK_STATUS(Validate_Ptr(hg_config));

    //enable acc high-g interrupts
    CHECK_STATUS(BNO_Enable_IRQ(usart, BNO_IRQ_ACC_HIGH_G));

    //configure threshold and duration
    CHECK_STATUS(BNO_Set_ACC_HG_Thres(usart, hg_config->thres));
    CHECK_STATUS(BNO_Set_ACC_HG_Dur(usart, hg_config->dur_ms));

    //enable selected axis/axes
    if (hg_config->x_axis) {
        CHECK_STATUS(BNO_Set_ACC_HG_Axis_State(usart, BNO_AXIS_X, BNO_IRQ_AXIS_ENABLED));
    }
    if (hg_config->y_axis) {
        CHECK_STATUS(BNO_Set_ACC_HG_Axis_State(usart, BNO_AXIS_Y, BNO_IRQ_AXIS_ENABLED));
    }
    if (hg_config->z_axis) {
        CHECK_STATUS(BNO_Set_ACC_HG_Axis_State(usart, BNO_AXIS_Z, BNO_IRQ_AXIS_ENABLED));
    }

    return SUCCESS;
}

/**
 * @brief  Sets the acc high-g interrupt threshold
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  thres_mg: Threshold (in mg) to configure
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_ACC_HG_Thres(USART_Config_t *usart, float thres_mg) {
    //validate thres_mg based on configured acc range
    uint8_t acc_range = 0U;
    CHECK_STATUS(BNO_Get_ACC_Range(usart, &acc_range));
    float max_thres_mg[] = {2000.0f, 4000.0f, 8000.0f, 16000.0f};
    if (thres_mg > max_thres_mg[acc_range] || thres_mg < 0.0f) {
        return INVALID_PARAM;
    }

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //configure threshold    
    float acc_hg_lsb_vals[] = {7.81f, 15.63f, 31.25f, 62.5f};
    float acc_hg_lsb_sel    = acc_hg_lsb_vals[acc_range];
    uint8_t thres = (uint8_t) (thres_mg / acc_hg_lsb_sel);
    if (thres > 255U) {
        thres = 255U;
    }
    CHECK_STATUS(BNO_Write_Reg(usart, BNO_ACC_HG_THRES_REG, 1U, &thres));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));
    
    return SUCCESS;
}

/**
 * @brief  Gets the acc high-g interrupt threshold
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  thres_mg: Pointer to a variable used to store the retrieved threshold (in mg)
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_ACC_HG_Thres(USART_Config_t *usart, float *thres_mg) {
    CHECK_STATUS(Validate_Ptr(thres_mg));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read raw high-g threshold
    uint8_t raw_thres = 0U;
    CHECK_STATUS(BNO_Get_Setting(usart, BNO_ACC_HG_THRES_REG, BNO_ACC_HG_THRES, &raw_thres));

    //convert and store high-g threshold
    uint8_t acc_range = 0U;
    CHECK_STATUS(BNO_Get_ACC_Range(usart, &acc_range));
    float acc_hg_lsb_vals[] = {7.81f, 15.63f, 31.25f, 62.5f};
    float acc_hg_lsb_sel    = acc_hg_lsb_vals[acc_range];
    *thres_mg = (((float) raw_thres) * acc_hg_lsb_sel);

    return SUCCESS;
}

/**
 * @brief  Sets the acc high-g interrupt duration
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  dur_ms: Duration (in ms) to configure
 * @retval Status indicating success, invalid parameters or error
 * @note   dur_ms can only be in the range between 2ms and 512ms
 */
Status BNO_Set_ACC_HG_Dur(USART_Config_t *usart, uint16_t dur_ms) {
    //validate delay_ms is between 2ms and 512ms
    if (dur_ms < 2U || dur_ms > 512U) {
        return INVALID_PARAM;
    }

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //configure duration
    uint8_t duration = ((dur_ms / 2U) - 1U);
    CHECK_STATUS(BNO_Write_Reg(usart, BNO_ACC_HG_DURATION_REG, 1U, &duration));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));
    
    return SUCCESS;
}

/**
 * @brief  Gets the acc high-g interrupt duration
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  thres_mg: Pointer to a variable used to store the retrieved duration (in ms)
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_ACC_HG_Dur(USART_Config_t *usart, uint16_t *dur_ms) {
    CHECK_STATUS(Validate_Ptr(dur_ms));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read raw high-g duration, then convert and store high-g duration
    uint8_t raw_dur = 0U;
    CHECK_STATUS(BNO_Get_Setting(
        usart, 
        BNO_ACC_HG_DURATION_REG, 
        BNO_ACC_HG_DURATION, 
        &raw_dur
    ));
    *dur_ms = (uint16_t) ((raw_dur + 1U) * 2U);

    return SUCCESS;
}

/**
 * @brief  Sets the enable state of a specified axis monitored for an acc high-g interrupt
 * @param  usart: Pointer to a struct containing USART settings
 * @param  axis:  Target axis to configure (X/Y/Z)
 * @param  state: Enable state to configure
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_ACC_HG_Axis_State(USART_Config_t *usart, BNO_Axis axis, BNO_IRQ_Axis_State state) {
    CHECK_STATUS(Validate_Enum(state, BNO_IRQ_AXIS_DISABLED, BNO_IRQ_AXIS_ENABLED));

    uint8_t mask     = (0x01U << (BNO_ACC_INT_SETTINGS_HG_X_AXIS_Pos + axis));
    uint8_t axis_val = (((uint8_t) state) << (BNO_ACC_INT_SETTINGS_HG_X_AXIS_Pos + axis));
    CHECK_STATUS(BNO_Set_Axis_State(usart, axis, BNO_ACC_INT_SETTINGS_REG, mask, axis_val));

    return SUCCESS;
}

/**
 * @brief  Gets the enable state of a specified axis monitored for an acc high-g interrupt
 * @param  usart: Pointer to a struct containing USART settings
 * @param  axis:  Target axis to query (X/Y/Z)
 * @param  state: Pointer to a variable used to store the retrieved axis state
 * @retval Status indicating success, invalid parameters or error
 * @note   If state = 0, axis is disabled; otherwise axis is enabled
 */
Status BNO_Get_ACC_HG_Axis_State(USART_Config_t *usart, BNO_Axis axis, uint8_t *state) {
    uint8_t mask = (0x01U << (BNO_ACC_INT_SETTINGS_HG_X_AXIS_Pos + axis));
    CHECK_STATUS(BNO_Get_Axis_State(usart, axis, BNO_ACC_INT_SETTINGS_REG, mask, state));

    return SUCCESS;
}

/**
 * @brief  Configures the gyr high rate interrupt
 * @param  usart:     Pointer to a struct containing USART settings
 * @param  hr_config: Pointer to a struct containing config settings
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_GYR_HR_Config(USART_Config_t *usart, BNO_GYR_HR_Config_t *hr_config) {
    CHECK_STATUS(Validate_Ptr(hr_config));

    //enable gyr high rate interrupts
    CHECK_STATUS(BNO_Enable_IRQ(usart, BNO_IRQ_GYR_HIGH_RATE));

    //enable selected axis/axes and configure settings
    if (hr_config->x_axis) {
        CHECK_STATUS(BNO_Set_GYR_HR_Axis_State(usart, BNO_AXIS_X, BNO_IRQ_AXIS_ENABLED));
        CHECK_STATUS(BNO_Set_GYR_HR_Thres     (usart, BNO_AXIS_X, hr_config->x_set_thres));
        CHECK_STATUS(BNO_Set_GYR_HR_Hyst      (usart, BNO_AXIS_X, hr_config->x_set_hyst));
        CHECK_STATUS(BNO_Set_GYR_HR_Dur       (usart, BNO_AXIS_X, hr_config->x_set_dur_ms));
    }
    if (hr_config->y_axis) {
        CHECK_STATUS(BNO_Set_GYR_HR_Axis_State(usart, BNO_AXIS_Y, BNO_IRQ_AXIS_ENABLED));
        CHECK_STATUS(BNO_Set_GYR_HR_Thres     (usart, BNO_AXIS_Y, hr_config->y_set_thres));
        CHECK_STATUS(BNO_Set_GYR_HR_Hyst      (usart, BNO_AXIS_Y, hr_config->y_set_hyst));
        CHECK_STATUS(BNO_Set_GYR_HR_Dur       (usart, BNO_AXIS_Y, hr_config->y_set_dur_ms));
    }
    if (hr_config->z_axis) {
        CHECK_STATUS(BNO_Set_GYR_HR_Axis_State(usart, BNO_AXIS_Z, BNO_IRQ_AXIS_ENABLED));
        CHECK_STATUS(BNO_Set_GYR_HR_Thres     (usart, BNO_AXIS_Z, hr_config->z_set_thres));
        CHECK_STATUS(BNO_Set_GYR_HR_Hyst      (usart, BNO_AXIS_Z, hr_config->z_set_hyst));
        CHECK_STATUS(BNO_Set_GYR_HR_Dur       (usart, BNO_AXIS_Z, hr_config->z_set_dur_ms));
    }

    //configure filter
    CHECK_STATUS(BNO_Set_GYR_HR_Filter(usart, hr_config->filter));

    return SUCCESS;
}

/**
 * @brief  Sets the gyr high rate interrupt threshold
 * @param  usart:     Pointer to a struct containing USART settings
 * @param  thres_dps: Threshold (in dps) to configure
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_GYR_HR_Thres(USART_Config_t *usart, BNO_Axis axis, float thres_dps) {
    CHECK_STATUS(Validate_Enum(axis, BNO_AXIS_X, BNO_AXIS_Y));

    //validate thres_dps based on configured gyr range
    uint8_t gyr_range = 0U;
    CHECK_STATUS(BNO_Get_GYR_Range(usart, &gyr_range));
    float max_thres_dps[] = {2000.0f, 1000.0f, 500.0f, 250.0f, 125.0f};
    if (thres_dps > max_thres_dps[gyr_range] || thres_dps < 0.0f) {
        return INVALID_PARAM;
    }

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //select appropriate axis base address
    uint8_t axis_base_adr = 0U;
    if (axis == BNO_AXIS_X) {
        axis_base_adr = BNO_GYR_HR_X_SET_REG;
    } else if (axis == BNO_AXIS_Y) {
        axis_base_adr = BNO_GYR_HR_Y_SET_REG;
    } else {
        axis_base_adr = BNO_GYR_HR_Z_SET_REG;
    }

    //configure threshold
    float gyr_hr_lsb_vals[] = {62.5f, 31.25f, 15.625f, 7.8125f, 3.90625f};
    float gyr_hr_lsb_sel    = gyr_hr_lsb_vals[gyr_range];
    uint8_t thres = (uint8_t) (thres_dps / gyr_hr_lsb_sel);
    if (thres > 31U) {
        thres = 31U;
    }
    uint8_t mask = (0x1FU);
    CHECK_STATUS(BNO_Set_Setting(usart, axis_base_adr, mask, thres));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the gyr high rate interrupt threshold
 * @param  usart:     Pointer to a struct containing USART settings
 * @param  thres_dps: Pointer to a variable used to store the retrieved threshold (in dps)
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GYR_HR_Thres(USART_Config_t *usart, BNO_Axis axis, float *thres_dps) {
    CHECK_STATUS(Validate_Enum(axis, BNO_AXIS_X, BNO_AXIS_Y));
    CHECK_STATUS(Validate_Ptr(thres_dps));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //select appropriate axis base address
    uint8_t axis_base_adr = 0U;
    if (axis == BNO_AXIS_X) {
        axis_base_adr = BNO_GYR_HR_X_SET_REG;
    } else if (axis == BNO_AXIS_Y) {
        axis_base_adr = BNO_GYR_HR_Y_SET_REG;
    } else {
        axis_base_adr = BNO_GYR_HR_Z_SET_REG;
    }

    //read raw high rate threshold
    uint8_t mask      = (0x1FU);
    uint8_t raw_thres = 0U;
    CHECK_STATUS(BNO_Get_Setting(usart, axis_base_adr, mask, &raw_thres));

    //convert and store high rate threshold
    uint8_t gyr_range = 0U;
    CHECK_STATUS(BNO_Get_GYR_Range(usart, &gyr_range));
    float gyr_hr_lsb_vals[]  = {62.5f, 31.25f, 15.625f, 7.8125f, 3.90625f};
    float gyr_hr_zero_vals[] = {62.26f, 31.13f, 15.56f, 7.78f, 3.89f};
    if (raw_thres == 0U) {
        *thres_dps = gyr_hr_zero_vals[gyr_range];
    } else {
        float gyr_hr_lsb_sel = gyr_hr_lsb_vals[gyr_range];
        *thres_dps = (((float) raw_thres) * gyr_hr_lsb_sel);
    }

    return SUCCESS;
}

/**
 * @brief  Sets the gyr high rate interrupt hysteresis
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  hyst_dps: Hysteresis (in dps) to configure
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_GYR_HR_Hyst(USART_Config_t *usart, BNO_Axis axis, float hyst_dps) {
    CHECK_STATUS(Validate_Enum(axis, BNO_AXIS_X, BNO_AXIS_Y));

    //validate hyst_dps based on configured gyrscope range
    uint8_t gyr_range = 0U;
    CHECK_STATUS(BNO_Get_GYR_Range(usart, &gyr_range));
    float max_hyst_dps[] = {187.5f, 93.75f, 46.875f, 23.4375f, 11.71875f};
    if (((float) hyst_dps) > max_hyst_dps[gyr_range] || hyst_dps < 0.0f) {
        return INVALID_PARAM;
    }

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //select appropriate axis base address
    uint8_t axis_base_adr = 0U;
    if (axis == BNO_AXIS_X) {
        axis_base_adr = BNO_GYR_HR_X_SET_REG;
    } else if (axis == BNO_AXIS_Y) {
        axis_base_adr = BNO_GYR_HR_Y_SET_REG;
    } else {
        axis_base_adr = BNO_GYR_HR_Z_SET_REG;
    }

    //configure hysteresis
    float gyr_hr_lsb_vals[] = {62.5f, 31.25f, 15.625f, 7.8125f, 3.90625f};
    float gyr_hr_lsb_sel    = gyr_hr_lsb_vals[gyr_range];
    uint8_t hyst = (uint8_t) (hyst_dps / gyr_hr_lsb_sel);
    if (hyst > 3U) {
        hyst = 3U;
    }
    uint8_t mask = (0x03U << 5U);
    CHECK_STATUS(BNO_Set_Setting(usart, axis_base_adr, mask, hyst));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the gyr high rate interrupt hysteresis
 * @param  usart:    Pointer to a struct containing USART settings
 * @param  hyst_dps: Pointer to a variable used to store the retrieved hysteresis (in dps)
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GYR_HR_Hyst(USART_Config_t *usart, BNO_Axis axis, float *hyst_dps) {
    CHECK_STATUS(Validate_Ptr(hyst_dps));
    CHECK_STATUS(Validate_Enum(axis, BNO_AXIS_X, BNO_AXIS_Y));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //select appropriate axis base address and offset
    uint8_t axis_base_adr = 0U;
    if (axis == BNO_AXIS_X) {
        axis_base_adr = BNO_GYR_HR_X_SET_REG;
    } else if (axis == BNO_AXIS_Y) {
        axis_base_adr = BNO_GYR_HR_Y_SET_REG;
    } else {
        axis_base_adr = BNO_GYR_HR_Z_SET_REG;
    }

    //read raw high rate hysteresis
    uint8_t mask     = (0x03U << BNO_GYR_HR_X_SET_HYST_Pos);
    uint8_t raw_hyst = 0U;
    CHECK_STATUS(BNO_Get_Setting(usart, axis_base_adr, mask, &raw_hyst));
    raw_hyst = (raw_hyst >> BNO_GYR_HR_X_SET_HYST_Pos);

    //convert and store high rate hysteresis
    uint8_t gyr_range = 0U;
    CHECK_STATUS(BNO_Get_GYR_Range(usart, &gyr_range));
    float gyr_hr_lsb_vals[]  = {62.5f, 31.25f, 15.625f, 7.8125f, 3.90625f};
    float gyr_hr_zero_vals[] = {62.26f, 31.13f, 15.56f, 7.78f, 3.89f};
    if (raw_hyst == 0U) {
        *hyst_dps = gyr_hr_zero_vals[gyr_range];
    } else {
        float gyr_hr_lsb_sel = gyr_hr_lsb_vals[gyr_range];
        *hyst_dps = (((float) raw_hyst) * gyr_hr_lsb_sel);
    }

    return SUCCESS;
}

/**
 * @brief  Sets the gyr high rate interrupt duration
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  dur_ms: Duration (in ms) to configure
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_GYR_HR_Dur(USART_Config_t *usart, BNO_Axis axis, uint16_t dur_ms) {
    CHECK_STATUS(Validate_Enum(axis, BNO_AXIS_X, BNO_AXIS_Y));

    //validate dur_ms is between 2.5ms and 640ms
    if (((float) dur_ms) < 2.5 || dur_ms > 640U) {
        return INVALID_PARAM;
    }

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //select appropriate axis base address
    uint8_t axis_base_adr = 0U;
    if (axis == BNO_AXIS_X) {
        axis_base_adr = BNO_GYR_DUR_X_REG;
    } else if (axis == BNO_AXIS_Y) {
        axis_base_adr = BNO_GYR_DUR_Y_REG;
    } else {
        axis_base_adr = BNO_GYR_DUR_Z_REG;
    }

    //configure duration
    uint8_t duration = ((dur_ms / 2.5) - 1U);
    CHECK_STATUS(BNO_Write_Reg(usart, axis_base_adr, 1U, &duration));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the gyr high rate interrupt duration
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  dur_ms: Pointer to a variable used to store the retrieved duration (in ms)
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GYR_HR_Dur(USART_Config_t *usart, BNO_Axis axis, uint16_t *dur_ms) {
    CHECK_STATUS(Validate_Enum(axis, BNO_AXIS_X, BNO_AXIS_Y));
    CHECK_STATUS(Validate_Ptr(dur_ms));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //select appropriate axis base address
    uint8_t axis_base_adr = 0U;
    if (axis == BNO_AXIS_X) {
        axis_base_adr = BNO_GYR_DUR_X_REG;
    } else if (axis == BNO_AXIS_Y) {
        axis_base_adr = BNO_GYR_DUR_Y_REG;
    } else {
        axis_base_adr = BNO_GYR_DUR_Z_REG;
    }

    //read raw high rate duration, convert and store high rate
    uint8_t raw_dur = 0U;
    uint8_t mask    = (0xFFU);
    CHECK_STATUS(BNO_Get_Setting(usart, axis_base_adr, mask, &raw_dur));
    *dur_ms = (uint16_t) ((raw_dur + 1U) * 2.5);

    return SUCCESS;
}

/**
 * @brief  Sets the gyr high rate interrupt filter
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  filter: Filter to configure
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_GYR_HR_Filter(USART_Config_t *usart, BNO_GYR_Filter filter) {
    CHECK_STATUS(Validate_Enum(filter, BNO_GYR_FILTERED, BNO_GYR_UNFILTERED));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));
    
    //configure filter
    uint8_t mask        = (0x00U);
    uint8_t setting_val = (((uint8_t) filter) << BNO_GYR_INT_SETTINGS_HR_FILTER_Pos);
    CHECK_STATUS(BNO_Set_Setting(usart, BNO_GYR_INT_SETTINGS_REG, mask, setting_val));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the gyr high rate interrupt filter
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  filter: Pointer to a variable used to store the retrieved filter
 * @retval Status indicating success, invalid parameters or error
 * @note   If filter = 0, filter is enabled; otherwise unfiltered
 */
Status BNO_Get_GYR_HR_Filter(USART_Config_t *usart, uint8_t *filter) {
    CHECK_STATUS(Validate_Ptr(filter));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read filter setting
    CHECK_STATUS(BNO_Get_Setting(
        usart, 
        BNO_GYR_INT_SETTINGS_REG, 
        BNO_GYR_INT_SETTINGS_HR_FILTER, 
        filter
    ));
    *filter = (*filter >> BNO_GYR_INT_SETTINGS_HR_FILTER_Pos);

    return SUCCESS;
}

/**
 * @brief  Sets the enable state of a specified axis monitored for a gyr high rate interrupt
 * @param  usart: Pointer to a struct containing USART settings
 * @param  axis:  Target axis to configure (X/Y/Z)
 * @param  state: Enable state to configure
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_GYR_HR_Axis_State(USART_Config_t *usart, BNO_Axis axis, BNO_IRQ_Axis_State state) {
    CHECK_STATUS(Validate_Enum(state, BNO_IRQ_AXIS_DISABLED, BNO_IRQ_AXIS_ENABLED));

    uint8_t mask     = (0x01U << (BNO_GYR_INT_SETTINGS_HR_X_AXIS_Pos + axis));
    uint8_t axis_val = (((uint8_t) state) << (BNO_GYR_INT_SETTINGS_HR_X_AXIS_Pos + axis));
    CHECK_STATUS(BNO_Set_Axis_State(usart, axis, BNO_GYR_INT_SETTINGS_REG, mask, axis_val));

    return SUCCESS;
}

/**
 * @brief  Gets the enable state of a specified axis monitored for a gyr high rate interrupt
 * @param  usart: Pointer to a struct containing USART settings
 * @param  axis:  Target axis to query (X/Y/Z)
 * @param  state: Pointer to a variable used to store the retrieved axis state
 * @retval Status indicating success, invalid parameters or error
 * @note   If state = 0, axis is disabled; otherwise axis is enabled
 */
Status BNO_Get_GYR_HR_Axis_State(USART_Config_t *usart, BNO_Axis axis, uint8_t *state) {
    uint8_t mask = (0x01U << (3U + axis));
    CHECK_STATUS(BNO_Get_Axis_State(usart, axis, BNO_GYR_INT_SETTINGS_REG, mask, state));

    return SUCCESS;
}

/**
 * @brief  Configures the gyr any motion interrupt
 * @param  usart:     Pointer to a struct containing USART settings
 * @param  am_config: Pointer to a struct containing config settings
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_GYR_AM_Config(USART_Config_t *usart, BNO_GYR_AM_Config_t *am_config) {
    CHECK_STATUS(Validate_Ptr(am_config));

    //enable gyr any motion interrupts
    CHECK_STATUS(BNO_Enable_IRQ(usart, BNO_IRQ_GYR_AM));

    //configure threshold, filter, awake duration, and slope samples
    CHECK_STATUS(BNO_Set_GYR_AM_Thres     (usart, am_config->thres));
    CHECK_STATUS(BNO_Set_GYR_AM_Filter    (usart, am_config->filter));
    CHECK_STATUS(BNO_Set_GYR_AM_Awake_Dur (usart, am_config->awake_dur));
    CHECK_STATUS(BNO_Set_GYR_AM_Slpe_Samps(usart, am_config->samples));

    //enable selected axis/axes
    if (am_config->x_axis) {
        CHECK_STATUS(BNO_Set_GYR_AM_Axis_State(usart, BNO_AXIS_X, BNO_IRQ_AXIS_ENABLED));
    }
    if (am_config->y_axis) {
        CHECK_STATUS(BNO_Set_GYR_AM_Axis_State(usart, BNO_AXIS_Y, BNO_IRQ_AXIS_ENABLED));
    }
    if (am_config->z_axis) {
        CHECK_STATUS(BNO_Set_GYR_AM_Axis_State(usart, BNO_AXIS_Z, BNO_IRQ_AXIS_ENABLED));
    }

    return SUCCESS;
}

/**
 * @brief  Sets the gyr any motion interrupt threshold
 * @param  usart:     Pointer to a struct containing USART settings
 * @param  thres_dps: Threshold (in dps) to configure
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_GYR_AM_Thres(USART_Config_t *usart, float thres_dps) {
    //validate thres_dps based on configured gyr range
    uint8_t gyr_range = 0U;
    CHECK_STATUS(BNO_Get_GYR_Range(usart, &gyr_range));
    float max_thres_dps[] = {125.0f, 62.5f, 31.25f, 15.625f, 7.8125f};
    if (thres_dps > max_thres_dps[gyr_range] || thres_dps > 0.0f) {
        return INVALID_PARAM;
    }

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //configure threshold
    float gyr_am_lsb_vals[] = {1.0f, 0.5f, 0.25f, 0.125f, 0.0625f};
    float gyr_am_lsb_sel    = gyr_am_lsb_vals[gyr_range];
    uint8_t thres = (uint8_t) (thres_dps / gyr_am_lsb_sel);
    CHECK_STATUS(BNO_Write_Reg(usart, BNO_GYR_AM_THRES_REG, 1U, &thres));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the gyr any motion interrupt threshold
 * @param  usart:     Pointer to a struct containing USART settings
 * @param  thres_dps: Pointer to a variable used to store the retrieved threshold (in dps)
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GYR_AM_Thres(USART_Config_t *usart, float *thres_dps) {
    CHECK_STATUS(Validate_Ptr(thres_dps));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read raw any motion threshold
    uint8_t raw_thres = 0U;
    CHECK_STATUS(BNO_Get_Setting(usart, BNO_GYR_AM_THRES_REG, BNO_GYR_AM_THRES, &raw_thres));

    //convert and store any motion threshold
    uint8_t gyr_range = 0U;
    CHECK_STATUS(BNO_Get_GYR_Range(usart, &gyr_range));
    float gyr_am_lsb_vals[] = {1.0f, 0.5f, 0.25f, 0.125f, 0.0625f};
    float gyr_am_lsb_sel    = gyr_am_lsb_vals[gyr_range];
    *thres_dps = (((float) raw_thres) * gyr_am_lsb_sel);

    return SUCCESS;
}

/**
 * @brief  Sets the gyr any motion interrupt slope samples
 * @param  usart:   Pointer to a struct containing USART settings
 * @param  samples: Slope samples to configure
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_GYR_AM_Slpe_Samps(USART_Config_t *usart, uint8_t samples) {
    //validate slope samples is below max
    uint8_t max_samples = 16U;
    if (samples > max_samples) {
        return INVALID_PARAM;
    }

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //configure slope samples
    uint8_t n_samples = (uint8_t) ((samples / 4U) - 1);
    CHECK_STATUS(BNO_Set_Setting(
        usart, 
        BNO_GYR_AM_SET_REG, 
        BNO_GYR_AM_SET_SLPE_SAMPLES, 
        n_samples
    ));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the gyr any motion interrupt slope samples
 * @param  usart:   Pointer to a struct containing USART settings
 * @param  samples: Pointer to a variable used to store the retrieved slope samples
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GYR_AM_Slpe_Samps(USART_Config_t *usart, uint8_t *samples) {
    CHECK_STATUS(Validate_Ptr(samples));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read raw slope samples, then convert and store any motion slope samples
    uint8_t raw_n_samples = 0U;
    CHECK_STATUS(BNO_Get_Setting(
        usart, 
        BNO_GYR_AM_SET_REG, 
        BNO_GYR_AM_SET_SLPE_SAMPLES, 
        &raw_n_samples
    ));
    *samples = ((raw_n_samples + 1U) / 4U);

    return SUCCESS;
}

/**
 * @brief  Sets the gyr any motion interrupt awake duration
 * @param  usart:     Pointer to a struct containing USART settings
 * @param  awake_dur: Awake duration to configure
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_GYR_AM_Awake_Dur(USART_Config_t *usart, BNO_GYR_Awake_Dur awake_dur) {
    CHECK_STATUS(Validate_Enum(awake_dur, BNO_GYR_AWAKE_DUR_8_SAMPS, BNO_GYR_AWAKE_DUR_64_SAMPS));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //configure awake duration
    uint8_t setting_val = (awake_dur << BNO_GYR_AM_SET_AWAKE_DUR_Pos);
    CHECK_STATUS(BNO_Set_Setting(
        usart, 
        BNO_GYR_AM_SET_REG, 
        BNO_GYR_AM_SET_AWAKE_DUR, 
        setting_val
    ));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the gyr any motion interrupt awake duration
 * @param  usart:     Pointer to a struct containing USART settings
 * @param  awake_dur: Pointer to a variable used to store the retrieved awake duration
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Get_GYR_AM_Awake_Dur(USART_Config_t *usart, uint8_t *awake_dur) {
    CHECK_STATUS(Validate_Ptr(awake_dur));
    
    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read and store awake duration
    CHECK_STATUS(BNO_Get_Setting(
        usart,
        BNO_GYR_AM_SET_REG,
        BNO_GYR_AM_SET_AWAKE_DUR,
        awake_dur
    ));
    *awake_dur = (*awake_dur >> BNO_GYR_AM_SET_AWAKE_DUR_Pos);

    return SUCCESS;
}

/**
 * @brief  Sets the gyr any motion interrupt filter
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  filter: Filter to configure
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_GYR_AM_Filter(USART_Config_t *usart, BNO_GYR_Filter filter) {
    CHECK_STATUS(Validate_Enum(filter, BNO_GYR_FILTERED, BNO_GYR_UNFILTERED));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //store current operating mode and switch to CONFIG_MODE
    uint8_t current_opr_mode = 0U;
    CHECK_STATUS(BNO_Set_Config_Mode(usart, &current_opr_mode));

    //configure filter
    uint8_t mask        = (0x00U);
    uint8_t setting_val = (uint8_t) (filter << BNO_GYR_INT_SETTINGS_AM_FILTER_Pos);
    CHECK_STATUS(BNO_Set_Setting(usart, BNO_GYR_INT_SETTINGS_REG, mask, setting_val));

    //restore previous operating mode
    CHECK_STATUS(BNO_Set_OPR_Mode(usart, current_opr_mode));

    return SUCCESS;
}

/**
 * @brief  Gets the gyr any motion interrupt filter
 * @param  usart:  Pointer to a struct containing USART settings
 * @param  filter: Pointer to a variable used to store the retrieved filter
 * @retval Status indicating success, invalid parameters or error
 * @note   If filter = 0, filter is enabled; otherwise unfiltered
 */
Status BNO_Get_GYR_AM_Filter(USART_Config_t *usart, uint8_t *filter) {
    CHECK_STATUS(Validate_Ptr(filter));

    CHECK_STATUS(BNO_Select_Page(usart, BNO_PAGE_1));

    //read filter settings
    CHECK_STATUS(BNO_Get_Setting(
        usart,
        BNO_GYR_INT_SETTINGS_REG,
        BNO_GYR_INT_SETTINGS_AM_FILTER,
        filter
    ));
    *filter = (*filter >> BNO_GYR_INT_SETTINGS_AM_FILTER_Pos);

    return SUCCESS;
}

/**
 * @brief  Sets the enable state of a specified axis monitored for a gyr any motion interrupt
 * @param  usart: Pointer to a struct containing USART settings
 * @param  axis:  Target axis to configure (X/Y/Z)
 * @param  state: Enable state to configure
 * @retval Status indicating success, invalid parameters or error
 */
Status BNO_Set_GYR_AM_Axis_State(USART_Config_t *usart, BNO_Axis axis, BNO_IRQ_Axis_State state) {
    CHECK_STATUS(Validate_Enum(state, BNO_IRQ_AXIS_DISABLED, BNO_IRQ_AXIS_ENABLED));

    uint8_t mask     = (0x01U << (BNO_GYR_INT_SETTINGS_AM_X_AXIS_Pos + axis));
    uint8_t axis_val = (((uint8_t) state) << (BNO_GYR_INT_SETTINGS_AM_X_AXIS_Pos + axis));
    CHECK_STATUS(BNO_Set_Axis_State(usart, axis, BNO_GYR_INT_SETTINGS_REG, mask, axis_val));

    return SUCCESS;
}

/**
 * @brief  Gets the enable state of a specified axis monitored for a gyr any motion interrupt
 * @param  usart: Pointer to a struct containing USART settings
 * @param  axis:  Target axis to query (X/Y/Z)
 * @param  state: Pointer to a variable used to store the retrieved axis state
 * @retval Status indicating success, invalid parameters or error
 * @note   If state = 0, axis is disabled; otherwise axis is enabled
 */
Status BNO_Get_GYR_AM_Axis_State(USART_Config_t *usart, BNO_Axis axis, uint8_t *state) {
    uint8_t mask = (0x01U << axis);
    CHECK_STATUS(BNO_Get_Axis_State(usart, axis, BNO_GYR_INT_SETTINGS_REG, mask, state));

    return SUCCESS;
}
