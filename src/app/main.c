#include "main.h"

#include <stdbool.h>
#include <stdio.h>

#include "can.h"
#include "clock.h"
#include "gpio.h"
#include "adc.h"
#include "error_handler.h"
#include "core_config.h"
#include "spi.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "rtt.h"

#include <stm32g4xx_hal.h>

#define SPI_Enable_Write 0x06
#define SPI_Disable_Write 0x04
#define SPI_Write 0x02
#define SPI_Read 0x03
#define SPI_Read_Register 0x05
#define SPI_Write_Register 0x01
#define Start_addy 0x00

/* Function prototypes */
uint8_t Get_NextAddress(void);
void EnableWrite(void);
void DisableWrite(void);
void WriteSPI(uint8_t address, uint8_t data);
void Increase_NextAddress(void);
void Read_To_Free(void *pvParameters);
void ReadRegister(void);

// Blinky Light Code

void heartbeat_task(void *pvParameters)
{
    (void)pvParameters;
    while (true)
    {
        core_GPIO_toggle_heartbeat();
        vTaskDelay(200 * portTICK_PERIOD_MS);
    }
}

void port14_task(void *pvParameters)
{
    (void)pvParameters;
    while (true)
    {
        core_GPIO_digital_write(GPIOB, GPIO_PIN_14, true);
        vTaskDelay(300 * portTICK_PERIOD_MS);
        core_GPIO_digital_write(GPIOB, GPIO_PIN_14, false);
        vTaskDelay(300 * portTICK_PERIOD_MS);
    }
}

void port15_task(void *pvParameters)
{
    (void)pvParameters;
    while (true)
    {
        core_GPIO_digital_write(GPIOB, GPIO_PIN_15, true);
        vTaskDelay(1000 * portTICK_PERIOD_MS);
        core_GPIO_digital_write(GPIOB, GPIO_PIN_15, false);
        vTaskDelay(1000 * portTICK_PERIOD_MS);
    }
}

void Print_Port12(void *pvParameters)
{
    (void)pvParameters;
    uint16_t adc_value;
    while (true)
    {
        if (core_ADC_read_channel(GPIOB, GPIO_PIN_12, &adc_value))
        {
            rprintf("PB12 ADC: %u\n", adc_value);
        }
        vTaskDelay(100 * portTICK_PERIOD_MS);
    }
}

void lightToFreq(void *pvParameters)
{
    (void)pvParameters;
    uint16_t adc_value;
    while (true)
    {
        core_ADC_read_channel(GPIOB, GPIO_PIN_12, &adc_value);
        if (adc_value <= 2000 && adc_value >= 0)
        {
            core_GPIO_digital_write(GPIOB, GPIO_PIN_15, true);
            core_GPIO_digital_write(GPIOB, GPIO_PIN_14, true);
            core_GPIO_digital_write(GPIOB, GPIO_PIN_13, true);
            vTaskDelay((100 * 10) * portTICK_PERIOD_MS);
            core_GPIO_digital_write(GPIOB, GPIO_PIN_14, false);
            core_GPIO_digital_write(GPIOB, GPIO_PIN_13, false);
            core_GPIO_digital_write(GPIOB, GPIO_PIN_15, false);
            vTaskDelay((100 * 10) * portTICK_PERIOD_MS);
        }
    }
}

// Can Code
void SendCanMessage(void *pvParameters)
{
    (void)pvParameters;
    while (true)
    {
        core_CAN_send_from_tx_queue_task(FDCAN1);
    }
}

void AddMessage()
{
    core_CAN_add_message_to_tx_queue(FDCAN1, 0x123, 1, 0xFF);
}

// // ########################## SPI Code #######################

void Read_To_Free(void *pvParameters)
{
    (void)pvParameters;
    while (true)
    {
        // WriteSPI(Get_NextAddress(), 0x07);
        // // wait(1)
        // // WriteSPI(Get_NextAddress(), 0xAF);
        // // WriteSPI(Get_NextAddress(), 0xBC);
        // for (uint8_t address = 0x00; address < Free_Address; address += 3)
        // {
        //     uint8_t read_tx[] = {SPI_Read, address, 0x00, 0x00, 0x00};
        //     uint8_t rx[5] = {0};
        //     core_SPI_start(SPI1);
        //     core_SPI_read_write(SPI1, read_tx, 5, rx, 5);
        //     core_SPI_stop(SPI1);
        //     rprintf("RX:%02X %02X %02X\n", rx[2], rx[3], rx[4]);
        //     vTaskDelay(pdMS_TO_TICKS(1000));
        // }

        uint8_t tx[] = {SPI_Write, 0x00, 0x1F, 0xAB, 0xCD};
        EnableWrite();
        core_SPI_start(SPI1);
        core_SPI_read_write(SPI1, tx, 5, NULL, 0);
        core_SPI_stop(SPI1);
        DisableWrite();
        vTaskDelay(pdMS_TO_TICKS(10));
        uint8_t read_tx[] = {SPI_Read, 0x00, 0x00, 0x00, 0x00};
        uint8_t rx[5] = {0};
        core_SPI_start(SPI1);
        core_SPI_read_write(SPI1, read_tx, 5, rx, 5);
        core_SPI_stop(SPI1);
        rprintf("RX:%02X %02X %02X\n", rx[2], rx[3], rx[4]);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// void WriteSPI(uint8_t address, uint8_t data)
// {
//     uint8_t tx[] = {SPI_Write, address, data};
//     EnableWrite();
//     core_SPI_start(SPI1);
//     core_SPI_read_write(SPI1, tx, 3, NULL, 0);
//     core_SPI_stop(SPI1);
//     DisableWrite();
//     if (address == Get_NextAddress())
//     {
//         Increase_NextAddress();
//     }
// }

// void Increase_NextAddress(void)
// {
//     uint8_t address = Get_NextAddress();
//     address++;
//     WriteSPI(Start_addy, address);
// }

// uint8_t Get_NextAddress(void)
// {
//     uint8_t tx[] = {SPI_Read, Start_addy, 0x00};
//     uint8_t rx[3] = {0};
//     core_SPI_start(SPI1);
//     core_SPI_read_write(SPI1, tx, 3, rx, 3);
//     core_SPI_stop(SPI1);
//     if (rx[2] == 0xFF || rx[2] == 0x8F)
//     {
//         WriteSPI(Start_addy, 0x01);
//         return 0x01;
//     }
//     return rx[2];
// }

void EnableWrite(void)
{
    uint8_t enable[] = {SPI_Enable_Write}; // WREN
    core_SPI_start(SPI1);
    core_SPI_read_write(SPI1, enable, 1, NULL, 0);
    core_SPI_stop(SPI1);
}

void DisableWrite(void)
{
    uint8_t disable[] = {SPI_Disable_Write};
    core_SPI_start(SPI1);
    core_SPI_read_write(SPI1, disable, 1, NULL, 0);
    core_SPI_stop(SPI1);
}

//  Main Duh

int main(void)
{
    HAL_Init();

    // Drivers
    core_heartbeat_init(GPIOB, GPIO_PIN_13);
    core_GPIO_init(GPIOB, GPIO_PIN_14, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
    core_GPIO_init(GPIOB, GPIO_PIN_15, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
    core_GPIO_init(GPIOB, GPIO_PIN_11, GPIO_MODE_INPUT, GPIO_NOPULL);
    if (!core_CAN_init(FDCAN1, 1000000))
    {
        error_handler();
    }

    if (!core_SPI_init(SPI1, GPIOA, GPIO_PIN_4))
        error_handler();
    if (!core_ADC_init(ADC1))
        error_handler();
    if (!core_ADC_setup_pin(GPIOB, GPIO_PIN_12, 0))
        error_handler();

    core_GPIO_digital_write(GPIOB, GPIO_PIN_13, false);
    core_GPIO_digital_write(GPIOB, GPIO_PIN_14, false);
    core_GPIO_digital_write(GPIOB, GPIO_PIN_15, false);

    if (!core_clock_init())
        error_handler();
    if (!core_CAN_init(FDCAN1, 1000000))
        error_handler();

    // int err6 = xTaskCreate(SendCanMessage, "can", 1000, NULL, 4, NULL);
    // if (err6 != pdPASS)
    // {
    //     error_handler();
    // }

    int err7 = xTaskCreate(Read_To_Free, "SendSPI", 1000, NULL, 4, NULL);
    if (err7 != pdPASS)
    {
        error_handler();
    }

    //  Light Code
    // while (1)
    // {
    //     if (core_GPIO_digital_read(GPIOB, GPIO_PIN_11))
    //     {
    //         break; // start action here
    //     }
    // }

    // int err = xTaskCreate(heartbeat_task, "heartbeat", 1000, NULL, 4, NULL);
    // if (err != pdPASS)
    // {
    //     error_handler();
    // }

    // int err2 = xTaskCreate(port14_task, "port14", 1000, NULL, 4, NULL);
    // if (err2 != pdPASS)
    // {
    //     error_handler();
    // }

    // int err3 = xTaskCreate(port15_task, "port15", 1000, NULL, 4, NULL);
    // if (err3 != pdPASS)
    // {
    //     error_handler();
    // }

    // int err4 = xTaskCreate(Print_Port12, "adc_pb12", 1000, NULL, 4, NULL);
    // if (err4 != pdPASS)
    // {
    //     error_handler();
    // }

    // int err5 = xTaskCreate(lightToFreq, "light_to_freq", 1000, NULL, 4, NULL);
    // if (err5 != pdPASS)
    // {
    //     error_handler();
    // }

    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    // hand control over to FreeRTOS
    vTaskStartScheduler();

    // we should not get here ever
    error_handler();
    return 1;
}

// Called when stack overflows from rtos
// Not needed in header, since included in FreeRTOS-Kernel/include/task.h
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;

    error_handler();
}
