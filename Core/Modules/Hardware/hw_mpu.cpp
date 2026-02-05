#include "hw_mpu.hpp"
#include "display.hpp"

extern uint8_t err_code;

namespace Hardware
{
  namespace MemoryProtectionUnit
  {
    void init(void)
    {
      MPU_Region_InitTypeDef MPU_InitStruct = {0};

      HAL_MPU_Disable();

      MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
      MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
      MPU_InitStruct.BaseAddress      = 0x00000000;
      MPU_InitStruct.Size             = MPU_REGION_SIZE_4GB;
      MPU_InitStruct.SubRegionDisable = 0x87;
      MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
      MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
      MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
      MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
      MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
      MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;

      HAL_MPU_ConfigRegion(&MPU_InitStruct);

      MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
      MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
      MPU_InitStruct.BaseAddress      = 0x30040000;
      MPU_InitStruct.Size             = MPU_REGION_SIZE_128KB;
      MPU_InitStruct.SubRegionDisable = 0x00;
      MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL2;
      MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
      MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
      MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
      MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
      MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;

      HAL_MPU_ConfigRegion(&MPU_InitStruct);

      HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
      __DMB();

      SCB_DisableICache();
      SCB_DisableDCache();

      SCB_InvalidateDCache();
      SCB_CleanDCache();
    }
  }
}