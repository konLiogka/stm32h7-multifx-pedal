#include "hw_timer.hpp"
#include "display.hpp"

extern uint8_t err_code;

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim8;

namespace Hardware
{
  namespace Timer
  {
    void init(void)
    {
      htim6.Instance               = TIM6;
      htim6.Init.Prescaler         = 0;                              // 200MHz
      htim6.Init.Period            = 2499;                           // 200M / 2499 = 96kHz
      htim6.Init.CounterMode       = TIM_COUNTERMODE_UP;
      htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

      __HAL_RCC_TIM6_CLK_ENABLE();

      err_code = HAL_TIM_Base_Init(&htim6);
      if (err_code != HAL_OK)
      {
        Display::printf("TIM6 Init: %d", err_code);
      }

      TIM_MasterConfigTypeDef sMasterConfig = {0};
      sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
      sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
      HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig);

      htim8.Instance               = TIM8;
      htim8.Init.Prescaler         = 149999;                           // 200MHz / 150000 = 1.333kHz
      htim8.Init.Period            = 499;                             // 1.333kHz / 500 = 2,66Hz (37ms period)
      htim8.Init.CounterMode       = TIM_COUNTERMODE_UP;
      htim8.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
      htim8.Init.RepetitionCounter = 0;
      htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

      __HAL_RCC_TIM8_CLK_ENABLE();
      err_code = HAL_TIM_Base_Init(&htim8);
      if (err_code != HAL_OK)
      {
        Display::printf("TIM8 Init: %d", err_code);
      }

      HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, 6, 0);
      HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);
    }
  }
}
