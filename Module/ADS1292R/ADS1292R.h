/**
 ******************************************************************************
 * @file    bsp_ads1292_hal.h
 * @author
 * @version V1.0
 * @date
 * @brief   ADS1292 ECG 模块驱动 - HAL 头文件
 ******************************************************************************
 */

#ifndef ADS1292R_H
#define ADS1292R_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h" // 这里包含 HAL 相关的头文件，如 stm32f4xx_hal.h 等
// #include "bsp_loop_buffer.h"
#include "stdlib.h"
#include "bsp_gpio.h"
#include "memory.h"

//========================== 引脚操作宏定义（请根据实际硬件修改） =========================//

/*
   请在 CubeMX 或手动在 main.h 等文件里定义类似：

   #define ADS_RESET_Pin       GPIO_PIN_6
   #define ADS_RESET_GPIO_Port GPIOC
   #define ADS_START_Pin       GPIO_PIN_7
   #define ADS_START_GPIO_Port GPIOC
   #define ADS_DRDY_Pin        GPIO_PIN_5
   #define ADS_DRDY_GPIO_Port  GPIOC
   #define ADS_CS_Pin          GPIO_PIN_9
   #define ADS_CS_GPIO_Port    GPIOC
   #define ADS_CLKSEL_Pin      GPIO_PIN_8
   #define ADS_CLKSEL_GPIO_Port GPIOC

   然后在这里通过宏来简化操作
*/

#define ADS_RESET_Pin        GPIO_PIN_6
#define ADS_RESET_GPIO_Port  GPIOC
#define ADS_START_Pin        GPIO_PIN_7
#define ADS_START_GPIO_Port  GPIOC
#define ADS_DRDY_Pin         GPIO_PIN_0
#define ADS_DRDY_GPIO_Port   GPIOA
#define ADS_CS_Pin           GPIO_PIN_9
#define ADS_CS_GPIO_Port     GPIOC
#define ADS_CLKSEL_Pin       GPIO_PIN_8
#define ADS_CLKSEL_GPIO_Port GPIOC

#define ADS_RESET_HIGH()     HAL_GPIO_WritePin(ADS_RESET_GPIO_Port, ADS_RESET_Pin, GPIO_PIN_SET)
#define ADS_RESET_LOW()      HAL_GPIO_WritePin(ADS_RESET_GPIO_Port, ADS_RESET_Pin, GPIO_PIN_RESET)

#define ADS_START_HIGH()     HAL_GPIO_WritePin(ADS_START_GPIO_Port, ADS_START_Pin, GPIO_PIN_SET)
#define ADS_START_LOW()      HAL_GPIO_WritePin(ADS_START_GPIO_Port, ADS_START_Pin, GPIO_PIN_RESET)

#define ADS_CS_HIGH()        HAL_GPIO_WritePin(ADS_CS_GPIO_Port, ADS_CS_Pin, GPIO_PIN_SET)
#define ADS_CS_LOW()         HAL_GPIO_WritePin(ADS_CS_GPIO_Port, ADS_CS_Pin, GPIO_PIN_RESET)

#define ADS_CLKSEL_HIGH()    HAL_GPIO_WritePin(ADS_CLKSEL_GPIO_Port, ADS_CLKSEL_Pin, GPIO_PIN_SET)
#define ADS_CLKSEL_LOW()     HAL_GPIO_WritePin(ADS_CLKSEL_GPIO_Port, ADS_CLKSEL_Pin, GPIO_PIN_RESET)

#define ADS_DRDY_READ()      HAL_GPIO_ReadPin(ADS_DRDY_GPIO_Port, ADS_DRDY_Pin)

//========================== ADS1292 寄存器/命令宏定义 =========================//

// 系统命令
#define WAKEUP    0x02
#define STANDBY   0x04
#define RESET     0x06
#define START     0x08
#define STOP      0x0A
#define OFFSETCAL 0x1A

// 数据读取命令
#define RDATAC 0x10
#define SDATAC 0x11
#define RDATA  0x12

// 寄存器读写命令
#define RREG 0x20
#define WREG 0x40

// ADS1292 内部寄存器地址
#define ID        0x00
#define CONFIG1   0x01
#define CONFIG2   0x02
#define LOFF      0x03
#define CH1SET    0x04
#define CH2SET    0x05
#define RLD_SENS  0x06
#define LOFF_SENS 0x07
#define LOFF_STAT 0x08
#define RESP1     0x09
#define RESP2     0x0A
#define GPIO      0x0B

//========================== 以下为寄存器配置宏（与原文件保持一致） =========================//

// ID
#define DEVICE_ID_ADS1292  0x53
#define DEVICE_ID_ADS1292R 0x73

// CONFIG1
#define DATA_RATE_125SPS 0x00
#define DATA_RATE_250SPS 0x01
#define DATA_RATE_500SPS 0x02
#define DATA_RATE_1kSPS  0x03
#define DATA_RATE_2kSPS  0x04
#define DATA_RATE_4kSPS  0x05
#define DATA_RATE_8kSPS  0x06

// CONFIG2
#define PDB_LOFF_COMP_OFF 0
#define PDB_LOFF_COMP_ON  1
#define PDB_REFBUF_OFF    0
#define PDB_REFBUF_ON     1
#define VREF_242V         0
#define VREF_4V           1
#define CLK_EN_OFF        0
#define CLK_EN_ON         1
#define INT_TEST_OFF      0
#define INT_TEST_ON       1

// CHSET
#define PD_ON             0
#define PD_OFF            1
#define GAIN_6            0
#define GAIN_1            1
#define GAIN_2            2
#define GAIN_3            3
#define GAIN_4            4
#define GAIN_8            5
#define GAIN_12           6
#define MUX_Normal_input  0
#define MUX_input_shorted 1
#define MUX_Test_signal   5
#define MUX_RLD_DRP       6
#define MUX_RLD_DRM       7
#define MUX_RLD_DRPM      8
#define MUX_RSP_IN3P      9

// RLD_SENS
#define PDB_RLD_OFF        0
#define PDB_RLD_ON         1
#define RLD_LOFF_SENSE_OFF 0
#define RLD_LOFF_SENSE_ON  1
#define RLD_CANNLE_OFF     0
#define RLD_CANNLE_ON      1

// LOFF_SENS
#define FLIP2_OFF       0
#define FLIP2_ON        1
#define FLIP1_OFF       0
#define FLIP1_ON        1
#define LOFF_CANNLE_OFF 0
#define LOFF_CANNLE_ON  1

// RESP1
#define RESP_DEMOD_OFF           0
#define RESP_DEMOD_ON            1
#define RESP_MOD_OFF             0
#define RESP_MOD_ON              1
#define RESP_CTRL_CLOCK_INTERNAL 0
#define RESP_CTRL_CLOCK_EXTERNAL 1

// RESP2
#define CALIB_OFF             0
#define CALIB_ON              1
#define FREQ_32K              0
#define FREQ_64K              1
#define RLDREF_INT_EXTERN     0
#define RLDREF_INT_INTERNALLY 1

//========================== 结构体定义 =========================//

typedef struct {
    GPIO_Instance *gpio;        // 绑定的GPIO实例
    uint8_t raw_data[9];        // 存储ADS1292读取的9字节原始数据
    float ecg_data;           // 解析后的ECG数据
    float respirat_impedance; // 解析后的呼吸阻抗数据
    uint8_t status;             // 状态字节
    uint8_t lead_left;          // 左导联是否脱落
    uint8_t lead_right;         // 右导联是否脱落
    uint8_t ecg_active;         // ECG是否正在采样
    uint8_t device_id;          // 设备ID
    uint8_t is_collecting;      // 是否正在采集
} ADS1292_Instance;

typedef struct
{
    GPIO_Init_Config_s gpio_config; // GPIO配置
} ADS1292_Config_s;

typedef struct
{
    uint8_t Data_Rate; // ADC 通道采样率
} ADS1292_CONFIG1;

typedef struct
{
    uint8_t Pdb_Loff_Comp; // 导联脱落比较器掉电/使能
    uint8_t Pdb_Refbuf;    // 内部参考缓冲器掉电/使能
    uint8_t Vref;          // 内部参考电压设置
    uint8_t Clk_EN;        // 振荡器时钟输出设置
    uint8_t Int_Test;      // 内部测试信号使能
} ADS1292_CONFIG2;

typedef struct
{
    uint8_t PD;   // 通道断电？
    uint8_t GAIN; // PGA 增益
    uint8_t MUX;  // 通道输入方式
} ADS1292_CHSET;

typedef struct
{
    uint8_t Pdb_Rld;        // RLD 缓冲电源状态
    uint8_t Rld_Loff_Sense; // RLD 导联脱落检测使能
    uint8_t Rld2N;          // 通道2负输入 -> 右腿驱动输出
    uint8_t Rld2P;          // 通道2正输入 -> 右腿驱动输出
    uint8_t Rld1N;          // 通道1负输入 -> 右腿驱动输出
    uint8_t Rld1P;          // 通道1正输入 -> 右腿驱动输出
} ADS1292_RLD_SENS;

typedef struct
{
    uint8_t Flip2;  // 导联脱落检测通道2的电流方向
    uint8_t Flip1;  // 导联脱落检测通道1的电流方向
    uint8_t Loff2N; // 通道2负输入端导联脱落检测
    uint8_t Loff2P; // 通道2正输入端导联脱落检测
    uint8_t Loff1N; // 通道1负输入端导联脱落检测
    uint8_t Loff1P; // 通道1正输入端导联脱落检测
} ADS1292_LOFF_SENS;

typedef struct
{
    uint8_t RESP_DemodEN;
    uint8_t RESP_modEN;
    uint8_t RESP_ph;
    uint8_t RESP_Ctrl;
} ADS1292_RESP1;

typedef struct
{
    uint8_t Calib;
    uint8_t freq;
    uint8_t Rldref_Int;
} ADS1292_RESP2;

//========================== 接口函数声明 =========================//

ADS1292_Instance *ADS1292_Init(void);
void ADS1292_PowerOnInit(void);
uint8_t ADS1292_SPI(uint8_t com);
void ADS1292_Recv_Start(void);

void ADS1292_Send_CMD(uint8_t data);
void ADS1292_WR_REGS(uint8_t reg, uint8_t len, uint8_t *data);
uint8_t ADS1292_Read_Data(uint8_t *data);

void ADS1292_SET_REGBUFF(void);
uint8_t ADS1292_WRITE_REGBUFF(void);

uint8_t ADS1292_Noise_Test(void);
uint8_t ADS1292_Single_Test(void);
uint8_t ADS1292_Single_Read(void);
uint8_t Set_ADS1292_Collect(uint8_t mode);

uint8_t ADS1292_ReadDeviceID(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_ADS1292_HAL_H */
