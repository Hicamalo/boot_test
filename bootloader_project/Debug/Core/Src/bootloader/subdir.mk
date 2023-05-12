################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/bootloader/bootloader.c \
../Core/Src/bootloader/bootloader_execution.c \
../Core/Src/bootloader/bootloader_flash.c \
../Core/Src/bootloader/bootloader_uart.c \
../Core/Src/bootloader/bootloader_utilities.c 

OBJS += \
./Core/Src/bootloader/bootloader.o \
./Core/Src/bootloader/bootloader_execution.o \
./Core/Src/bootloader/bootloader_flash.o \
./Core/Src/bootloader/bootloader_uart.o \
./Core/Src/bootloader/bootloader_utilities.o 

C_DEPS += \
./Core/Src/bootloader/bootloader.d \
./Core/Src/bootloader/bootloader_execution.d \
./Core/Src/bootloader/bootloader_flash.d \
./Core/Src/bootloader/bootloader_uart.d \
./Core/Src/bootloader/bootloader_utilities.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/bootloader/%.o Core/Src/bootloader/%.su: ../Core/Src/bootloader/%.c Core/Src/bootloader/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-bootloader

clean-Core-2f-Src-2f-bootloader:
	-$(RM) ./Core/Src/bootloader/bootloader.d ./Core/Src/bootloader/bootloader.o ./Core/Src/bootloader/bootloader.su ./Core/Src/bootloader/bootloader_execution.d ./Core/Src/bootloader/bootloader_execution.o ./Core/Src/bootloader/bootloader_execution.su ./Core/Src/bootloader/bootloader_flash.d ./Core/Src/bootloader/bootloader_flash.o ./Core/Src/bootloader/bootloader_flash.su ./Core/Src/bootloader/bootloader_uart.d ./Core/Src/bootloader/bootloader_uart.o ./Core/Src/bootloader/bootloader_uart.su ./Core/Src/bootloader/bootloader_utilities.d ./Core/Src/bootloader/bootloader_utilities.o ./Core/Src/bootloader/bootloader_utilities.su

.PHONY: clean-Core-2f-Src-2f-bootloader

