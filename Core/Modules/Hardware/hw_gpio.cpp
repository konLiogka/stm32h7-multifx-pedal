#include "hw_gpio.hpp"

extern uint8_t err_code;

namespace Hardware
{
  namespace GPIO
  {
    void init(void)
    {
      /*
       * PA0 SELECT VIEW BUTTON
       * PA1 PEDAL BUTTON 4
       * PA2 POT 2
       * PA3 POT 3
       * PA4 DAC OUT
       * PA5 DISPLAY SPI SCK
       * PA6 PEDAL BUTTON 1
       * PA7 DISPLAY SPI SDA
       * PA8 PEDAL 1
       * PA9 PEDAL 0
       * PA10 PEDAL BUTTON 3
       * PA15 PEDAL BUTTON 2
       *
       * PB2 QUADSPI
       * PB6 QUADSPI
       *
       * PC0 POT 1
       * PC1 ADC IN
       * PC4 SETTINGS BUTTON
       *
       * PD2 DISPLAY SPI CS
       * PD3 DISPLAY SPI RST
       * PD4 DISPLAY SPI DC
       * PD11 QUAD SPI
       * PD12 QUAD SPI
       * PD13 QUAD SPI
       *
       * PE2 QUADSPI

      */
      GPIO_InitTypeDef GPIO_InitStruct = {0};

      // Enable GPIO clocks
      __HAL_RCC_GPIOA_CLK_ENABLE();
      __HAL_RCC_GPIOB_CLK_ENABLE();
      __HAL_RCC_GPIOD_CLK_ENABLE();
      __HAL_RCC_GPIOC_CLK_ENABLE();
      __HAL_RCC_GPIOE_CLK_ENABLE();

      HAL_NVIC_SetPriority(EXTI0_IRQn, 6, 0);
      HAL_NVIC_EnableIRQ(EXTI0_IRQn);

      HAL_NVIC_SetPriority(EXTI1_IRQn, 6, 0);
      HAL_NVIC_EnableIRQ(EXTI1_IRQn);

      HAL_NVIC_SetPriority(EXTI9_5_IRQn, 6, 0);
      HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

      HAL_NVIC_SetPriority(EXTI15_10_IRQn, 6, 0);
      HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

      GPIO_InitStruct.Pin       = GPIO_PIN_2;
      GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull      = GPIO_NOPULL;
      GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

      GPIO_InitStruct.Pin       = GPIO_PIN_6;
      GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

      GPIO_InitStruct.Pin       = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
      GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull      = GPIO_NOPULL;
      GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
      HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

      GPIO_InitStruct.Pin       = GPIO_PIN_2;
      GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull      = GPIO_NOPULL;
      GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
      HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

      GPIO_InitStruct.Pin  = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_6 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_15;
      GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

      GPIO_InitStruct.Pin       = GPIO_PIN_5 | GPIO_PIN_7;
      GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull      = GPIO_NOPULL;
      GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
      GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

      GPIO_InitStruct.Pin  = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4;
      GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

      GPIO_InitStruct.Pin  = GPIO_PIN_0 | GPIO_PIN_1;
      GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

      GPIO_InitStruct.Pin  = GPIO_PIN_4;
      GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

      GPIO_InitStruct.Pin   = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4;
      GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull  = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
      HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

      GPIO_InitStruct.Pin  = GPIO_PIN_1;
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

      __enable_irq();
    }
  }
}
