#include "robot.h"
#include "roboTask.h"
#include "ecg.h"

void robot_init(void)
{
    // Initialize the robot
    // DWT
    __disable_irq();
    DWT_Init(168);
    lcd_init();

    OS_Task_Init();
    __enable_irq();
    // Initialize LCD
}