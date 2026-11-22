#include "motor.h"

#include "esp_log.h"

static const char *TAG = "motor";

static percentage_t uint8_to_percentage(uint8_t val) {
	if (val >= 100) val = 100;
	return val;
}

static uint8_t percentage_to_duty_cycle(percentage_t percentage) {
	if (percentage >= 100) return LEDC_MAX_DUTY_CYCLE;
	return (LEDC_MAX_DUTY_CYCLE / 100) * percentage;
}

static void motor_update_pwm_duty_cycle(motor_t *motor) {
	uint8_t pwm_duty_cycle = percentage_to_duty_cycle(motor->speed);
	ESP_LOGI(TAG, "Motor %s: set pwm duty cycle to %u based on speed %u", motor->name, pwm_duty_cycle, motor->speed);

	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, motor->config.ledc_channel, pwm_duty_cycle));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, motor->config.ledc_channel));
}

static void motor_update_speed(motor_t *motor, uint8_t speed) {
	motor->speed = uint8_to_percentage(speed);
}
static void motor_update_state(motor_t *motor, motor_state_t state) { 
	uint32_t level_in1 = (state == MOTOR_STATE_CW || state == MOTOR_STATE_SHORT_BRAKE) ? 1 : 0;
	uint32_t level_in2 = (state == MOTOR_STATE_CCW || state == MOTOR_STATE_SHORT_BRAKE) ? 1 : 0;

	ESP_ERROR_CHECK(gpio_set_level(motor->config.motor_in1, level_in1));
	ESP_ERROR_CHECK(gpio_set_level(motor->config.motor_in2, level_in2));
	motor->state = state;
}

esp_err_t motor_init(motor_t *motor) {
	ESP_LOGI(TAG, "Motor %s: Prepare PWM channel configuration", motor->name);
	ledc_channel_config_t ledc_channel = {
		.speed_mode     = LEDC_MODE,
		.channel        = motor->config.ledc_channel,
		.timer_sel      = motor->config.ledc_timer,
		.gpio_num       = motor->config.motor_pwm,
		.duty           = 0, // Set duty to 0%
		.hpoint         = 0,
	};

	esp_err_t err = ledc_channel_config(&ledc_channel);
	if (err != ESP_OK) return err;

	ESP_LOGI(TAG, "Motor %s: Setting GPIO", motor->name);
	gpio_config_t io_conf = {
		.intr_type = GPIO_INTR_DISABLE,
		.mode = GPIO_MODE_OUTPUT,
		.pin_bit_mask = (1LL << motor->config.motor_in1 | 1LL << motor->config.motor_in2),
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.pull_up_en = GPIO_PULLUP_DISABLE
	};
	err = gpio_config(&io_conf);
	if (err != ESP_OK) return err;
	//gpio_dump_io_configuration(stdout, GPIO_OUTPUT_PIN_SEL);

	// Init values
	motor->state = MOTOR_STATE_STOP;
	motor->speed = 0;

	return ESP_OK;
}

void motor_set_state(motor_t *motor, motor_state_t state) {
	if (motor->target_state != state) motor->target_state = state;
}

void motor_set_speed(motor_t *motor, uint8_t speed) {
	speed = uint8_to_percentage(speed);
	if (motor->target_speed != speed) motor->target_speed = speed;
}

void motor_update(motor_t *motor) {
	motor_state_t target_state = motor->target_state;
	if (motor->state != target_state) {
		switch (target_state) {
			case (MOTOR_STATE_STOP):
				motor_update_state(motor, target_state);
				break;
			case (MOTOR_STATE_SHORT_BRAKE):
				motor_update_state(motor, target_state);
				motor_set_speed(motor, 0);
				break;
			case (MOTOR_STATE_CW):
				if (motor->state == MOTOR_STATE_CCW) {
					motor_set_speed(motor, 0);
					if (motor->speed == 0) motor_update_state(motor, target_state);
				} else  {
					motor_update_state(motor, target_state);
				}
				break;
			case (MOTOR_STATE_CCW):
				if (motor->state == MOTOR_STATE_CW) {
					motor_set_speed(motor, 0);
					if (motor->speed == 0) motor_update_state(motor, target_state);
				} else  {
					motor_update_state(motor, target_state);
				}
				break;

		}
	}

	// TODO: inquire how to use bezier curve for speed transitions
	uint8_t target_speed = motor->target_speed;
	if (motor->speed != target_speed) {
		int8_t factor = motor->speed < target_speed ? 5 : -5;
		motor->speed+=factor;
	}

	motor_update_pwm_duty_cycle(motor);
}
