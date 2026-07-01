/*
 * @Description:蜂鸣器驱动实验
 * @Author: dwl
 * @Date: 2024-11-17 15:09:37
 * @LastEditTime: 2024-11-17 18:01:07
 * @LastEditors: Do not edit
 * @FilePath: /character_device/11_16_beep/beep.c
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/uaccess.h>

#include <asm/uaccess.h>

#define BEEP_CNT 1
#define BEEP_NAME "beep"

#define BEEP_ON 0
#define BEEP_OFF 1

struct beep_dev
{
    dev_t devid;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    int major;
    int minor;
    struct device_node *nd;
    int beep_gpio;
};

struct beep_dev beep;

static int beep_open(struct inode *inode, struct file *file)
{
    file->private_data = &beep;
    return 0;
}

static int beep_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t beep_read(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
    return 0;
}

static ssize_t beep_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
    int ret = 0;
    unsigned char databuf[1];
    unsigned char beepsta;
    struct beep_dev *dev = file->private_data;

    ret = copy_from_user(databuf, buf, count);
    if (ret < 0)
    {
        printk(KERN_ERR "beep: copy_from_user failed!\n");
        return -EFAULT;
    }

    // 获取状态值
    beepsta = databuf[0];

    if (beepsta == BEEP_ON)
    {
        gpio_set_value(dev->beep_gpio, 0);
    }
    else if (beepsta == BEEP_OFF)
    {
        gpio_set_value(dev->beep_gpio, 1);
    }
    else
    {
        printk(KERN_ERR "beep: beepsta not enabled!\n");
        return -EFAULT;
    }

    return 0;
}

static struct file_operations beep_fops = {
    .owner = THIS_MODULE,
    .open = beep_open,
    .release = beep_release,
    .read = beep_read,
    .write = beep_write,
};

static int __init beep_init(void)
{
    int ret = 0;

    /**
     * 设置beep所使用的GPIO
     */
    // 获取设备节点
    beep.nd = of_find_node_by_path("/beep");
    if (beep.nd == NULL)

    {
        printk(KERN_ERR "beep: get beep node failed!\n");
        return -EINVAL;
    }
    else
    {
        printk(KERN_INFO "beep: get beep node succeeded!\n");
    }

    // 获取gpio属性及其编号
    beep.beep_gpio = of_get_named_gpio(beep.nd, "beep-gpio", 0);
    if (beep.beep_gpio < 0)
    {
        printk(KERN_ERR "beep: get gpio name failed!\n");
        return -EINVAL;
    }
    else
    {
        printk(KERN_INFO "beep: get gpio name num is %d.\n", beep.beep_gpio);
    }

    // 设置gpio为输出并初始化为高，默认关闭beep
    ret = gpio_direction_output(beep.beep_gpio, 1);
    if (ret < 0)
    {
        printk(KERN_ERR "beep: gpio set failed!\n");
    }

    /**
     * 注册字符设备
     */
    // 创建设备号
    if (beep.major) // 定义了主设备号
    {
        // 申请设备号
        beep.devid = MKDEV(beep.major, 0);
        // 注册字符设备
        ret = register_chrdev_region(beep.devid, BEEP_CNT, BEEP_NAME);
        if (ret)
        {
            printk(KERN_ERR "beep: register_chrdev_region failed!\n");
            goto register_chrdev_region_failed;
        }
    }
    else
    {
        // 注册字符设备
        ret = alloc_chrdev_region(&beep.devid, 0, BEEP_CNT, BEEP_NAME);
        if (ret)
        {
            printk(KERN_ERR "beep: alloc_chrdev_region failed!\n");
            goto alloc_chrdev_region_failed;
        }
        // 获取主设备号和次设备号
        beep.major = MAJOR(beep.devid);
        beep.minor = MINOR(beep.devid);
    }
    printk("beep major: %d and minor: %d.\n", beep.major, beep.minor);

    // 初始化cdev
    beep.cdev.owner = THIS_MODULE;
    cdev_init(&beep.cdev, &beep_fops);

    // 添加一个cdev设备
    ret = cdev_add(&beep.cdev, beep.devid, BEEP_CNT);
    if (ret)
    {
        printk(KERN_ERR "beep: cdev_add falied!\n");
        goto cdev_add_failed;
    }

    // 创建类
    beep.class = class_create(THIS_MODULE, BEEP_NAME);
    if (IS_ERR(beep.class))
    {
        printk(KERN_ERR "beep: class_create failed!\n");
        goto class_create_failed;
    }
    // 创建设备
    beep.device = device_create(beep.class, NULL, beep.devid, NULL, BEEP_NAME);
    if (IS_ERR(beep.device))
    {
        printk(KERN_ERR "beep: device_create faile!\n");
        goto device_create_failed;
    }

    return 0;

device_create_failed:
    class_destroy(beep.class);
class_create_failed:
    cdev_del(&beep.cdev);
cdev_add_failed:
    unregister_chrdev_region(beep.devid, BEEP_CNT);
alloc_chrdev_region_failed:
register_chrdev_region_failed:
    return ret;
}

static void __exit beep_exit(void)
{
    // 注销设备
    device_destroy(beep.class, beep.devid);
    class_destroy(beep.class);
    // 删除cdev
    cdev_del(&beep.cdev);
    // 注销字符设备驱动
    unregister_chrdev_region(beep.devid, BEEP_CNT);
}

module_init(beep_init);
module_exit(beep_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("duanwenling");