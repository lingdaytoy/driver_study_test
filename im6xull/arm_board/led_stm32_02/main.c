#include "imx6ull.h"

/*使能外设时钟*/
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
/*初始化led*/
void led_init(void)
{
    IOMUX_SW_MUX->GPIO1_IO03 = 0x5;     //复用引脚为GPIO

    /* 2、配置GPIO1_IO03的IO属性	
	 *bit 16:0 HYS关闭
	 *bit [15:14]: 00 默认下拉
     *bit [13]: 0 kepper功能
     *bit [12]: 1 pull/keeper使能
     *bit [11]: 0 关闭开路输出
     *bit [7:6]: 10 速度100Mhz
     *bit [5:3]: 110 R0/6驱动能力
     *bit [0]: 0 低转换率
     */
    IOMUX_SW_PAD->GPIO1_IO03 = 0xb010;  

    GPIO1->GDIR = 0X8;              //设置GPIO为输出
    GPIO1->DR &= ~(1 << 3);         //设置GPIO为低电平，打开led
}

/*led开*/
void led_on(void)
{
    GPIO1->DR &= ~(1 << 3);
}

/*led关*/
void led_off(void)
{
    GPIO1->DR |= (1 << 3);
}

/*短延时*/
void delay_short(volatile unsigned int n)
{
    while (n--)
    {
        
    }
    
}
/*延时*/
void delay(volatile unsigned int n)
{
    while (n--)
    {
        delay_short(0x7ff);
    }
    
}

int main(int argc, char const *argv[])
{
    clk_enable();   //使能所有时钟
    led_init();     //初始化led

    /*led闪烁*/
    while (1)
    {
        led_off();  //关闭led
        delay(500); //延时500ms

        led_on();   //开启led
        delay(500); //延时500ms
    }
    
    return 0;
}
