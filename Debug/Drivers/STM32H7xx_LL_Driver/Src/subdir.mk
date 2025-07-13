################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_adc.c \
../Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_dac.c \
../Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_delayblock.c \
../Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_tim.c 

C_DEPS += \
./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_adc.d \
./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_dac.d \
./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_delayblock.d \
./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_tim.d 

OBJS += \
./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_adc.o \
./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_dac.o \
./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_delayblock.o \
./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_tim.o 


# Each subdirectory must supply rules for building sources it contributes
Drivers/STM32H7xx_LL_Driver/Src/%.o Drivers/STM32H7xx_LL_Driver/Src/%.su Drivers/STM32H7xx_LL_Driver/Src/%.cyclo: ../Drivers/STM32H7xx_LL_Driver/Src/%.c Drivers/STM32H7xx_LL_Driver/Src/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=c17 -g -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I"/home/konlio/STM32CubeIDE/workspace_1/oled_test/Core/Effects/Inc" -I"/home/konlio/STM32CubeIDE/workspace_1/oled_test/Drivers/STM32H7xx_LL_Driver/Inc" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-STM32H7xx_LL_Driver-2f-Src

clean-Drivers-2f-STM32H7xx_LL_Driver-2f-Src:
	-$(RM) ./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_adc.cyclo ./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_adc.d ./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_adc.o ./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_adc.su ./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_dac.cyclo ./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_dac.d ./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_dac.o ./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_dac.su ./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_delayblock.cyclo ./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_delayblock.d ./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_delayblock.o ./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_delayblock.su ./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_tim.cyclo ./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_tim.d ./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_tim.o ./Drivers/STM32H7xx_LL_Driver/Src/stm32h7xx_ll_tim.su

.PHONY: clean-Drivers-2f-STM32H7xx_LL_Driver-2f-Src

