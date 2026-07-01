/*
 * @Description:
 * 并发与竞争
 *  并发
 *      多个任务同时访问一个共享资源
 *  竞争
 *      并发会带来竞争
 *  处理并发与竞争的方法
 *      原子操作
 *          不能进行分割的指令，一般用于变量或位操作
 * @Author: dwl
 * @Date: 2024-12-02 19:28:39
 * @LastEditTime: 2024-12-02 20:34:05
 * @LastEditors: Do not edit
 * @FilePath: /character_device/12_02_atomic.c/atomic.c
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
#include <asm/atomic.h>

#define GPIOLED_CNT 1
#define GPIOLED_NAME "gpioled"

#define LED_ON 1
#define LED_OFF 0

// 5.gpioled设备结构体
struct gpioled_dev
{
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node *node;
    int led_gpio;
    atomic_t lock; // 原子变量
};
struct gpioled_dev gpioled;

static int gpioled_open(struct inode *inode, struct file *file)
{
    if (!atomic_dec_and_test(&gpioled.lock))
    {
        atomic_inc(&gpioled.lock);
        return -EBUSY;
    }
    file->private_data = &gpioled;
    return 0;
}

static int gpioled_release(struct inode *inode, struct file *file)
{
    struct gpioled_dev *dev = file->private_data;
    atomic_inc(&dev->lock);
    return 0;
}

static ssize_t gpioled_read(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
    return 0;
}

static ssize_t gpioled_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
    // 9.设置led
    int ret = 0;
    unsigned char databuf[1];
    int ledstatus = 0;
    struct gpioled_dev *dev = file->private_data;

    ret = copy_from_user(databuf, buf, count);
    if (ret < 0)
    {
        printk(KERN_ERR "gpioled: write failed.\n");
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
        printk(KERN_ERR "gpioled: ledstatus unknown value.\n");
        return -EINVAL;
    }

    return 0;
}

// 6.设备操作函数
static struct file_operations gpioled_fops = {
    .owner = THIS_MODULE,
    .open = gpioled_open,
    .release = gpioled_release,
    .read = gpioled_read,
    .write = gpioled_write,
};

// 3.驱动注册
static int __init gpioled_init(void)
{
    int ret = 0;

    // 初始化原子变量
    atomic_set(&gpioled.lock, 1);

    // 8.初始化led
    // 获取设备节点
    gpioled.node = of_find_node_by_path("/gpioled");
    if (gpioled.node == NULL)
    {
        printk(KERN_ERR "gpioled:node not found.\n");
        return -EINVAL;
    }
    else
    {
        printk(KERN_INFO "gpioled: node foound.\n");
    }

    // 获取设备树中gpioled的属性，得到led所使用的编号
    gpioled.led_gpio = of_get_named_gpio(gpioled.node, "led-gpio", 0);
    if (gpioled.led_gpio < 0)
    {
        printk(KERN_ERR "gpioled:get led-gpio failed.\n");
        return -EINVAL;
    }
    else
    {
        printk(KERN_INFO "gpioled: get led-gpio num is %d.\n", gpioled.led_gpio);
    }

    // 设置led默认为关，即gpio输出且为高
    ret = gpio_direction_output(gpioled.led_gpio, 1);
    if (ret < 0)
    {
        printk(KERN_ERR "gpioled: set led-gpio output failed.\n");
        return ret;
    }

    // 7.注册字符设备驱动
    // 创建设备号
    if (gpioled.major)
    {
        gpioled.devid = MKDEV(gpioled.major, 0);
        ret = register_chrdev_region(gpioled.devid, GPIOLED_CNT, GPIOLED_NAME);
        if (ret)
        {
            printk(KERN_ERR "gpioled:could not register device.\n");
            goto create_devid_error;
        }
    }
    else
    {
        ret = alloc_chrdev_region(&gpioled.devid, 0, GPIOLED_CNT, GPIOLED_NAME);
        if (ret)
        {
            printk(KERN_ERR "gpioled:could not region device.\n");
            goto region_devid_error;
        }
        // 获取分配的主次设备号
        gpioled.major = MAJOR(gpioled.devid);
        gpioled.minor = MINOR(gpioled.devid);
    }
    printk("gpioled:magor = %d,minor = %d.\n", gpioled.major, gpioled.minor);

    // 初始化cdev
    gpioled.cdev.owner = THIS_MODULE;
    cdev_init(&gpioled.cdev, &gpioled_fops);

    // 添加一个cedv
    cdev_add(&gpioled.cdev, gpioled.devid, GPIOLED_CNT);

    // 创建类
    gpioled.class = class_create(THIS_MODULE, GPIOLED_NAME);
    if (IS_ERR(gpioled.class))
    {
        printk(KERN_ERR "gpioled: create class failed.\n");
        goto create_class_error;
    }

    // 创建设备
    gpioled.device = device_create(gpioled.class, NULL, gpioled.devid, NULL, GPIOLED_NAME);
    if (IS_ERR(gpioled.class))
    {
        printk(KERN_ERR "gpioled: create device failed.\n");
        goto create_device_error;
    }

    return 0;

create_device_error:
    class_destroy(gpioled.class);
create_class_error:
    cdev_del(&gpioled.cdev);
    unregister_chrdev_region(gpioled.devid, GPIOLED_CNT);
region_devid_error:
create_devid_error:
    return ret;
}

// 4.驱动注销
static void __exit gpioled_exit(void)
{
    device_destroy(gpioled.class, gpioled.devid);
    class_destroy(gpioled.class);
    cdev_del(&gpioled.cdev);
    unregister_chrdev_region(gpioled.devid, GPIOLED_CNT);
}

// 1.模块驱动的注册与注销
module_init(gpioled_init);
module_exit(gpioled_exit);

// 2.声明模块信息和作者信息
MODULE_LICENSE("GPL");
MODULE_AUTHOR("duanwenling");