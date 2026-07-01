#include "bsp_led.h"

void led_init(void)
{
    //初始化IO，复用为GPIO
    IOMUXC_SetPinMux(IOMUXC_GPIO1_IO03_GPIO1_IO03,0);
    /* 2、、配置GPIO1_IO03的IO电气属性	
	 *bit 16:0 HYS关闭
	 *bit [15:14]: 00 默认下拉
     *bit [13]: 0 kepper功能
     *bit [12]: 1 pull/keeper使能
     *bit [11]: 0 关闭开路输出
     *bit [7:6]: 10 速度100Mhz
     *bit [5:3]: 110 R0/6驱动能力
     *bit [0]: 0 低转换率
     */
    IOMUXC_SetPinConfig(IOMUXC_GPIO1_IO03_GPIO1_IO03,0x10b0);
    //设置GPIO1_IO03为输出
    GPIO1->GDIR |= (1 << 3);
    GPIO1->DR   &= ~(1 << 3);
}

void led_switch(int led, int status){
    switch (led)
    {
    case LED0:
        if (status == ON)
        {
            GPIO1->DR   &= ~(1 << 3);
        }
        else if (status == OFF)
        {
            GPIO1->DR   |= (1 << 3);
        }
            
        break;
    
    default:
        break;
    }
}