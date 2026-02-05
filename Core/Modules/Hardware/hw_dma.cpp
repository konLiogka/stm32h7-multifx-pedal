#include "hw_dma.hpp"
#include "display.hpp"

extern uint8_t err_code;

DMA_HandleTypeDef hdma_adc1;
DMA_HandleTypeDef hdma_adc2;
DMA_HandleTypeDef hdma_dac1;

namespace Hardware
{
  namespace DMA
  {
    void init(void)
    {
      __HAL_RCC_DMA1_CLK_ENABLE();

      hdma_adc1.Instance = DMA1_Stream0;
      hdma_adc1.Init.Request = DMA_REQUEST_ADC1;
      hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
      hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
      hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
      hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
      hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
      hdma_adc1.Init.Mode = DMA_CIRCULAR;
      hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
      hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

      err_code = HAL_DMA_Init(&hdma_adc1);

      if (err_code != HAL_OK)
      {
        Display::printf("DMA ADC1 Init: %d", err_code);
      }

      HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 1, 0);
      HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
      __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);

      __HAL_RCC_DMA1_CLK_ENABLE();

      hdma_dac1.Instance = DMA1_Stream1;
      hdma_dac1.Init.Request = DMA_REQUEST_DAC1;
      hdma_dac1.Init.Direction = DMA_MEMORY_TO_PERIPH;
      hdma_dac1.Init.PeriphInc = DMA_PINC_DISABLE;
      hdma_dac1.Init.MemInc = DMA_MINC_ENABLE;
      hdma_dac1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
      hdma_dac1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
      hdma_dac1.Init.Mode = DMA_CIRCULAR;
      hdma_dac1.Init.Priority = DMA_PRIORITY_VERY_HIGH;
      hdma_dac1.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
      hdma_dac1.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
      hdma_dac1.Init.MemBurst = DMA_MBURST_INC4;
      hdma_dac1.Init.PeriphBurst = DMA_PBURST_SINGLE;

      err_code = HAL_DMA_Init(&hdma_dac1);
      if (err_code != HAL_OK)
      {
        Display::printf("DMA DAC1 Init: %d", err_code);
      }
      HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 1, 0);
      HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
      __HAL_LINKDMA(&hdac1, DMA_Handle1, hdma_dac1);
    }
  }
}
