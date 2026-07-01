/*
 * @Description: 字符设备驱动开发步骤
    1-驱动模块的加载与卸载
    2-字符设备的注册与注销
    3-实现设备的具体操作函数
    4-添加LICENSE和作者信息
 * @Author: dwl
 * @Date: 2024-10-17 22:12:43
 * @LastEditTime: 2024-10-26 20:14:24
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>

#define CHRDEVBASE_MAJOR 200         // 主设备号
#define CHRDEVBASE_NAME "chrdevbase" // 设备名

// 读写缓冲区
static char readbuff[100];
static char writebuff[100];
static char kerneldata[] = {"kernel data!"};

static int chrdevbase_open(struct inode *inode, struct file *filp)
{
   return 0;
}

static int chrdevbase_release(struct inode *inode, struct file *filp)
{
   return 0;
}

static ssize_t chrdevbase_read(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
   int res = 0;
   // 向用户空间发送数据
   memcpy(readbuff, kerneldata, sizeof(kerneldata));
   res = copy_to_user(buf, readbuff, count);
   if (res == 0)
   {
      printk("kernel send data ok!\r\n");
   }
   else
   {
      printk(KERN_ERR "kernel send data error!\r\n");
   }
   return 0;
}

static ssize_t chrdevbase_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
   int res = 0;
   res = copy_from_user(writebuff, buf, count);
   if (res == 0)
   {
      printk("kernel receive data:%s.\r\n", writebuff);
   }
   else
   {
      printk(KERN_ERR "kernel receive data error!\r\n");
   }

   return 0;
}

// 设备操作函数的结构体
static const struct file_operations chrdevbase_fops = {
    .owner = THIS_MODULE,
    .open = chrdevbase_open,
    .release = chrdevbase_release,
    .read = chrdevbase_read,
    .write = chrdevbase_write,

};

/**
 * 驱动入口函数
 */
static int __init chrdevbase_init(void)
{
   int res;
   // 注册设备驱动
   res = register_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME, &chrdevbase_fops);
   if (res < 0)
   {
      printk(KERN_ERR "chrdevbase driver register failed!\r\n");
      return res;
   }
   printk("chrdevbase_init()\r\n");
   return 0;
}

/**
 * 驱动出口函数
 */
static void __exit chrdevbase_exit(void)
{
   // 注销设备驱动
   unregister_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME);
   printk("chrdevbase_exit()\r\n");
}
module_init(chrdevbase_init); // 注册模块加载函数
module_exit(chrdevbase_exit); // 注册模块卸载函数

MODULE_LICENSE("GPL");
MODULE_AUTHOR("duanwenling");
