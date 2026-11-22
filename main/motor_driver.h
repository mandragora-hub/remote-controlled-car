#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include "motor.h"
#include "esp_err.h"
#include "driver/gpio.h"


typedef enum {
	MOTOR_DRIVER_STANDBY_MODE,
	MOTOR_DRIVER_NORMAL_MODE
} motor_driver_mode_t;

typedef struct {
	motor_t left_motor;
	motor_t right_motor;

	motor_driver_mode_t mode;
	gpio_num_t stby_gpio;
} motor_driver_t;

esp_err_t motor_driver_init(motor_driver_t *motor_driver);
void motor_driver_set_mode(motor_driver_t *motor_driver, motor_driver_mode_t mode);
//void motor_driver_set_left_motor_speed();
void motor_driver_forward(motor_driver_t *motor_driver, uint8_t speed);
void motor_driver_reverse(motor_driver_t *motor_driver, uint8_t speed);
void motor_driver_turn_right(motor_driver_t *motor_driver, uint8_t speed);
void motor_driver_turn_left(motor_driver_t *motor_driver, uint8_t speed);
void motor_driver_stop(motor_driver_t *motor_driver);
void motor_driver_short_brake(motor_driver_t *motor_driver);

#endif // MOTOR_DRIVER_H
