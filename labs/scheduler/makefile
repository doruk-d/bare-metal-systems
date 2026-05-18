CC:= arm-none-eabi-gcc

INCLUDES:= -Iinclude/ \
           -I../drivers/uart/include/ \
		   -I../boot/stm32f4x/ 

MCUFLAGS:= -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb
CFLAGS:= $(MCUFLAGS) -Wall -O0 -g $(INCLUDES)
LDFLAGS:= $(MCUFLAGS) -T ../boot/stm32f4x/linker.ld -nostartfiles --specs=nano.specs --specs=nosys.specs
DEPFLAGS:= -MMD -MP

BUILD_DIR:= build
UART_LIB:= ../drivers/uart/build/libuart.a

vpath %.c src/ ../boot/stm32f4x/ ../tests/test_scheduler/
vpath %.S src/

TARGET:= $(BUILD_DIR)/scheduler.elf
SRCS:= test_scheduler.c startup.c mpu_config.c system_clock.c dwt.c  scheduler.c  svc.c  systick.c fault_handlers.c measure_gpio.c
ASM_SRCS:= context_switch.S
OBJS:= $(SRCS:%.c=$(BUILD_DIR)/%.o)
ASM_OBJS:= $(ASM_SRCS:%.S=$(BUILD_DIR)/%.o)
DEPS:= $(OBJS:.o=.d)
ASM_DEPS:= $(ASM_OBJS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJS) $(ASM_OBJS) $(UART_LIB) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.S | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) $< -c -o $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) $< -c -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
-include $(ASM_DEPS)

flash:
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "init" -c "reset halt" -c "program $(TARGET) verify reset exit"


.PHONY: all clean flash
