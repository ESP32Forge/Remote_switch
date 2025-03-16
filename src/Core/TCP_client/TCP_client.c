/**
 * @file      TCP_client.c
 * @authors   Álvaro Velasco García
 * @date      March 16, 2025
 *
 * @brief     This source file defines the functions to initialize the WiFi peripheral,
 *            connect and send messages to the access point.
 */

/***************************************************************************************
 * Includes
 ***************************************************************************************/
#include <TCP_client.h>
#include <WiFi.h>
#include <Debug.h>
#include <freertos/FreeRTOS.h>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include <lwip/dns.h>

/***************************************************************************************
 * Defines
 ***************************************************************************************/

/* Lenght of the commands TX queue. */
#define RX_QUEUE_LEN 10u

/* Maximum time in milliseconds to wait to perform a send operation. */
#define MAX_TIME_TO_WAIT_TO_SEND 1000u/portTICK_PERIOD_MS

#if DEBUG_MODE_ENABLE == 1
  /* Tag to show traces in core TCP module. */
  #define TAG "CORE_TCP_CLIENT"
#endif

/***************************************************************************************
 * Global Variables
 ***************************************************************************************/

/* Handler of the task that will send the commands trough TCP/IP. */
static TaskHandle_t send_cmd_task_handler;

/* Handler of the queue where the commands will be allocated. .... */
static QueueHandle_t cmd_TX_queue;

/* Indicates if the client is connected to the server or not. */
static bool connection_state;

/* Semaphore to ensure atomizity in acces to connection_state variable. */
static SemaphoreHandle_t connection_state_sempahore;

/* Flag that indicates if the module was previously initialized or not. */
static bool module_was_initialized;

/***************************************************************************************
 * Functions Prototypes
 ***************************************************************************************/

/**
 * @brief Handler of the WiFi events.
 * 
 * @param event_handler_arg Arguments to pass to the handler.
 * 
 * @param event_base Which events should wait this function.
 * 
 * @param event_id Which event happened.
 * 
 * @param event_data Data associated to happened event.
 *
 * @return void
 */
static void WiFi_event_handler(void *event_handler_arg, esp_event_base_t event_base,
  int32_t event_id, void *event_data);

/**
 * @brief Handler of the IP events.
 * 
 * @param event_handler_arg Arguments to pass to the handler.
 * 
 * @param event_base Which events should wait this function.
 * 
 * @param event_id Which event happened.
 * 
 * @param event_data Data associated to happened event.
 *
 * @return void
 */
static void IP_event_handler(void *event_handler_arg, esp_event_base_t event_base,
  int32_t event_id, void *event_data);

/**
 * @brief Function that will stablish a connection with the gateway, wait for commands
 *        and send them to the gateway.
 *
 * @param args arguments to pass to the function.
 *
 * @return void
 */
static void cmd_TX_func(void *args);

/**
 * @brief Gets the state of the connection ensuring atomicity.
 *
 * @param time_to_wait Time to wait to access connection_state variable
 *
 * @return True if the connection was stablished, false if the time to wait
 *         expired or the connection was not stablished.
 */
static bool get_connection_state(const TickType_t time_to_wait);

/**
 * @brief Sets the state of the connection ensuring atomicity.
 *
 * @param time_to_wait Time to wait to access connection_state variable.
 * 
 * @param state State to set to the connection_state variable.
 *
 * @return True if the operation went well, otherwise false.
 */
static bool set_connection_state(const TickType_t time_to_wait, const bool state);

/***************************************************************************************
 * Functions
 ***************************************************************************************/

TCP_client_return init_TCP_client(void)
{

  send_cmd_task_handler = NULL;

  /* Create the sempahore than controls the cmd TX task. */
  cmd_TX_queue = xQueueCreate(RX_QUEUE_LEN, sizeof(TCP_COMMAND_TYPE));
  if(cmd_TX_queue == NULL)
  {
    return CORE_TCP_CLIENT_INIT_QUEUE_ERR; 
  }

  /* Create the semaphore that will alert of the connection state. */
  connection_state_sempahore = xSemaphoreCreateCounting(1, 1);
  if(connection_state_sempahore == NULL)
  {
    vSemaphoreDelete(connection_state_sempahore);
    return CORE_TCP_CLIENT_INIT_SEMAPHORE_ERR;
  }

  /* Confiuration of the station. */
  const wifi_config_t config = 
  {
    .sta = 
    {
      .ssid = WIFI_SSID,
      .password = WIFI_PASS,
      .threshold.authmode = WIFI_AUTH_MODE,
    }
  };

  /* WiFi and IP events handlers. */
  const event_handlers handlers =
  {
    .WiFi_events_to_handle = ESP_EVENT_ANY_ID,
    .WiFi_event_handler = WiFi_event_handler,
    
    .IP_events_to_handle = IP_EVENT_STA_GOT_IP,
    .IP_event_handler = IP_event_handler,
  };

  /* Start WiFi in station mode. */
  if(core_WiFi_LOG(WiFi_init(WIFI_MODE_STA, config, handlers) != CORE_WIFI_OK))
  {
    return CORE_TCP_CLIENT_INIT_ERR;
  }

  while(get_connection_state(MAX_TIME_TO_WAIT_TO_SEND) != true)
  {
    #if DEBUG_MODE_ENABLE == 1
      ESP_LOGI(TAG, "Waiting for connection.");
    #endif
    vTaskDelay(MAX_TIME_TO_WAIT_TO_SEND);
  }

  module_was_initialized = true;

  return CORE_TCP_CLIENT_OK;
}

TCP_client_return de_init_TCP_client(void)
{

  connection_state = false;

  if(core_WiFi_LOG(de_init_WiFi()) != CORE_WIFI_OK)
  {
    return CORE_TCP_CLIENT_DE_INIT_ERR;
  }

  return CORE_TCP_CLIENT_OK;
}

TCP_client_return send_message(const TCP_COMMAND_TYPE cmd)
{

  if(!module_was_initialized)
  {
    return CORE_TCP_CLIENT_MODULE_WAS_NOT_INIT_ERR;
  }

  if(get_connection_state(MAX_TIME_TO_WAIT_TO_SEND))
  {
    if(xQueueSend(cmd_TX_queue, (void *)&cmd, (TickType_t)portMAX_DELAY-1) != pdPASS)
    {
      return CORE_TCP_CLIENT_CANT_INSERT_IN_QUEUE_ERR;
    }
  }
  else
  {
    return CORE_TCP_CLIENT_SEND_TIME_OUT_WARN;
  }

  return CORE_TCP_CLIENT_OK;
}

inline TCP_client_return core_TCP_client_LOG(const TCP_client_return ret)
{
  #if DEBUG_MODE_ENABLE == 1
    switch(ret)
    {
      #define TCP_CLIENT_RETURN(enumerate) \
        case enumerate:                    \
          if(ret > 0)                      \
          {                                \
            ESP_LOGE(TAG, #enumerate);     \
          }                                \
          else                             \
          {                                \
            ESP_LOGI(TAG, #enumerate);     \
          }                                \
          break;       
        TCP_CLIENT_RETURNS
      #undef TCP_CLIENT_RETURN
      default:
        ESP_LOGE(TAG, "Undefined return.");
        break;
    }
  #endif
  return ret;
}

static void cmd_TX_func(void *args)
{

  int sock_fd;
  struct sockaddr_in serv_addr;
  TCP_COMMAND_TYPE cmd;

  /** Assign IP, port and the IP protocol. **/

  /* Set IPV4. */
  serv_addr.sin_family = AF_INET;

  /* Obtain the IP of the gateway. */
  esp_netif_ip_info_t ip_info;
  esp_netif_t* esp_netif = esp_netif_next_unsafe(NULL);
  ESP_error_check(esp_netif_get_ip_info(esp_netif, &ip_info));
  /* Convert the gateway IP in string format. */
  char IP_string[32];
  sprintf(IP_string, IPSTR, IP2STR(&ip_info.gw));
  /* Set the IP of the gateway */
  inet_pton(AF_INET, IP_string, &serv_addr.sin_addr);

  /* Set the port defined in Network_config.h */
  serv_addr.sin_port = htons(TCP_IP_PORT);

  while(true)
  {

    /* Wait for new commands to send. */
    if(xQueueReceive(cmd_TX_queue, &(cmd), (TickType_t)portMAX_DELAY-1) == pdPASS)
    {

      /* Create socket and verify the initialization. */
      sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
      if(sock_fd < 0)
      {
        #if DEBUG_MODE_ENABLE == 1
          ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        #endif
      }

      /* Connect the client socket to the server socket. */
      if(connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
      {
        #if DEBUG_MODE_ENABLE == 1
          ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
        #endif
      }

      /* Send command to the gateway. */
      if(write(sock_fd, (void*)&cmd, TCP_COMMAND_SIZE) < 0)
      {
        #if DEBUG_MODE_ENABLE == 1
          ESP_LOGE(TAG, "Send error: errno %d", errno);
        #endif
      }

      bzero((void*)&cmd, sizeof(cmd));

      shutdown(sock_fd, 0);
      close(sock_fd);

    }
  }
  
}

static void WiFi_event_handler(void *event_handler_arg, esp_event_base_t event_base,
  int32_t event_id, void *event_data)
{
    switch (event_id)
    {
      case WIFI_EVENT_STA_START:
        /* Try to connect to the gateway. */
        ESP_error_check(esp_wifi_connect());
        break;
      case WIFI_EVENT_STA_DISCONNECTED:
        
        /* Alerts that the station lost the communications. */
        set_connection_state((TickType_t)portMAX_DELAY-1, false);

        /* If it was initialized before, delete the send command task. */
        if(send_cmd_task_handler != NULL)
        {
          vTaskDelete(send_cmd_task_handler);
        }
 
        /* Try to connect again to the gateway. */
        ESP_error_check(esp_wifi_connect());

        break;
      default:
        break;
    }
}

static void IP_event_handler(void *event_handler_arg, esp_event_base_t event_base,
  int32_t event_id, void *event_data)
{
    switch (event_id)
    {
      case IP_EVENT_STA_GOT_IP:
      
        /* Alerts that the station starts the communications. */
        set_connection_state((TickType_t)portMAX_DELAY-1, true);

        /* Create the task that will send the messages. */
        const BaseType_t ret = xTaskCreate(cmd_TX_func, "cmd_TX_func", 2048, (void *) 0, 
                                            (UBaseType_t)configMAX_PRIORITIES-2,
                                            &send_cmd_task_handler);

        if(ret != pdPASS)
        {
          /* TODO: Handle this corner case. */
        }
        #if DEBUG_MODE_ENABLE == 1
          ESP_LOGI(TAG, "WiFi got IP");
        #endif
        break;
      default:
        #if DEBUG_MODE_ENABLE == 1
          ESP_LOGE(TAG, "Unregistered IP event happened: %" PRIu32, event_id);
        #endif
        break;
    }
}

static bool get_connection_state(const TickType_t time_to_wait)
{

  bool state = false;

  if(xSemaphoreTake(connection_state_sempahore, time_to_wait) == pdTRUE)
  {

    state = connection_state;
    if(xSemaphoreGive(connection_state_sempahore) != pdTRUE)
    {
      /* TODO: Handle this corner case */
    } 

    return state;
  }
  else
  {
    return false;
  }
}

static bool set_connection_state(const TickType_t time_to_wait, const bool state)
{

  if(xSemaphoreTake(connection_state_sempahore, time_to_wait) == pdTRUE)
  {
    connection_state = state;
    if(xSemaphoreGive(connection_state_sempahore) != pdTRUE)
    {
      return false;
    }

  }
  else
  {
    return false;
  }

  return true;
}

