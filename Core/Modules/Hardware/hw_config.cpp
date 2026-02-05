#include "hw_config.hpp"
#include "display.hpp"

void Hardware::init(void)
{
  Hardware::Clock::init();

  Hardware::GPIO::init();
  Hardware::SPI::init();

  Display::init();

  Hardware::Timer::init();

  Hardware::ADC::init();
  Hardware::DAC::init();
  
  err_code = HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
  if (err_code != HAL_OK)
  {
    Display::printf("ADC1 Calib: %d", err_code);
  }

  err_code = HAL_ADCEx_Calibration_Start(&hadc2, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
  if (err_code != HAL_OK)
  {
    Display::printf("ADC2 Calib: %d", err_code);
  }

  err_code = HAL_DACEx_SelfCalibrate(&hdac1, DAC_CHANNEL_1, DAC_TRIGGER_NONE);
  if (err_code != HAL_OK)
  {
    Display::printf("DAC1 Calib: %d", err_code);
  }

  err_code = HAL_TIM_Base_Start_IT(&htim8);
  if (err_code != HAL_OK)
  {
    Display::printf("TIM8 Start: %d", err_code);
  }

  Hardware::DMA::init();
  Hardware::MemoryProtectionUnit::init();

}