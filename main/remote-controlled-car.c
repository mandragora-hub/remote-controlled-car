#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/ledc.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_8_BIT 
#define LEDC_MAX_DUTY_CYCLE			(255) // (2 ** duty_resolution)
#define LEDC_CLK_SRC            LEDC_AUTO_CLK
#define LEDC_FREQUENCY          (20000) // TODO: verify what frequency should be more suitable based on battery consumption

#define MOTOR_VELOCITY							5


#define REAR_MOTOR_CHANNEL_APWM			GPIO_NUM_2
#define REAR_MOTOR_CHANNEL_AIN1			GPIO_NUM_13
#define REAR_MOTOR_CHANNEL_AIN2			GPIO_NUM_12
#define REAR_MOTOR_STBY							GPIO_NUM_17
#define REAR_MOTOR_CHANNEL_BPWM			GPIO_NUM_5
#define REAR_MOTOR_CHANNEL_BIN1			GPIO_NUM_14
#define REAR_MOTOR_CHANNEL_BIN2			GPIO_NUM_27
#define FRONT_MOTOR_CHANNEL_APWM		GPIO_NUM_19
#define FRONT_MOTOR_CHANNEL_AIN1		GPIO_NUM_32
#define FRONT_MOTOR_CHANNEL_AIN2		GPIO_NUM_33
#define FRONT_MOTOR_STBY 						GPIO_NUM_22
#define FRONT_MOTOR_CHANNEL_BPWM		GPIO_NUM_18
#define FRONT_MOTOR_CHANNEL_BIN1		GPIO_NUM_25
#define FRONT_MOTOR_CHANNEL_BIN2		GPIO_NUM_26

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<REAR_MOTOR_CHANNEL_AIN1) \
		| (1ULL<<REAR_MOTOR_CHANNEL_AIN2) \
		| (1ULL<<REAR_MOTOR_STBY) \
		| (1ULL<<REAR_MOTOR_CHANNEL_BIN1) \
		| (1ULL<<REAR_MOTOR_CHANNEL_BIN2) \
		| (1ULL<<FRONT_MOTOR_CHANNEL_AIN1) \
		| (1ULL<<FRONT_MOTOR_CHANNEL_AIN2) \
		| (1ULL<<FRONT_MOTOR_STBY) \
		| (1ULL<<FRONT_MOTOR_CHANNEL_BIN1) \
		| (1ULL<<FRONT_MOTOR_CHANNEL_BPWM) \
		| (1ULL<<REAR_MOTOR_CHANNEL_BPWM) \
		| (1ULL<<REAR_MOTOR_CHANNEL_APWM) \
		| (1ULL<<FRONT_MOTOR_CHANNEL_BIN2))


static const char *TAG = "app_main";
typedef uint8_t percentage_t;

static uint8_t percentage_to_duty_cycle(percentage_t percentage) {
	if (percentage >= 100) return LEDC_MAX_DUTY_CYCLE;
	//if (percentage <= 0) return 0

	return (LEDC_MAX_DUTY_CYCLE / 100) * percentage;
}

static void motor_driver_set_standby_mode() {
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_STBY, 0));
	ESP_ERROR_CHECK(gpio_set_level(FRONT_MOTOR_STBY, 0));
}

static void motor_driver_set_active_mode() {
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_STBY, 1));
	ESP_ERROR_CHECK(gpio_set_level(FRONT_MOTOR_STBY, 1));
}

static void motor_update_pwm_duty_cycle(uint8_t pwm_duty_cycle) {
	ESP_LOGI(TAG, "Update pwm duty cycle: %u", pwm_duty_cycle);

	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, pwm_duty_cycle));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}

static void motor_move_foward() {
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_CHANNEL_AIN1, 1));
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_CHANNEL_AIN2, 0));
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_CHANNEL_BIN1, 1));
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_CHANNEL_BIN2, 0));
	ESP_ERROR_CHECK(gpio_set_level(FRONT_MOTOR_CHANNEL_AIN1, 1));
	ESP_ERROR_CHECK(gpio_set_level(FRONT_MOTOR_CHANNEL_AIN2, 0));
	ESP_ERROR_CHECK(gpio_set_level(FRONT_MOTOR_CHANNEL_BIN1, 1));
	ESP_ERROR_CHECK(gpio_set_level(FRONT_MOTOR_CHANNEL_BIN2, 0));
}

static void motor_move_reverse() {
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_CHANNEL_AIN1, 0));
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_CHANNEL_AIN2, 1));
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_CHANNEL_BIN1, 0));
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_CHANNEL_BIN2, 1));
	ESP_ERROR_CHECK(gpio_set_level(FRONT_MOTOR_CHANNEL_AIN1, 0));
	ESP_ERROR_CHECK(gpio_set_level(FRONT_MOTOR_CHANNEL_AIN2, 1));
	ESP_ERROR_CHECK(gpio_set_level(FRONT_MOTOR_CHANNEL_BIN1, 0));
	ESP_ERROR_CHECK(gpio_set_level(FRONT_MOTOR_CHANNEL_BIN2, 1));
}

static void motor_move_stop() {
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_CHANNEL_AIN1, 0));
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_CHANNEL_AIN2, 0));
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_CHANNEL_BIN1, 0));
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_CHANNEL_BIN2, 0));
	ESP_ERROR_CHECK(gpio_set_level(FRONT_MOTOR_CHANNEL_AIN1, 0));
	ESP_ERROR_CHECK(gpio_set_level(FRONT_MOTOR_CHANNEL_AIN2, 0));
	ESP_ERROR_CHECK(gpio_set_level(FRONT_MOTOR_CHANNEL_BIN1, 0));
	ESP_ERROR_CHECK(gpio_set_level(FRONT_MOTOR_CHANNEL_BIN2, 0));
}

static void motor_short_brake() {
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_CHANNEL_AIN1, 1));
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_CHANNEL_AIN2, 1));
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_CHANNEL_BIN1, 1));
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_CHANNEL_BIN2, 1));
	ESP_ERROR_CHECK(gpio_set_level(FRONT_MOTOR_CHANNEL_AIN1, 1));
	ESP_ERROR_CHECK(gpio_set_level(FRONT_MOTOR_CHANNEL_AIN2, 1));
	ESP_ERROR_CHECK(gpio_set_level(FRONT_MOTOR_CHANNEL_BIN1, 1));
	ESP_ERROR_CHECK(gpio_set_level(FRONT_MOTOR_CHANNEL_BIN2, 1));
}

static void sweep_demo_task(void *pvParameters) {

	ESP_ERROR_CHECK(gpio_set_level(FRONT_MOTOR_CHANNEL_BPWM, 1));
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_CHANNEL_APWM, 1));
	ESP_ERROR_CHECK(gpio_set_level(REAR_MOTOR_CHANNEL_BPWM, 1));

	for(;;) {
		motor_driver_set_active_mode();

		ESP_LOGI(TAG, "Moving foward");
		motor_move_foward();

		ESP_LOGI(TAG, "Accelerating");
		motor_update_pwm_duty_cycle(percentage_to_duty_cycle(0));
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		motor_update_pwm_duty_cycle(percentage_to_duty_cycle(25));
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		motor_update_pwm_duty_cycle(percentage_to_duty_cycle(50));
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		motor_update_pwm_duty_cycle(percentage_to_duty_cycle(75));
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		motor_update_pwm_duty_cycle(percentage_to_duty_cycle(100));
		vTaskDelay(5000 / portTICK_PERIOD_MS);

		ESP_LOGI(TAG, "Slow down");
		motor_update_pwm_duty_cycle(percentage_to_duty_cycle(75));
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		motor_update_pwm_duty_cycle(percentage_to_duty_cycle(50));
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		motor_update_pwm_duty_cycle(percentage_to_duty_cycle(25));
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		motor_update_pwm_duty_cycle(percentage_to_duty_cycle(0));
		vTaskDelay(5000 / portTICK_PERIOD_MS);

		ESP_LOGI(TAG, "Stopped");
		motor_move_stop();
		vTaskDelay(2000 / portTICK_PERIOD_MS);

		ESP_LOGI(TAG, "Moving in reverse");
		motor_move_reverse();

		ESP_LOGI(TAG, "Accelerating");
		motor_update_pwm_duty_cycle(percentage_to_duty_cycle(0));
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		motor_update_pwm_duty_cycle(percentage_to_duty_cycle(25));
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		motor_update_pwm_duty_cycle(percentage_to_duty_cycle(50));
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		motor_update_pwm_duty_cycle(percentage_to_duty_cycle(75));
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		motor_update_pwm_duty_cycle(percentage_to_duty_cycle(100));
		vTaskDelay(5000 / portTICK_PERIOD_MS);

		ESP_LOGI(TAG, "Slow down");
		motor_update_pwm_duty_cycle(percentage_to_duty_cycle(75));
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		motor_update_pwm_duty_cycle(percentage_to_duty_cycle(50));
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		motor_update_pwm_duty_cycle(percentage_to_duty_cycle(25));
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		motor_update_pwm_duty_cycle(percentage_to_duty_cycle(0));
		vTaskDelay(5000 / portTICK_PERIOD_MS);

		ESP_LOGI(TAG, "Standby");
		motor_driver_set_standby_mode();
		vTaskDelay(2000 / portTICK_PERIOD_MS);


		//TODO: add short break mode from datasheet

		//TODO: DEFINE PWM pins
		//ESP_LOGI(TAG, "Turning right");
		//ESP_LOGI(TAG, "Turning left");
	}
}

void app_main(void) {

	ESP_LOGI(TAG, "Prepare timer for PWM usage");
	ledc_timer_config_t ledc_timer = {
		.speed_mode       = LEDC_MODE,
		.duty_resolution  = LEDC_DUTY_RES,
		.timer_num        = LEDC_TIMER,
		.freq_hz          = LEDC_FREQUENCY, 
		.clk_cfg          = LEDC_CLK_SRC,
	};
	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

	ESP_LOGI(TAG, "Prepare PWM channel configuration");
	ledc_channel_config_t ledc_channel = {
		.speed_mode     = LEDC_MODE,
		.channel        = LEDC_CHANNEL,
		.timer_sel      = LEDC_TIMER,
		.gpio_num       = FRONT_MOTOR_CHANNEL_APWM,
		.duty           = 0, // Set duty to 0%
		.hpoint         = 0,
	};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

	ESP_LOGI(TAG, "Setting GPIO");
	gpio_config_t io_conf = {
		.intr_type = GPIO_INTR_DISABLE,
		.mode = GPIO_MODE_OUTPUT,
		.pin_bit_mask = GPIO_OUTPUT_PIN_SEL,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.pull_up_en = GPIO_PULLUP_DISABLE
	};

	gpio_config(&io_conf);
	gpio_dump_io_configuration(stdout, GPIO_OUTPUT_PIN_SEL);

	ESP_LOGI(TAG, "Init sweep task");
	xTaskCreate(sweep_demo_task, "sweep_demo_task", 2048, NULL, 10, NULL);
}
