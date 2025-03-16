/**
 * @file      TCP_client.h
 * @authors   Álvaro Velasco García
 * @date      March 16, 2025
 *
 * @brief     This header file declares the functions to initialize the WiFi peripheral,
 *            connect and send messages to the access point.
 */

#ifndef CORE_TCP_CLIENT_H_
#define CORE_TCP_CLIENT_H_

/***************************************************************************************
 * Includes
 ***************************************************************************************/
#include <Network_config.h>

/***************************************************************************************
 * Defines
 ***************************************************************************************/

/* List of the possible return codes that module TCP client can return. */
#define TCP_CLIENT_RETURNS                                    \
  /* Info codes */                                            \
  TCP_CLIENT_RETURN(CORE_TCP_CLIENT_OK)                       \
  /* Error codes */                                           \
  TCP_CLIENT_RETURN(CORE_TCP_CLIENT_INIT_ERR)                 \
  TCP_CLIENT_RETURN(CORE_TCP_CLIENT_INIT_QUEUE_ERR)           \
  TCP_CLIENT_RETURN(CORE_TCP_CLIENT_INIT_SEMAPHORE_ERR)       \
  TCP_CLIENT_RETURN(CORE_TCP_CLIENT_DE_INIT_ERR)              \
  TCP_CLIENT_RETURN(CORE_TCP_CLIENT_MODULE_WAS_NOT_INIT_ERR)  \   
  TCP_CLIENT_RETURN(CORE_TCP_CLIENT_CANT_INSERT_IN_QUEUE_ERR) \
  TCP_CLIENT_RETURN(CORE_TCP_CLIENT_SEND_TIME_OUT_WARN)  
 
/***************************************************************************************
 * Data Type Definitions
 ***************************************************************************************/

/* Enumerate that lists the posible return codes that the module can return. */
typedef enum
{
  #define TCP_CLIENT_RETURN(enumerate) enumerate,
    TCP_CLIENT_RETURNS
  #undef TCP_CLIENT_RETURN
  /* Last enumerate always, indicates the number of elements. Do not delete */
  NUM_OF_TCP_CLIENT_RETURNS,
} TCP_client_return;

/***************************************************************************************
 * Functions Prototypes
 ***************************************************************************************/

/**
 * @brief Initializes the WiFi peripheral starting the device as a WiFi station. 
 *        It is mandatory to call this function before any other function of this module.
 *        This function performs a wait until the connection is stablished and can not be 
 *        called from ISR.
 *
 * @param void
 *
 * @return CORE_TCP_CLIENT_OK if the operation went well,
 *         otherwise:
 * 
 *           - CORE_TCP_CLIENT_INIT_QUEUE_ERR:
 *               Error trying to create a queue.
 * 
 *           - CORE_TCP_CLIENT_INIT_SEMAPHORE_ERR:
 *               Error trying to create a sempahore.
 * 
 *           - CORE_TCP_CLIENT_INIT_ERR:
 *               An error ocurred in an intermediate function.
 * 
 *           
 */
TCP_client_return init_TCP_client(void);

/**
 * @brief De-Initialize the WiFi peripheral.
 *
 * @param void
 *
 * @return CORE_TCP_SERVER_OK if the operation went well,
 *         otherwise:
 *     
 *           - CORE_TCP_CLIENT_DE_INIT_ERR:
 *               An error ocurred in an intermediate function.
 *           
 */
TCP_client_return de_init_TCP_client(void);

/**
 * @brief Sends a command(TCP/IP frame) to the gateway.
 *
 * @param cmd Command to send, this parameter type is defined in Network_config.h
 *
 * @return CORE_TCP_CLIENT_OK if the operation went well,
 *         otherwise:
 * 
 *           - CORE_TCP_CLIENT_MODULE_WAS_NOT_INIT_ERR: 
 *               Module was not initialized before.
 * 
 *           - CORE_TCP_CLIENT_CANT_INSERT_IN_QUEUE_ERR: 
 *              If it is not possible to insert the message in the queue.
 * 
 *           - CORE_TCP_CLIENT_SEND_TIME_OUT_WARN:
 *               Time out expires while trying to send the message.
 *           
 */
TCP_client_return send_message(const TCP_COMMAND_TYPE cmd);

/**
 * @brief Prints the return of a TCP client module function if the system was configured 
 *        in debug mode.
 *
 * @param ret Received return from a TCP client module function.
 *
 * @return The given return.
 */
TCP_client_return core_TCP_client_LOG(const TCP_client_return ret);

#endif /* CORE_TCP_CLIENT_H_ */
 