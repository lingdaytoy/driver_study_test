/*
 * @Author: dwl
 * @Date: 2024-10-26 17:04:42
 * @LastEditors: Do not edit
 * @LastEditTime: 2024-10-26 17:44:53
 * @Description:驱动测试APP
 * @FilePath: /character_device/10_26_chrdevbase/chrdevbaseApp.c
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

static char userdata[] = {"user data!"};

int main(int argc, char const *argv[])
{
    char *filename;
    int fd = 0;
    char readbuff[100], writebuff[100];
    int retvalue = 0;

    if (argc != 3)
    {
        printf("Error Usage!\r\n");
        return -1;
    }

    filename = argv[1];

    // 打开驱动文件
    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("Can't open file %s\r\n", filename);
        return -1;
    }
    else
    {
        printf("read data %s\r\n", readbuff);
    }

    // 从驱动文件读取数据
    if (atoi(argv[2]) == 1)
    {
        retvalue = read(fd, readbuff, 50);
        if (retvalue < 0)
        {
            printf("read file %s failed!\r\n", filename);
            return -1;
        }
        else
        {
            printf("read data:%s.\r\n", readbuff);
            return 0;
        }
    }

    if (atoi(argv[2]) == 2)
    {
        // 向驱动写数据
        memcpy(writebuff, userdata, sizeof(userdata));
        retvalue = write(fd, writebuff, 50);
        if (retvalue < 0)
        {
            printf("write file %s failed!\r\n", filename);
            return -1;
        }
    }

    // 关闭设备
    retvalue = close(fd);
    if (retvalue < 0)
    {
        printf("Cant't close file %s!\r\n", filename);
        return -1;
    }

    return 0;
}
