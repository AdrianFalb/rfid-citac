################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../MFRC522/mfrc522.c 

OBJS += \
./MFRC522/mfrc522.o 

C_DEPS += \
./MFRC522/mfrc522.d 


# Each subdirectory must supply rules for building sources it contributes
MFRC522/%.o MFRC522/%.su MFRC522/%.cyclo: ../MFRC522/%.c MFRC522/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F303x8 -c -I../Core/Inc -I../Drivers/STM32F3xx_HAL_Driver/Inc/Legacy -I../Drivers/STM32F3xx_HAL_Driver/Inc -I../Drivers/CMSIS/Device/ST/STM32F3xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/mocar/Desktop/VRS/Zadanie_RFID/rfid-citac/MFRC522" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-MFRC522

clean-MFRC522:
	-$(RM) ./MFRC522/mfrc522.cyclo ./MFRC522/mfrc522.d ./MFRC522/mfrc522.o ./MFRC522/mfrc522.su

.PHONY: clean-MFRC522

