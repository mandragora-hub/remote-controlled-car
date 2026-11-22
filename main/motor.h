#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include "esp_err.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_8_BIT
#define LEDC_MAX_DUTY_CYCLE			(255) // (2 ** duty_resolution)
#define LEDC_CLK_SRC            LEDC_AUTO_CLK
#define LEDC_FREQUENCY          (20000) // TODO: verify what frequency should be more suitable based on battery consumption


#define DEFAULT_PWM_SPEED_TRANSITION_FACTOR 5

typedef uint8_t percentage_t;

typedef enum {
	MOTOR_STATE_CW, // CW stand for clockwise
	MOTOR_STATE_CCW, // CCW stand for counterclockwise
	MOTOR_STATE_SHORT_BRAKE,
	MOTOR_STATE_STOP // Stop or coast
} motor_state_t;

typedef struct {
	ledc_timer_t ledc_timer;
	ledc_channel_t ledc_channel;

	gpio_num_t motor_in1;
	gpio_num_t motor_in2;
	gpio_num_t motor_pwm;
} motor_config_t;

typedef struct {
	const char *name;
	motor_state_t target_state;
	percentage_t target_speed; 

	// private members (TODO: inquire how to make private in c)
	motor_state_t state;
	percentage_t speed; // speed values is in percentage: 0 - 100. Maybe a better name is capacity.
	motor_config_t config;
} motor_t;

esp_err_t motor_init(motor_t *motor);
void motor_set_state(motor_t *motor, motor_state_t motor_state);
void motor_set_speed(motor_t *motor, uint8_t speed);
void motor_update(motor_t *motor);

//void motor_task(void *pvParameters);



#endif // MOTOR_H
