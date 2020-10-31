#include "main.h"
#include "cmsis_os.h"
#include "stm32f7xx_nucleo_144.h"
#include "bsp_ethernet.h"

static void thread_led(void const *pvArg);
static void thread_ethernet(void const *pvArg);
static void ethernet_receive_cb(ETH_HandleTypeDef *pstHandle);

osThreadId stLedThreadHandle;
osThreadId stEthThreadHandle;

int main(void)
{
  HAL_Init();

  osThreadDef(led, thread_led, osPriorityNormal, 0, 128);
  stLedThreadHandle = osThreadCreate(osThread(led), NULL);

  osThreadDef(ethernet, thread_ethernet, osPriorityNormal, 0, 128);
  stEthThreadHandle = osThreadCreate(osThread(ethernet), NULL);

  osKernelStart();
  while(1)
  {}
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

static void thread_ethernet(void const *pvArg)
{
  bsp_ethernet_init();
  bsp_ethernet_register_cb(ethernet_receive_cb);

  while(1)
  {
    osDelay(1000);
  }
}

static void ethernet_receive_cb(ETH_HandleTypeDef *pstHandle)
{
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
