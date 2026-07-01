CROSS_COMPILE	?=	arm-linux-gnueabihf-
NAME		 	?=	ledc

CC				:=	$(CROSS_COMPILE)gcc
LD				:=	$(CROSS_COMPILE)ld
OBJCOPY			:=	$(CROSS_COMPILE)objcopy
OBJDUMP 		:=	$(CROSS_COMPILE)objdump

INCDIRS			:=	imx6ull	\
					bsp/clk	\
					bsp/led	\
					bsp/delay
SRCDIRS			:=	project	\
					bsp/clk	\
					bsp/led	\
					bsp/delay

INCLUDE			:=	$(patsubst %, -I %, $(INCDIRS))

SFILES			:=	$(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.S))
CFILES			:=	$(foreach dir, $(SRCDIRS), $(wildcard $(dir)/*.c))

SFILENDIR		:=	$(notdir	$(SFILES))
CFILENDIR		:=	$(notdir	$(CFILES))

SOBJS			:=	$(patsubst %, obj/%, $(SFILENDIR:.S=.o))
COBJS			:=	$(patsubst %, obj/%, $(CFILENDIR:.c=.o))
OBJS			:=	$(SOBJS)$(COBJS)

VPATH			:=	$(SRCDIRS)

.PHONY: clean

$(NAME).bin:$(OBJS)
	$(LD) -Timx6ull.lds -o $(NAME).elf $^
	$(OBJCOPY) -O binary -S $(NAME).elf $@
	$(OBJDUMP) -D -m arm $(NAME).elf > $(NAME).dis

$(SOBJS) : obj/%.o : %.S
	$(CC) -Wall -nostdlib -c -O2 $(INCLUDE) -o $@ $<
$(COBJS) : obj/%.o : %.c
	$(CC) -Wall -nostdlib -c -O2 $(INCLUDE) -o $@ $<

clean:
	rm -rf *.o $(NAME).bin $(NAME).elf $(NAME).dis $(OBJS)