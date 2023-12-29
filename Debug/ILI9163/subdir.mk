################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ILI9163/ili9163.c 

OBJS += \
./ILI9163/ili9163.o 

C_DEPS += \
./ILI9163/ili9163.d 


# Each subdirectory must supply rules for building sources it contributes
ILI9163/%.o ILI9163/%.su ILI9163/%.cyclo: ../ILI9163/%.c ILI9163/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F303x8 -c -I../Core/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -I"C:/Users/mocar/Desktop/VRS/Zadanie_RFID/rfid_citac/ILI9163" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-ILI9163

clean-ILI9163:
	-$(RM) ./ILI9163/ili9163.cyclo ./ILI9163/ili9163.d ./ILI9163/ili9163.o ./ILI9163/ili9163.su

.PHONY: clean-ILI9163

