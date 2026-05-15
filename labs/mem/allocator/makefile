# lang rules
CC:= arm-none-eabi-gcc

INCLUDES:= -Iinclude/ \
		   -Isrc/ \
		   -I../../drivers/uart/include/ \
		   -I../../boot/stm32f4x/ \
		   -I/usr/local/include/cmsis/CMSIS/Core/Include/ \
		   -I/usr/local/include/cmsis/CMSIS/Device/ST/STM32F4xx/Include/

MCU_FLAGS:= -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard  -mthumb 
CFLAGS:= $(MCU_FLAGS) -Wall -O0 -g $(INCLUDES) 
LDFLAGS:= $(MCU_FLAGS) -T ../../boot/stm32f4x/linker.ld -nostartfiles --specs=nano.specs --specs=nosys.specs
DEPFLAGS:= -MMD -MP


# files
BUILD_DIR:= build
UART_DIR:=  ../../drivers/uart/build/
UART_LIB:=  ../../drivers/uart/build/libuart.a

vpath %.c src ../../boot/stm32f4x/ ../../tests/test_allocator/


TARGET:= $(BUILD_DIR)/libmyalloc.a
SRCS:= my_malloc.c 
OBJS:= $(SRCS:%.c=$(BUILD_DIR)/%.o)
DEPS:= $(OBJS:.o=.d)


TEST_TARGET:= $(BUILD_DIR)/test_my_malloc.elf 
TEST_SRCS:= test_allocator.c test_startup.c system_clock.c mpu_config.c 
TEST_OBJS:= $(TEST_SRCS:%.c=$(BUILD_DIR)/%.o)
TEST_DEPS:= $(TEST_OBJS:.o=.d)


#rules
all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	ar rcs $@ $^

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) $< -c -o $@ 


test: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS) $(TARGET) $(UART_LIB)| $(BUILD_DIR)
	$(CC) $(LDFLAGS) $(TEST_OBJS) -L$(BUILD_DIR) -lmyalloc -L$(UART_DIR) -luart -o $@


# mkdir step
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)


clean:
	rm -rf $(BUILD_DIR)


-include $(DEPS)

-include $(TEST_DEPS)

flash:
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "init" -c "reset halt" -c "program $(TEST_TARGET) verify reset exit"

.PHONY: all test clean flash 
