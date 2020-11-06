#include "stm32f7xx_hal.h"

void BSP_RNG_Init(void);
uint32_t BSP_RNG_Generate(void);
RNG_HandleTypeDef BSP_RNG_Get_Handle(void);
void BSP_RNG_Exit(void);
