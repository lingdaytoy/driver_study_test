/*
 * @Description:
 * @Author: dwl
 * @Date: 2024-12-08 20:46:58
 * @LastEditTime: 2024-12-08 21:52:05
 * @LastEditors: Do not edit
 * @FilePath: /character_device/12_08_key/keyApp.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <string.h>

#define INVAKEY 0x00   // 无效的按键值
#define KEY0VALUE 0xf0 // 按键值

int main(int argc, char const *argv[])
{
    char *filename;
    int fd = 0;
    int ret = 0;
    unsigned int keyvalue;

    if (argc != 2)
    {
        printf("error usage!\n");
        return -1;
    }

    filename = argv[1];

    // 打开key驱动
    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("file %s open failed!\n", filename);
        return -1;
    }

    // 循环读取按键数值
    while (1)
    {
        read(fd, &keyvalue, sizeof(keyvalue));
        // printf("keyvalue = %s.\n", keyvalue);
        if (keyvalue == KEY0VALUE)
        {
            printf("key0 press,value = %#X.\n", keyvalue);
        }
    }

    ret = close(fd);
    if (ret < 0)
    {
        printf("close file %s failed!\n", filename);
        return ret;
    }

    return 0;
}
