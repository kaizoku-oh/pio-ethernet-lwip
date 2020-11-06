#include "main.h"
#include "cmsis_os.h"
#include "stm32f7xx_nucleo_144.h"
#include "net_ethernet.h"
#include "bsp_rng.h"

static void app_thread(void const *pvArg);
static void ethernet_receive_cb(ETH_HandleTypeDef *pstHandle);

osThreadId stLedThreadHandle;

int main(void)
{
  HAL_Init();
  BSP_RNG_Init();
  BSP_ETHERNET_Init();
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_BLUE);
  BSP_LED_Init(LED_RED);

  osThreadDef(app, app_thread, osPriorityNormal, 0, 128);
  stLedThreadHandle = osThreadCreate(osThread(app), NULL);

  osKernelStart();
  while(1)
  {}
}

static void app_thread(void const *pvArg)
{
  BSP_LED_Toggle(LED_BLUE);
  BSP_ETHERNET_Register_Cb(ethernet_receive_cb);

  while(1)
  {
    BSP_LED_Toggle(LED_GREEN);
    BSP_LED_Toggle(LED_BLUE);
    BSP_LED_Toggle(LED_RED);
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
