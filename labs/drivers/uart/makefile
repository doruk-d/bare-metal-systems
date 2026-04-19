CC := arm-none-eabi-gcc

MCU_FLAGS := -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb 
CFLAGS := $(MCU_FLAGS) -Wall -O0 -g -MMD -MP -Iinclude/
LDFLAGS := $(MCU_FLAGS) -T ../../boot/stm32f4x/linker.ld -nostartfiles --specs=nano.specs --specs=nosys.specs 
DEPFLAGS := -MMD -MP

BUILD_DIR := build

vpath %.c ../../boot/stm32f4x/ ../../tests/test_uart/ src/

TARGET := $(BUILD_DIR)/libuart.a
SRCS := uart.c  
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

TEST_TARGET := $(BUILD_DIR)/test_output.elf
TEST_SRCS := test_uart.c startup.c  system_clock.c fault_handlers.c 
TEST_OBJS := $(TEST_SRCS:%.c=$(BUILD_DIR)/%.o)
TEST_DEPS := $(TEST_OBJS:.o=.d)


all: $(TARGET)


$(TARGET): $(OBJS) | $(BUILD_DIR)
	ar rcs $@ $^


$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@


test: $(TEST_TARGET)


$(TEST_TARGET): $(TEST_OBJS) $(TARGET) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) $(TEST_OBJS) -L$(BUILD_DIR) -luart -o $@


-include $(DEPS)
-include $(TEST_DEPS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)


flash: $(TEST_TARGET)
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "init" -c "reset halt" -c "program $(TEST_TARGET) verify reset exit"


clean:
	rm -rf $(BUILD_DIR)

.PHONY: all test flash clean

