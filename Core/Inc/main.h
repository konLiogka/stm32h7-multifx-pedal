#pragma once
#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

    #include "stm32h7xx_hal.h"
    #include "stm32h7xx_hal_adc.h"
    #include "stm32h7xx_hal_spi.h"
    #include "stm32h7xx_hal_tim.h"
    #include "stm32h7xx_hal_qspi.h"

    extern SPI_HandleTypeDef hspi1;
    extern QSPI_HandleTypeDef hqspi;
    extern ADC_HandleTypeDef hadc1;
    extern DAC_HandleTypeDef hdac1;
    extern ADC_HandleTypeDef hadc2;

    extern DMA_HandleTypeDef hdma_adc1;

    extern DMA_HandleTypeDef hdma_dac1;
    extern TIM_HandleTypeDef htim6;


    void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
    #include "mainApp.hpp"
#endif

#endif /* MAIN_H */
