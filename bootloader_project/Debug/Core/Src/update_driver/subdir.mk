################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/update_driver/update_driver.c 

OBJS += \
./Core/Src/update_driver/update_driver.o 

C_DEPS += \
./Core/Src/update_driver/update_driver.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/update_driver/%.o Core/Src/update_driver/%.su: ../Core/Src/update_driver/%.c Core/Src/update_driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-update_driver

clean-Core-2f-Src-2f-update_driver:
	-$(RM) ./Core/Src/update_driver/update_driver.d ./Core/Src/update_driver/update_driver.o ./Core/Src/update_driver/update_driver.su

.PHONY: clean-Core-2f-Src-2f-update_driver

