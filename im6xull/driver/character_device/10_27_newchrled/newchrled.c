/*
 * @Description:自动创建设备节点的led驱动程序
    1-驱动模块的注册与注销
    2-字符设备驱动的注册与注销
        a.分配和释放设备号
        b.自动创建设备节点
        c.设置文件私有数据
        d.初始化led
    3-实现设备的具体操作函数
    4-添加LINCENSE和作者信息
 * @Author: dwl
 * @Date: 2024-10-27 11:48:32
 * @LastEditTime: 2024-10-28 21:15:29
 * @LastEditors: Do not edit
 * @FilePath: /character_device/10_27_newchrled/newchrled.c
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/file.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/ide.h>
#include <linux/gpio.h>
#include <asm/mach/map.h>

#define NEWCHRLED_CNT 1            // 设备数量
#define NEWCHRLED_NAME "newchrled" // 设备名字

#define LEDOFF 0 // led关
#define LEDON 1  // led开

// 各个寄存器地址
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

// newchrled设备结构体
struct newchrled_dev
{
    dev_t devid; // 设备号
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major; // 主设备号
    int minor; // 次设备号
};

struct newchrled_dev newchrled; // led设备

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

static int newchrled_open(struct inode *inode, struct file *file)
{
    file->private_data = &newchrled; // 设置私有数据
    return 0;
}

static int newchrled_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t newchrled_read(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
    return 0;
}

static ssize_t newchrled_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
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

static const struct file_operations newchrled_fops = {
    .owner = THIS_MODULE,
    .open = newchrled_open,
    .release = newchrled_release,
    .read = newchrled_read,
    .write = newchrled_write,
};

static int __init newchrled_init(void)
{
    u32 val = 0;

    // 初始化led
    //  1.映射GPIO寄存器
    CCM_CCGR1 = ioremap(CCM_CCGR1_BASE, 4);
    SW_MUX_CTL_PAD_GPIO1_IO03 = ioremap(SW_MUX_CTL_PAD_GPIO1_IO03_BASE, 4);
    SW_PAD_CTL_PAD_GPIO1_IO03 = ioremap(SW_PAD_CTL_PAD_GPIO1_IO03_BASE, 4);
    GPIO1_GDIR = ioremap(GPIO1_GDIR_BASE, 4);
    GPIO1_DR = ioremap(GPIO1_DR_BASE, 4);

    // 使能时钟
    val = readl(CCM_CCGR1);
    val &= ~(3 << 26);
    val |= (3 << 26);
    writel(val, CCM_CCGR1);

    // 2.设置GPIO1_IO03的复用功能，将其复用为GPIO1_IO03
    writel(5, SW_MUX_CTL_PAD_GPIO1_IO03);

    // 3.设置GPIO1_IO03的IO属性
    writel(0x10b0, SW_PAD_CTL_PAD_GPIO1_IO03);

    // 4.设置GPIO1_IO03为输出功能
    val = readl(GPIO1_GDIR);
    val &= ~(1 << 3);
    val |= (1 << 3);
    writel(val, GPIO1_GDIR);

    // 5.默认关闭led
    val = readl(GPIO1_DR);
    val |= (1 << 3);
    writel(val, GPIO1_DR);

    // 注册设备驱动
    /**
     * 创建设备号
     * 如果定义了设备号
     *  注册设备号和设备驱动
     * 如果没有定义设备号
     *  分配设备号，注册设备驱动
     */
    if (newchrled.major)
    {
        // 注册设备号
        newchrled.devid = MKDEV(newchrled.major, 0);
        // 注册设备驱动
        register_chrdev_region(newchrled.devid, NEWCHRLED_CNT, NEWCHRLED_NAME);
    }
    else
    {
        // 分配设备号
        alloc_chrdev_region(&newchrled.devid, 0, NEWCHRLED_CNT, NEWCHRLED_NAME);
        newchrled.major = MAJOR(newchrled.devid); // 获取主设备号
        newchrled.minor = MINOR(newchrled.devid); // 获取次设备号
    }
    printk("newchrled devid major = %d,minor = %d.\r\n", newchrled.major, newchrled.minor);

    // 初始化cdev
    newchrled.cdev.owner = THIS_MODULE;
    cdev_init(&newchrled.cdev, &newchrled_fops);

    // 添加cdev
    cdev_add(&newchrled.cdev, newchrled.devid, NEWCHRLED_CNT);

    // 创建类
    newchrled.class = class_create(THIS_MODULE, NEWCHRLED_NAME);
    if (IS_ERR(newchrled.class))
    {
        return PTR_ERR(newchrled.class);
    }

    // 创建设备
    newchrled.device = device_create(newchrled.class, NULL, newchrled.devid, NULL, NEWCHRLED_NAME);
    if (IS_ERR(newchrled.device))
    {
        return PTR_ERR(newchrled.device);
    }

    return 0;
}

static void __exit newchrled_exit(void)
{
    // 取消地址映射
    iounmap(CCM_CCGR1);
    iounmap(SW_MUX_CTL_PAD_GPIO1_IO03);
    iounmap(SW_PAD_CTL_PAD_GPIO1_IO03);
    iounmap(GPIO1_GDIR);
    iounmap(GPIO1_DR);

    device_destroy(newchrled.class, newchrled.devid);
    class_destroy(newchrled.class);
    cdev_del(&newchrled.cdev);
    unregister_chrdev_region(newchrled.devid, NEWCHRLED_CNT);
}

module_init(newchrled_init);
module_exit(newchrled_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("duanwenling");