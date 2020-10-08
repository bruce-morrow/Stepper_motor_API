/**
  ******************************************************************************
  * @file    STEP_MOTOR_28BYJ-48-5V_NUCLEO-64_STMF103.c
  * @brief   This file contains the implementation of an API for controlling the
  * 		 28BYJ-48-5V stepper motor.
  ******************************************************************************
  *
  * Version 1.0 - API with the following implemented functions:
  * void set_pins(uint16_t blue_wire, uint16_t pink_wire, uint16_t yellow_wire, uint16_t orange_wire);
  * void set_step_mode(StepMode mode);
  * void turn_steps(uint16_t num_steps, uint16_t speed, Direction direction);
  * void turn(uint16_t speed, Direction direction);
  * void stop(StopMode mode);
  * void continue_movement(Direction direction);
  *
  * Copyright (C) 2020  Bruce N. Morrow <bruce.morrow@hotmail.com> and
  * 					Matheus H. Silva <mateusilva73@gmail.com>

  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.

  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.

  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
  *
  * Created on: Oct 07 of 2020
  * Instituition: Universade Federal de Minas Gerais - UFMG
  *
  * This API contains functions to drive the 28BYJ-48-5V stepper motor. In
  * order to do that a driver circuit is needed, such as the module containing
  * the ULN2003 buffer.
  * Four digital output pins are used to drive the motor's four coils.
  * Timer TIM1 is used to generate the time base to caclulate the time between
  * steps, therefore defining the rotation speed.
  * Wave step, full step and half step driving modes are available.
  * Polling programming technique.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "STEP_MOTOR_28BYJ-48-5V_NUCLEO-64_STMF103.h"

static uint16_t blue_wire_pin;
static uint16_t pink_wire_pin;
static uint16_t yellow_wire_pin;
static uint16_t orange_wire_pin;
static GPIO_TypeDef* port;
static StepMode step_mode = NULL;
static Direction current_direction;
static StopMode stop_mode = FREE;
static uint16_t total_steps;
static int16_t remaning_steps;
static int8_t current_step = 8;
static uint32_t current_millis = 0;
static uint16_t current_speed;
static uint8_t is_infinite_turn;

/* configuration functions ---------------------------------------------------*/
/**
  * @brief  Sets which pins the user wants to connect to each phase.
  * @param  blue_wire: GPIO related to the blue wire phase.
  * @param  pink_wire: GPIO related to the pink wire phase.
  * @param  blue_wire: GPIO related to the yellow wire phase.
  * @param  blue_wire: GPIO related to the orange wire phase.
  * @retval None
  */
void set_pins(GPIO_TypeDef* GPIOx, uint16_t blue_wire, uint16_t pink_wire, uint16_t yellow_wire, uint16_t orange_wire) {
	port = GPIOx;
	blue_wire_pin = blue_wire;
	pink_wire_pin = pink_wire;
	yellow_wire_pin = yellow_wire;
	orange_wire_pin = orange_wire;
}

/**
  * @brief  Sets which drive step mode is to be used.
  * @param  mode: the desired mode from the StepMode enum.
  * @retval None
  */
void set_step_mode(StepMode mode) {
	step_mode = mode;
	if(current_step == 8) {
		if(step_mode == FULL_STEP) {
			current_step = 1;
		}
		else {
			current_step = 0;
		}
	}
	else {
		if((step_mode == FULL_STEP) && (current_step % 2 == 0)) {
			current_step++;
		}
		else if((step_mode == WAVE_STEP) && (current_step % 2 == 1)) {
			current_step--;
		}
	}
}

/* movement functions --------------------------------------------------------*/
/**
  * @brief  Turns the motor a given number of steps with a given
  * 		speed and direction.
  * @param  num_steps: number of steps to turn.
  * @param  speed: turning speed given in steps per second.
  * @param  direction: element from the Direction enum to tell to
  * 		turn clockwise or counterclockwise.
  * @retval None
  */
void turn_steps(uint16_t num_steps, uint16_t speed, Direction direction) {
	current_direction = direction;
	total_steps = num_steps;
	remaning_steps = num_steps;
	is_infinite_turn = 0;
	current_speed = speed;
	stop_mode = TURNING;
}

/**
  * @brief  Turns the motor infinitely with a given speed and direction
  * 		until the stop() function is called.
  * @param  speed: turning speed given in steps per second.
  * @param  direction: element from the Direction enum to tell to
  * 		turn clockwise or counterclockwise.
  * @retval None
  */
void turn_infinite(uint16_t speed, Direction direction) {
	is_infinite_turn = 1;
	current_speed = speed;
	current_direction = direction;
	stop_mode = TURNING;
}

/**
  * @brief  Stops the motor if it is turning with or without torque.
  * @param  mode: element from the StopMode enum to tell if it is to
  * 		stop the motor with torque (pahse on) or free (all phases
  * 		off).
  * @retval None
  */
void stop(StopMode mode) {
	stop_mode = mode;
}

/**
  * @brief  Continues the movement the motor was doing before the
  * 		stop() function was called.
  * @param  direction: element from the Direction enum to tell to
  * 		turn clockwise or counterclockwise.
  * @retval None
  */
void continue_movement(Direction direction) {
	if(stop_mode != TURNING) {
		if(current_direction != direction) {
			total_steps = total_steps - remaning_steps;
			current_direction = direction;
		}
		stop_mode = TURNING;
	}
}

/**
  * @brief  This is the brain of the API, it must be called inside
  * the interruption routine of a timer set to generate interrupts
  * every 1ms.
  * @param  None
  * @retval None
  */
void turn_cotroller(void) {
	if(current_millis >= (1000/current_speed)) {
		current_millis = 0;
		GPIO_PinState output[4];
		if(stop_mode == FREE) {
			for(int i = 0; i < 4; i++) {
				output[i] = OFF_STATE[i];
			}
		}
		else if(stop_mode == TORQUE) {
			for(int i = 0; i < 4; i++) {
				output[i] = PHASE_STATES[current_step][i];
			}
		}
		else if((is_infinite_turn == 1) || (remaning_steps > 0)) {
			current_step = (int8_t)(current_step + current_direction*step_mode)%8;
			if(current_step < 0) {
				current_step = 8 - current_step;
			}
			for(int i = 0; i < 4; i++) {
				output[i] = PHASE_STATES[current_step][i];
			}
			remaning_steps--;
			if(remaning_steps < 0){
				remaning_steps =0;
			}
		}
		HAL_GPIO_WritePin(port,blue_wire_pin, output[0]);
		HAL_GPIO_WritePin(port, pink_wire_pin, output[1]);
		HAL_GPIO_WritePin(port, yellow_wire_pin, output[2]);
		HAL_GPIO_WritePin(port, orange_wire_pin, output[3]);
	}
	current_millis++;
}
