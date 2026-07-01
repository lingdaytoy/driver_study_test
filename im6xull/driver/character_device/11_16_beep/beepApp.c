/*
 * @Description:beep测试程序
 * @Author: dwl
 * @Date: 2024-11-17 18:02:47
 * @LastEditTime: 2024-11-17 18:16:58
 * @LastEditors: Do not edit
 * @FilePath: /character_device/11_16_beep/beepApp.c
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char const *argv[])
{
    unsigned const char *filename;
    int fd = 0;
    int ret = 0;
    unsigned char databuf[1];

    if (argc != 3)
    {
        printf("Error Usage!\n");
        return -1;
    }

    // 获取设备名
    filename = argv[1];

    // 打开设备
    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("open %s failed!\n", filename);
        return -1;
    }

    databuf[0] = atoi(argv[2]);

    ret = write(fd, databuf, sizeof(databuf));
    if (ret < 0)
    {
        printf("write failed!\n");
        return -1;
    }

    ret = close(fd);
    if (ret < 0)
    {
        printf("close failed!\n");
        return -1;
    }

    return 0;
}
