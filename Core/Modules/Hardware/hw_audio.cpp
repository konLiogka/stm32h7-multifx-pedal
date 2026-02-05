#include "hw_audio.hpp"
#include "display.hpp"

extern uint8_t err_code;

ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;
DAC_HandleTypeDef hdac1;

namespace Hardware
{
  namespace ADC
  {
    void init(void)
    {
      ADC_ChannelConfTypeDef sConfig = {0};

      hadc1.Instance                      = ADC1;
      hadc1.Init.ClockPrescaler           = ADC_CLOCK_ASYNC_DIV2;
      hadc1.Init.Resolution               = ADC_RESOLUTION_12B;
      hadc1.Init.ScanConvMode             = DISABLE;
      hadc1.Init.EOCSelection             = ADC_EOC_SEQ_CONV;
      hadc1.Init.LowPowerAutoWait         = DISABLE;
      hadc1.Init.ContinuousConvMode       = DISABLE;
      hadc1.Init.NbrOfConversion          = 1;
      hadc1.Init.DiscontinuousConvMode    = DISABLE;
      hadc1.Init.ExternalTrigConv         = ADC_EXTERNALTRIG_T6_TRGO;
      hadc1.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_RISING;
      hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;
      hadc1.Init.Overrun                  = ADC_OVR_DATA_OVERWRITTEN;
      hadc1.Init.OversamplingMode         = DISABLE;

      __HAL_RCC_ADC12_CLK_ENABLE();

      err_code = HAL_ADC_Init(&hadc1);
      if (err_code != HAL_OK)
      {
        Display::printf("ADC1 Init: %d", err_code);
      }

      sConfig.Channel      = ADC_CHANNEL_11;
      sConfig.Rank         = ADC_REGULAR_RANK_1;
      sConfig.SamplingTime = ADC_SAMPLETIME_8CYCLES_5;
      sConfig.SingleDiff   = ADC_SINGLE_ENDED;
      sConfig.OffsetNumber = ADC_OFFSET_NONE;
      sConfig.Offset = 0;

      err_code = HAL_ADC_ConfigChannel(&hadc1, &sConfig);
      if (err_code != HAL_OK)
      {
        Display::printf("ADC1 CH0: %d", err_code);
      }

      hadc2.Instance                      = ADC2;
      hadc2.Init.ClockPrescaler           = ADC_CLOCK_ASYNC_DIV4;
      hadc2.Init.Resolution               = ADC_RESOLUTION_8B;
      hadc2.Init.ScanConvMode             = DISABLE;
      hadc2.Init.EOCSelection             = ADC_EOC_SINGLE_CONV;
      hadc2.Init.LowPowerAutoWait         = DISABLE;
      hadc2.Init.ContinuousConvMode       = DISABLE;
      hadc2.Init.NbrOfConversion          = 1;
      hadc2.Init.DiscontinuousConvMode    = DISABLE;
      hadc2.Init.ExternalTrigConv         = ADC_SOFTWARE_START;
      hadc2.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE;
      hadc2.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
      hadc2.Init.Overrun                  = ADC_OVR_DATA_OVERWRITTEN;
      hadc2.Init.OversamplingMode         = DISABLE;

      __HAL_RCC_ADC12_CLK_ENABLE();
      err_code = HAL_ADC_Init(&hadc2);
      if (err_code != HAL_OK)
      {
        Display::printf("ADC2 Init: %d", err_code);
        return;
      }
    }
  }
  namespace DAC
  {
    void init(void)
    {
      __HAL_RCC_DAC12_CLK_ENABLE();

      hdac1.Instance = DAC1;
      err_code = HAL_DAC_Init(&hdac1);
      if (err_code != HAL_OK)
      {
        Display::printf("DAC Init: %d", err_code);
      }

      DAC_ChannelConfTypeDef sConfig = {0};
      sConfig.DAC_SampleAndHold           = DAC_SAMPLEANDHOLD_ENABLE;
      sConfig.DAC_Trigger                 = DAC_TRIGGER_T6_TRGO;
      sConfig.DAC_OutputBuffer            = DAC_OUTPUTBUFFER_DISABLE;
      sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
      sConfig.DAC_UserTrimming            = DAC_TRIMMING_FACTORY;

      err_code = HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1);
      if (err_code != HAL_OK)
      {
        Display::printf("DAC Chan: %d", err_code);
      }
    }
  }
}

