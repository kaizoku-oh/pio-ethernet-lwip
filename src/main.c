#include "main.h"
#include "cmsis_os.h"

static void thread_led(void const * argument);

osThreadId stThreadHandle;

int main(void)
{
  HAL_Init();

  osThreadDef(thread, thread_led, osPriorityNormal, 0, 128);
  stThreadHandle = osThreadCreate(osThread(thread), NULL);

  osKernelStart();
  while(1)
  {
  }
}

static void thread_led(void const * argument)
{
  BSP_LED_Init(LED_BLUE);
  while(1)
  {
    BSP_LED_Toggle(LED_BLUE);
    osDelay(1000);
  }
}
