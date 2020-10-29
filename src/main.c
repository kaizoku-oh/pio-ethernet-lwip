#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

void vTask1(void *pvParameters);

int main(void)
{
  HAL_Init();

  xTaskCreate(vTask1, "Task 1", 1000, NULL, 1 , NULL);
  vTaskStartScheduler();
  while(1)
  {
  }
}

void vTask1(void *pvParameters)
{
  BSP_LED_Init(LED_BLUE);
  while(1)
  {
    BSP_LED_Toggle(LED_BLUE);
    vTaskDelay(1000);
  }
}
