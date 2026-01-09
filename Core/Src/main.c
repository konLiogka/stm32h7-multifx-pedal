#include <main.h>
#include "display.hpp"

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_SPI1_Init(void);
void MX_DMA_Init(void);
void MX_ADC1_Init(void);
void MX_TIM8_Init(void);
void MX_DAC1_Init(void);
void MX_DMA_ADC1_Init(void);
void MX_DMA_DAC1_Init(void);
void MX_ADC2_Init(void);

void MX_TIM6_Init(void);
static void MPU_Config(void);

/* SPI handler for display  */
SPI_HandleTypeDef hspi1;

/* Initialize ADC1 handler */
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/* Initialize DAC handler */
DAC_HandleTypeDef hdac1;
DMA_HandleTypeDef hdma_dac1;

/* Synchronization timers for ADC/DAC */
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim8;

/* Initialize ADC2 handler */
ADC_HandleTypeDef hadc2;
DMA_HandleTypeDef hdma_adc2;
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

  /* Initialize main peripherals */
  HAL_Init();
  SystemClock_Config();
  __HAL_RCC_SYSCFG_CLK_ENABLE();

  MX_GPIO_Init();
  MX_SPI1_Init();

  Display::init();
  Display::clear();

  showStartupScreen();

  HAL_Delay(3000);

  // Initialize ADCs
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_DAC1_Init();
  MX_TIM8_Init();
  MX_TIM6_Init();

  MX_DMA_ADC1_Init();
  MX_DMA_DAC1_Init();

  err_code = HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
  if (err_code != HAL_OK)
  {
    Display::displayError("ADC1 Calib", err_code);
  }

  err_code = HAL_ADCEx_Calibration_Start(&hadc2, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
  if (err_code != HAL_OK)
  {
    Display::displayError("ADC2 Calib", err_code);
  }


  err_code = HAL_DACEx_SelfCalibrate(&hdac1, DAC_CHANNEL_1, DAC_TRIGGER_NONE);
  if (err_code != HAL_OK)
  {
    Display::displayError("DAC1 Calib", err_code);
  }

  // Start TIM8 with interrupt for ADC2 sampling
  err_code = HAL_TIM_Base_Start_IT(&htim8);
  if (err_code != HAL_OK)
  {
    Display::displayError("TIM8 Start", err_code);
  }

  MPU_Config();

  mainApp();

  while (1)
  {
    __NOP();
  }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  RCC_OscInitStruct.PLL.PLLFRACN  = 0;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Display::displayError("RCC OSC", 1);
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 |
                                RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource  = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;          // SYSCLK = 480 MHz

  // MAX AHB = 240 MHz
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2; // HCLK = 240 MHz

  // MAX APB = 120 MHz
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2; // APB3 = 120 MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2; // APB1 = 120 MHz (TIM6 gets ×2 = 240 MHz)
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2; // APB2 = 120 MHz
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2; // APB4 = 120 MHz

  // Flash latency for 480 MHz (VOS0) = 5 wait states
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Display::displayError("RCC Clock", 1);
  }

  // Configure PLL2 for MAX ADC performance
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;

  // MAX ADC clock for 12-bit mode = ~60 MHz
  // Set PLL2 for 60 MHz ADC clock
  PeriphClkInit.PLL2.PLL2M      = 5;                   // 25 MHz / 5 = 5 MHz
  PeriphClkInit.PLL2.PLL2N      = 96;                  // 5 MHz × 96 = 480 MHz (VCO)
  PeriphClkInit.PLL2.PLL2P      = 8;                   // 480 MHz / 8 = 60 MHz (MAX for 12-bit ADC)
  PeriphClkInit.PLL2.PLL2Q      = 2;                   // 480 MHz / 2 = 240 MHz
  PeriphClkInit.PLL2.PLL2R      = 2;                   // 480 MHz / 2 = 240 MHz
  PeriphClkInit.PLL2.PLL2RGE    = RCC_PLL2VCIRANGE_1;
  PeriphClkInit.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInit.PLL2.PLL2FRACN  = 0;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Display::displayError("ADC Clock", 1);
  }

  SCB_EnableICache();
  SCB_EnableDCache();

  HAL_SYSTICK_Config(SystemCoreClock / 1000);
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

/**
 * @brief SPI Initialization Function for display SSD1309
 * @param None
 * @retval None
 */
void MX_SPI1_Init(void)
{
  hspi1.Instance                     = SPI1;
  hspi1.Init.Mode                    = SPI_MODE_MASTER;
  hspi1.Init.Direction               = SPI_DIRECTION_2LINES_TXONLY;
  hspi1.Init.DataSize                = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity             = SPI_POLARITY_LOW;                       // CPOL = 0
  hspi1.Init.CLKPhase                = SPI_PHASE_1EDGE;                        // CPHA = 0
  hspi1.Init.NSS                     = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler       = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit                = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode                  = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation          = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial           = 7;
  hspi1.Init.NSSPMode                = SPI_NSS_PULSE_DISABLE;
  hspi1.Init.NSSPolarity             = SPI_NSS_POLARITY_LOW;
  hspi1.Init.FifoThreshold           = SPI_FIFO_THRESHOLD_01DATA;
  hspi1.Init.MasterSSIdleness        = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi1.Init.MasterReceiverAutoSusp  = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi1.Init.MasterKeepIOState       = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi1.Init.IOSwap                  = SPI_IO_SWAP_DISABLE;

  err_code = HAL_SPI_Init(&hspi1);
  if (err_code != HAL_OK)
  {
    Display::displayError("SPI1 Init", err_code);
  }
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
void MX_GPIO_Init(void)
{
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

  GPIO_InitStruct.Pin  = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_6 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Configure SPI1 pins (PA5=SCK, PA7=MOSI) with strong drive
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

  // Configure QUADSPI pins
  // CLK (PB2) and NCS (PB6)
  GPIO_InitStruct.Pin       = GPIO_PIN_2;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin       = GPIO_PIN_6;
  GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin  = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  // Configure display control pins (CS=PD2, RES=PD3, DC=PD4)
  GPIO_InitStruct.Pin   = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  // IO0 (PD11), IO1 (PD12), IO3 (PD13)
  GPIO_InitStruct.Pin       = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
  GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  // Configure PD1 as input with pull-up
  GPIO_InitStruct.Pin  = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  // IO2 (PE2)
  GPIO_InitStruct.Pin       = GPIO_PIN_2;
  GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  __enable_irq();
}

void MX_ADC2_Init(void)
{
  // ADC2 Configuration for polling mode
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler           = ADC_CLOCK_ASYNC_DIV4;
  hadc2.Init.Resolution               = ADC_RESOLUTION_8B;
  hadc2.Init.ScanConvMode             = DISABLE;                        
  hadc2.Init.EOCSelection             = ADC_EOC_SINGLE_CONV;
  hadc2.Init.LowPowerAutoWait         = DISABLE;
  hadc2.Init.ContinuousConvMode       = ENABLE;                         
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
    Display::displayError("ADC2 Init", err_code);
    return;
  }
}

void MX_ADC1_Init(void)
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

  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Display::displayError("ADC1 Init", 1);
  }

  sConfig.Channel      = ADC_CHANNEL_11;            // PC1
  sConfig.Rank         = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_8CYCLES_5;
  sConfig.SingleDiff   = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;

  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Display::displayError("ADC1 CH0", 1);
  }
}

void MX_DMA_ADC1_Init(void)
{
  __HAL_RCC_DMA1_CLK_ENABLE();

  hdma_adc1.Instance                 = DMA1_Stream0;
  hdma_adc1.Init.Request             = DMA_REQUEST_ADC1;
  hdma_adc1.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hdma_adc1.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_adc1.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  hdma_adc1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
  hdma_adc1.Init.Mode                = DMA_CIRCULAR;
  hdma_adc1.Init.Priority            = DMA_PRIORITY_MEDIUM;
  hdma_adc1.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;

  if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
  {
    Display::displayError("DMA ADC1 Init", 1);
  }

  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc1);
}

void MX_DAC1_Init(void)
{
  __HAL_RCC_DAC12_CLK_ENABLE();

  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Display::displayError("DAC Init", HAL_ERROR);
    Error_Handler();
  }

  DAC_ChannelConfTypeDef sConfig = {0};
  sConfig.DAC_SampleAndHold           = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger                 = DAC_TRIGGER_T6_TRGO;
  sConfig.DAC_OutputBuffer            = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
  sConfig.DAC_UserTrimming            = DAC_TRIMMING_FACTORY;

  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Display::displayError("DAC Chan", HAL_ERROR);
    Error_Handler();
  }
}

void MX_DMA_DAC1_Init(void)
{
  __HAL_RCC_DMA1_CLK_ENABLE();

  hdma_dac1.Instance                 = DMA1_Stream1;
  hdma_dac1.Init.Request             = DMA_REQUEST_DAC1;
  hdma_dac1.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  hdma_dac1.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_dac1.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_dac1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  hdma_dac1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
  hdma_dac1.Init.Mode                = DMA_CIRCULAR;
  hdma_dac1.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
  hdma_dac1.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  hdma_dac1.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
  hdma_dac1.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  hdma_dac1.Init.MemBurst = DMA_MBURST_INC4;
  hdma_dac1.Init.PeriphBurst = DMA_PBURST_SINGLE;

  if (HAL_DMA_Init(&hdma_dac1) != HAL_OK)
  {
    Display::displayError("DMA DAC1 Init", 1);
  }
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
  __HAL_LINKDMA(&hdac1, DMA_Handle1, hdma_dac1);
}

void MX_TIM6_Init(void)
{
  htim6.Instance               = TIM6;
  htim6.Init.Prescaler         = 0;
  htim6.Init.Period            = 2499;
  htim6.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

  __HAL_RCC_TIM6_CLK_ENABLE();

  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Display::displayError("TIM6 Init", 1);
  }

  TIM_MasterConfigTypeDef sMasterConfig                     = {0};
                          sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
                          sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig);
}

void MX_TIM8_Init(void)
{
  htim8.Instance               = TIM8;
  htim8.Init.Prescaler         = 39999;                           // 200MHz / 40000 = 5kHz (APB2 timer clock is 200MHz)
  htim8.Init.Period            = 499;                             // 5kHz / 500 = 10Hz (100ms period)
  htim8.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim8.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim8.Init.RepetitionCounter = 0;
  htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  __HAL_RCC_TIM8_CLK_ENABLE();
  err_code = HAL_TIM_Base_Init(&htim8);
  if (err_code != HAL_OK)
  {
    Display::displayError("TIM8 Init", err_code);
  }

  HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);
}

static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  HAL_MPU_Disable();
  
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x00000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;  
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;  
  MPU_InitStruct.BaseAddress = 0x30040000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_128KB;
  MPU_InitStruct.SubRegionDisable = 0x00;   
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL2;   
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;   
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;   
  
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
  __DMB();
  
  SCB_DisableICache();
  SCB_DisableDCache();
  
  SCB_InvalidateDCache();
  SCB_CleanDCache();
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