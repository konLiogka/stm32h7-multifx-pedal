/*
 * mainApp.cpp
 *
 *  Created on: May 30, 2025
 *      Author: kliogka
 */

#include "mainApp.hpp"
#include "qspi_flash.hpp"
#include <cstdio>


#define BUFFER_SIZE 1024
uint16_t adc_buf[BUFFER_SIZE];
uint16_t dac_buf[BUFFER_SIZE];
 
 
 
void mainApp(void)
{
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf, BUFFER_SIZE);
	HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)dac_buf, BUFFER_SIZE, DAC_ALIGN_12B_R);

    Display::init();
    Display::clear();
    Display::drawBitmap(base_chain_bitmap, 0, 0);
    
    if(QSPIFlash::init() != HAL_OK)
    {
        Error_Handler();
    }
    EffectsChain chain;
    chain.setPedal(0, PedalType::REVERB);
    chain.setPedal(1, PedalType::OVERDRIVE_DISTORTION);
    chain.setPedal(2, PedalType::PASS_THROUGH);
    chain.setPedal(3, PedalType::PASS_THROUGH);

    chain.draw();
    HAL_Delay(1000);    

    if (QSPIFlash::saveEffectsChain(&chain) != HAL_OK) {
        Error_Handler();
    }
    
    EffectsChain loadedChain;
    if (QSPIFlash::loadEffectsChain(&loadedChain) != HAL_OK) {
        Error_Handler();
    }
    
    loadedChain.setPedal(3, PedalType::OVERDRIVE_DISTORTION);
    Display::setContrast(0x02);
    while(1)
    {
        HAL_Delay(1000);
        Display::clear();
        HAL_Delay(1);
        Display::drawBitmap(base_chain_bitmap, 0, 0);
        HAL_Delay(1);
        loadedChain.draw();
    }

}






