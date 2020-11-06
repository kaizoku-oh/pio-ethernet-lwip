#include "bsp_rng.h"

RNG_HandleTypeDef *pstRngHandle;

void BSP_RNG_Init(void)
{
  HAL_RNG_Init(pstRngHandle);
}

uint32_t BSP_RNG_Generate(void)
{
  uint32_t u32Random;

  HAL_RNG_GenerateRandomNumber(pstRngHandle, &u32Random);
  return u32Random;
}

RNG_HandleTypeDef BSP_RNG_Get_Handle(void)
{
  return *pstRngHandle;
}

void BSP_RNG_Exit(void)
{
  HAL_RNG_DeInit(pstRngHandle);
}

void HAL_RNG_MspInit(RNG_HandleTypeDef *stHandle)
{
  __HAL_RCC_RNG_CLK_ENABLE();
}

void HAL_RNG_MspDeInit(RNG_HandleTypeDef *stHandle)
{
  __HAL_RCC_RNG_CLK_DISABLE();
}
