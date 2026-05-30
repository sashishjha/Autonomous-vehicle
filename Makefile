CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

CFLAGS = -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb -O2 -ffunction-sections -fdata-sections -Wall
LDFLAGS = -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb -Wl,--gc-sections -T nrf52833.ld -specs=nosys.specs --specs=nano.specs

INCLUDES = -I. -IRTE/Device/nRF52833_xxAA

# Bare-metal sources only (no FreeRTOS)
SRCS = main.c uart.c radio.c printf.c system_dummy.c
ASM_SRCS = gcc_startup_nrf52833.S

OBJS = $(SRCS:.c=_laptop.o) $(ASM_SRCS:.S=_laptop.o)

.PHONY: all clean

all: laptop_baremetal.hex

%_laptop.o: %.c
	$(CC) $(CFLAGS) -DROLE_LAPTOP $(INCLUDES) -c $< -o $@

%_laptop.o: %.S
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

laptop_baremetal.elf: $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

laptop_baremetal.hex: laptop_baremetal.elf
	$(OBJCOPY) -O ihex $< $@

clean:
	rm -f *.o RTE/Device/nRF52833_xxAA/*.o *.elf *.hex
