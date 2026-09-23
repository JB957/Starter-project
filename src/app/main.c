#include "main.h"

#include <stdbool.h>
#include <stdio.h>

#include "can.h"
#include "clock.h"
#include "gpio.h"
#include "adc.h"
#include "error_handler.h"
#include "core_config.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "rtt.h"

#include <stm32g4xx_hal.h>


void heartbeat_task(void *pvParameters) {
    (void) pvParameters;
    while(true) {
        core_GPIO_toggle_heartbeat();
        vTaskDelay(200 * portTICK_PERIOD_MS);
    }
}

void port14_task(void *pvParameters) {
    (void) pvParameters;
    while(true) {
        core_GPIO_digital_write(GPIOB, GPIO_PIN_14, true);
        vTaskDelay(300 * portTICK_PERIOD_MS);
        core_GPIO_digital_write(GPIOB, GPIO_PIN_14, false);
        vTaskDelay(300 * portTICK_PERIOD_MS);
    }
}

void port15_task(void *pvParameters) {
    (void) pvParameters;
    while(true) {
        core_GPIO_digital_write(GPIOB, GPIO_PIN_15, true);
        vTaskDelay(1000 * portTICK_PERIOD_MS);
        core_GPIO_digital_write(GPIOB, GPIO_PIN_15, false);
        vTaskDelay(1000 * portTICK_PERIOD_MS);
    }
}

void Print_Port12(void *pvParameters) {
    (void) pvParameters;
    uint16_t adc_value;
    while(true) {
        if (core_ADC_read_channel(GPIOB, GPIO_PIN_12, &adc_value)) {
            rprintf("PB12 ADC: %u\n", adc_value);
        }
        vTaskDelay(100 * portTICK_PERIOD_MS);
    }
}

int main(void) {
    HAL_Init();

    // Drivers

    core_heartbeat_init(GPIOB, GPIO_PIN_13);
    core_GPIO_init(GPIOB, GPIO_PIN_14, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
    core_GPIO_init(GPIOB, GPIO_PIN_15, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
    core_GPIO_init(GPIOB, GPIO_PIN_11, GPIO_MODE_INPUT, GPIO_NOPULL);
    if (!core_ADC_init(ADC1)) error_handler();
    if (!core_ADC_setup_pin(GPIOB, GPIO_PIN_12, 0)) error_handler();

    core_GPIO_digital_write(GPIOB, GPIO_PIN_13, false);
    core_GPIO_digital_write(GPIOB, GPIO_PIN_14, false);
    core_GPIO_digital_write(GPIOB, GPIO_PIN_15, false);
            
    
    if (!core_clock_init()) error_handler();
    if (!core_CAN_init(FDCAN1, 1000000)) error_handler();

    while (1) {
    if (core_GPIO_digital_read(GPIOB, GPIO_PIN_11)) {
        break;   // start action here
        }
    }

    int err = xTaskCreate(heartbeat_task, "heartbeat", 1000, NULL, 4, NULL);
    if (err != pdPASS) {
        error_handler();
    }

    int err2 = xTaskCreate(port14_task, "port14", 1000, NULL, 4, NULL);
    if (err2 != pdPASS) {
        error_handler();
    }

    int err3 = xTaskCreate(port15_task, "port15", 1000, NULL, 4, NULL);
    if (err3 != pdPASS) {
        error_handler();
    }

    int err4 = xTaskCreate(Print_Port12, "adc_pb12", 1000, NULL, 4, NULL);
    if (err4 != pdPASS) {
        error_handler();
    }


    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    // hand control over to FreeRTOS
    vTaskStartScheduler();

    // we should not get here ever
    error_handler();
    return 1;
}

// Called when stack overflows from rtos
// Not needed in header, since included in FreeRTOS-Kernel/include/task.h
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName) {
    (void) xTask;
    (void) pcTaskName;

    error_handler();
}
