#include "qspi_flash.hpp"

// W25Q64JV Commands
#define W25Q64_WRITE_ENABLE          0x06
#define W25Q64_READ_STATUS_REG       0x05
#define W25Q64_PAGE_PROGRAM          0x02
#define W25Q64_SECTOR_ERASE          0x20
#define W25Q64_CHIP_ERASE            0xC7
#define W25Q64_READ_DATA             0x03
#define W25Q64_JEDEC_ID              0x9F

// W25Q64JV Parameters
#define W25Q64_PAGE_SIZE             256
#define W25Q64_SECTOR_SIZE           4096
#define W25Q64_STATUS_BUSY           0x01
#define W25Q64_STATUS_WEL            0x02

QSPI_HandleTypeDef hqspi;

namespace QSPIFlash {

    static HAL_StatusTypeDef wait_for_ready(uint32_t timeout = 5000) {
        uint32_t start_time = HAL_GetTick();
        QSPI_CommandTypeDef cmd = {0};
        uint8_t status;
        
        cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
        cmd.Instruction       = W25Q64_READ_STATUS_REG;
        cmd.AddressMode       = QSPI_ADDRESS_NONE;
        cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
        cmd.DataMode          = QSPI_DATA_1_LINE;
        cmd.DummyCycles       = 0;
        cmd.NbData            = 1;
        cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
        cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
        cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
        
        while (HAL_GetTick() - start_time < timeout) {
            if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
                return HAL_ERROR;
            }
            if (HAL_QSPI_Receive(&hqspi, &status, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
                return HAL_ERROR;
            }
            if (!(status & W25Q64_STATUS_BUSY)) {
                return HAL_OK;
            }
            HAL_Delay(1);
        }
        return HAL_TIMEOUT;
    }

    bool is_ready() {
        return wait_for_ready(0) == HAL_OK;
    }
    
    static HAL_StatusTypeDef write_enable() {
        QSPI_CommandTypeDef cmd = {0};
        
        cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
        cmd.Instruction       = W25Q64_WRITE_ENABLE;
        cmd.AddressMode       = QSPI_ADDRESS_NONE;
        cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
        cmd.DataMode          = QSPI_DATA_NONE;
        cmd.DummyCycles       = 0;
        cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
        cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
        cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
        
        if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            return HAL_ERROR;
        }
        return HAL_OK;
    }

    HAL_StatusTypeDef init() {
        hqspi.Instance                = QUADSPI;
        hqspi.Init.ClockPrescaler     = 2;
        hqspi.Init.FifoThreshold      = 4;
        hqspi.Init.SampleShifting     = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
        hqspi.Init.FlashSize          = 22;                              // 2^(22+1) = 8MB
        hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
        hqspi.Init.ClockMode          = QSPI_CLOCK_MODE_0;
        hqspi.Init.FlashID            = QSPI_FLASH_ID_1;
        hqspi.Init.DualFlash          = QSPI_DUALFLASH_DISABLE;
        
        __HAL_RCC_QSPI_CLK_ENABLE();
        
        if (HAL_QSPI_Init(&hqspi) != HAL_OK) {
            return HAL_ERROR;
        }
        
        QSPI_CommandTypeDef cmd = {0};
        uint8_t id[3];
        
        cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
        cmd.Instruction       = W25Q64_JEDEC_ID;
        cmd.AddressMode       = QSPI_ADDRESS_NONE;
        cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
        cmd.DataMode          = QSPI_DATA_1_LINE;
        cmd.DummyCycles       = 0;
        cmd.NbData            = 3;
        cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
        cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
        cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
        
        if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            return HAL_ERROR;
        }
        if (HAL_QSPI_Receive(&hqspi, id, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            return HAL_ERROR;
        }
        
        // W25Q64 ID: 0xEF4017
        if (id[0] != 0xEF || id[1] != 0x40 || id[2] != 0x17) {
            return HAL_ERROR;
        }

        return HAL_OK;
    }

    HAL_StatusTypeDef read(uint32_t address, uint8_t* data, uint32_t size) {
        QSPI_CommandTypeDef cmd = {0};
        
        cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
        cmd.Instruction       = W25Q64_READ_DATA;
        cmd.AddressMode       = QSPI_ADDRESS_1_LINE;
        cmd.AddressSize       = QSPI_ADDRESS_24_BITS;
        cmd.Address           = address;
        cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
        cmd.DataMode          = QSPI_DATA_1_LINE;
        cmd.DummyCycles       = 0;
        cmd.NbData            = size;
        cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
        cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
        cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
        
        if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            return HAL_ERROR;
        }
        if (HAL_QSPI_Receive(&hqspi, data, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            return HAL_ERROR;
        }

        return HAL_OK;
    }

    HAL_StatusTypeDef write(uint32_t address, const uint8_t* data, uint32_t size) {
        uint32_t current_addr = address;
        uint32_t remaining = size;
        const uint8_t* current_data = data;
        
        while (remaining > 0) {
            if (write_enable() != HAL_OK) {
                return HAL_ERROR;
            }
            
            uint32_t page_offset = current_addr % W25Q64_PAGE_SIZE;
            uint32_t page_remaining = W25Q64_PAGE_SIZE - page_offset;
            uint32_t write_size = (remaining < page_remaining) ? remaining : page_remaining;
            
            QSPI_CommandTypeDef cmd = {0};
            cmd.InstructionMode     = QSPI_INSTRUCTION_1_LINE;
            cmd.Instruction         = W25Q64_PAGE_PROGRAM;
            cmd.AddressMode         = QSPI_ADDRESS_1_LINE;
            cmd.AddressSize         = QSPI_ADDRESS_24_BITS;
            cmd.Address             = current_addr;
            cmd.AlternateByteMode   = QSPI_ALTERNATE_BYTES_NONE;
            cmd.DataMode            = QSPI_DATA_1_LINE;
            cmd.DummyCycles         = 0;
            cmd.NbData              = write_size;
            cmd.DdrMode             = QSPI_DDR_MODE_DISABLE;
            cmd.DdrHoldHalfCycle    = QSPI_DDR_HHC_ANALOG_DELAY;
            cmd.SIOOMode            = QSPI_SIOO_INST_EVERY_CMD;
            
            if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
                return HAL_ERROR;
            }
            if (HAL_QSPI_Transmit(&hqspi, (uint8_t*)current_data, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
                return HAL_ERROR;
            }
            
            if (wait_for_ready() != HAL_OK) {
                return HAL_TIMEOUT;
            }
            
            current_addr += write_size;
            current_data += write_size;
            remaining -= write_size;
        }
        return HAL_OK;
    }

    HAL_StatusTypeDef erase_sector(uint32_t address) {
        if (write_enable() != HAL_OK) {
            return HAL_ERROR;
        }
        
        QSPI_CommandTypeDef cmd = {0};
        cmd.InstructionMode     = QSPI_INSTRUCTION_1_LINE;
        cmd.Instruction         = W25Q64_SECTOR_ERASE;
        cmd.AddressMode         = QSPI_ADDRESS_1_LINE;
        cmd.AddressSize         = QSPI_ADDRESS_24_BITS;
        cmd.Address             = address;
        cmd.AlternateByteMode   = QSPI_ALTERNATE_BYTES_NONE;
        cmd.DataMode            = QSPI_DATA_NONE;
        cmd.DummyCycles         = 0;
        cmd.DdrMode             = QSPI_DDR_MODE_DISABLE;
        cmd.DdrHoldHalfCycle    = QSPI_DDR_HHC_ANALOG_DELAY;
        cmd.SIOOMode            = QSPI_SIOO_INST_EVERY_CMD;
        
        if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            return HAL_ERROR;
        }
        
        return wait_for_ready();
    }

    HAL_StatusTypeDef erase_chip() {
        if (write_enable() != HAL_OK) {
            return HAL_ERROR;
        }
        
        QSPI_CommandTypeDef cmd = {0};
        cmd.InstructionMode     = QSPI_INSTRUCTION_1_LINE;
        cmd.Instruction         = W25Q64_CHIP_ERASE;
        cmd.AddressMode         = QSPI_ADDRESS_NONE;
        cmd.AlternateByteMode   = QSPI_ALTERNATE_BYTES_NONE;
        cmd.DataMode            = QSPI_DATA_NONE;
        cmd.DummyCycles         = 0;
        cmd.DdrMode             = QSPI_DDR_MODE_DISABLE;
        cmd.DdrHoldHalfCycle    = QSPI_DDR_HHC_ANALOG_DELAY;
        cmd.SIOOMode            = QSPI_SIOO_INST_EVERY_CMD;
        
        if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            return HAL_ERROR;
        }
        
        return wait_for_ready(120000);  
    }

    HAL_StatusTypeDef loadEffectsChain(EffectsChain* chain) {
        if (!chain) {
            return HAL_ERROR;
        }
        uint8_t pedalData[NUM_PEDALS * PEDAL_LEN] = {};

        if (read(CHAIN_STORAGE_ADDR, pedalData, sizeof(pedalData)) != HAL_OK) {
            return HAL_ERROR;
        }

        for (uint8_t i = 0; i < NUM_PEDALS; i++) {
            uint8_t* p = &pedalData[i * PEDAL_LEN];
            PedalType type = static_cast<PedalType>(p[0]);
            chain->setPedal(i, type);
            Pedal* pedal = chain->getPedal(i);
            if (pedal) {
                float* params = reinterpret_cast<float*>(p + 1);
                pedal->setParams(params);
            }
        }
        return HAL_OK;
    }
    
    HAL_StatusTypeDef   saveEffectsChain(const EffectsChain* chain) {
        if (!chain) {
            return HAL_ERROR;
        }

        uint8_t pedalData[NUM_PEDALS * PEDAL_LEN] = {};
        for (uint8_t i = 0; i < NUM_PEDALS; i++) {
            Pedal* pedal = chain->getPedal(i);
            PedalType type = pedal ? pedal->getType() : PedalType::PASS_THROUGH;
            pedalData[i * PEDAL_LEN] = static_cast<uint8_t>(type);

            if (pedal) {
                float* params = reinterpret_cast<float*>(&pedalData[i * PEDAL_LEN + 1]);
                pedal->getParams(params);
            }
        }

        if (erase_sector(CHAIN_STORAGE_ADDR) != HAL_OK) {
            return HAL_ERROR;
        }
        return write(CHAIN_STORAGE_ADDR, pedalData, sizeof(pedalData));
    }

} 
