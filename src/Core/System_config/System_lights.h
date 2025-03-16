/**
 * @file      System_lights.h
 * @authors   Álvaro Velasco García
 * @date      March 16, 2025
 *
 * @brief     File that declares macros to define the light elements in the system.
 */

#ifndef SYSTEM_LIGHTS_H_
#define SYSTEM_LIGHTS_H_

/***************************************************************************************
 * Defines
 ***************************************************************************************/

/* Macro that enlist the system LEDs. It is mandatory to not set values to the 
 * enumerates.
 */
#define LEDS \
  LED(LED_0)

/* Maximum and minimun duty cycle that can be set to the PWM LED's in terms of
 * percentage.
 */
#define MAX_DUTY_CYCLE_PERC 100u
#define MIN_DUTY_CYCLE_PERC 20u

/* Checks if the MIN_DUTY_CYCLE_PERCENTAGE and MIN_DUTY_CYCLE_PERCENTAGE have a 
 * valid value. 
 */
#if MIN_DUTY_CYCLE_PERCENTAGE < 0 || MAX_DUTY_CYCLE_PERCENTAGE > 100
  #error "Invalid PWM duty cycle: [0-100]:"
  #error "refer to (MAX_DUTY_CYCLE_PERCENTAGE, MIN_DUTY_CYCLE_PERCENTAGE)"
#endif
 
/***************************************************************************************
 * Data Type Definitions
 ***************************************************************************************/

/* Enumerate that enlist the system LEDs. */
typedef enum
{
  #define LED(enumerate) enumerate,
    LEDS
  #undef LED
  /* Last enumerate always, indicates the number of elements. Do not delete */
  NUM_OF_LEDS,
} LED_ID;

#endif /* SYSTEM_LIGHTS_H_ */