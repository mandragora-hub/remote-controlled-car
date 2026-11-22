#include "motor_driver.h"

#include "esp_log.h" 

static const char *TAG = "motor_driver";

esp_err_t motor_driver_init(motor_driver_t *motor_driver) {
	esp_err_t err = motor_init(&motor_driver->left_motor);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "An error occurred while initializing left motor.");
		return err;
	}

	err = motor_init(&motor_driver->right_motor);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "An error occurred while initializing right motor.");
		return err;
	}

	ESP_LOGI(TAG, "Setting STDBY GPIO");
	gpio_config_t io_conf = {
		.intr_type = GPIO_INTR_DISABLE,
		.mode = GPIO_MODE_OUTPUT,
		.pin_bit_mask = 1LL << motor_driver->stby_gpio,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.pull_up_en = GPIO_PULLUP_DISABLE
	};

	err = gpio_config(&io_conf);
	return err;
}

void motor_driver_set_mode(motor_driver_t *motor_driver, motor_driver_mode_t mode) {
	if (motor_driver->mode == mode) return;

	ESP_ERROR_CHECK(gpio_set_level(motor_driver->stby_gpio, mode == MOTOR_DRIVER_NORMAL_MODE ? 1 : 0));
	motor_driver->mode = mode;
}

void motor_driver_forward(motor_driver_t *motor_driver, uint8_t speed) {
	motor_driver_set_mode(motor_driver, MOTOR_DRIVER_NORMAL_MODE);
	motor_set_state(&motor_driver->left_motor, MOTOR_STATE_CW);
	motor_set_state(&motor_driver->right_motor, MOTOR_STATE_CW);
	motor_set_speed(&motor_driver->left_motor, speed);
	motor_set_speed(&motor_driver->right_motor, speed);
}

void motor_driver_reverse(motor_driver_t *motor_driver, uint8_t speed) {
	motor_driver_set_mode(motor_driver, MOTOR_DRIVER_NORMAL_MODE);
	motor_set_state(&motor_driver->left_motor, MOTOR_STATE_CCW);
	motor_set_state(&motor_driver->right_motor, MOTOR_STATE_CCW);
	motor_set_speed(&motor_driver->left_motor, speed);
	motor_set_speed(&motor_driver->right_motor, speed);
}

void motor_driver_turn_right(motor_driver_t *motor_driver, uint8_t speed) {
	motor_driver_set_mode(motor_driver, MOTOR_DRIVER_NORMAL_MODE);
	motor_set_state(&motor_driver->left_motor, MOTOR_STATE_STOP);
	motor_set_state(&motor_driver->right_motor, MOTOR_STATE_CW);
	motor_set_speed(&motor_driver->left_motor, speed);
	motor_set_speed(&motor_driver->right_motor, speed);
}

void motor_driver_turn_left(motor_driver_t *motor_driver, uint8_t speed) {
	motor_driver_set_mode(motor_driver, MOTOR_DRIVER_NORMAL_MODE);
	motor_set_state(&motor_driver->left_motor, MOTOR_STATE_CW);
	motor_set_state(&motor_driver->right_motor, MOTOR_STATE_STOP);
	motor_set_speed(&motor_driver->left_motor, speed);
	motor_set_speed(&motor_driver->right_motor, speed);
}

void motor_driver_stop(motor_driver_t *motor_driver) {
	motor_driver_set_mode(motor_driver, MOTOR_DRIVER_NORMAL_MODE);
	motor_set_state(&motor_driver->left_motor, MOTOR_STATE_STOP);
	motor_set_state(&motor_driver->right_motor, MOTOR_STATE_STOP);
}

void motor_driver_short_brake(motor_driver_t *motor_driver) {
	motor_driver_set_mode(motor_driver, MOTOR_DRIVER_NORMAL_MODE);
	motor_set_state(&motor_driver->left_motor, MOTOR_STATE_SHORT_BRAKE);
	motor_set_state(&motor_driver->right_motor, MOTOR_STATE_SHORT_BRAKE);
}

