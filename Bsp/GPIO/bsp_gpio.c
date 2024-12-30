#include "bsp_gpio.h"
#include "memory.h"
#include "stdlib.h"

static uint8_t idx;
static GPIO_Instance *gpio_instances[GPIO_DEVICE_MAX_NUM] = {NULL};

/**
 * @brief EXTI中断回调函数,根据GPIO_Pin找到对应的GPIOInstance,并调用模块回调函数(如果有)
 * @note 当需要添加中断时,记得查看CubeMX是否已经配置了中断,否则不会触发
 *       如何判断具体是哪一个GPIO的引脚连接到这个EXTI中断线上?
 *       一个EXTI中断线只能连接一个GPIO引脚,因此可以通过GPIO_Pin来判断,PinX对应EXTIX
 *       一个Pin号只会对应一个EXTI
 * @param GPIO_Pin 发生中断的GPIO_Pin
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    // 如有必要,可以根据pinstate和HAL_GPIO_ReadPin来判断是上升沿还是下降沿/rise&fall等
    GPIO_Instance *gpio = NULL;
    for (uint8_t i = 0; i < idx; i++) {
        gpio = gpio_instances[i];
        if (gpio->GPIO_Pin == GPIO_Pin && gpio->gpio_model_callback != NULL) {
            gpio->gpio_model_callback(gpio);
            return;
        }
    }
}

/**
 * @brief GPIO的注册函数
 *
 * @param GPIO_config
 * @return GPIO_Instance*
 */
GPIO_Instance *GPIORegister(GPIO_Init_Config_s *GPIO_config)
{
    GPIO_Instance *gpio = (GPIO_Instance *)malloc(sizeof(GPIO_Instance));
    memset(gpio, 0, sizeof(GPIO_Instance)); // 清零,防止原先的地址有脏数据

    gpio->GPIOx               = GPIO_config->GPIOx;
    gpio->GPIO_Pin            = GPIO_config->GPIO_Pin;
    gpio->pin_state           = GPIO_config->pin_state;
    gpio->exti_mode           = GPIO_config->exti_mode;
    gpio->id                  = GPIO_config->id;
    gpio->gpio_model_callback = GPIO_config->gpio_model_callback;

    gpio_instances[idx++] = gpio;
    return gpio;
}

// ----------------- GPIO API -----------------
// 都是对HAL的形式上的封装,后续考虑增加GPIO state变量,可以直接读取state

void GPIOToggel(GPIO_Instance *_instance)
{
    HAL_GPIO_TogglePin(_instance->GPIOx, _instance->GPIO_Pin);
}

void GPIOSet(GPIO_Instance *_instance)
{
    HAL_GPIO_WritePin(_instance->GPIOx, _instance->GPIO_Pin, GPIO_PIN_SET);
}

void GPIOReset(GPIO_Instance *_instance)
{
    HAL_GPIO_WritePin(_instance->GPIOx, _instance->GPIO_Pin, GPIO_PIN_RESET);
}

GPIO_PinState GPIORead(GPIO_Instance *_instance)
{
    return HAL_GPIO_ReadPin(_instance->GPIOx, _instance->GPIO_Pin);
}
