#include "freertos/FreeRTOS.h"
#include "esp_log.h"

#include "car_motor_controller.h"
#include "motor.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_8_BIT 
#define LEDC_MAX_DUTY_CYCLE			(255) // (2 ** duty_resolution)
#define LEDC_CLK_SRC            LEDC_AUTO_CLK
#define LEDC_FREQUENCY          (20000) // TODO: verify what frequency should be more suitable based on battery consumption

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

static const char *TAG = "car_motor_controller";

void car_motor_controller_update_task(void *pvParameters) {
	car_motor_controller_t *car = (car_motor_controller_t *)pvParameters;

	for (;;) {
		motor_update(&car->front_motor_driver.left_motor);
		motor_update(&car->front_motor_driver.right_motor);
		motor_update(&car->rear_motor_driver.left_motor);
		motor_update(&car->rear_motor_driver.right_motor);
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

esp_err_t car_motor_controller_init(car_motor_controller_t *car) {
	ESP_LOGI(TAG, "Prepare timer for PWM usage");
	ledc_timer_config_t ledc_timer = {
		.speed_mode       = LEDC_MODE,
		.duty_resolution  = LEDC_DUTY_RES,
		.timer_num        = LEDC_TIMER,
		.freq_hz          = LEDC_FREQUENCY, 
		.clk_cfg          = LEDC_CLK_SRC,
	};
	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

	car->front_motor_driver.left_motor = (motor_t) {
		.name = "front_left",
		.config = {
			.ledc_timer = LEDC_TIMER,
			.ledc_channel = LEDC_CHANNEL_0,
			.motor_in1 = FRONT_MOTOR_CHANNEL_AIN1,
			.motor_in2 = FRONT_MOTOR_CHANNEL_AIN2,
			.motor_pwm = FRONT_MOTOR_CHANNEL_APWM
		}
	};

	car->front_motor_driver.right_motor = (motor_t) {
		.name = "front_right",
		.config = {
			.ledc_timer = LEDC_TIMER,
			.ledc_channel = LEDC_CHANNEL_1,
			.motor_in1 = FRONT_MOTOR_CHANNEL_BIN1,
			.motor_in2 = FRONT_MOTOR_CHANNEL_BIN2,
			.motor_pwm = FRONT_MOTOR_CHANNEL_BPWM
		}
	};

	car->rear_motor_driver.left_motor = (motor_t) {
		.name = "rear_right",
		.config = {
			.ledc_timer = LEDC_TIMER,
			.ledc_channel = LEDC_CHANNEL_2,
			.motor_in1 = REAR_MOTOR_CHANNEL_AIN1,
			.motor_in2 = REAR_MOTOR_CHANNEL_AIN2,
			.motor_pwm = REAR_MOTOR_CHANNEL_APWM
		}
	};

	car->rear_motor_driver.right_motor = (motor_t) {
		.name = "rear_right",
		.config = {
			.ledc_timer = LEDC_TIMER,
			.ledc_channel = LEDC_CHANNEL_3,
			.motor_in1 = REAR_MOTOR_CHANNEL_BIN1,
			.motor_in2 = REAR_MOTOR_CHANNEL_BIN2,
			.motor_pwm = REAR_MOTOR_CHANNEL_BPWM
		}
	};

	car->front_motor_driver.mode = MOTOR_DRIVER_STANDBY_MODE;
	car->front_motor_driver.stby_gpio = FRONT_MOTOR_STBY;

	car->rear_motor_driver.mode = MOTOR_DRIVER_STANDBY_MODE;
	car->rear_motor_driver.stby_gpio = REAR_MOTOR_STBY;
	
	ESP_ERROR_CHECK(motor_driver_init(&car->front_motor_driver));
	ESP_ERROR_CHECK(motor_driver_init(&car->rear_motor_driver));

	return ESP_OK;
}

void car_motor_controller_forward(car_motor_controller_t *car, uint8_t speed) {
	ESP_LOGI(TAG, "Moving foward");
	motor_driver_forward(&car->front_motor_driver, speed);
	motor_driver_forward(&car->rear_motor_driver, speed);
}

void car_motor_controller_reverse(car_motor_controller_t *car, uint8_t speed) {
	ESP_LOGI(TAG, "Moving reverse");
	motor_driver_reverse(&car->front_motor_driver, speed);
	motor_driver_reverse(&car->rear_motor_driver, speed);
}

void car_motor_controller_turn_right(car_motor_controller_t *car, uint8_t speed) {
	ESP_LOGI(TAG, "turn right");
	motor_driver_turn_right(&car->front_motor_driver, speed);
	motor_driver_turn_right(&car->rear_motor_driver, speed);
}

void car_motor_controller_turn_left(car_motor_controller_t *car, uint8_t speed) {
	ESP_LOGI(TAG, "turn left");
	motor_driver_turn_left(&car->front_motor_driver, speed);
	motor_driver_turn_left(&car->rear_motor_driver, speed);
}

void car_motor_controller_stop(car_motor_controller_t *car) {
	ESP_LOGI(TAG, "Stopped (coast)");
	motor_driver_stop(&car->front_motor_driver);
	motor_driver_stop(&car->rear_motor_driver);
}

void car_motor_controller_short_brake(car_motor_controller_t *car) {
	ESP_LOGI(TAG, "Short brake");
	motor_driver_short_brake(&car->front_motor_driver);
	motor_driver_short_brake(&car->rear_motor_driver);
}

void car_motor_controller_standby(car_motor_controller_t *car) {
	ESP_LOGI(TAG, "Standby");
	motor_driver_set_mode(&car->front_motor_driver, MOTOR_DRIVER_STANDBY_MODE);
	motor_driver_set_mode(&car->rear_motor_driver, MOTOR_DRIVER_STANDBY_MODE);
}

