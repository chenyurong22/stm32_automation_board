#include "ethernet.h"
#include "main.h"

static ETH_HandleTypeDef heth;
static eth_rx_callback_t eth_rx_callback = NULL;
static uint8_t eth_link_up = 0;
static uint8_t eth_tx_buffer[ETH_TX_BUF_SIZE] __attribute__((__aligned__(4)));
static uint8_t eth_rx_buffer[ETH_RX_BUF_SIZE] __attribute__((__aligned__(4)));

void HAL_ETH_MspInit(ETH_HandleTypeDef *heth)
{
    (void)heth;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_ETH_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};

    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = GPIO_AF11_ETH;

    /* RMII_REF_CLK */
    gpio.Pin  = ETH_REF_CLK_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ETH_REF_CLK_PORT, &gpio);

    /* MDIO */
    gpio.Pin  = ETH_MDIO_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ETH_MDIO_PORT, &gpio);

    /* RMII_CRS_DV */
    gpio.Pin  = ETH_CRS_DV_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ETH_CRS_DV_PORT, &gpio);

    /* MDC */
    gpio.Pin  = ETH_MDC_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ETH_MDC_PORT, &gpio);

    /* RXD0 */
    gpio.Pin  = ETH_RXD0_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ETH_RXD0_PORT, &gpio);

    /* RXD1 */
    gpio.Pin  = ETH_RXD1_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ETH_RXD1_PORT, &gpio);

    /* TX_EN */
    gpio.Pin  = ETH_TX_EN_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ETH_TX_EN_PORT, &gpio);

    /* TXD0 */
    gpio.Pin  = ETH_TXD0_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ETH_TXD0_PORT, &gpio);

    /* TXD1 */
    gpio.Pin  = ETH_TXD1_PIN;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ETH_TXD1_PORT, &gpio);

    HAL_NVIC_SetPriority(ETH_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ETH_IRQn);
}

void ethernet_init(void)
{
    heth.Instance = ETH;
    heth.Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
    heth.Init.PhyAddress = ETH_PHY_ADDRESS;
    heth.Init.MACAddr[0] = MAC_ADDR0;
    heth.Init.MACAddr[1] = MAC_ADDR1;
    heth.Init.MACAddr[2] = MAC_ADDR2;
    heth.Init.MACAddr[3] = MAC_ADDR3;
    heth.Init.MACAddr[4] = MAC_ADDR4;
    heth.Init.MACAddr[5] = MAC_ADDR5;
    heth.Init.RxMode = ETH_RXINTERRUPT_MODE;
    heth.Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
    heth.Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;
    HAL_ETH_Init(&heth);

    eth_link_up = 1;
}

eth_status_t ethernet_send(uint8_t *data, uint16_t len)
{
    if (!eth_link_up) return ETH_LINK_DOWN;
    if (!data || len == 0 || len > ETH_TX_BUF_SIZE) return ETH_ERROR;

    for (uint16_t i = 0; i < len; i++) {
        eth_tx_buffer[i] = data[i];
    }

    HAL_StatusTypeDef status = HAL_ETH_TransmitFrame(&heth, len);
    if (status != HAL_OK) return ETH_ERROR;

    uint32_t timeout = 100000;
    while (timeout--) {
        if (__HAL_ETH_DESC_GET_FLAG(&heth, heth.TxDesc, ETH_DMATXDESC_OWN) == RESET) {
            break;
        }
    }
    return (timeout > 0) ? ETH_OK : ETH_ERROR;
}

eth_status_t ethernet_get_status(void)
{
    return eth_link_up ? ETH_OK : ETH_LINK_DOWN;
}

void ethernet_set_rx_callback(eth_rx_callback_t callback)
{
    eth_rx_callback = callback;
}

void ethernet_process(void)
{
    uint32_t frame_len = 0;
    uint8_t *frame = NULL;

    if (HAL_ETH_IsRxDataAvailable(&heth)) {
        HAL_ETH_GetRxDataBuffer(&heth, (uint8_t **)&frame);
        HAL_ETH_GetRxDataLength(&heth, &frame_len);
        if (frame && frame_len > 0 && eth_rx_callback) {
            eth_rx_callback(frame, (uint16_t)frame_len);
        }
        HAL_ETH_FlushRxData(&heth);
    }
}

uint8_t ethernet_is_link_up(void)
{
    return eth_link_up;
}

void ETH_IRQHandler(void)
{
    HAL_ETH_IRQHandler(&heth);
}

void ETH_WKUP_IRQHandler(void)
{
    HAL_ETH_IRQHandler(&heth);
}
