/*参考stm32寄存器定义方法定义IMX6ULL相关寄存器*/

/*外设寄存器基地址*/
#define CCM_BASE					(0X020C4000)
#define IOMUX_SW_MUX_BASE			(0X020E0044)
#define IOMUX_SW_PAD_BASE			(0X020E02D0)
#define GPIO1_BASE                  (0x0209C000)

typedef struct 
{
	volatile unsigned int CCR;
	volatile unsigned int CCDR;
	volatile unsigned int CSR;
	volatile unsigned int CCSR;
	volatile unsigned int CACRR;
	volatile unsigned int CBCDR;
	volatile unsigned int CBCMR;
	volatile unsigned int CSCMR1;
	volatile unsigned int CSCMR2;
	volatile unsigned int CSCDR1;
	volatile unsigned int CS1CDR;
	volatile unsigned int CS2CDR;
	volatile unsigned int CDCDR;
	volatile unsigned int CHSCCDR;
	volatile unsigned int CSCDR2;
	volatile unsigned int CSCDR3;	
	volatile unsigned int RESERVED_1[2];
	volatile unsigned int CDHIPR;  
	volatile unsigned int RESERVED_2[2];
	volatile unsigned int CLPCR;
	volatile unsigned int CISR;
	volatile unsigned int CIMR;
	volatile unsigned int CCOSR;
	volatile unsigned int CGPR;
	volatile unsigned int CCGR0;
	volatile unsigned int CCGR1;
	volatile unsigned int CCGR2;
	volatile unsigned int CCGR3;
	volatile unsigned int CCGR4;
	volatile unsigned int CCGR5;
	volatile unsigned int CCGR6;
	volatile unsigned int RESERVED_3[1];
	volatile unsigned int CMEOR;	
} CCM_TYPE; 

typedef struct 
{
    volatile unsigned int JTAG_MOD;
	volatile unsigned int JTAG_TMS;
	volatile unsigned int JTAG_TDO;
	volatile unsigned int JTAG_TDI;
	volatile unsigned int JTAG_TCK;
	volatile unsigned int JTAG_TRST_B;
	volatile unsigned int GPIO1_IO00;
	volatile unsigned int GPIO1_IO01;
	volatile unsigned int GPIO1_IO02;
	volatile unsigned int GPIO1_IO03;
	volatile unsigned int GPIO1_IO04;
	volatile unsigned int GPIO1_IO05;
	volatile unsigned int GPIO1_IO06;
	volatile unsigned int GPIO1_IO07;
	volatile unsigned int GPIO1_IO08;
	volatile unsigned int GPIO1_IO09;
}IOMUX_SW_MUX_TYPE;

typedef struct 
{
    volatile unsigned int JTAG_MOD;
	volatile unsigned int JTAG_TMS;
	volatile unsigned int JTAG_TDO;
	volatile unsigned int JTAG_TDI;
	volatile unsigned int JTAG_TCK;
	volatile unsigned int JTAG_TRST_B;
	volatile unsigned int GPIO1_IO00;
	volatile unsigned int GPIO1_IO01;
	volatile unsigned int GPIO1_IO02;
	volatile unsigned int GPIO1_IO03;
	volatile unsigned int GPIO1_IO04;
	volatile unsigned int GPIO1_IO05;
	volatile unsigned int GPIO1_IO06;
	volatile unsigned int GPIO1_IO07;
	volatile unsigned int GPIO1_IO08;
	volatile unsigned int GPIO1_IO09;
}IOMUX_SW_PAD_TYPE;

/* 
 * GPIO寄存器结构体
 */
typedef struct 
{
	volatile unsigned int DR;							
	volatile unsigned int GDIR; 							
	volatile unsigned int PSR;								
	volatile unsigned int ICR1; 							
	volatile unsigned int ICR2; 							 
	volatile unsigned int IMR;								 
	volatile unsigned int ISR;			
	volatile unsigned int EDGE_SEL;  
}GPIO_Type;

/*外设指针*/
#define CCM                 ((CCM_TYPE *)CCM_BASE)
#define IOMUX_SW_MUX		((IOMUX_SW_MUX_TYPE *)IOMUX_SW_MUX_BASE)
#define IOMUX_SW_PAD        ((IOMUX_SW_PAD_TYPE *)IOMUX_SW_PAD_BASE)
#define GPIO1				((GPIO_Type *)GPIO1_BASE)

