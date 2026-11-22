#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/ledc.h"

#include "motor.h"
#include "motor_driver.h"
#include "car_motor_controller.h"

static const char *TAG = "app_main";

static car_motor_controller_t car;
extern void car_motor_controller_update_task(void *pvParameters);

static void sweep_demo_task(void *pvParameters) {
	//car_motor_controller_t *car = (car_motor_controller_t *)pvParameters;

	for(;;) {
		car_motor_controller_forward(&car, 100);
		vTaskDelay(5000 / portTICK_PERIOD_MS);

		car_motor_controller_forward(&car, 0);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
		car_motor_controller_forward(&car, 100);
		vTaskDelay(5000 / portTICK_PERIOD_MS);

		car_motor_controller_stop(&car);
		vTaskDelay(5000 / portTICK_PERIOD_MS);

		car_motor_controller_reverse(&car, 100);
		vTaskDelay(5000 / portTICK_PERIOD_MS);

		car_motor_controller_short_brake(&car);
		vTaskDelay(2000 / portTICK_PERIOD_MS);

		car_motor_controller_turn_right(&car, 100);
		vTaskDelay(5000 / portTICK_PERIOD_MS);

		car_motor_controller_turn_left(&car, 100);
		vTaskDelay(5000 / portTICK_PERIOD_MS);

		car_motor_controller_standby(&car);
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}

void app_main(void) {
	ESP_ERROR_CHECK(car_motor_controller_init(&car));

	ESP_LOGI(TAG, "Init car update routine");
	xTaskCreate(car_motor_controller_update_task, "car_motor_controller_update_task", 2048, &car, 10, NULL);

	ESP_LOGI(TAG, "Init sweep demo task");
	xTaskCreate(sweep_demo_task, "sweep_demo_task", 2048, NULL, 10, NULL);
}
