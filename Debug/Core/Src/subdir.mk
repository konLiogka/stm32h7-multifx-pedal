################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Src/bitmaps.cpp \
../Core/Src/display.cpp \
../Core/Src/mainApp.cpp \
../Core/Src/qspi_flash.cpp 

C_SRCS += \
../Core/Src/main.c 

C_DEPS += \
./Core/Src/main.d 

OBJS += \
./Core/Src/bitmaps.o \
./Core/Src/display.o \
./Core/Src/main.o \
./Core/Src/mainApp.o \
./Core/Src/qspi_flash.o 

CPP_DEPS += \
./Core/Src/bitmaps.d \
./Core/Src/display.d \
./Core/Src/mainApp.d \
./Core/Src/qspi_flash.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.cpp Core/Src/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=c++17 -g -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I"/home/konlio/STM32CubeIDE/workspace_1/oled_test/Core/Effects/Inc" -I"/home/konlio/STM32CubeIDE/workspace_1/oled_test/Drivers/STM32H7xx_LL_Driver/Inc" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -lstdc++ -lsupc++ -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=c17 -g -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I"/home/konlio/STM32CubeIDE/workspace_1/oled_test/Core/Effects/Inc" -I"/home/konlio/STM32CubeIDE/workspace_1/oled_test/Drivers/STM32H7xx_LL_Driver/Inc" -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/bitmaps.cyclo ./Core/Src/bitmaps.d ./Core/Src/bitmaps.o ./Core/Src/bitmaps.su ./Core/Src/display.cyclo ./Core/Src/display.d ./Core/Src/display.o ./Core/Src/display.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/mainApp.cyclo ./Core/Src/mainApp.d ./Core/Src/mainApp.o ./Core/Src/mainApp.su ./Core/Src/qspi_flash.cyclo ./Core/Src/qspi_flash.d ./Core/Src/qspi_flash.o ./Core/Src/qspi_flash.su

.PHONY: clean-Core-2f-Src

