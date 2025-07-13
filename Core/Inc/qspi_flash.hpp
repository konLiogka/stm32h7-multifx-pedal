/*

For Winbond Q25W64JV NOR Flash Memory (8MB)

*/

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_qspi.h"
#include <stdint.h>
#include "effectsChain.hpp"

#ifndef QSPI_FLASH_H
#define QSPI_FLASH_H

#define CHAIN_STORAGE_ADDR 0x1000  
extern QSPI_HandleTypeDef hqspi;

namespace QSPIFlash 
{
    
    bool is_ready();
    HAL_StatusTypeDef init();
    HAL_StatusTypeDef read(uint32_t address, uint8_t* data, uint32_t size);
    HAL_StatusTypeDef write(uint32_t address, const uint8_t* data, uint32_t size);
    HAL_StatusTypeDef erase_sector(uint32_t address);
    HAL_StatusTypeDef erase_chip();
    HAL_StatusTypeDef loadEffectsChain(EffectsChain* chain);
    HAL_StatusTypeDef saveEffectsChain(const EffectsChain* chain);
} 

#endif /* QSPI_FLASH_H */
