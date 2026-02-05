#pragma once
#ifndef HW_SPI_H
#define HW_SPI_H

#include "hw_config.hpp"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern DAC_HandleTypeDef hdac1;

namespace Hardware
{
    namespace SPI
    {
        void init(void);
    }
}

#endif 