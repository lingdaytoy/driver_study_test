/**
 * 开发板启动文件
 * 完成C语言环境的初始化
 * C环境初始化后跳转到C代码
*/
.global _start

.global __bss_start
_bss_start:
	.word __bss_start

.global __bss_end
_bss_end:
    .word __bss_end

/*_start函数，程序从此函数开始执行--主要用于设置C运行环境 */
_start:
    /*进入SVC模式 */
    mrs r0, cpsr
    bic r0, r0, #0x1f   //将r0寄存器中的低5位清零
    orr r0, r0, #0x13   //r0或0x13，表示使用SVC模式
    msr cpsr, r0        //将r0的数据写入到cpsr_c中

    /*将BSS段清零，进行初始化 */
    ldr r0, _bss_start
    ldr r1, _bss_end
    mov r2, #0
bss_loop:
    stmia r0!, {r2}     //向r0地址写入0，然后r0寄存器保存的地址值加1
    cmp r0, r1          //比较r0,r1，也就是比较__bss_start,__bss_end
    ble bss_loop        //小于就跳转到bss_loop,继续清0

    /*设置sp指针 */
    ldr sp, =0x80200000
    b main