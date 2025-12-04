/**
 * @file    main.c
 * @brief   Basic BNO055 sensor reading application via USART for the STM32F411
 * @details Reads sensor values from the BNO055 IMU and sends them to a terminal emulator using
 *          USART via a Serial-to-USB converter.
 * 
 *          The circuit layout is:
 *          - Pin A9 (USART1 TX) connects to FT232 RXD
 *          - Pin A10 (USART1 RX) connects to FT232 TXD
 *          - Pin A2 (USART2 TX) connects to BNO055 SCL
 *          - Pin A3 (USART2 RX) connects to BNO055 SDA
 *          - PS0 is connected to GND
 *          - PS1 is connected to 3.3/5V
 * 
 * @note    The local .bin path is:
 *          ~/projects/bno_basic/.pio/build/blackpill_f411ce/firmware.bin
 */


#include <string.h>
#include "main.h"


int main(void) {
    //reset all peripherals
    Peripheral_Reset();

    //configure system clock
    CHECK_STATUS(Sys_Clock_Init(HSI_CLOCK));

    //configure systick time-base
    CHECK_STATUS(Systick_Init(SYSTICK_UNIT_MSEC));

    GPIO_Reset_Pin(GPIOC, GPIO_PIN_13);

    //configure GPIO for USART1 TX
    GPIO_Config_t term_tx_config = {
        .port         = GPIOA,
        .pin          = GPIO_PIN_9,
        .mode         = GPIO_MODE_AF,
        .alt_function = GPIO_AF_7,
        .output_speed = GPIO_OSPEED_HIGH,
        .output_type  = GPIO_OTYPE_PUSH_PULL
    };
    CHECK_STATUS(GPIO_Init(&term_tx_config));

    //configure GPIO for USART1 RX
    GPIO_Config_t term_rx_config = {
        .port         = GPIOA,
        .pin          = GPIO_PIN_10,
        .mode         = GPIO_MODE_AF,
        .alt_function = GPIO_AF_7,
        .pupd         = GPIO_PUPD_NO
    };
    CHECK_STATUS(GPIO_Init(&term_rx_config));

    //configure GPIO for USART2 TX
    GPIO_Config_t bno_tx_config = {
        .port         = GPIOA,
        .pin          = GPIO_PIN_2,
        .mode         = GPIO_MODE_AF,
        .alt_function = GPIO_AF_7,
        .output_speed = GPIO_OSPEED_HIGH,
        .output_type  = GPIO_OTYPE_PUSH_PULL
    };
    CHECK_STATUS(GPIO_Init(&bno_tx_config));

    //configure GPIO for USART2 RX
    GPIO_Config_t bno_rx_config = {
        .port         = GPIOA,
        .pin          = GPIO_PIN_3,
        .mode         = GPIO_MODE_AF,
        .alt_function = GPIO_AF_7,
        .pupd         = GPIO_PUPD_PULLUP
    };
    CHECK_STATUS(GPIO_Init(&bno_rx_config));

    //configure USART1 to communicate with the terminal
    USART_Config_t usart_term_config = {
        .instance         = USART1,
        .baud_rate        = 115200,
        .irq_priority     = 1
    };
    CHECK_STATUS(USART_Init(&usart_term_config));

    //configure USART2 to communicate with BNO055
    USART_Config_t usart_bno_config = {
        .instance         = USART2,
        .baud_rate        = 115200,
        .irq_priority     = 2
    };
    CHECK_STATUS(USART_Init(&usart_bno_config));

    Delay_Loop(2000);

    // initialise BNO055
    BNO_Config_t bno_config = {
        .pwr_mode = BNO_PWR_NORMAL_MODE,
        .opr_mode = BNO_OPR_AMG_MODE
    };
    CHECK_STATUS(BNO_Init(&usart_bno_config, &bno_config));

    //check POST result
    uint8_t post_result = 0U;
    CHECK_STATUS(BNO_Get_MCU_POST_Result(&usart_bno_config, &post_result));
    if (post_result == 0U) {
        return ERROR;
    }

    //wait for system to calibrate
    uint8_t sys_calib_status = 0;
    CHECK_STATUS(BNO_Get_Sys_Calib_Status(&usart_bno_config, &sys_calib_status));
    while (sys_calib_status == 0) {
        NOP();
    }

    //read calibration profile
    BNO_Calib_Profile_t calib_profile = {0};
    CHECK_STATUS(BNO_Get_Calib_Profile(&usart_bno_config, &calib_profile));

    //transmit calibration profile to terminal
    CHECK_STATUS(BNO_Transmit_Calib_Profile(&usart_term_config, &calib_profile));

    //for subsequent programs, write the offset values
    // CHECK_STATUS(BNO_Write_Calib_Profile(&usart_bno_config, &calib_profile));

    while (1) {
        //initialise storage variables
        BNO_ODR_Float_t acc_data = {0.0f, 0.0f, 0.0f};
        BNO_ODR_Float_t mag_data = {0.0f, 0.0f, 0.0f};
        BNO_ODR_Float_t gyr_data = {0.0f, 0.0f, 0.0f};
        BNO_ODR_Float_t lia_data = {0.0f, 0.0f, 0.0f};
        BNO_ODR_Float_t grv_data = {0.0f, 0.0f, 0.0f};
        BNO_ODR_Float_t eul_data = {0.0f, 0.0f, 0.0f};
        BNO_QUA_Float_t qua_data = {0.0f, 0.0f, 0.0f, 0.0f};

        //get sensor data
        CHECK_STATUS(BNO_Get_ACC_XYZ(&usart_bno_config, &acc_data));
        CHECK_STATUS(BNO_Get_MAG_XYZ(&usart_bno_config, &mag_data));
        CHECK_STATUS(BNO_Get_GYR_XYZ(&usart_bno_config, &gyr_data));
        CHECK_STATUS(BNO_Get_LIA_XYZ(&usart_bno_config, &lia_data));
        CHECK_STATUS(BNO_Get_GRV_XYZ(&usart_bno_config, &grv_data));
        CHECK_STATUS(BNO_Get_EUL_HRP(&usart_bno_config, &eul_data));
        CHECK_STATUS(BNO_Get_QUA_WXYZ(&usart_bno_config, &qua_data));

        // compose message
        uint8_t data_read_msg[TX_BUFFER_SIZE] = {0};
        snprintf(
            (char *) data_read_msg, sizeof(data_read_msg), 
            "ACC -> %8.4f | %8.4f | %8.4f\n\r"
            "MAG -> %8.4f | %8.4f | %8.4f\n\r"
            "GYR -> %8.4f | %8.4f | %8.4f\n\r"
            "LIA -> %8.4f | %8.4f | %8.4f\n\r"
            "GRV -> %8.4f | %8.4f | %8.4f\n\r"
            "EUL -> %8.4f | %8.4f | %8.4f\n\r"
            "QUA -> %8.4f | %8.4f | %8.4f | %8.4f\n\n\r", 
            acc_data.x_float, acc_data.y_float, acc_data.z_float,
            mag_data.x_float, mag_data.y_float, mag_data.z_float,
            gyr_data.x_float, gyr_data.y_float, gyr_data.z_float,
            lia_data.x_float, lia_data.y_float, lia_data.z_float,
            grv_data.x_float, grv_data.y_float, grv_data.z_float,
            eul_data.x_float, eul_data.y_float, eul_data.z_float,
            qua_data.w_float, qua_data.x_float, qua_data.y_float, qua_data.z_float
        );

        //transmit message
        CHECK_STATUS(
            USART_Transmit_IRQ(&usart_term_config, data_read_msg, strlen((char *) data_read_msg))
        );
        
        Delay_Loop(20);
    }
}
