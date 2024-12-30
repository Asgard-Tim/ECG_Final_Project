/**
 ******************************************************************************
 * @file    bsp_ads1292_hal.c
 * @author
 * @version V1.0
 * @date
 * @brief   ADS1292 ECG 模块驱动 - HAL 源文件
 ******************************************************************************
 */

#include "ADS1292R.h"
#include "bsp_spi.h" // 确保你有一个基于 HAL 的 SPI 驱动
#include "bsp_dwt.h" // 如果你有自己的微秒级延时，否则可用 HAL_Delay()

//========================== 全局变量 =========================//

// 寄存器数组
uint8_t ADS1292_REG[12];
static ADS1292_Instance *ads1292_instance;

// 各种配置结构体，根据你的需求初始化
ADS1292_CONFIG1 Ads1292_Config1     = {DATA_RATE_250SPS};
ADS1292_CONFIG2 Ads1292_Config2     = {PDB_LOFF_COMP_ON, PDB_REFBUF_ON, VREF_242V, CLK_EN_OFF, INT_TEST_OFF};
ADS1292_CHSET Ads1292_Ch1set        = {PD_ON, GAIN_6, MUX_Normal_input};
ADS1292_CHSET Ads1292_Ch2set        = {PD_ON, GAIN_6, MUX_Normal_input};
ADS1292_RLD_SENS Ads1292_Rld_Sens   = {PDB_RLD_ON, RLD_LOFF_SENSE_ON, RLD_CANNLE_ON, RLD_CANNLE_ON, RLD_CANNLE_OFF, RLD_CANNLE_OFF};
ADS1292_LOFF_SENS Ads1292_Loff_Sens = {FLIP2_OFF, FLIP1_OFF, LOFF_CANNLE_ON, LOFF_CANNLE_ON, LOFF_CANNLE_ON, LOFF_CANNLE_ON};
ADS1292_RESP1 Ads1292_Resp1         = {RESP_DEMOD_ON, RESP_MOD_ON, 0x0D, RESP_CTRL_CLOCK_INTERNAL};
ADS1292_RESP2 Ads1292_Resp2         = {CALIB_OFF, FREQ_32K, RLDREF_INT_INTERNALLY};

//========================== 函数实现 =========================//

void ADS1292Update(GPIO_Instance *gpio)
{
    ADS1292_Instance *ads1292 = (ADS1292_Instance *)gpio->id;
    ads1292->is_collecting    = 1;
}

ADS1292_Instance *ADS1292Register(ADS1292_Config_s *config)
{
    ADS1292_Instance *ads1292 = (ADS1292_Instance *)malloc(sizeof(ADS1292_Instance));
    memset(ads1292, 0, sizeof(ADS1292_Instance));

    // 初始化GPIO
    config->gpio_config.gpio_model_callback = ADS1292Update;
    config->gpio_config.id                  = ads1292;
    ads1292->gpio                           = GPIORegister(&config->gpio_config);

    return ads1292;
}

/**
 * @brief  ADS1292 相关 GPIO、SPI 以及内部上电复位流程
 * @note   在此函数中初始化 SPI, 并做最初步的复位配置
 */
ADS1292_Instance *ADS1292_Init(void)
{
    // 1) 你需要先初始化 SPI 硬件（根据项目情况而定）。
    // MX_SPI1_Init();

    // 2) 其余 GPIO（CS、RESET、START、CLKSEL、DRDY 中断等），
    // 注册DRDY引脚中断
    ADS1292_Config_s ads1292_config                = {0};
    ads1292_config.gpio_config.GPIOx               = ADS_DRDY_GPIO_Port;
    ads1292_config.gpio_config.GPIO_Pin            = ADS_DRDY_Pin;
    ads1292_config.gpio_config.exti_mode           = GPIO_EXTI_MODE_RISING_FALLING;
    ads1292_config.gpio_config.gpio_model_callback = ADS1292Update;

    ads1292_instance = ADS1292Register(&ads1292_config);

    // 3) 拉高 CS，避免干扰
    ADS_CS_HIGH();
    ADS_START_HIGH();
    ADS_RESET_LOW();

    // 4) 上电复位
    ADS1292_PowerOnInit();

    // while (Set_ADS1292_Collect(0)) {
    //     /* code */
    // }
    // ADS1292R_CMD(ADS1292R_RDATAC); // 回到连续读取数据模式，检测噪声数据
    ADS1292_Send_CMD(RDATAC); // 回到连续读取数据模式，检测噪声数据
    ADS_START_HIGH();
    // ADS1292R_START_H;              // 启动转换

    return ads1292_instance;
}

uint8_t ADS1292R_SPI_RW(uint8_t data)
{
    uint8_t TxData = data;
    uint8_t RxData = 0;
    HAL_SPI_TransmitReceive(&hspi1, &TxData, &RxData, 1, 10); //	HAL_SPI_TransmitReceive(&SPI3_Handler, &data, &rx_data, 1, 10);
    return RxData;
}

// 对ADS1292R内部寄存器进行操作
uint8_t ADS1292R_REG_2(uint8_t cmd, uint8_t data) // 只读一个数据
{
    ADS_CS_LOW();
    DWT_Delay_us(100);
    ADS1292R_SPI_RW(cmd);             // 读写指令
    ADS1292R_SPI_RW(0X00);            // 需要写几个数据（n+1个）
    if ((cmd & 0x20) == 0x20)         // 判断是否为读寄存器指令
        return ADS1292R_SPI_RW(0X00); // 返回寄存器值
    else
        ADS1292R_SPI_RW(data); // 写入数值
    DWT_Delay_us(100);
    ADS_CS_HIGH();

    return 0;
}

/**
 * @brief  上电复位初始化：内部时钟、复位时序、写寄存器初始值等
 */
void ADS1292_PowerOnInit(void)
{
    ADS_START_LOW();
    ADS_CS_HIGH();
    ADS_RESET_LOW();
    ADS_RESET_HIGH();
    DWT_Delay_ms(100); // 等待稳定
    // ADS1292R_CMD(ADS1292R_SDATAC); // 发送停止连续读取数据命令
    ADS1292_Send_CMD(SDATAC);
    DWT_Delay_ms(100);
    // ADS1292R_CMD(ADS1292R_RESET); // 发送复位命令
    ADS1292_Send_CMD(RESET);
    DWT_Delay_ms(100);
    // ADS1292R_CMD(ADS1292R_SDATAC); // 发送停止连续读取数据命令
    ADS1292_Send_CMD(SDATAC);
    DWT_Delay_ms(100);

    while (ads1292_instance->device_id != 0x73 && ads1292_instance->device_id != 0x53) // 识别芯片型号，1292为0x53，也就是这里的十进制数83  1292r为0x73，即115
    {
        // ads1292_instance->device_id = ADS1292R_REG(ADS1292R_RREG | ADS1292R_ID, 0x00);
        ads1292_instance->device_id = ADS1292_ReadDeviceID();

        DWT_Delay_ms(100);
    }
    printf("Correct ID:%d\n",ads1292_instance->device_id);

    ADS1292R_REG_2(WREG | CONFIG2, 0xa3); // 使用内部参考电压
    // ADS1292R_REG(ADS1292R_WREG|ADS1292R_CONFIG2,    0xa3);//使用测试信号
    DWT_Delay_ms(10);                     // 等待内部参考电压稳定
    ADS1292R_REG_2(WREG | CONFIG1, 0x02); // 设置转换速率为500SPS
    ADS1292R_REG_2(WREG | CH1SET, 0x00);
    // ADS1292R_REG(ADS1292R_WREG|ADS1292R_CH1SET,     0x05);//采集测试信号（方波）
    ADS1292R_REG_2(WREG | CH2SET, 0x05);
    ADS1292R_REG_2(WREG | RLD_SENS, 0x2c); // green 0x2c
    ADS1292R_REG_2(WREG | RESP1, 0x02);    // 0xea
    ADS1292R_REG_2(WREG | RESP2, 0x03);    // 0x03

    DWT_Delay_ms(100);
}

/**
 * @brief  SPI 读写一个字节，并返回从 ADS1292 读到的字节
 */
uint8_t ADS1292_SPI(uint8_t com)
{
    // 这里的 SPI1_ReadWriteByte() 由你自己的 bsp_spi.c 提供
    // 内部调用 HAL_SPI_TransmitReceive(&hspi1, ...) 等
    return SPI1_ReadWriteByte(com);
}

/**
 * @brief  发送一条命令字节
 */
void ADS1292_Send_CMD(uint8_t cmd)
{
    ADS_CS_LOW();
    // 多字节命令之间需要一定延时(>~8us)，这里预留
    DWT_Delay_us(100);
    ADS1292_SPI(cmd);
    DWT_Delay_us(100);
    ADS_CS_HIGH();
}

/**
 * @brief  读写多个寄存器
 * @param  reg: RREG/WREG + 起始寄存器地址
 * @param  len: 读或写的寄存器个数
 * @param  data: 数据指针
 */
void ADS1292_WR_REGS(uint8_t reg, uint8_t len, uint8_t *data)
{
    uint8_t i;
    ADS_CS_LOW();
    DWT_Delay_us(100);

    ADS1292_SPI(reg);
    DWT_Delay_us(100);

    // length - 1
    ADS1292_SPI(len - 1);

    // 写寄存器
    if (reg & 0x40) {
        for (i = 0; i < len; i++) {
            DWT_Delay_us(100);
            ADS1292_SPI(*data++);
        }
    }
    // 读寄存器
    else {
        for (i = 0; i < len; i++) {
            DWT_Delay_us(100);
            *data++ = ADS1292_SPI(0xFF);
        }
    }

    DWT_Delay_us(100);
    ADS_CS_HIGH();
}

/**
 * @brief  读取 9 个字节(状态 + CH1 + CH2)
 * @param  data: 数据保存缓冲区 (至少 9 字节)
 */
uint8_t ADS1292_Read_Data(uint8_t *data)
{
    uint8_t i;

    ADS_CS_LOW();
    DWT_Delay_us(10);

    for (i = 0; i < 9; i++) {
        *data++ = ADS1292_SPI(0x00);
    }

    DWT_Delay_us(10);
    ADS_CS_HIGH();

    return 0;
}

/**
 * @brief  开启 DRDY 中断（或恢复中断）
 */
void ADS1292_Recv_Start(void)
{
    // 如果使用 HAL 库中断，可以启用 NVIC
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/**
 * @brief  设置全局寄存器数组
 */
void ADS1292_SET_REGBUFF(void)
{

    // ID 只读，此处无须设置
    ADS1292_REG[ID] = 0x73; // 根据第一份代码示例，设备 ID 为 0x73

    // CONFIG1: 设置数据速率 500 SPS
    ADS1292_REG[CONFIG1] = 0x02;

    // CONFIG2: 启用内部参考电压
    ADS1292_REG[CONFIG2] = 0xA0;

    // LOFF: 导联断开检测 (本例固定设置成 0xF0，可根据需求修改)
    // ADS1292_REG[LOFF] = 0xF0;

    // CH1SET: 通道 1 配置
    ADS1292_REG[CH1SET] = 0x00; // 通道 1，输入为正常模式

    // CH2SET: 通道 2 配置
    ADS1292_REG[CH2SET] = 0x00; // 通道 2，输入为正常模式

    // RLD_SENS: 右腿驱动检测和使能
    ADS1292_REG[RLD_SENS] = 0x2c;

    // LOFF_SENS: 导联断开检测敏感度配置
    // ADS1292_REG[LOFF_SENS] = 0x3F;

    // RESP1: 呼吸阻抗检测配置
    ADS1292_REG[RESP1] = 0x02;

    // RESP2: 呼吸阻抗检测配置
    ADS1292_REG[RESP2] = 0x03;

    // GPIO: 默认配置 (本例未使用 GPIO，保持默认)
    ADS1292_REG[GPIO] = 0x0C;
}

/**
 * @brief  将寄存器数组写入芯片，再读回校验
 * @retval 0 表示成功，1 表示失败
 */
uint8_t ADS1292_WRITE_REGBUFF(void)
{
    uint8_t i, res = 0;
    uint8_t REG_Cache[12];

    ADS1292_SET_REGBUFF(); // 设置本地寄存器数组

    // 写入除 ID(0x00) 以外的 11 个寄存器
    ADS1292_WR_REGS(WREG | CONFIG1, 11, ADS1292_REG + 1);
    DWT_Delay_ms(10);

    // 读回 12 个寄存器
    ADS1292_WR_REGS(RREG | ID, 12, REG_Cache);
    DWT_Delay_ms(10);

    // 校验
    for (i = 0; i < 12; i++) {
        if (ADS1292_REG[i] != REG_Cache[i]) {
            // 0(ID)、8(LOFF_STAT)、11(GPIO) 可能与写入不一致，不做严格校验
            if (i != 0 && i != 8 && i != 11) {
                res = 1;
            }
        }
    }

    return res;
}

/**
 * @brief  设置通道 1/2 为内部测试信号输入
 */
uint8_t ADS1292_Single_Test(void)
{
    uint8_t res              = 0;
    Ads1292_Config2.Int_Test = INT_TEST_ON;
    Ads1292_Ch1set.MUX       = MUX_Test_signal;
    Ads1292_Ch2set.MUX       = MUX_Test_signal;

    if (ADS1292_WRITE_REGBUFF())
        res = 1;

    DWT_Delay_ms(10);
    return res;
}

/**
 * @brief  设置内部噪声测试
 */
uint8_t ADS1292_Noise_Test(void)
{
    uint8_t res              = 0;
    Ads1292_Config2.Int_Test = INT_TEST_OFF;
    Ads1292_Ch1set.MUX       = MUX_input_shorted;
    Ads1292_Ch2set.MUX       = MUX_input_shorted;

    if (ADS1292_WRITE_REGBUFF())
        res = 1;

    DWT_Delay_ms(10);
    return res;
}

/**
 * @brief  设置正常信号采集模式
 */
uint8_t ADS1292_Single_Read(void)
{
    uint8_t res              = 0;
    Ads1292_Config2.Int_Test = INT_TEST_OFF;
    Ads1292_Ch1set.MUX       = MUX_Normal_input;
    Ads1292_Ch2set.MUX       = MUX_Normal_input;

    if (ADS1292_WRITE_REGBUFF())
        res = 1;

    DWT_Delay_ms(10);
    return res;
}

/**
 * @brief  配置 ADS1292 的采集方式 (0=正常，1=测试，2=噪声)
 */
uint8_t Set_ADS1292_Collect(uint8_t mode)
{
    uint8_t res = 0;

    DWT_Delay_ms(10);
    switch (mode) {
        case 0:
            res = ADS1292_Single_Read();
            break;
        case 1:
            res = ADS1292_Single_Test();
            break;
        case 2:
            res = ADS1292_Noise_Test();
            break;
        default:
            res = 1;
            break;
    }

    if (res) return 1; // 配置寄存器失败

    // 开始转换
    ADS1292_Send_CMD(START);
    DWT_Delay_ms(10);

    // 连续读取模式
    ADS1292_Send_CMD(RDATAC);
    DWT_Delay_ms(10);

    return 0;
}

/**
 * @brief  读取设备 ID (寄存器 0x00)
 * @note   需要先发送 SDATAC 停止连续输出
 * @retval 读到的设备 ID
 */
uint8_t ADS1292_ReadDeviceID(void)
{
    uint8_t device_id;

    // 1) 停止连续输出
    ADS1292_Send_CMD(SDATAC);
    DWT_Delay_ms(2);

    // 2) 拉低 CS，发送 RREG 命令
    ADS_CS_LOW();
    DWT_Delay_us(10);

    ADS1292_SPI(RREG | 0x00); // RREG + 地址=0
    ADS1292_SPI(0x00);        // 读取 1 个寄存器 => length - 1 = 0

    // 3) 读取
    device_id = ADS1292_SPI(0xFF);
    DWT_Delay_us(10);

    ADS_CS_HIGH();
    DWT_Delay_ms(2);

    return device_id;
}
