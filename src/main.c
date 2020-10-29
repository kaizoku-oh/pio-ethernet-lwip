#include "main.h"

int main(void)
{
  HAL_Init();
  BSP_LED_Init(LED_BLUE);

  while(1)
  {
    BSP_LED_Toggle(LED_BLUE);
    HAL_Delay(1000);
  }
}

void SysTick_Handler(void)
{
  HAL_IncTick();
}
