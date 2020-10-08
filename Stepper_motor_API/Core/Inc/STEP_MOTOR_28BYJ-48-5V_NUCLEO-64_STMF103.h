/**
  ******************************************************************************
  * @file    STEP_MOTOR_28BYJ-48-5V_NUCLEO-64_STMF103.h
  * @brief   This file contains the headers of an API for controlling the
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
  *
  * This API was developed as a work from the Embedded Systems Programming
  * discipline at UFMG - Prof. Ricardo de Oliveira Duarte - Electronics
  * Engineering Department.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef INC_STEP_MOTOR_28BYJ_48_5V_NUCLEO_64_STMF103_H_
#define INC_STEP_MOTOR_28BYJ_48_5V_NUCLEO_64_STMF103_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stm32f1xx_hal_gpio.h"

/* Exported types ------------------------------------------------------------*/
/**
  * @brief  step modes enumeration
  */
typedef enum
{
  WAVE_STEP = 2,
  FULL_STEP = 2,
  HALF_STEP = 1
} StepMode;

/**
  * @brief  turning directions enumeration
  */
typedef enum
{
	CLOCKWISE = 1,
	COUNTERCLOCKWISE = -1
} Direction;

/**
  * @brief  stop modes enumeration
  */
typedef enum
{
	TORQUE = 1,
	FREE = 0,
	TURNING
} StopMode;

/**
  * @brief  struct to hold which pin is connected to each phase
  */
struct PhasePins {
	uint16_t blue_wire;
	uint16_t pink_wire;
	uint16_t yellow_wire;
	uint16_t orange_wire;
} PhasePins;

/* Exported constants --------------------------------------------------------*/
/**
  * @brief  this is the possible phase states matrix. Each line is a state and
  * each column is a phase, considering that the phases are on when the respective
  * output is low (common wire is on 5V). The wave step is obtained by traversing
  * through the odd lines, the full step mode by the even lines and the half step
  * by all the lines.
  */
const GPIO_PinState PHASE_STATES[8][4] = {{GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET}, \
					  	  	  	  	  	  {GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET}, \
										  {GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET}, \
										  {GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET}, \
										  {GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET}, \
										  {GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET}, \
										  {GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET}, \
										  {GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET}};

/**
  * @brief  this is the state of phase when you use the StopMode without torque
  */
const GPIO_PinState OFF_STATE[4] = {GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET};
/* configuration functions ---------------------------------------------------*/
/**
  * @brief  Sets which pins the user wants to connect to each phase.
  * @param  GPIOx: GPIO port used.
  * @param  blue_wire: GPIO related to the blue wire phase.
  * @param  pink_wire: GPIO related to the pink wire phase.
  * @param  blue_wire: GPIO related to the yellow wire phase.
  * @param  blue_wire: GPIO related to the orange wire phase.
  * @retval None
  */
void set_pins(GPIO_TypeDef* GPIOx,uint16_t blue_wire, uint16_t pink_wire, uint16_t yellow_wire, uint16_t orange_wire);

/**
  * @brief  Sets which drive step mode is to be used.
  * @param  mode: the desired mode from the StepMode enum.
  * @retval None
  */
void set_step_mode(StepMode mode);

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
void turn_steps(uint16_t num_steps, uint16_t speed, Direction direction);

/**
  * @brief  Turns the motor infinitely with a given speed and direction
  * 		until the stop() function is called.
  * @param  speed: turning speed given in steps per second.
  * @param  direction: element from the Direction enum to tell to
  * 		turn clockwise or counterclockwise.
  * @retval None
  */
void turn_infinite(uint16_t speed, Direction direction);

/**
  * @brief  Stops the motor if it is turning with or without torque.
  * @param  mode: element from the StopMode enum to tell if it is to
  * 		stop the motor with torque (pahse on) or free (all phases
  * 		off).
  * @retval None
  */
void stop(StopMode mode);

/**
  * @brief  Continues the movement the motor was doing before the
  * 		stop() function was called.
  * @param  direction: element from the Direction enum to tell to
  * 		turn clockwise or counterclockwise.
  * @retval None
  */
void continue_movement(Direction direction);

/**
  * @brief  This is the brain of the API, it must be called inside
  * the interruption routine of a timer set to generate interrupts
  * every 1ms.
  * @param  None
  * @retval None
  */
void turn_cotroller(void);

#endif /* STEP_MOTOR_28BYJ-48-5V_NUCLEO-64_STMF103_H */

/********** (C) COPYRIGHT Bruce N. Morrow and Matheus H. Silva *****END OF FILE****/
