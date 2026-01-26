/*
 * mainApp.cpp
 *
 *  Created on: May 30, 2025
 *      Author: kliogka
 */

#include "mainApp.hpp"
#include "editPedal.hpp"
#include "modPedal.hpp"
#include "qspi_flash.hpp"
#include <cstdio>

#define THRESHOLD 4
#define SEL_VIEW_B GPIO_PIN_0
#define SELECT_PEDAL_0 GPIO_PIN_6
#define SELECT_PEDAL_1 GPIO_PIN_15
#define SELECT_PEDAL_2 GPIO_PIN_10
#define SELECT_PEDAL_3 GPIO_PIN_1
#define PEDAL_0 GPIO_PIN_9
#define PEDAL_1 GPIO_PIN_8
#define BUFFER_SIZE 2048
#define HALF_BUFFER_SIZE (BUFFER_SIZE / 2)

__attribute__((section(".sram3"))) __attribute__((aligned(32)))
uint16_t adc_buf[BUFFER_SIZE];

__attribute__((section(".sram3"))) __attribute__((aligned(32)))
uint16_t dac_buf[BUFFER_SIZE];

EffectsChain loadedChain;

uint32_t potValues[3];
uint8_t potTouchedFlags = 0; // one hot encoded : bit 0 = pot0, bit 1 = pot1, bit 2 = pot2
Pedal *selectedPedal = Pedal::createPedal(PedalType::PASS_THROUGH);
uint8_t page = 0;

enum class displayView
{
    PEDALCHAIN_VIEW,
    PEDALSELECT_VIEW,
    PEDALEDIT_VIEW,
};

displayView currentView = displayView::PEDALCHAIN_VIEW;

static ADC_ChannelConfTypeDef adcChannelConfigs[3] = {
    {.Channel = ADC_CHANNEL_10,
     .Rank = ADC_REGULAR_RANK_1,
     .SamplingTime = ADC_SAMPLETIME_810CYCLES_5,
     .SingleDiff = ADC_SINGLE_ENDED,
     .OffsetNumber = ADC_OFFSET_NONE,
     .Offset = 0},
    {.Channel = ADC_CHANNEL_14,
     .Rank = ADC_REGULAR_RANK_1,
     .SamplingTime = ADC_SAMPLETIME_810CYCLES_5,
     .SingleDiff = ADC_SINGLE_ENDED,
     .OffsetNumber = ADC_OFFSET_NONE,
     .Offset = 0},
    {.Channel = ADC_CHANNEL_15,
     .Rank = ADC_REGULAR_RANK_1,
     .SamplingTime = ADC_SAMPLETIME_810CYCLES_5,
     .SingleDiff = ADC_SINGLE_ENDED,
     .OffsetNumber = ADC_OFFSET_NONE,
     .Offset = 0}};

void updateSelectedPedal(uint8_t index)
{
    selectedPedal = loadedChain.getPedal(index);
    loadedChain.selectedPedal = index;

    for (uint8_t x = 0; x < 128; x += 8)
    {
        Display::drawString("        ", x, 15);
    }

    Display::drawBitmap(indicator_bitmap, 26 * index + 21, 15);
}

void togglePedal(uint8_t index, Pedal *pedal)
{
    if (pedal != nullptr)
    {
        pedal->setEnabled(!pedal->isEnabled());

        const Bitmap &bmp = pedal->getImage();
        Display::drawBitmap(bmp, pedalPositionsX[index], PEDAL_PAGE_START);
    }
}

void mainApp(void)
{
    Display::init();
    Display::clear();
    err_code = QSPIFlash::init();
    if (err_code != HAL_OK)
    {
        Display::printf("QSPI Flash: %d", err_code);
        Error_Handler();
    }
    EffectsChain chain;
    chain.setPedal(0, PedalType::OVERDRIVE_DISTORTION);
    chain.setPedal(1, PedalType::ECHO);
    chain.setPedal(2, PedalType::PASS_THROUGH);
    chain.setPedal(3, PedalType::PASS_THROUGH);


    err_code = QSPIFlash::erase_sector(CHAIN_STORAGE_ADDR);

    if (QSPIFlash::erase_sector(CHAIN_STORAGE_ADDR) != HAL_OK)
    {
        Display::printf("QSPI erase Flash: %d", err_code);
        Error_Handler();
    }

    err_code = QSPIFlash::saveEffectsChain(&chain);
    if (QSPIFlash::saveEffectsChain(&chain) != HAL_OK)
    {
        Display::printf("QSPI save Flash: %d", err_code);
        Error_Handler();
    }

    loadedChain.clear();

    err_code = QSPIFlash::loadEffectsChain(&loadedChain);
    if (err_code != HAL_OK)
    {
        Display::printf("QSPI load Flash %d", err_code);
        Error_Handler();
    }

    Display::drawBitmap(base_chain_bitmap, 0, 0);
    loadedChain.draw();
    loadedChain.selectedPedal = 0;
    updateSelectedPedal(loadedChain.selectedPedal);
    currentView = displayView::PEDALCHAIN_VIEW;

    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        adc_buf[i] = 2048;
        dac_buf[i] = 2048;
    }

    __DSB();
    __ISB();
    SCB_CleanDCache_by_Addr((uint32_t *)dac_buf, BUFFER_SIZE * sizeof(uint16_t));

    HAL_ADC_Start_DMA(&hadc1, reinterpret_cast<uint32_t *>(adc_buf), BUFFER_SIZE);
    HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, reinterpret_cast<uint32_t *>(dac_buf), BUFFER_SIZE, DAC_ALIGN_12B_R);

    HAL_TIM_Base_Start(&htim6);

    while (1)
    {
        __NOP();
    }
}

extern "C" void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        loadedChain.process(adc_buf, dac_buf, 0, HALF_BUFFER_SIZE);
        SCB_CleanDCache_by_Addr((uint32_t *)dac_buf, 
                                HALF_BUFFER_SIZE * sizeof(uint16_t));
    }
}

extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        loadedChain.process(adc_buf, dac_buf, HALF_BUFFER_SIZE, HALF_BUFFER_SIZE);
        SCB_CleanDCache_by_Addr((uint32_t *)&dac_buf[HALF_BUFFER_SIZE], 
                                HALF_BUFFER_SIZE * sizeof(uint16_t));
    }
}

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    static uint32_t last_tick = 0;
    uint32_t tick = HAL_GetTick();

    if ((tick - last_tick) > 150)
    {

        switch (GPIO_Pin)
        {
        case SEL_VIEW_B:
            switch (currentView)
            {
            case displayView::PEDALCHAIN_VIEW:
                displaySelectedPedal(selectedPedal);
                currentView = displayView::PEDALSELECT_VIEW;
                break;

            case displayView::PEDALSELECT_VIEW:
                Display::drawBitmap(mod_pedal_bitmap, 0, 0);
                displayPedalSettings(selectedPedal, 0);
                potTouchedFlags = 0;
                currentView = displayView::PEDALEDIT_VIEW;
                break;

            case displayView::PEDALEDIT_VIEW:
                Display::drawBitmap(base_chain_bitmap, 0, 0);
                loadedChain.draw();
                currentView = displayView::PEDALCHAIN_VIEW;
                updateSelectedPedal(0);
                break;
            default:
                break;
            }
            break;

        case PEDAL_0:
            if (currentView == displayView::PEDALEDIT_VIEW)
            {
                Display::drawBitmap(mod_pedal_bitmap, 0, 0);

                displayPedalSettings(selectedPedal, 1);
                page = 1;
                break;
            }

        case PEDAL_1:
            if (currentView == displayView::PEDALSELECT_VIEW)
            {
                PedalType currentType = selectedPedal->getType();

                int direction = (GPIO_Pin == PEDAL_0) ? 1 : -1;
                int newTypeIndex = (static_cast<int>(currentType) + direction + 4) % 4;
                if (newTypeIndex < 0)
                    newTypeIndex += 4;

                selectedPedal = Pedal::createPedal(static_cast<PedalType>(newTypeIndex));

                loadedChain.setPedal(loadedChain.selectedPedal, selectedPedal->getType());
                displaySelectedPedal(selectedPedal);
            }
            else if (currentView == displayView::PEDALEDIT_VIEW)
            {
                Display::drawBitmap(mod_pedal_bitmap, 0, 0);
                displayPedalSettings(selectedPedal, 0);
                page = 0;
            }

            break;

        case SELECT_PEDAL_0:

            switch (currentView)
            {
            case displayView::PEDALCHAIN_VIEW:
                if (loadedChain.selectedPedal == 0)
                {
                    togglePedal(0, selectedPedal);
                }
                else
                {
                    updateSelectedPedal(0);
                }
                break;
            }
            break;

        case SELECT_PEDAL_2:
            switch (currentView)
            {
            case displayView::PEDALCHAIN_VIEW:
                if (loadedChain.selectedPedal == 2)
                {
                    togglePedal(2, selectedPedal);
                }
                else
                {
                    updateSelectedPedal(2);
                }
                break;
            }
            break;

        case SELECT_PEDAL_1:
            switch (currentView)
            {
            case displayView::PEDALCHAIN_VIEW:
                if (loadedChain.selectedPedal == 1)
                {
                    togglePedal(1, selectedPedal);
                }
                else
                {
                    updateSelectedPedal(1);
                }
                break;
            }
            break;

        case SELECT_PEDAL_3:
            switch (currentView)
            {
            case displayView::PEDALCHAIN_VIEW:
                if (loadedChain.selectedPedal == 3)
                {
                    togglePedal(3, selectedPedal);
                }
                else
                {
                    updateSelectedPedal(3);
                }
                break;
            }
            break;

        default:
            break;
        }

        last_tick = tick;
    }
}

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

    if (htim->Instance == TIM8)
    {

        for (int channel = 0; channel < 3; channel++)
        {
            HAL_ADC_ConfigChannel(&hadc2, &adcChannelConfigs[channel]);
            HAL_ADC_Start(&hadc2);

            if (HAL_ADC_PollForConversion(&hadc2, 5) == HAL_OK)
            {
                uint32_t rawValue = 255 - HAL_ADC_GetValue(&hadc2);
                uint32_t normalized_value = (((rawValue * 20) >> 8) * 5) + 5;

                if (normalized_value > 100)
                    normalized_value = 100;

                if (abs((int)normalized_value - (int)potValues[channel]) >= 5)
                {
                    potValues[channel] = normalized_value;
                    if (currentView == displayView::PEDALEDIT_VIEW)
                    {
                        potTouchedFlags |= (1 << channel);
                    }
                }
            }
            HAL_ADC_Stop(&hadc2);
        }

        if (potTouchedFlags && currentView == displayView::PEDALEDIT_VIEW)
        {
            uint8_t numParams = selectedPedal->getMemberSize();
            uint8_t index = (page == 1 && numParams > 3) ? 3 : 0;
            uint8_t endIndex = (numParams < index + 3) ? numParams : index + 3;

            float *params = new float[numParams];
            selectedPedal->getParams(params);

            for (int i = index; i < endIndex; i++)
            {
                uint8_t potIndex = i - index;
                if (potTouchedFlags & (1 << potIndex))
                {
                    params[i] = potValues[potIndex] * 0.01f;
                }
            }
            selectedPedal->setParams(params);
            delete[] params;

            displayPedalSettings(selectedPedal, page);
        }

        potTouchedFlags = 0;
    }
}