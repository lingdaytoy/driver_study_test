/*
 * @Description:led驱动程序
 * @Author: dwl
 * @Date: 2024-10-26 20:30:06
 * @LastEditTime: 2024-10-26 23:00:21
 * @LastEditors: Do not edit
 * @FilePath: /character_device/10_26_led/leddriver.c
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/uaccess.h>

#define LED_MAJOR 200        // 主设备号
#define LED_NAME "leddriver" // 设备名字

#define LEDOFF 0 // led关
#define LEDON 1  // led开

// 个寄存器地址
#define CCM_CCGR1_BASE 0x020c406c
#define SW_MUX_CTL_PAD_GPIO1_IO03_BASE 0x020e0068
#define SW_PAD_CTL_PAD_GPIO1_IO03_BASE 0x020e02f4
#define GPIO1_GDIR_BASE 0x0209c004
#define GPIO1_DR_BASE 0x0209c000

// 地址映射后的寄存器虚拟地址指针
static void __iomem *CCM_CCGR1;
static void __iomem *SW_MUX_CTL_PAD_GPIO1_IO03;
static void __iomem *SW_PAD_CTL_PAD_GPIO1_IO03;
static void __iomem *GPIO1_GDIR;
static void __iomem *GPIO1_DR;

void led_switch(u8 status)
{
    u32 val = 0;

    if (status == LEDON)
    {
        // 打开led灯
        val = readl(GPIO1_DR);
        val &= ~(1 << 3);
        writel(val, GPIO1_DR);
    }
    else if (status == LEDOFF)
    {
        // 关闭led灯
        val = readl(GPIO1_DR);
        val |= (1 << 3);
        writel(val, GPIO1_DR);
    }
}

static int leddriver_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int leddriver_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t leddriver_read(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
    return 0;
}

static ssize_t leddriver_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
    int res = 0;
    unsigned char databuf[1];
    unsigned char ledstatus;

    res = copy_from_user(databuf, buf, count);
    if (res < 0)
    {
        printk(KERN_ERR "kernel write failed!\r\n");
        return -EFAULT;
    }

    ledstatus = databuf[0];
    if (ledstatus == 1)
    {
        led_switch(LEDON); // 开灯
    }
    else
    {
        led_switch(LEDOFF); // 关灯
    }

    return 0;
}

/**
 * 设备操作函数的结构体
 */
static const struct file_operations leddriver_fops = {
    .owner = THIS_MODULE,
    .open = leddriver_open,
    .release = leddriver_release,
    .read = leddriver_read,
    .write = leddriver_write,
};
/**
 * 驱动入口函数
 */
static int __init leddriver_init(void)
{
    int res = 0;
    u32 val = 0;
    // 初始化led
    // 寄存器地址映射
    CCM_CCGR1 = ioremap(CCM_CCGR1_BASE, 4);
    SW_MUX_CTL_PAD_GPIO1_IO03 = ioremap(SW_MUX_CTL_PAD_GPIO1_IO03_BASE, 4);
    SW_PAD_CTL_PAD_GPIO1_IO03 = ioremap(SW_PAD_CTL_PAD_GPIO1_IO03_BASE, 4);
    GPIO1_GDIR = ioremap(GPIO1_GDIR_BASE, 4);
    GPIO1_DR = ioremap(GPIO1_DR_BASE, 4);

    // 使能GPIO时钟
    val = readl(CCM_CCGR1);
    val &= ~(3 << 26);
    val |= (3 << 26);
    writel(val, CCM_CCGR1);

    // 设置引脚复用为GPIO
    writel(5, SW_MUX_CTL_PAD_GPIO1_IO03);

    // 设置GPIO的电气属性
    writel(0x10b0, SW_PAD_CTL_PAD_GPIO1_IO03);
    // 设置GPIO为输出
    val = readl(GPIO1_GDIR);
    val &= ~(1 << 3);
    val |= (1 << 3);
    writel(val, GPIO1_GDIR);
    // 设置GPIO为低，默认led为关
    val = readl(GPIO1_DR);
    val |= (1 << 3);
    writel(val, GPIO1_DR);

    // 注册字符设备驱动
    res = register_chrdev(LED_MAJOR, LED_NAME, &leddriver_fops);
    if (res < 0)
    {
        printk(KERN_ERR "leddriver driver register failed!\r\n");
        return res;
    }
    return 0;
}

/**
 * 驱动出口函数
 */
static void __exit leddriver_exit(void)
{
    int res = 0;
    // 取消地址映射
    iounmap(CCM_CCGR1);
    iounmap(SW_MUX_CTL_PAD_GPIO1_IO03);
    iounmap(SW_PAD_CTL_PAD_GPIO1_IO03);
    iounmap(GPIO1_GDIR);
    iounmap(GPIO1_DR);
    // 注销字符设备驱动
    unregister_chrdev(LED_MAJOR, LED_NAME);
}

module_init(leddriver_init); // 注册模块加载函数
module_exit(leddriver_exit); // 注册模块注销函数

MODULE_LICENSE("GPL");        // 添加许可
MODULE_AUTHOR("duanwenling"); // 添加作者