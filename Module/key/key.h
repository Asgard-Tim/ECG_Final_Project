/**
 * @file key.h
 * @author Bi Kaixiang (wexhicy@gmail.com)
 * @brief 按键模块
 * @version 0.1
 * @date 2024-01-02
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef KEY_H
#define KEY_H

#include "stdint.h"
#include "bsp_gpio.h"

#define KEY_MAX_NUM 3 // 最大按键数量,此处考虑到原本C板上就有一个按键,最多支持3个

/* 按键实例结构体 */
typedef struct
{
    GPIO_Instance *gpio; // gpio实例
    uint8_t state;       // 按键状态
    uint8_t last_state;  // 上一次按键状态
    uint16_t count;      // 按键计数
} KEY_Instance;

/* 按键初始化配置结构体 */
typedef struct
{
    GPIO_Init_Config_s gpio_config; // gpio初始化配置
    uint8_t init_state;             // 初始化状态
} KEY_Config_s;

/**
 * @brief 按键注册
 *
 * @param config
 * @return KEY_Instance*
 */
KEY_Instance *KEYRegister(KEY_Config_s *config);

#endif // KEY_H