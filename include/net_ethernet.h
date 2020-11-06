#include "stm32f7xx_hal.h"

typedef void (*bsp_ethernet_pf_recv_cb_t)(ETH_HandleTypeDef *pstHandle);

HAL_StatusTypeDef BSP_ETHERNET_Init(void);
void BSP_ETHERNET_Register_Cb(bsp_ethernet_pf_recv_cb_t pfCb);
ETH_HandleTypeDef BSP_ETHERNET_Get_Handle(void);

err_t net_ethernet_init(struct netif *pstNet);
void net_ethernet_input(void const *pvArg);
