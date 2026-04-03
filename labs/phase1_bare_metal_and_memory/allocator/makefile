# lang rules
CC:= arm-none-eabi-gcc

INCLUDES:= -I../../phase3_peripheral_control_and_dma/stm32f401_UART_driver/ \
		   -I/usr/local/include/cmsis/CMSIS/Core/Include \
		   -I/usr/local/include/cmsis/CMSIS/Device/ST/STM32F4xx/Include

MCU_FLAGS:= -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard  -mthumb 
CFLAGS:= $(MCU_FLAGS) -Wall -O0 -g $(INCLUDES) 
LDFLAGS:= $(MCU_FLAGS) -T ../../shared/linker.ld -nostartfiles --specs=nano.specs --specs=nosys.specs
DEPFLAGS:= -MMD -MP

# files
BUILD_DIR:= build

TARGET:= $(BUILD_DIR)/my_malloc.elf

vpath %c ../../phase3_peripheral_control_and_dma/stm32f401_UART_driver/

SRC:= my_malloc.c startup.c test_driver.c uart.c system_clock.c
OBJS:= $(addprefix $(BUILD_DIR)/, $(notdir $(SRC:.c=.o)))
DEPS:= $(OBJS:%.o=%.d) 

#rules
all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) $^ -o $@ 

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) $< -c -o $@ 

# mkdir step
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(TARGET) $(BUILD_DIR)


-include $(DEPS)

flash:
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "init" -c "reset halt" -c "program $(TARGET) verify reset exit"

.PHONY: all clean flash 
