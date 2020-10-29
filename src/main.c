#include "main.h"
#include "cmsis_os.h"
#include "stm32f7xx_nucleo_144.h"

static void thread_led(void const *pvArg);

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

static void thread_led(void const *pvArg)
{
  BSP_LED_Init(LED_BLUE);
  while(1)
  {
    BSP_LED_Toggle(LED_BLUE);
    osDelay(1000);
  }
}

void vApplicationStackOverflowHook(osThreadId stThread, signed char *pcThreadName)
{
  configASSERT(0);
}

void vApplicationMallocFailedHook(void)
{
  configASSERT(0);
}

void HardFault_Handler(void)
{
  configASSERT(0);
}
