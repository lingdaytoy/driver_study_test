/*
 * @Description: 按键实验
 * @Author: dwl
 * @Date: 2024-12-08 15:41:47
 * @LastEditTime: 2024-12-08 21:50:54
 * @LastEditors: Do not edit
 * @FilePath: /character_device/12_08_key/key.c
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
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>

#define KEY_CNT 1
#define KEY_NAME "key"

#define INVAKEY 0x00   // 无效的按键值
#define KEY0VALUE 0xf0 // 按键值

// 5.key设备结构体
struct key_dev
{
    dev_t devid;              // 设备号
    struct cdev cdev;         // 字符设备结构体
    struct class *class;      // 类
    struct device *device;    // 设备
    int major;                // 主设备号
    int minor;                // 次设备号
    struct device_node *node; // 设备节点
    int key_gpio;             // key所使用gpio编号
    atomic_t keyvalue;        // 按键值
};
struct key_dev keydev;

// 初始化按键IO
static int keyio_init(void)
{
    int ret = 0;

    // 获取设备节点
    keydev.node = of_find_node_by_path("/key");
    if (keydev.node == NULL)
    {
        printk(KERN_ERR "key: node not found.\n");
        return -EINVAL;
    }
    else
    {
        printk(KERN_INFO "key: node found.\n");
    }

    // 获取设备树中key的属性，得到key所使用的gpio编号
    keydev.key_gpio = of_get_named_gpio(keydev.node, "key-gpio", 0);
    if (keydev.key_gpio < 0)
    {
        printk(KERN_ERR "key: get key-gpio failed.\n");
        return -EINVAL;
    }
    else
    {
        printk(KERN_INFO "key: get key-gpio is %d.\n", keydev.key_gpio);
    }

    // 初始化key0所用的gpio
    gpio_request(keydev.key_gpio, "key0");
    ret = gpio_direction_input(keydev.key_gpio);
    if (ret < 0)
    {
        printk(KERN_ERR "gpioled: set led-gpio input failed.\n");
        return ret;
    }

    return 0;
}

static int key_open(struct inode *inode, struct file *file)
{
    int ret = 0;
    // 设置私有数据
    file->private_data = &keydev;

    // 初始化按键io
    ret = keyio_init();
    if (ret < 0)
    {
        printk(KERN_ERR "key: ley io init failed.\n");
        return ret;
    }

    return 0;
}

static int key_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t key_read(struct file *file, char __user *buf, size_t count, loff_t *off)
{
    int ret = 0;
    int value; // 如果是unsigned char value 在执行./keyApp /dev/key 并按下key0时，会报错Segmentation fault
    // char value;
    struct key_dev *dev = file->private_data;

    // 处理按键
    if (gpio_get_value(dev->key_gpio) == 0)
    {
        while (!gpio_get_value(dev->key_gpio))
            ;
        atomic_set(&dev->keyvalue, KEY0VALUE);
    }
    else
    {
        atomic_set(&dev->keyvalue, INVAKEY);
    }

    // 读取按键值
    value = atomic_read(&dev->keyvalue);
    ret = copy_to_user(buf, &value, sizeof(value));
    if (ret < 0)
    {
        printk(KERN_ERR "key: copy failed.\n");
        return ret;
    }

    return 0;
}

static ssize_t key_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
    return 0;
}

// 6.设备操作函数
static struct file_operations key_fops = {
    .owner = THIS_MODULE,
    .open = key_open,
    .release = key_release,
    .read = key_read,
    .write = key_write,
};

// 3.驱动注册
static int __init key_init(void)
{
    int ret = 0;

    // 初始化原子变量
    atomic_set(&keydev.keyvalue, INVAKEY);

    // 注册字符设备
    if (keydev.major)
    {
        keydev.devid = MKDEV(keydev.major, 0);
        ret = register_chrdev_region(keydev.devid, KEY_CNT, KEY_NAME);
        if (ret)
        {
            printk(KERN_ERR "key:could not register region.\n");
            goto register_chrdev_region_failed;
        }
    }
    else
    {
        ret = alloc_chrdev_region(&keydev.devid, 0, KEY_CNT, KEY_NAME);
        if (ret)
        {
            printk(KERN_ERR "key:could not chrdev region.\n");
            goto alloc_chrdev_region_failed;
        }
    }

    // 获取主次设备号
    keydev.major = MAJOR(keydev.devid);
    keydev.minor = MINOR(keydev.devid);
    printk(KERN_INFO "key: major = %d,minor = %d.\n", keydev.major, keydev.minor);

    // 初始化cdev
    keydev.cdev.owner = THIS_MODULE;
    cdev_init(&keydev.cdev, &key_fops);

    // 添加一个cdev
    cdev_add(&keydev.cdev, keydev.devid, KEY_CNT);

    // 创建类
    keydev.class = class_create(THIS_MODULE, KEY_NAME);
    if (IS_ERR(keydev.class))
    {
        printk(KERN_ERR "key: create class failed.\n");
        goto create_class_error;
    }

    // 创建设备
    keydev.device = device_create(keydev.class, NULL, keydev.devid, NULL, KEY_NAME);
    if (keydev.device == NULL)
    {
        printk(KERN_ERR "key: create device failed.\n");
        goto create_device_error;
    }

    return 0;

create_device_error:
    class_destroy(keydev.class);
create_class_error:
    cdev_del(&keydev.cdev);
    unregister_chrdev_region(keydev.devid, KEY_CNT);
alloc_chrdev_region_failed:
register_chrdev_region_failed:
    return ret;
}

// 4.驱动注销
static void __exit key_exit(void)
{
    // 注销gpio
    gpio_free(keydev.key_gpio);
    // 注销设备
    device_destroy(keydev.class, keydev.devid);
    // 注销类
    class_destroy(keydev.class);
    // 注销cdev
    cdev_del(&keydev.cdev);
    // 注销字符设备
    unregister_chrdev_region(keydev.devid, KEY_CNT);
}
// 1.模块驱动的注册与注销
module_init(key_init);
module_exit(key_exit);

// 2.声明模块信息和作者信息
MODULE_LICENSE("GPL");
MODULE_AUTHOR("duanwneling");