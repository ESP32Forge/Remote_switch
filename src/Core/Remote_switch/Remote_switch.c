/**
 * @file      Remote_switch.c
 * @authors   Álvaro Velasco García
 * @date      March 16, 2025
 *
 * @brief     This source file defines the functions to control a remote switch.
 */

/***************************************************************************************
 * Includes
 ***************************************************************************************/
#include <Remote_switch.h>
#include <freertos/FreeRTOS.h>
#include <Debug.h>
#include <TCP_client.h>

/***************************************************************************************
 * Defines
 ***************************************************************************************/

#if DEBUG_MODE_ENABLE == 1
/* Tag to show traces in button BSP module. */
#define TAG "CORE_REMOTE_SWITCH"
#endif

/***************************************************************************************
 * Data Type Definitions
 ***************************************************************************************/

/* Structure that contains the information of remote switch. */
typedef struct
{
  /* Identifier of button that belongs to the remote swtich. */
  Button_ID button;
  /* Handler of the task that will perform an action when the button is pressed. */
  TaskHandle_t task_handler;
  /* Semaphore to coordinate the ISR interruption from the button with the remote 
   * switch task. 
   */
  SemaphoreHandle_t semaphore;
} remote_switch_info;

/***************************************************************************************
 * Global Variables
 ***************************************************************************************/

/* Array that contains the configuration of the all the system lamps. */
static remote_switch_info remote_switches_infos[NUM_OF_BUTTONS];

/* Variable that stored which was the last pressed button. */
static Button_ID pressed_button;

/***************************************************************************************
 * Functions Prototypes
 ***************************************************************************************/

/**
 * @brief Function that will handle the action when the button of the remote switch
 *
 * @param args arguments to pass to the function.
 *
 * @return void
 */
static void remote_switch_handler_func(void *args);

/***************************************************************************************
 * Functions
 ***************************************************************************************/

Remote_switch_return init_remote_switch(const Button_ID ID)
{

  /* Initialize button. */
  if(init_button(ID) != BSP_BUTTON_OK)
  {
    return CORE_REMOTE_SWITCH_INIT_ERR;
  }

  /* Initialize Semaphore. Create first sempahore to void start task and try to
   * acquire an initialized semaphore.
   */
  remote_switches_infos[ID].semaphore = xSemaphoreCreateCounting(1, 0);
  if(remote_switches_infos[ID].semaphore == NULL)
  {
    vSemaphoreDelete(remote_switches_infos[ID].semaphore);
    return CORE_REMOTE_SWITCH_INIT_ERR;
  }

  /* Initialize remote switch task. */
  const BaseType_t task_create_ret = xTaskCreate(remote_switch_handler_func, 
    "remote_switch_handler_func", 2048, (void *) 0, configMAX_PRIORITIES-1, 
    &remote_switches_infos[ID].task_handler);
  if(task_create_ret != pdPASS)
  {
    if(remote_switches_infos[ID].task_handler != NULL)
    {
      vTaskDelete(remote_switches_infos[ID].task_handler);
    } 
    return CORE_REMOTE_SWITCH_INIT_ERR;
  }

  return CORE_REMOTE_SWITCH_OK;
}

Remote_switch_return de_init_remote_switch(const Button_ID ID)
{

  /* De-initialize button. */
  const Button_return but_ret = de_init_button(ID);
  if(but_ret != BSP_BUTTON_OK)
  {
    BPS_button_LOG(but_ret);
    return CORE_REMOTE_SWITCH_DE_INIT_ERR;
  }

  if(remote_switches_infos[ID].task_handler != NULL)
  {
    vTaskDelete(remote_switches_infos[ID].task_handler);
  }
  vSemaphoreDelete(remote_switches_infos[ID].semaphore);

  if(core_TCP_client_LOG(de_init_TCP_client()) != CORE_TCP_CLIENT_OK)
  {
    return CORE_REMOTE_SWITCH_DE_INIT_ERR;
  }

  return CORE_REMOTE_SWITCH_OK;
}

Remote_switch_return remote_switch_start_client(void)
{

  /* Initialize and try to connect to the gateway. */
  if(core_TCP_client_LOG(init_TCP_client()) != CORE_TCP_CLIENT_OK)
  {
    return CORE_REMOTE_SWITCH_START_CLIENT_ERR;
  }

  return CORE_REMOTE_SWITCH_OK;
}

Remote_switch_return remote_switch_stop_client(void)
{

  /* Stops the WiFi peripheral and their mechanics. */
  if(core_TCP_client_LOG(de_init_TCP_client()) != CORE_TCP_CLIENT_OK)
  {
    return CORE_REMOTE_SWITCH_STOP_CLIENT_ERR;
  }

  return CORE_REMOTE_SWITCH_OK;
}

inline Remote_switch_return core_remote_switch_LOG(const Remote_switch_return ret)
{
  #if DEBUG_MODE_ENABLE == 1
    switch(ret)
    {
      #define REMOTE_SWITCH_RETURN(enumerate) \
        case enumerate:                       \
          if(ret > 0)                         \
          {                                   \
            ESP_LOGE(TAG, #enumerate);        \
          }                                   \
          else                                \
          {                                   \
            ESP_LOGI(TAG, #enumerate);        \
          }                                   \
          break;       
        REMOTE_SWITCH_RETURNS
      #undef REMOTE_SWITCH_RETURN
      default:
        ESP_LOGE(TAG, "Unkown return.");
        break;
    }
  #endif
  return ret;
}

/* Implemtation of the button callback. */
void __attribute__((weak)) button_CB(const Button_ID ID)
{
  pressed_button = ID;

  BaseType_t higher_priority_task_woken = pdFALSE;
  xSemaphoreGiveFromISR(remote_switches_infos[BUTTON_0].semaphore, 
    &higher_priority_task_woken);  
}

static void remote_switch_handler_func(void *args)
{

  TCP_COMMAND_TYPE cmd;

  while(true)
  {
    if(xSemaphoreTake(remote_switches_infos[BUTTON_0].semaphore, 
         portMAX_DELAY) == pdTRUE)
    {
      switch(pressed_button)
      {
        case BUTTON_0:
          cmd.ID = LED_0;
          cmd.action = TOOGLE_LED;
          break;
        case BUTTON_1:

          cmd.ID = LED_0;
          cmd.action = SET_PWM;

          uint64_t num_of_presses = 0u;
          get_num_of_presses(BUTTON_1, &num_of_presses);

          /* Per each press, increment in 10 points the duty cycle. */
          cmd.pwm = ((num_of_presses%9)*10u) + MIN_DUTY_CYCLE_PERC;

          /* If the value is higher than 100%, return to 0%. */
          if(cmd.pwm > MAX_DUTY_CYCLE_PERC)
          {
            cmd.pwm = MIN_DUTY_CYCLE_PERC;
          }
        default:
          /* TODO: Handle this corner case. */
          break;
      }

      core_TCP_client_LOG(send_message(cmd));
      
    }

  }

  vTaskDelete(NULL);
}



