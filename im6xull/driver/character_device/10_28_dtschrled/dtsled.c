/*
 * @Description:
 * @Author: dwl
 * @Date: 2024-10-28 19:08:42
 * @LastEditTime: 2024-10-28 22:35:14
 * @LastEditors: Do not edit
 * @FilePath: /character_device/10_28_dtschrled/dtsled.c
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
#include <linux/of.h>
#include <linux/of_address.h>

#define DTSLED_CNT 1         // 设备数量
#define DTSLED_NAME "dtsled" // 设备名字

#define LEDOFF 0
#define LEDON 1

// 地址映射后的寄存器虚拟地址指针
static void __iomem *CCM_CCGR1;
static void __iomem *SW_MUX_CTL_PAD_GPIO1_IO03;
static void __iomem *SW_PAD_CTL_PAD_GPIO1_IO03;
static void __iomem *GPIO1_GDIR;
static void __iomem *GPIO1_DR;

struct dtsled_dev
{
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node *nd; // 设备节点
};

struct dtsled_dev dtsled; // led设备

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

static int dtsled_open(struct inode *inode, struct file *file)
{
    file->private_data = &dtsled;
    return 0;
}

static int dtsled_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t dtsled_read(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
    return 0;
}

static ssize_t dtsled_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
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

static struct file_operations dtsled_fops =
    {
        .owner = THIS_MODULE,
        .open = dtsled_open,
        .release = dtsled_release,
        .read = dtsled_read,
        .write = dtsled_write,
};

static int __init dtsled_init(void)
{
    int res = 0;
    struct property *proper;
    const char *str;
    u32 regdata[14];
    u32 val = 0;
    /**
     * 获取设备树中设备节点的属性
     */
    // 获取设备节点
    dtsled.nd = of_find_node_by_path("/alphaled");
    if (dtsled.nd == NULL)
    {
        printk(KERN_ERR "alphaled node can not found!\r\n");
        return -EINVAL;
    }
    else
    {
        printk("alphaled node  has been  found!\r\n");
    }

    // 获取compatiable属性
    proper = of_find_property(dtsled.nd, "compatible", NULL);
    if (proper == NULL)
    {
        printk(KERN_ERR "compatible property find failed!\r\n");
    }
    else
    {
        printk("compatible = %s.\r\n", (char *)proper->value);
    }
    // 获取status属性
    res = of_property_read_string(dtsled.nd, "status", &str);
    if (res < 0)
    {
        printk(KERN_ERR "status read failed!\r\n");
    }
    else
    {
        printk("status = %s.\r\n", str);
    }
    // 获取reg
    res = of_property_read_u32_array(dtsled.nd, "reg", regdata, 10);
    if (res < 0)
    {
        printk(KERN_ERR "reg property read dailed!\r\n");
    }
    else
    {
        u8 i = 0;
        printk("regdata:\r\n");
        for (i; i < 10; i++)
        {
            printk("%#X ", regdata[i]);
        }
        printk("\r\n");
    }
    // printk("-----------%d----------\r\n", __LINE__);
    // 初始化led
    CCM_CCGR1 = of_iomap(dtsled.nd, 0);
    // printk("-----------%d----------\r\n", __LINE__);
    SW_MUX_CTL_PAD_GPIO1_IO03 = of_iomap(dtsled.nd, 1);
    // printk("-----------%d----------\r\n", __LINE__);
    SW_PAD_CTL_PAD_GPIO1_IO03 = of_iomap(dtsled.nd, 2);
    // printk("-----------%d----------\r\n", __LINE__);
    GPIO1_GDIR = of_iomap(dtsled.nd, 3);
    // printk("-----------%d----------\r\n", __LINE__);
    GPIO1_DR = of_iomap(dtsled.nd, 4);
    // printk("-----------%d----------\r\n", __LINE__);

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

    /**
     * 注册设备驱动
     */
    // 创建设备号
    if (dtsled.major)
    {
        dtsled.devid = MKDEV(dtsled.major, 0);
        register_chrdev_region(dtsled.devid, DTSLED_CNT, DTSLED_NAME);
    }
    else
    {
        alloc_chrdev_region(&dtsled.devid, 0, DTSLED_CNT, DTSLED_NAME);
        dtsled.major = MAJOR(dtsled.devid);
        dtsled.minor = MINOR(dtsled.minor);
    }
    printk("dtsled major = %d,minor = %d.\r\n", dtsled.major, dtsled.minor);

    // 初始化cdev
    dtsled.cdev.owner = THIS_MODULE;
    cdev_init(&dtsled.cdev, &dtsled_fops);
    // 添加一个cdev
    cdev_add(&dtsled.cdev, dtsled.devid, DTSLED_CNT);
    // 创建类
    dtsled.class = class_create(THIS_MODULE, DTSLED_NAME);
    if (IS_ERR(dtsled.class))
    {
        return PTR_ERR(dtsled.class);
    }

    //  创建设备
    dtsled.device = device_create(dtsled.class, NULL, dtsled.devid, NULL, DTSLED_NAME);
    if (IS_ERR(dtsled.device))
    {
        return PTR_ERR(dtsled.device);
    }

    return 0;
}

static void __exit dtsled_exit(void)
{
    // 取消地址映射
    iounmap(CCM_CCGR1);
    iounmap(SW_MUX_CTL_PAD_GPIO1_IO03);
    iounmap(SW_PAD_CTL_PAD_GPIO1_IO03);
    iounmap(GPIO1_GDIR);
    iounmap(GPIO1_DR);

    // 注销字符设备
    cdev_del(&dtsled.cdev);
    unregister_chrdev_region(dtsled.devid, DTSLED_CNT);
    device_destroy(dtsled.class, dtsled.devid);
    class_destroy(dtsled.class);
}

module_init(dtsled_init);
module_exit(dtsled_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("duanwenling");