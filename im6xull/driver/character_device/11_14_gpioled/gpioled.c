/*
 * @Description:利用pinctrl和gpio子系统点灯
    1 - 在设备树中添加led设备节点
        a.添加pinctrl节点
        b.设置led所使用的pin对应的pinctrl节点
        c.设置led所使用的gpio
    2 - 编写驱动程序
        a.注册字符设备驱动
        b.初始化led设备
        c.编写设备操作函数
        d.注销
    3 - 编写测试APP
 * @Author: dwl
 * @Date: 2024-11-14 21:24:32
 * @LastEditTime: 2024-12-02 20:14:54
 * @LastEditors: Do not edit
 * @FilePath: /character_device/11_14_gpioled/gpioled.c
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
#include <linux/gpio.h>
#include <linux/of_gpio.h>

#define GPIOLED_CNT 1          // 设备号个数
#define GPIOLED_NAME "gpioled" // 设备名

#define LED_ON 1
#define LED_OFF 0

// gpioled设备结构体
struct gpioled_dev
{
    dev_t devid;              // 设备号
    struct cdev cdev;         // cdev
    struct class *class;      // 类
    struct device *device;    // 设备
    int major;                // 主设备号
    int minor;                // 次设备号
    struct device_node *node; // 设备节点
    int led_gpio;             // led所使用gpio编号
};
struct gpioled_dev gpioled; // led设备

static int gpioled_open(struct inode *inode, struct file *file)
{
    file->private_data = &gpioled; // 设置私有数据
    return 0;
}

static int gpioled_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t gpioled_read(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
    return 0;
}

static ssize_t gpioled_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
    int ret = 0;
    unsigned char databuf[1];
    unsigned char ledstatus;
    struct gpioled_dev *dev = file->private_data;

    ret = copy_from_user(databuf, buf, count);
    if (ret < 0)
    {
        printk(KERN_ERR "gpioled:write failed\n");
        return -EFAULT;
    }

    // 获取led状态值
    ledstatus = databuf[0];

    if (ledstatus == LED_ON)
    {
        gpio_set_value(dev->led_gpio, LED_ON);
    }
    else if (ledstatus == LED_OFF)
    {
        gpio_set_value(dev->led_gpio, LED_OFF);
    }
    else
    {
        printk(KERN_ERR "gpioled:ledstatus unknown value\n");
    }

    return 0;
}

// 设备操作函数
static struct file_operations gpioled_fops = {
    .owner = THIS_MODULE,
    .open = gpioled_open,
    .release = gpioled_release,
    .read = gpioled_read,
    .write = gpioled_write,
};

static int __init gpioled_init(void)
{
    int ret = 0;

    // 初始化led
    // 获取设备节点：gpioled
    gpioled.node = of_find_node_by_path("/gpioled");
    if (gpioled.node == NULL)
    {
        printk(KERN_ERR "gpioled:node not found\n");
        return -EINVAL;
    }
    else
    {
        printk(KERN_INFO "gpioled:node found\n");
    }

    // 获取设备树中gpioled的属性，得到led所使用的编号
    gpioled.led_gpio = of_get_named_gpio(gpioled.node, "led-gpio", 0);
    if (gpioled.led_gpio < 0)
    {
        printk(KERN_ERR "gpioled:get led-gpio failed\n");
        return -EINVAL;
    }
    else
    {
        printk(KERN_INFO "gpiold:led-gpio num is %d\n", gpioled.led_gpio);
    }

    // 设置led为关，即gpio为输出且为高电平
    ret = gpio_direction_output(gpioled.led_gpio, 1);
    if (ret < 0)
    {
        printk(KERN_ERR "gpioled:set led-gpio output failed\n");
        return ret;
    }

    // 注册字符设备驱动
    // 创建设备号
    if (gpioled.major) // 如果主设备号存在
    {
        gpioled.devid = MKDEV(gpioled.major, 0);
        ret = register_chrdev_region(gpioled.devid, GPIOLED_CNT, GPIOLED_NAME);
        if (ret)
        {
            printk(KERN_ERR "gpioled:could not register device number\n");
            goto create_devid_error;
        }
    }
    else // 没有定义主设备号
    {
        ret = alloc_chrdev_region(&gpioled.devid, 0, GPIOLED_CNT, GPIOLED_NAME); // 申请设备号
        if (ret)
        {
            printk(KERN_ERR "gpioled:could not region device\n");
            goto region_devid_error;
        }
        gpioled.major = MAJOR(gpioled.devid); // 获取分配的主设备号
        gpioled.minor = MINOR(gpioled.devid); // 获取分配的次设备号
    }
    printk("gpioled: major = %d, minor = %d\n", gpioled.major, gpioled.minor);

    // 初始化cdev
    gpioled.cdev.owner = THIS_MODULE;
    cdev_init(&gpioled.cdev, &gpioled_fops);

    // 添加一个cdev
    cdev_add(&gpioled.cdev, gpioled.devid, GPIOLED_CNT);

    // 创建类
    gpioled.class = class_create(THIS_MODULE, GPIOLED_NAME);
    if (IS_ERR(gpioled.class))
    {
        printk(KERN_ERR "gpioled: create class failed\n");
        goto creatr_class_error;
    }

    // 创建设备
    gpioled.device = device_create(gpioled.class, NULL, gpioled.devid, NULL, GPIOLED_NAME);
    if (IS_ERR(gpioled.device))
    {
        printk(KERN_ERR "gpioled: create device failed\n");
        goto create_device_error;
    }

    return 0;

create_device_error:
    class_destroy(gpioled.class);
    cdev_del(&gpioled.cdev);
    unregister_chrdev_region(gpioled.devid, GPIOLED_CNT);
    return PTR_ERR(gpioled.device);
creatr_class_error:
    cdev_del(&gpioled.cdev);
    unregister_chrdev_region(gpioled.devid, GPIOLED_CNT);
    return PTR_ERR(gpioled.class);
region_devid_error:
create_devid_error:
    return ret;
}

static void __exit gpioled_exit(void)
{
    // 注销设备驱动
    device_destroy(gpioled.class, gpioled.devid);
    class_destroy(gpioled.class);
    cdev_del(&gpioled.cdev);
    unregister_chrdev_region(gpioled.devid, GPIOLED_CNT);
}

module_init(gpioled_init);
module_exit(gpioled_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("duanwenling");