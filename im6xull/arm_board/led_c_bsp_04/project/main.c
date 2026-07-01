/**
 * 点灯实验--使用NXP的SDK开发包
 * SDK包中提供了IMX6ULL所有相关寄存器定义
 * 需要移植的文件有：
 *  fsl_common.h
 *  fsl_iomuxc.h
 *  MCIMX6Y2.h
*/
#include "bsp_clk.h"
#include "bsp_delay.h"
#include "bsp_led.h"



int main(int argc, char const *argv[])
{
    //使能所有时钟
    clk_enable();
    //初始化led
    led_init();
    //led闪烁
    while (1)
    {
        led_switch(LED0,ON);
        delay(500);

        led_switch(LED0,OFF);
        delay(500);
    }
    
    return 0;
}
