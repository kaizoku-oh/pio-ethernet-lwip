#include "stm32f7xx_hal.h"

/* Global Ethernet handle*/
ETH_HandleTypeDef stEthernetHandle;
/* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef tstRxDMADesc[ETH_RXBUFNB] __attribute__((section(".tstRxDMADescSection")));
/* Ethernet Tx DMA Descriptors */
ETH_DMADescTypeDef tstTxDMADesc[ETH_TXBUFNB] __attribute__((section(".tstTxDMADescSection")));
/* Ethernet Receive Buffers */
uint8_t tu08RxBuffer[ETH_RXBUFNB][ETH_RX_BUF_SIZE] __attribute__((section(".tu08RxBufferSection"))); 
/* Ethernet Transmit Buffers */
uint8_t tu08TxBuffer[ETH_TXBUFNB][ETH_TX_BUF_SIZE] __attribute__((section(".tu08TxBufferSection"))); 
/* Ethernet MAC address */
uint8_t tu08MACAddress[6]= { MAC_ADDR0, MAC_ADDR1, MAC_ADDR2, MAC_ADDR3, MAC_ADDR4, MAC_ADDR5 };

void bsp_ethernet_init(void)
{
  stEthernetHandle.Instance = ETH;  
  stEthernetHandle.Init.MACAddr = tu08MACAddress;
  stEthernetHandle.Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
  stEthernetHandle.Init.Speed = ETH_SPEED_100M;
  stEthernetHandle.Init.DuplexMode = ETH_MODE_FULLDUPLEX;
  stEthernetHandle.Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;
  stEthernetHandle.Init.RxMode = ETH_RXINTERRUPT_MODE;
  stEthernetHandle.Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
  stEthernetHandle.Init.PhyAddress = 1;
  /* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */
  if(HAL_OK == HAL_ETH_Init(&stEthernetHandle))
  {
    /* Set netif link flag */
  }
  /* Initialize Tx Descriptors list: Chain Mode */
  HAL_ETH_DMATxDescListInit(&stEthernetHandle, tstTxDMADesc, &tu08TxBuffer[0][0], ETH_TXBUFNB);
  /* Initialize Rx Descriptors list: Chain Mode  */
  HAL_ETH_DMARxDescListInit(&stEthernetHandle, tstRxDMADesc, &tu08RxBuffer[0][0], ETH_RXBUFNB);
  /* Enable MAC and DMA transmission and reception */
  HAL_ETH_Start(&stEthernetHandle);
}

void HAL_ETH_MspInit(ETH_HandleTypeDef *pstHandle)
{
  GPIO_InitTypeDef stGPIO;

  /* Enable GPIOs clocks */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  /* Configure PA1, PA2 and PA7 */
  stGPIO.Speed = GPIO_SPEED_HIGH;
  stGPIO.Mode = GPIO_MODE_AF_PP;
  stGPIO.Pull = GPIO_NOPULL; 
  stGPIO.Alternate = GPIO_AF11_ETH;
  stGPIO.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
  HAL_GPIO_Init(GPIOA, &stGPIO);
  /* Configure PB13 */
  stGPIO.Pin = GPIO_PIN_13;
  HAL_GPIO_Init(GPIOB, &stGPIO);
  /* Configure PC1, PC4 and PC5 */
  stGPIO.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
  HAL_GPIO_Init(GPIOC, &stGPIO);
  /* Configure PG2, PG11, PG13 and PG14 */
  stGPIO.Pin =  GPIO_PIN_2 | GPIO_PIN_11 | GPIO_PIN_13;
  HAL_GPIO_Init(GPIOG, &stGPIO);
  /* Enable the Ethernet global Interrupt */
  HAL_NVIC_SetPriority(ETH_IRQn, 0x7, 0);
  HAL_NVIC_EnableIRQ(ETH_IRQn);
  /* Enable ETHERNET clock  */
  __HAL_RCC_ETH_CLK_ENABLE();
  /* Output HSE clock (25MHz) on MCO pin (PA8) to clock the PHY */
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_PLLCLK, RCC_MCODIV_4);
}

void ETH_IRQHandler(void)
{
  HAL_ETH_IRQHandler(&stEthernetHandle);
}

void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *pstHandle)
{
}
