idf_component_register(SRCS "remote-controlled-car.c" "motor.c" "motor_driver.c" "car_motor_controller.c"
										REQUIRES esp_driver_gpio esp_driver_ledc
                    INCLUDE_DIRS ".")
