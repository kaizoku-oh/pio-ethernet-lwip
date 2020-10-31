#include "bsp_ethernet.h"
#include <string.h>

#define MAC_ADDR_0                               2U
#define MAC_ADDR_1                               0U
#define MAC_ADDR_2                               0U
#define MAC_ADDR_3                               0U
#define MAC_ADDR_4                               0U
#define MAC_ADDR_5                               0U
#define ETH_RX_BUFFERS_COUNT                     4U
#define ETH_TX_BUFFERS_COUNT                     4U
#define ETH_RX_BUFFER_SIZE                       ETH_MAX_PACKET_SIZE
#define ETH_TX_BUFFER_SIZE                       ETH_MAX_PACKET_SIZE

typedef struct
{
  /* Global Ethernet handle*/
  ETH_HandleTypeDef stEthernetHandle;
  /* Ethernet Rx DMA Descriptors list */
  ETH_DMADescTypeDef tstRxDMADescList[ETH_RX_BUFFERS_COUNT];
  /* Ethernet Tx DMA Descriptors list */
  ETH_DMADescTypeDef tstTxDMADescList[ETH_TX_BUFFERS_COUNT];
  /* Ethernet Receive Buffers: List of  */
  uint8_t tu08RxBuffer[ETH_RX_BUFFERS_COUNT][ETH_RX_BUFFER_SIZE];
  /* Ethernet Transmit Buffers */
  uint8_t tu08TxBuffer[ETH_TX_BUFFERS_COUNT][ETH_TX_BUFFER_SIZE];
  /* Ethernet Receive callback */
  bsp_ethernet_pf_recv_cb_t pf_cb_receive;
}bsp_ethernet_ctx_t;

/* Ethernet bsp context */
static bsp_ethernet_ctx_t stCtx;

/* Ethernet MAC address */
static uint8_t tu08MACAddress[6]= { MAC_ADDR_0,
                                    MAC_ADDR_1,
                                    MAC_ADDR_2,
                                    MAC_ADDR_3,
                                    MAC_ADDR_4,
                                    MAC_ADDR_5 };

void bsp_ethernet_init(void)
{
  stCtx.stEthernetHandle.Instance = ETH;
  stCtx.stEthernetHandle.Init.MACAddr = tu08MACAddress;
  stCtx.stEthernetHandle.Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
  stCtx.stEthernetHandle.Init.Speed = ETH_SPEED_100M;
  stCtx.stEthernetHandle.Init.DuplexMode = ETH_MODE_FULLDUPLEX;
  stCtx.stEthernetHandle.Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;
  stCtx.stEthernetHandle.Init.RxMode = ETH_RXINTERRUPT_MODE;
  stCtx.stEthernetHandle.Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
  stCtx.stEthernetHandle.Init.PhyAddress = 1;
  /* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */
  if(HAL_OK != HAL_ETH_Init(&stCtx.stEthernetHandle))
  {
    /* Set netif link flag */
  }
  /* Initialize Tx Descriptors list: Chain Mode */
  HAL_ETH_DMATxDescListInit(&stCtx.stEthernetHandle,
                            stCtx.tstTxDMADescList,
                            &stCtx.tu08TxBuffer[0][0],
                            ETH_RX_BUFFERS_COUNT);
  /* Initialize Rx Descriptors list: Chain Mode  */
  HAL_ETH_DMARxDescListInit(&stCtx.stEthernetHandle,
                            stCtx.tstRxDMADescList,
                            &stCtx.tu08RxBuffer[0][0],
                            ETH_RX_BUFFERS_COUNT);
  /* Enable MAC and DMA transmission and reception */
  if(HAL_OK != HAL_ETH_Start(&stCtx.stEthernetHandle))
  {
  }
}

void bsp_ethernet_register_cb(bsp_ethernet_pf_recv_cb_t pf_cb)
{
  stCtx.pf_cb_receive = pf_cb;
}

ETH_HandleTypeDef bsp_ethernet_get_handle(void)
{
  return stCtx.stEthernetHandle;
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
  HAL_ETH_IRQHandler(&stCtx.stEthernetHandle);
}

void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *pstHandle)
{
  stCtx.pf_cb_receive(pstHandle);
}
