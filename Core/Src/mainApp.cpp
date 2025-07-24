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

#define BUFFER_SIZE 1024
#define THRESHOLD 4

enum class displayView {
    PEDALCHAIN_VIEW,
    PEDALSELECT_VIEW,
    PEDALEDIT_VIEW,
};  
EffectsChain loadedChain;
displayView currentView =  displayView::PEDALCHAIN_VIEW;

#define BUFFER_SIZE 1024
uint16_t adc_buf[BUFFER_SIZE] __attribute__((aligned(4)));  
uint16_t dac_buf[BUFFER_SIZE] __attribute__((aligned(4))); 

uint32_t potValues[3]; 
bool potTouched[3] = {false, false, false};
Pedal* selectedPedal =  Pedal::createPedal(PedalType::PASS_THROUGH);



void updateSelectedPedal(uint8_t index)
{
    selectedPedal = loadedChain.getPedal(index); 
    loadedChain.selectedPedal = index;
}

void mainApp(void)
{
	// HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)dac_buf, BUFFER_SIZE, DAC_ALIGN_12B_R);

    Display::init();
    Display::clear();
    Display::drawBitmap(base_chain_bitmap, 0, 0);
    
 
    if(QSPIFlash::init() != HAL_OK)
    {
        Display::displayError("QSPI Flash", QSPIFlash::init() );
        Error_Handler();
    }

    EffectsChain chain;

    chain.setPedal(0, PedalType::OVERDRIVE_DISTORTION);
    chain.setPedal(1, PedalType::REVERB);
    chain.setPedal(2, PedalType::ECHO);
    chain.setPedal(3, PedalType::PASS_THROUGH);

    if (QSPIFlash::erase_sector(CHAIN_STORAGE_ADDR) != HAL_OK) {
        Display::displayError("QSPI erase Flash", QSPIFlash::erase_sector(CHAIN_STORAGE_ADDR));
        Error_Handler();
    }

    if (QSPIFlash::saveEffectsChain(&chain) != HAL_OK) {
        Display::displayError("QSPI save Flash", QSPIFlash::saveEffectsChain(&chain)  );
        Error_Handler();
    }
    loadedChain.clear();
    if (QSPIFlash::loadEffectsChain(&loadedChain) != HAL_OK) {
        Display::displayError("QSPI load Flash", QSPIFlash::loadEffectsChain(&loadedChain)  );
        Error_Handler();
    }

    loadedChain.draw();
    loadedChain.selectedPedal = 0;
    updateSelectedPedal(loadedChain.selectedPedal); 

    currentView = displayView::PEDALCHAIN_VIEW;

    while(true)
    {
        HAL_Delay(100);
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
        case GPIO_PIN_0: 
            switch (currentView)
            {
            case displayView::PEDALCHAIN_VIEW:
                displaySelectedPedal(selectedPedal);
                currentView = displayView::PEDALSELECT_VIEW;
                break;

            case displayView::PEDALSELECT_VIEW:
                Display::drawBitmap(mod_pedal_bitmap, 0, 0);
                displayPedalSettings(selectedPedal, 0);
                potTouched[0] = potTouched[1] = potTouched[2] = false;
                currentView = displayView::PEDALEDIT_VIEW;
                break;

            case displayView::PEDALEDIT_VIEW:
                Display::drawBitmap(base_chain_bitmap, 0, 0);
                loadedChain.draw();
                currentView = displayView::PEDALCHAIN_VIEW;
                break;
            default:
                break;
            }
            break;  
        

        case GPIO_PIN_1: 
            break;
        case GPIO_PIN_6: 

            switch (currentView)
            {
            case displayView::PEDALCHAIN_VIEW:
                if (loadedChain.selectedPedal == 0) {
                    // disablePedal(selectedPedal);
                } else {
                    updateSelectedPedal(0);
                }
                break;
            
            }
            break;
        

        case GPIO_PIN_8: 
            switch (currentView)
            {
                // handle view here
            }
            break;
        

        case GPIO_PIN_9:
        case GPIO_PIN_10: 
            switch (currentView)
            {
            case displayView::PEDALCHAIN_VIEW:
                if (loadedChain.selectedPedal == 2) {
                    // disablePedal(selectedPedal);
                } else {
                    updateSelectedPedal(2);
                }
                break;
            }
            break;
        
 


        case GPIO_PIN_15: 
            switch (currentView)
            {
            case displayView::PEDALCHAIN_VIEW:
                if (loadedChain.selectedPedal == 1) {
                    // disablePedal(selectedPedal);
                } else {
                    updateSelectedPedal(1);
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

void displayVolume(void)
{
    
}

static ADC_ChannelConfTypeDef adcChannelConfigs[3] = {
    {
        .Channel = ADC_CHANNEL_18,
        .Rank = ADC_REGULAR_RANK_1,
        .SamplingTime = ADC_SAMPLETIME_810CYCLES_5,
        .SingleDiff = ADC_SINGLE_ENDED,
        .OffsetNumber = ADC_OFFSET_NONE,
        .Offset = 0
    },
    {
        .Channel = ADC_CHANNEL_14,
        .Rank = ADC_REGULAR_RANK_1,
        .SamplingTime = ADC_SAMPLETIME_810CYCLES_5,
        .SingleDiff = ADC_SINGLE_ENDED,
        .OffsetNumber = ADC_OFFSET_NONE,
        .Offset = 0
    },
    {
        .Channel = ADC_CHANNEL_15,
        .Rank = ADC_REGULAR_RANK_1,
        .SamplingTime = ADC_SAMPLETIME_810CYCLES_5,
        .SingleDiff = ADC_SINGLE_ENDED,
        .OffsetNumber = ADC_OFFSET_NONE,
        .Offset = 0
    }
};

extern "C"  void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM8) {
        bool anyPotTouched = false;
        
        for (int channel = 0; channel < 3; channel++) {
            HAL_ADC_ConfigChannel(&hadc2, &adcChannelConfigs[channel]);
            
            HAL_ADC_Start(&hadc2);
            if (HAL_ADC_PollForConversion(&hadc2, 5) == HAL_OK) {
                uint32_t rawValue = HAL_ADC_GetValue(&hadc2);
                uint32_t newValue = (((rawValue * 20) >> 12) * 5) + 5;
                
                if (abs((int)newValue - (int)potValues[channel]) > THRESHOLD) {
                    potValues[channel] = newValue;
                    if (currentView == displayView::PEDALEDIT_VIEW) {
                        potTouched[channel] = true;
                        anyPotTouched = true;
                    }
                }
            }
            HAL_ADC_Stop(&hadc2);
        }

        switch(currentView) {
            case displayView::PEDALCHAIN_VIEW:
            case displayView::PEDALSELECT_VIEW:
                displayVolume();
                break;
                
            case displayView::PEDALEDIT_VIEW:
                if (anyPotTouched) {
                    float* params = new float[selectedPedal->getMemberSize()];
                    selectedPedal->getParams(params);
                    
                    for (int i = 0; i < 3 && i < selectedPedal->getMemberSize(); i++) {
                        if (potTouched[i]) {
                            params[i] = potValues[i] / 100.0f;
                        }
                    }
                    
                    selectedPedal->setParams(params);
                    displayPedalSettings(selectedPedal, 0);
                    delete[] params;
                }
                break;
        }
    }
}


