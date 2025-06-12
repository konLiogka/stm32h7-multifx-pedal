/*
 * mainApp.cpp
 *
 *  Created on: May 30, 2025
 *      Author: kliogka
 */

#include "mainApp.hpp"

#define BUFFER_SIZE 1024
uint16_t adc_buf[BUFFER_SIZE];
uint16_t dac_buf[BUFFER_SIZE];
void mainApp(void)
{

	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buf, BUFFER_SIZE);
	HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)dac_buf, BUFFER_SIZE, DAC_ALIGN_12B_R);

	/* Initialize the display */
    Display::init();
    Display::setContrast(0x01);
    Display::clear();

    while(1)
    {
    	Display::drawBitmap(bitmap1, 128, 64, 0, 0);
    	Display::setContrast(0x01);
    	HAL_Delay(1000);
    	Display::drawBitmap(bitmap2, 128, 64, 0, 0);
    	Display::setContrast(0x01);
    	HAL_Delay(2000);


    }

}
