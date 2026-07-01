/*
 * @Description:led驱动应用程序
 * @Author: dwl
 * @Date: 2024-10-26 23:01:24
 * @LastEditTime: 2024-12-02 20:37:28
 * @LastEditors: Do not edit
 * @FilePath: /character_device/12_02_atomic.c/ledtestApp.c
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    char *filename;
    int fd = 0;
    int res = 0;
    unsigned char databuf[1];
    unsigned char cnt = 0;

    if (argc != 3)
    {
        printf("Error Usage!\r\n");
        return -1;
    }

    filename = argv[1];
    // 打开led驱动
    fd = open(filename, O_RDWR);
    if (fd < 0)
    {
        printf("open %s failed!\r\n", filename);
        return -1;
    }

    databuf[0] = atoi(argv[2]);
    res = write(fd, databuf, sizeof(databuf));
    if (res < 0)
    {
        printf("led control failed!\r\n");
        close(fd);
        return -1;
    }

    // 模拟占用led25s
    while (1)
    {
        sleep(5);
        cnt++;
        printf("LED APP running time:%d.\n", cnt);
        if (cnt >= 25)
        {
            break;
        }
    }

    // 关闭led驱动
    res = close(fd);
    if (res < 0)
    {
        printf("close %s failed!\r\n", filename);
    }

    return 0;
}
