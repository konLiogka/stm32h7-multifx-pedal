#include "hw_clock.hpp"
#include "display.hpp"

extern uint8_t err_code;
namespace Hardware
{
  namespace Clock
  {
    void init(void)
    {
      RCC_OscInitTypeDef       RCC_OscInitStruct = {0};
      RCC_ClkInitTypeDef       RCC_ClkInitStruct = {0};
      RCC_PeriphCLKInitTypeDef PeriphClkInit     = {0};

      HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
      __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
      while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
      {
      }

      // Main clock 480MHZ
      RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
      RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
      RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
      RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;

                                                             // HSE = 25 MHz
      RCC_OscInitStruct.PLL.PLLM      = 5;                   // 25 MHz / 5 = 5 MHz
      RCC_OscInitStruct.PLL.PLLN      = 192;                 // 5 MHz × 192 = 960 MHz (VCO)
      RCC_OscInitStruct.PLL.PLLP      = 2;                   // 960 MHz / 2 = 480 MHz (SYSCLK)
      RCC_OscInitStruct.PLL.PLLQ      = 4;                   // 960 MHz / 4 = 240 MHz (USB, RNG, SDMMC)
      RCC_OscInitStruct.PLL.PLLR      = 2;                   // 960 MHz / 2 = 480 MHz
      RCC_OscInitStruct.PLL.PLLRGE    = RCC_PLL1VCIRANGE_1;  // 2-4 MHz input
      RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;     // Wide VCO
      RCC_OscInitStruct.PLL.PLLFRACN = 0;

      err_code = HAL_RCC_OscConfig(&RCC_OscInitStruct);
      if (err_code != HAL_OK)
      {
        Display::printf("RCC OSC: %d", err_code);
      }

      RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 |
                                    RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
      RCC_ClkInitStruct.SYSCLKSource  = RCC_SYSCLKSOURCE_PLLCLK;
      RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;          // SYSCLK = 480 MHz

                                                        // MAX AHB = 240 MHz
      RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;  // HCLK = 240 MHz

                                                         // MAX APB = 120 MHz
      RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;  // APB3 = 120 MHz
      RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;  // APB1 = 120 MHz (TIM6 gets ×2 = 240 MHz)
      RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;  // APB2 = 120 MHz
      RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;  // APB4 = 120 MHz

      // Flash latency for 480 MHz (VOS0) = 5 wait states
      err_code = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
      if (err_code != HAL_OK)
      {
        Display::printf("RCC Clock: %d", err_code);
      }

      PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
      PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;

      PeriphClkInit.PLL2.PLL2M      = 5;                   // 25 MHz / 5 = 5 MHz
      PeriphClkInit.PLL2.PLL2N      = 96;                  // 5 MHz × 96 = 480 MHz (VCO)
      PeriphClkInit.PLL2.PLL2P      = 8;                   // 480 MHz / 8 = 60 MHz (MAX for 12-bit ADC)
      PeriphClkInit.PLL2.PLL2Q      = 2;                   // 480 MHz / 2 = 240 MHz
      PeriphClkInit.PLL2.PLL2R      = 2;                   // 480 MHz / 2 = 240 MHz
      PeriphClkInit.PLL2.PLL2RGE    = RCC_PLL2VCIRANGE_1;
      PeriphClkInit.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
      PeriphClkInit.PLL2.PLL2FRACN  = 0;

      err_code = HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
      if (err_code != HAL_OK)
      {
        Display::printf("ADC Clock", 1);
      }

      SCB_EnableICache();
      SCB_EnableDCache();

      HAL_SYSTICK_Config(SystemCoreClock / 1000);
      HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

      __HAL_RCC_SYSCFG_CLK_ENABLE();
    }
  }
}
