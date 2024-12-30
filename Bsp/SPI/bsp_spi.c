#include "bsp_spi.h"


/**
 * @brief  SPI1 发送 1 字节并接收 1 字节
 * @param  txData: 待发送的数据
 * @return rxData: 收到的数据
 */
uint8_t SPI1_ReadWriteByte(uint8_t txData)
{
    uint8_t rxData = 0;
    // 发送并接收一个字节
    // 第四个参数是超时时间，可根据实际需求设置
    if (HAL_SPI_TransmitReceive(&hspi1, &txData, &rxData, 1, 10) != HAL_OK) {
        // 超时或出错处理
        // 可根据需要加一些处理，比如置一个错误标志，或返回 0xFF
    }
    return rxData;
}

/* 如果需要，你也可以提供批量收发函数，例如：
uint8_t SPI1_TransmitReceive(uint8_t *txBuffer, uint8_t *rxBuffer, uint16_t size, uint32_t timeout)
{
    if (HAL_SPI_TransmitReceive(&hspi1, txBuffer, rxBuffer, size, timeout) != HAL_OK)
    {
        // 出错处理
        return 1;
    }
    return 0;
}
*/
