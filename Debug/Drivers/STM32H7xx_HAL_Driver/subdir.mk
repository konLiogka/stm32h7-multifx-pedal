################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/STM32H7xx_HAL_Driver/stm32h7xx_hal_msp.c \
../Drivers/STM32H7xx_HAL_Driver/stm32h7xx_it.c \
../Drivers/STM32H7xx_HAL_Driver/syscalls.c \
../Drivers/STM32H7xx_HAL_Driver/sysmem.c \
../Drivers/STM32H7xx_HAL_Driver/system_stm32h7xx.c 

C_DEPS += \
./Drivers/STM32H7xx_HAL_Driver/stm32h7xx_hal_msp.d \
./Drivers/STM32H7xx_HAL_Driver/stm32h7xx_it.d \
./Drivers/STM32H7xx_HAL_Driver/syscalls.d \
./Drivers/STM32H7xx_HAL_Driver/sysmem.d \
./Drivers/STM32H7xx_HAL_Driver/system_stm32h7xx.d 

OBJS += \
./Drivers/STM32H7xx_HAL_Driver/stm32h7xx_hal_msp.o \
./Drivers/STM32H7xx_HAL_Driver/stm32h7xx_it.o \
./Drivers/STM32H7xx_HAL_Driver/syscalls.o \
./Drivers/STM32H7xx_HAL_Driver/sysmem.o \
./Drivers/STM32H7xx_HAL_Driver/system_stm32h7xx.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/STM32H7xx_HAL_Driver/%.o Drivers/STM32H7xx_HAL_Driver/%.su Drivers/STM32H7xx_HAL_Driver/%.cyclo: ../Drivers/STM32H7xx_HAL_Driver/%.c Drivers/STM32H7xx_HAL_Driver/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=c17 -g -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I"/home/konlio/STM32CubeIDE/workspace_1/oled_test/Core/Effects/Inc" -I"/home/konlio/STM32CubeIDE/workspace_1/oled_test/Drivers/STM32H7xx_LL_Driver/Inc" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/STM32H7xx_HAL_Driver/syscalls.o: ../Drivers/STM32H7xx_HAL_Driver/syscalls.c Drivers/STM32H7xx_HAL_Driver/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=c17 -g -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I"/home/konlio/STM32CubeIDE/workspace_1/oled_test/Core/Effects/Inc" -I"/home/konlio/STM32CubeIDE/workspace_1/oled_test/Drivers/STM32H7xx_LL_Driver/Inc" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-STM32H7xx_HAL_Driver

clean-Drivers-2f-STM32H7xx_HAL_Driver:
	-$(RM) ./Drivers/STM32H7xx_HAL_Driver/stm32h7xx_hal_msp.cyclo ./Drivers/STM32H7xx_HAL_Driver/stm32h7xx_hal_msp.d ./Drivers/STM32H7xx_HAL_Driver/stm32h7xx_hal_msp.o ./Drivers/STM32H7xx_HAL_Driver/stm32h7xx_hal_msp.su ./Drivers/STM32H7xx_HAL_Driver/stm32h7xx_it.cyclo ./Drivers/STM32H7xx_HAL_Driver/stm32h7xx_it.d ./Drivers/STM32H7xx_HAL_Driver/stm32h7xx_it.o ./Drivers/STM32H7xx_HAL_Driver/stm32h7xx_it.su ./Drivers/STM32H7xx_HAL_Driver/syscalls.cyclo ./Drivers/STM32H7xx_HAL_Driver/syscalls.d ./Drivers/STM32H7xx_HAL_Driver/syscalls.o ./Drivers/STM32H7xx_HAL_Driver/syscalls.su ./Drivers/STM32H7xx_HAL_Driver/sysmem.cyclo ./Drivers/STM32H7xx_HAL_Driver/sysmem.d ./Drivers/STM32H7xx_HAL_Driver/sysmem.o ./Drivers/STM32H7xx_HAL_Driver/sysmem.su ./Drivers/STM32H7xx_HAL_Driver/system_stm32h7xx.cyclo ./Drivers/STM32H7xx_HAL_Driver/system_stm32h7xx.d ./Drivers/STM32H7xx_HAL_Driver/system_stm32h7xx.o ./Drivers/STM32H7xx_HAL_Driver/system_stm32h7xx.su

.PHONY: clean-Drivers-2f-STM32H7xx_HAL_Driver

