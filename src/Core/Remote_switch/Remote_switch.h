/**
 * @file      Remote_switch.h
 * @authors   Álvaro Velasco García
 * @date      March 16, 2025
 *
 * @brief     This header file declares the functions to control a remote switch.
 */

#ifndef CORE_REMOTE_SWITCH_H_
#define CORE_REMOTE_SWITCH_H_

/***************************************************************************************
 * Includes
 ***************************************************************************************/
#include <Button.h>

/***************************************************************************************
 * Defines
 ***************************************************************************************/

/* List of the possible return codes that module button can return. */
#define REMOTE_SWITCH_RETURNS                               \
  /* Info codes */                                          \
  REMOTE_SWITCH_RETURN(CORE_REMOTE_SWITCH_OK)               \
  /* Error codes */                                         \
  REMOTE_SWITCH_RETURN(CORE_REMOTE_SWITCH_INIT_ERR)         \
  REMOTE_SWITCH_RETURN(CORE_REMOTE_SWITCH_DE_INIT_ERR)      \
  REMOTE_SWITCH_RETURN(CORE_REMOTE_SWITCH_START_CLIENT_ERR) \
  REMOTE_SWITCH_RETURN(CORE_REMOTE_SWITCH_STOP_CLIENT_ERR)       
 
/***************************************************************************************
 * Data Type Definitions
 ***************************************************************************************/

/* Enumerate that lists the posible return codes that the module can return. */
typedef enum
{
  #define REMOTE_SWITCH_RETURN(enumerate) enumerate,
    REMOTE_SWITCH_RETURNS
  #undef REMOTE_SWITCH_RETURN
  /* Last enumerate always, indicates the number of elements. Do not delete */
  NUM_OF_REMOTE_SWITCH_RETURNS,
} Remote_switch_return;

/***************************************************************************************
 * Functions Prototypes
 ***************************************************************************************/

/**
 * @brief Initializes a remote switch.
 *
 * @param button Identifier of the button to associate to the remote switch.
 *
 * @return CORE_REMOTE_SWITCH_OK if the operation went well,
 *         otherwise:      
 * 
 *           -  CORE_REMOTE_SWITCH_INIT_ERR:
 *               An error ocurred in some intermediate function.    
 */
Remote_switch_return init_remote_switch(const Button_ID ID);

/**
 * @brief De-initializes a remote switch.
 *
 * @param ID Identifier of the button to de-initialize.
 *
 * @return CORE_REMOTE_SWITCH_OK if the operation went well,
 *         otherwise:      
 * 
 *           -  CORE_REMOTE_STOP_CLIENT_ERR:
 *               An error ocurred in some intermediate function.  
 * 
 */
Remote_switch_return de_init_remote_switch(const Button_ID ID);

/**
 * @brief Starts the TCP client that will send the commands
 *        when a remote switch is pressed.
 *
 * @param void
 *
 * @return CORE_REMOTE_SWITCH_OK if the operation went well,
 *         otherwise:      
 * 
 *           -  CORE_REMOTE_SWITCH_START_CLIENT_ERR:
 *               An error ocurred in some intermediate function. 
 * 
 */
Remote_switch_return remote_switch_start_client(void);

/**
 * @brief Stops the TCP client.
 *
 * @param void
 *
 * @return CORE_REMOTE_SWITCH_OK if the operation went well,
 *         otherwise:      
 * 
 *           -  CORE_REMOTE_SWITCH_STOP_CLIENT_ERR:
 *               An error ocurred in some intermediate function. 
 * 
 */
Remote_switch_return remote_switch_start_client(void);

/**
 * @brief Prints the return of a remote switch function if the system was configured 
 *        in debug mode.
 *
 * @param ret Received return from a remote switch module function.
 *
 * @return The given return.
 */
Remote_switch_return core_remote_switch_LOG(const Remote_switch_return ret);
 
#endif /* CORE_REMOTE_SWITCH_H_ */
 