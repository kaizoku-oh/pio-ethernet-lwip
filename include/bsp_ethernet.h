#include "stm32f7xx_hal.h"

typedef void (*bsp_ethernet_pf_recv_cb_t)(ETH_HandleTypeDef *pstHandle);

void bsp_ethernet_init(void);
void bsp_ethernet_register_cb(bsp_ethernet_pf_recv_cb_t pf_cb);