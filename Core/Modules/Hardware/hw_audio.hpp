#pragma once
#ifndef HW_AUDIO_H
#define HW_AUDIO_H

#include "hw_config.hpp"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern DAC_HandleTypeDef hdac1;

namespace Hardware
{
    namespace ADC
    {
        void init(void);
    }

    namespace DAC
    {
        void init(void);
    }
}

#endif 