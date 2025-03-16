/**
 * @file      Button_physical_connection.h
 * @authors   Álvaro Velasco García
 * @date      March 16, 2025
 *
 * @brief     This header file defines the board buttons, their physical connections
 *            and the possible states of them.
 */

#ifndef BUTTON_PHYSICAL_CONNECTION_H_
#define BUTTON_PHYSICAL_CONNECTION_H_

/***************************************************************************************
 * Includes
 ***************************************************************************************/
#include <driver/gpio.h>

/***************************************************************************************
 * Defines
 ***************************************************************************************/
 
/* Macro that enlist the board buttons. It is mandatory to not set values to the 
 * enumerates.
 */
#define BUTTONS    \
  BUTTON(BUTTON_0)    
 
/* Macro that describes the GPIO configurations of the buttons. 
 *
 * Parameters:
 * 
 *   1) Identifier of the button, it is mandatory to put a value defined inside BUTTONS.
 *   2) Identifier of the GPIO that reads the button state, it is mandtory to use
 *      an enumerate defined in gpio_num_t enum -> gpio_num.h
 *   3) Indicates the pull mode of the GPIO. It is mandtory to use an enumerate defined 
 *      in gpio_pull_mode_t enum -> gpio_types.h
 *   4) Which wntruption mode will be set to the button GPIO. It is mandatory to use
 *      an enumerate defined in gpio_int_type_t -> gpio_types.h
 *   5) Debounce time in milliseconds. To remove unwanted input noise. 
 *   
 */
#define BUTTONS_CONFIGURATIONS                                                     \
  BUTTON_CONFIG(BUTTON_0, GPIO_NUM_4, GPIO_PULLDOWN_ONLY, GPIO_INTR_POSEDGE, 800u)  

/***************************************************************************************
 * Data Type Definitions
 ***************************************************************************************/

/* Enumerate that enlist the board buttons. */
typedef enum
{
  #define BUTTON(enumerate) enumerate,
    BUTTONS
  #undef BUTTON
  /* Last enumerate always, indicates the number of elements. Do not delete */
  NUM_OF_BUTTONS,
} Button_ID;

/* Enumerate that lists the possible states of a button. */
typedef enum
{
  BUTTON_IS_NOT_PRESSED,
  BUTTON_IS_PRESSED,
} Button_state;

#endif /* BUTTON_PHYSICAL_CONNECTION_H_ */
 