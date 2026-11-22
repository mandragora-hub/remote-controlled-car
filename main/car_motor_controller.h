#ifndef CAR_MOTOR_CONTROLLER_H
#define CAR_MOTOR_CONTROLLER_H

#include "motor_driver.h"
#include "esp_err.h"

typedef struct {
	motor_driver_t front_motor_driver;
	motor_driver_t rear_motor_driver;
} car_motor_controller_t;


esp_err_t car_motor_controller_init(car_motor_controller_t *car);

void car_motor_controller_forward(car_motor_controller_t *car, uint8_t speed);
void car_motor_controller_reverse(car_motor_controller_t *car, uint8_t speed);
void car_motor_controller_turn_right(car_motor_controller_t *car, uint8_t speed);
void car_motor_controller_turn_left(car_motor_controller_t *car, uint8_t speed);
void car_motor_controller_short_brake(car_motor_controller_t *car);
void car_motor_controller_stop(car_motor_controller_t *car);
void car_motor_controller_standby(car_motor_controller_t *car);

#endif // CAR_MOTOR_CONTROLLER_H
