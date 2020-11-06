#include "net_ethernet.h"
#include "cmsis_os.h"
#include "lwip/opt.h"
#include "lwip/timeouts.h"
#include "lwip/tcpip.h"
#include "netif/ethernet.h"
#include "netif/etharp.h"
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
#define NET_IF_NAME_0                            'B'
#define NET_IF_NAME_1                            'G'

static void net_ethernet_input(void const *pvArg);

typedef struct
{
  /* Global Ethernet handle*/
  ETH_HandleTypeDef stEthernetHandle;
  /* Semaphore to signal incoming packets */
  osSemaphoreId stSemaphore;
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
}net_ethernet_ctx_t;

/* Ethernet bsp context */
static net_ethernet_ctx_t stCtx;

/* Ethernet MAC address */
static uint8_t tu08MACAddress[6]= { MAC_ADDR_0,
                                    MAC_ADDR_1,
                                    MAC_ADDR_2,
                                    MAC_ADDR_3,
                                    MAC_ADDR_4,
                                    MAC_ADDR_5 };

HAL_StatusTypeDef BSP_ETHERNET_Init(void)
{
  HAL_StatusTypeDef eRetVal;
  uint32_t u32RegVal;

  u32RegVal = 0;
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
  eRetVal |= HAL_ETH_Init(&stCtx.stEthernetHandle);
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
  eRetVal |= HAL_ETH_Start(&stCtx.stEthernetHandle);
  /* Read Register Configuration */
  HAL_ETH_ReadPHYRegister(&stCtx.stEthernetHandle, PHY_MICR, &u32RegVal);
  u32RegVal |= (PHY_MICR_INT_EN | PHY_MICR_INT_OE);
  /* Enable Interrupts */
  HAL_ETH_WritePHYRegister(&stCtx.stEthernetHandle, PHY_MICR, u32RegVal);
  /* Read Register Configuration */
  HAL_ETH_ReadPHYRegister(&stCtx.stEthernetHandle, PHY_MISR, &u32RegVal);
  u32RegVal |= PHY_MISR_LINK_INT_EN;
  /* Enable Interrupt on change of link status */
  HAL_ETH_WritePHYRegister(&stCtx.stEthernetHandle, PHY_MISR, u32RegVal);
}

void BSP_ETHERNET_Register_Cb(bsp_ethernet_pf_recv_cb_t pfCb)
{
  stCtx.pf_cb_receive = pfCb;
}

ETH_HandleTypeDef BSP_ETHERNET_Get_Handle(void)
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
  // stCtx.pf_cb_receive(pstHandle);
  osSemaphoreRelease(stCtx.stSemaphore);
}

static err_t BSP_ETHERNET_Transmit(struct netif *pstNet, struct pbuf *pstPacketBuff)
{
  err_t s08RetVal;
  struct pbuf *pstPBuff;
  uint8_t *pu08Buffer;
  __IO ETH_DMADescTypeDef *pstDmaDesc;
  uint32_t u32FrameLen;
  uint32_t u32BufferOffset;
  uint32_t u32BytesLeft;
  uint32_t u32PayloadOffset;

  *pu08Buffer = (uint8_t *)(stCtx.stEthernetHandle.TxDesc->Buffer1Addr);
  pstDmaDesc = stCtx.stEthernetHandle.TxDesc;
  u32BufferOffset = 0;
  u32FrameLen = 0;
  u32BufferOffset = 0;
  u32BytesLeft = 0;
  u32PayloadOffset = 0;
  
  /* copy frame from pbufs to driver buffers */
  for(pstPBuff=pstPacketBuff; pstPBuff!=NULL; pstPBuff=pstPBuff->next)
  {
    /* Is this buffer available? If not, goto error */
    if((pstDmaDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET)
    {
      s08RetVal = ERR_USE;
      goto error;
    }
    /* Get bytes in current lwIP buffer */
    u32BytesLeft = pstPBuff->len;
    u32PayloadOffset = 0;
    /* Check if the length of data to copy is bigger than Tx buffer size*/
    while( (u32BytesLeft + u32BufferOffset) > ETH_TX_BUF_SIZE )
    {
      /* Copy data to Tx buffer*/
      memcpy((uint8_t*)((uint8_t*)pu08Buffer + u32BufferOffset),
             (uint8_t*)((uint8_t*)pstPBuff->payload + u32PayloadOffset),
             (ETH_TX_BUF_SIZE - u32BufferOffset));
      /* Point to next descriptor */
      pstDmaDesc = (ETH_DMADescTypeDef *)(pstDmaDesc->Buffer2NextDescAddr);
      /* Check if the buffer is available */
      if((pstDmaDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET)
      {
        s08RetVal = ERR_USE;
        goto error;
      }
      pu08Buffer = (uint8_t *)(pstDmaDesc->Buffer1Addr);
      u32BytesLeft = u32BytesLeft - (ETH_TX_BUF_SIZE - u32BufferOffset);
      u32PayloadOffset = u32PayloadOffset + (ETH_TX_BUF_SIZE - u32BufferOffset);
      u32FrameLen = u32FrameLen + (ETH_TX_BUF_SIZE - u32BufferOffset);
      u32BufferOffset = 0;
    }
    /* Copy the remaining bytes */
    memcpy((uint8_t*)((uint8_t*)pu08Buffer + u32BufferOffset),
           (uint8_t*)((uint8_t*)pstPBuff->payload + u32PayloadOffset),
           u32BytesLeft);
    u32BufferOffset = u32BufferOffset + u32BytesLeft;
    u32FrameLen = u32FrameLen + u32BytesLeft;
  }
  /* Prepare transmit descriptors to give to DMA */ 
  HAL_ETH_TransmitFrame(&stCtx.stEthernetHandle, u32FrameLen);
  s08RetVal = ERR_OK;
error:
  /* When Transmit Underflow flag is set, clear it and issue a Transmit Poll Demand to resume transmission */
  if((stCtx.stEthernetHandle.Instance->DMASR & ETH_DMASR_TUS) != (uint32_t)RESET)
  {
    /* Clear TUS ETHERNET DMA flag */
    stCtx.stEthernetHandle.Instance->DMASR = ETH_DMASR_TUS;
    
    /* Resume DMA transmission*/
    stCtx.stEthernetHandle.Instance->DMATPDR = 0;
  }
  return s08RetVal;
}

static struct pbuf *BSP_ETHERNET_Receive(struct netif *pstNet)
{
  __IO ETH_DMADescTypeDef *pstDmaRxDesc;
  struct pbuf *pstPacketBuff;
  struct pbuf *pstPacket;
  uint16_t u16FrameLen;
  uint8_t *pu08Buffer;
  uint32_t u32BufferOffset;
  uint32_t u32PayloadOffset;
  uint32_t u32BytesLeft;
  uint32_t u32Index;

  pstPacketBuff = NULL;
  pstPacket = NULL;
  u16FrameLen = 0;
  u32BufferOffset = 0;
  u32PayloadOffset = 0;
  u32BytesLeft = 0;
  u32Index=0;
  /* get received frame */
  if(HAL_OK != HAL_ETH_GetReceivedFrame_IT(&stCtx.stEthernetHandle))
  {
    return NULL;
  }
  /* Obtain the size of the packet and put it into the "u16FrameLen" variable. */
  u16FrameLen = stCtx.stEthernetHandle.RxFrameInfos.length;
  pu08Buffer = (uint8_t *)stCtx.stEthernetHandle.RxFrameInfos.buffer;
  if(u16FrameLen > 0)
  {
    /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
    pstPacketBuff = pbuf_alloc(PBUF_RAW, u16FrameLen, PBUF_POOL);
  } 
  if(pstPacketBuff != NULL)
  {
    pstDmaRxDesc = stCtx.stEthernetHandle.RxFrameInfos.FSRxDesc;
    u32BufferOffset = 0;
    for(pstPacket = pstPacketBuff; pstPacket != NULL; pstPacket = pstPacket->next)
    {
      u32BytesLeft = pstPacket->len;
      u32PayloadOffset = 0;
      /* Check if the length of bytes to copy in current pbuf is bigger than Rx buffer size */
      while( (u32BytesLeft + u32BufferOffset) > ETH_RX_BUF_SIZE )
      {
        /* Copy data to pbuf */
        memcpy((uint8_t*)((uint8_t*)pstPacket->payload + u32PayloadOffset),
               (uint8_t*)((uint8_t*)pu08Buffer + u32BufferOffset),
               (ETH_RX_BUF_SIZE - u32BufferOffset));
        /* Point to next descriptor */
        pstDmaRxDesc = (ETH_DMADescTypeDef *)(pstDmaRxDesc->Buffer2NextDescAddr);
        pu08Buffer = (uint8_t *)(pstDmaRxDesc->Buffer1Addr);
        
        u32BytesLeft = u32BytesLeft - (ETH_RX_BUF_SIZE - u32BufferOffset);
        u32PayloadOffset = u32PayloadOffset + (ETH_RX_BUF_SIZE - u32BufferOffset);
        u32BufferOffset = 0;
      }
      
      /* Copy remaining data in pbuf */
      memcpy((uint8_t*)((uint8_t*)pstPacket->payload + u32PayloadOffset),
             (uint8_t*)((uint8_t*)pu08Buffer + u32BufferOffset),
             u32BytesLeft);
      u32BufferOffset = u32BufferOffset + u32BytesLeft;
    }
  }
  /* Point to first descriptor */
  pstDmaRxDesc = stCtx.stEthernetHandle.RxFrameInfos.FSRxDesc;
  /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
  for (u32Index=0; u32Index< stCtx.stEthernetHandle.RxFrameInfos.SegCount; u32Index++)
  {  
    pstDmaRxDesc->Status |= ETH_DMARXDESC_OWN;
    pstDmaRxDesc = (ETH_DMADescTypeDef *)(pstDmaRxDesc->Buffer2NextDescAddr);
  }
  /* Clear Segment_Count */
  stCtx.stEthernetHandle.RxFrameInfos.SegCount = 0;
  /* When Rx Buffer unavailable flag is set: clear it and resume reception */
  if((stCtx.stEthernetHandle.Instance->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET)  
  {
    /* Clear RBUS ETHERNET DMA flag */
    stCtx.stEthernetHandle.Instance->DMASR = ETH_DMASR_RBUS;
    /* Resume DMA reception */
    stCtx.stEthernetHandle.Instance->DMARPDR = 0;
  }
  return pstPacketBuff;
}


err_t net_ethernet_init(struct netif *pstNet)
{
  err_t eRetVal;

  if(pstNet)
  {
    pstNet->name[0] = NET_IF_NAME_0;
    pstNet->name[1] = NET_IF_NAME_0;

    pstNet->output = etharp_output;
    pstNet->linkoutput = BSP_ETHERNET_Transmit;
    
    pstNet->hwaddr_len = ETH_HWADDR_LEN;

    pstNet->hwaddr[0] =  MAC_ADDR_0;
    pstNet->hwaddr[1] =  MAC_ADDR_1;
    pstNet->hwaddr[2] =  MAC_ADDR_2;
    pstNet->hwaddr[3] =  MAC_ADDR_3;
    pstNet->hwaddr[4] =  MAC_ADDR_4;
    pstNet->hwaddr[5] =  MAC_ADDR_5;

    pstNet->mtu = 1500;

    pstNet->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

    if(HAL_OK == BSP_ETHERNET_Init())
    {
      pstNet->flags |= NETIF_FLAG_LINK_UP;
    }

    osSemaphoreDef(SEM);
    stCtx.stSemaphore = osSemaphoreCreate(osSemaphore(SEM) , 1);

    osThreadDef(net, net_ethernet_input, osPriorityRealtime, 0, 350);
    osThreadCreate(osThread(net), pstNet);

    eRetVal = ERR_OK;
  }
  else
  {
    eRetVal = ERR_ARG;
  }
  return eRetVal
}

void net_ethernet_input(void const *pvArg)
{
  struct pbuf *pstPacketBuff;
  struct netif *stNet;

  stNet = (struct netif *)pvArg;
  while(1)
  {
    if(osOK == osSemaphoreWait(stCtx.stSemaphore, osWaitForever))
    {
      do
      {
        LOCK_TCPIP_CORE();
        pstPacketBuff = low_level_input(stNet);
        if(pstPacketBuff != NULL)
        {
          if(ERR_OK != stNet->input(pstPacketBuff, stNet))
          {
            pbuf_free(pstPacketBuff);
          }
        }
        UNLOCK_TCPIP_CORE();
      }while(pstPacketBuff!=NULL);
    }
  }
}

