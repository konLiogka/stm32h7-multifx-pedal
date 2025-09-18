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
#define CONTROL_VIEW_B GPIO_PIN_0
#define SELECT_PEDAL_0 GPIO_PIN_6
#define SELECT_PEDAL_1 GPIO_PIN_15
#define SELECT_PEDAL_2 GPIO_PIN_10
#define SELECT_PEDAL_3 GPIO_PIN_1
#define PEDAL_0 GPIO_PIN_9
#define PEDAL_1 GPIO_PIN_8

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
uint8_t potTouchedFlags = 0; // Bitfield: bit 0 = pot0, bit 1 = pot1, bit 2 = pot2
Pedal* selectedPedal =  Pedal::createPedal(PedalType::PASS_THROUGH);
uint8_t page = 0;


void updateSelectedPedal(uint8_t index)
{
    selectedPedal = loadedChain.getPedal(index); 
    loadedChain.selectedPedal = index;
    
}

void showStartupScreen() {
    const uint8_t steps[] = {16, 8, 4, 2, 1};
    
    for(uint8_t step : steps) {
        Display::clear();
        
        for(uint8_t y = 0; y < 8; y += (step > 8 ? 1 : step)) {
            for(uint8_t x = 0; x < 128; x += step) {
                uint8_t blockX = x + (step/2);
                uint8_t blockY = y + (step > 8 ? 4 : step/2);
                
                if(startup_screen_bitmap.data[blockY * 128 + blockX] & 0x01) {
                    for(uint8_t by = y; by < y + (step > 8 ? 1 : step) && by < 8; by++) {
                        Display::setCursor(x, by);
                        uint8_t pixels = 0xFF;
                        for(uint8_t bx = 0; bx < step && (x + bx) < 128; bx++) {
                            Display::writeData(&pixels, 1);
                        }
                    }
                }
            }
        }
        
        HAL_Delay(150);
    }
    
    Display::drawBitmap(startup_screen_bitmap, 0, 0);
}

void mainApp(void)
{
	// HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)dac_buf, BUFFER_SIZE, DAC_ALIGN_12B_R);

    Display::init();
    Display::clear();
    
    showStartupScreen();

    HAL_Delay(3000);
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
        case CONTROL_VIEW_B: 
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
                break;
            default:
                break;
            }
            break;  
        

        case PEDAL_0: 
        if (currentView == displayView::PEDALEDIT_VIEW) {
                Display::drawBitmap(mod_pedal_bitmap, 0, 0);
                
                displayPedalSettings(selectedPedal, 1);
                page = 1;
                break;
        }
        
             
        case PEDAL_1: 
            if (currentView == displayView::PEDALSELECT_VIEW) {
                PedalType currentType = selectedPedal->getType();
                
                int direction = (GPIO_Pin == PEDAL_0) ? 1 : -1;
                int newTypeIndex = (static_cast<int>(currentType) + direction + 4) % 4;
                if (newTypeIndex < 0) newTypeIndex += 4;
                
                selectedPedal = Pedal::createPedal(static_cast<PedalType>(newTypeIndex));

                loadedChain.setPedal(loadedChain.selectedPedal,  selectedPedal->getType());
                displaySelectedPedal(selectedPedal);
            }else if (currentView == displayView::PEDALEDIT_VIEW) {
                Display::drawBitmap(mod_pedal_bitmap, 0, 0);
                displayPedalSettings(selectedPedal, 0);
                page = 0;
            }
             
            break;

        case SELECT_PEDAL_0: 

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
        



        case SELECT_PEDAL_2: 
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

        case SELECT_PEDAL_1: 
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
        case SELECT_PEDAL_3:
            switch (currentView)
            {
            case displayView::PEDALCHAIN_VIEW:
                if (loadedChain.selectedPedal == 3) {
                    // disablePedal(selectedPedal);
                } else {
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
    uint8_t anyPotTouched = 0x00;
    
    for (int channel = 0; channel < 3; channel++) {
        HAL_ADC_ConfigChannel(&hadc2, &adcChannelConfigs[channel]);
        HAL_ADC_Start(&hadc2);
        
        if (HAL_ADC_PollForConversion(&hadc2, 5) == HAL_OK) {
            uint32_t rawValue = HAL_ADC_GetValue(&hadc2);
            
            uint32_t scaledValue = (rawValue * 25) >> 10;  
            
            uint32_t newValue = ((scaledValue + 2) / 5) * 5;
            
            if (abs((int)newValue - (int)potValues[channel]) >= 10) {
                potValues[channel] = newValue;
                if (currentView == displayView::PEDALEDIT_VIEW) {
                    potTouchedFlags |= (1 << channel);
                    anyPotTouched = 0x01;
                }
            }
        }
        HAL_ADC_Stop(&hadc2);
    }

    if (potTouchedFlags && currentView == displayView::PEDALEDIT_VIEW) {
        uint8_t numParams = selectedPedal->getMemberSize();
        uint8_t index = (page == 1 && numParams > 3) ? 3 : 0;
        uint8_t endIndex = (numParams < index + 3) ? numParams : index + 3;
        
         
        float* params = new float[numParams];
        selectedPedal->getParams(params);
        
        for (int i = index; i < endIndex; i++) {
            uint8_t potIndex = i - index;
            if (potTouchedFlags & (1 << potIndex)) {
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


