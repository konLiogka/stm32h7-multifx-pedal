#ifndef QSPI_FLASH_H
#define QSPI_FLASH_H

#include "stm32h7xx_hal.h"
#include <stdint.h>
#include "effectsChain.hpp"

#define CHAIN_STORAGE_ADDR 0x1000  

namespace QSPIFlash {
    
    enum Status {
        OK = 0,
        ERROR = 1,
        TIMEOUT = 2
    };
    
    Status init();
    Status read(uint32_t address, uint8_t* data, uint32_t size);
    Status write(uint32_t address, const uint8_t* data, uint32_t size);
    Status erase_sector(uint32_t address);
    Status erase_chip();
    bool is_ready();
    Status loadEffectsChain(EffectsChain* chain);
    Status saveEffectsChain(const EffectsChain* chain);

} 


#endif /* QSPI_FLASH_H */
