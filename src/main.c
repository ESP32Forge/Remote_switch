/**
 * @file      main.c
 * @authors   Álvaro Velasco García
 * @date      March 16, 2025
 *
 * @brief     This source file is the entry point of the Proto pixel interview
 */

/***************************************************************************************
 * Includes
 ***************************************************************************************/
#include <Remote_switch.h>
#include <Debug.h>

/***************************************************************************************
 * Functions
 ***************************************************************************************/


void app_main() 
{

  if(BPS_button_LOG(init_BSP_button_module()) != BSP_BUTTON_OK)
  {
    #if DEBUG_MODE_ENABLE == 1
      ESP_LOGE("MAIN", "Can not initialize BSP button.");
    #endif
  }

  if(core_remote_switch_LOG(init_remote_switch(BUTTON_0)) != CORE_REMOTE_SWITCH_OK)
  {
    #if DEBUG_MODE_ENABLE == 1
      ESP_LOGE("MAIN", "Can not initialize remote switch 0.");
    #endif
  }

  if(core_remote_switch_LOG(remote_switch_start_client()) != CORE_REMOTE_SWITCH_OK)
  {
    #if DEBUG_MODE_ENABLE == 1
      ESP_LOGE("MAIN", "Can not initialize remote switch 1.");
    #endif
  }

}
