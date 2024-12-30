#include "key.h"
#include "stdlib.h"
#include "memory.h"

static uint8_t idx;
static KEY_Instance *key_instances[KEY_MAX_NUM] = {NULL}; // 一个指针数组，用于存放KEY实例的指针

static void KEYUpdate(GPIO_Instance *gpio)
{
    KEY_Instance *key = (KEY_Instance *)gpio->id;
    // key->state        = GPIORead(gpio);
    // if (key->state != key->last_state && !key->state) // 检测上升沿
    key->count++; // 按键计数,若选择上升沿/下降沿监测，则计数值会+2
    // key->last_state = key->state;
}

/**
 * @brief 按键注册
 *
 * @param config
 * @return KEY_Instance*
 */
KEY_Instance *KEYRegister(KEY_Config_s *config)
{
    KEY_Instance *key = (KEY_Instance *)malloc(sizeof(KEY_Instance));
    memset(key, 0, sizeof(KEY_Instance)); // 清零,防止原先的地址有脏数据

    // 初始化gpio
    config->gpio_config.gpio_model_callback = KEYUpdate;
    config->gpio_config.id                  = key;
    key->gpio                               = GPIORegister(&config->gpio_config);
    key->state                              = GPIORead(key->gpio);

    key_instances[idx++] = key;
    return key;
}