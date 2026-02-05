#include <main.h>
#include "display.hpp"


uint8_t err_code = 0;

void showStartupScreen()
{
  const uint8_t steps[] = {16, 8, 4, 2, 1};

  for (uint8_t step : steps)
  {
    Display::clear();

    for (uint8_t y = 0; y < 8; y += (step > 8 ? 1 : step))
    {
      for (uint8_t x = 0; x < 128; x += step)
      {
        uint8_t blockX = x + (step / 2);
        uint8_t blockY = y + (step > 8 ? 4 : step / 2);

        if (startup_screen_bitmap.data[blockY * 128 + blockX] & 0x01)
        {
          for (uint8_t by = y; by < y + (step > 8 ? 1 : step) && by < 8; by++)
          {
            Display::setCursor(x, by);
            uint8_t pixels = 0xFF;
            for (uint8_t bx = 0; bx < step && (x + bx) < 128; bx++)
            {
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

int main(void)
{
   
  HAL_Init();

  Hardware::init();
  Display::clear();


  showStartupScreen();

  HAL_Delay(200);

  mainApp();

  while (1)
  {
    __NOP();
  }
}

extern "C" void DMA1_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_adc1);
}

extern "C" void DMA1_Stream1_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_dac1);
}

extern "C" void TIM8_UP_TIM13_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim8);
}

extern "C" void EXTI0_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

extern "C" void EXTI1_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
}

extern "C" void EXTI9_5_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_6);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
}

extern "C" void EXTI15_10_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_10);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_11);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add implementation to report the file name and line number */
}
#endif