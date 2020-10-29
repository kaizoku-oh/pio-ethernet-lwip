#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_tim.h"

TIM_HandleTypeDef stTimerHandle;

HAL_StatusTypeDef HAL_InitTick(uint32_t u32TickPriority)
{
  HAL_StatusTypeDef eRetVal;
  RCC_ClkInitTypeDef stClockCfg;
  uint32_t u32TimerClock;
  uint32_t u32Prescaler;
  uint32_t u32FlashLatency;

  u32TimerClock = 0;
  u32Prescaler = 0;
  HAL_NVIC_SetPriority(TIM6_DAC_IRQn, u32TickPriority ,0);
  HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
  __HAL_RCC_TIM6_CLK_ENABLE();
  HAL_RCC_GetClockConfig(&stClockCfg, &u32FlashLatency);
  u32TimerClock = 2*HAL_RCC_GetPCLK1Freq();
  /* Compute the prescaler value to have TIM6 counter clock equal to 1MHz */
  u32Prescaler = (uint32_t)((u32TimerClock / 1000000) - 1);
  stTimerHandle.Instance = TIM6;
  /** Initialize TIMx peripheral as follow:
    *  Period = [(TIM6CLK/1000) - 1]. to have a (1/1000) s time base.
    *  Prescaler = (u32TimerClock/1000000 - 1) to have a 1MHz counter clock.
    *  ClockDivision = 0
    *  Counter direction = Up
    */
  stTimerHandle.Init.Period = (1000000 / 1000) - 1;
  stTimerHandle.Init.Prescaler = u32Prescaler;
  stTimerHandle.Init.ClockDivision = 0;

  stTimerHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
  eRetVal = (HAL_OK == HAL_TIM_Base_Init(&stTimerHandle))
            ?HAL_TIM_Base_Start_IT(&stTimerHandle)
            :HAL_ERROR;
  return eRetVal;
}

void HAL_SuspendTick(void)
{
  __HAL_TIM_DISABLE_IT(&stTimerHandle, TIM_IT_UPDATE);
}

void HAL_ResumeTick(void)
{
  __HAL_TIM_ENABLE_IT(&stTimerHandle, TIM_IT_UPDATE);
}

void TIM6_DAC_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&stTimerHandle);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *pstHandle)
{
  if(TIM6 == pstHandle->Instance)
  {
    HAL_IncTick();
  }
}