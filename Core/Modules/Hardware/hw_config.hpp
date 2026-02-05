#pragma once
#ifndef HW_CONFIG_H
#define HW_CONFIG_H


#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_adc.h"
#include "stm32h7xx_hal_dac.h"
#include "stm32h7xx_hal_spi.h"
#include "stm32h7xx_hal_tim.h"
#include "stm32h7xx_hal_qspi.h"
#include "hw_audio.hpp"
#include "hw_clock.hpp"
#include "hw_dma.hpp"
#include "hw_gpio.hpp"
#include "hw_spi.hpp"
#include "hw_timer.hpp"
#include "hw_mpu.hpp"

extern SPI_HandleTypeDef hspi1;
extern QSPI_HandleTypeDef hqspi;
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_dac1;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim8;

#ifdef __cplusplus
extern "C" {
#endif

namespace Hardware {
    void init();
}

#ifdef __cplusplus
}
#endif
#endif