/**
 * 点灯实验--使用NXP的SDK开发包
 * SDK包中提供了IMX6ULL所有相关寄存器定义
 * 需要移植的文件有：
 *  fsl_common.h
 *  fsl_iomuxc.h
 *  MCIMX6Y2.h
*/
#include "fsl_common.h"
#include "fsl_iomuxc.h"
#include "cc.h"

void clk_enable(void)
{
    CCM->CCGR0 = 0xffffffff;
    CCM->CCGR1 = 0xffffffff;
    CCM->CCGR2 = 0xffffffff;
    CCM->CCGR3 = 0xffffffff;
    CCM->CCGR4 = 0xffffffff;
    CCM->CCGR5 = 0xffffffff;
    CCM->CCGR6 = 0xffffffff;
}

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

void led_on(void)
{
    GPIO1->DR   &= ~(1 << 3);
}

void led_off(void)
{
    GPIO1->DR   |= (1 << 3);
}

void delay_short(volatile unsigned int n)
{
    while (n--)
    {
        
    }
}

void delay(volatile unsigned int n)
{
    while (n--)
    {
        delay_short(0x7ff);
    }
    
}

int main(int argc, char const *argv[])
{
    //使能所有时钟
    clk_enable();
    //初始化led
    led_init();
    //led闪烁
    while (1)
    {
        led_off();
        delay(1000);

        led_on();
        delay(1000);
    }
    
    return 0;
}
