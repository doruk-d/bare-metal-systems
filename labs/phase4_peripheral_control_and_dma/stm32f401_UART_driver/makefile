CC := arm-none-eabi-gcc

MCU_FLAGS := -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb 
CFLAGS := $(MCU_FLAGS) -Wall -O0 -g -MMD -MP -I. 
LDFLAGS := $(MCU_FLAGS) -T ../../shared/linker.ld -nostartfiles --specs=nano.specs --specs=nosys.specs 


BUILD_DIR := build

vpath %c ../../shared/

TARGET := $(BUILD_DIR)/output.elf
SRCS := startup.c uart.c main.c system_clock.c 
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)


all: $(TARGET)


$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) $^ -o $@


$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@


-include $(DEPS)


$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)


flash: $(TARGET)
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "init" -c "reset halt" -c "program $(TARGET) verify reset exit"


clean:
	rm -rf $(BUILD_DIR)

.PHONY: all flash clean

