/*
 * display.cpp
 *
 *  Created on: May 30, 2025
 *      Author: kliogka
 */

#include "display.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstring>


namespace Display {

void writeCommand(uint8_t cmd) {
    HAL_GPIO_WritePin(OLED_DC_Port, OLED_DC_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(OLED_CS_Port, OLED_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(OLED_CS_Port, OLED_CS_Pin, GPIO_PIN_SET);
}

void reset() {
    HAL_GPIO_WritePin(OLED_RST_Port, OLED_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(OLED_RST_Port, OLED_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(100);
}

void clear(void) {
    setCursor(0, 0);
    uint8_t zero = 0x00;
    for(uint16_t i = 0; i < FULL_SCREEN; i++) {
        writeData(&zero, 1);
    }
}
void init() {
    reset();

    writeCommand(0xAE); 					// Display OFF
    writeCommand(0xD5); writeCommand(0x80); // Clock div
    writeCommand(0xA8); writeCommand(0x3F); // Mux ratio 64
    writeCommand(0xD3); writeCommand(0x00); // Offset
    writeCommand(0x40); 					// Start line 0
    writeCommand(0x8D); writeCommand(0x14); // Charge pump ON (0x14 = enabled, not 0x04)
    writeCommand(0x20); writeCommand(0x00); // Horizontal addressing mode
    writeCommand(0xA1); 					// Segment remap
    writeCommand(0xC8); 					// COM scan direction remapped
    writeCommand(0xDA); writeCommand(0x12); // COM pins
    writeCommand(0x81); writeCommand(0x40); // Contrast (increased from 0x01 to 0x20 for visibility)
    writeCommand(0xD9); writeCommand(0x13); // Precharge
    writeCommand(0xDB); writeCommand(0x40); // VCOM deselect level (lower voltage)
    writeCommand(0xA4); 					// Resume RAM content display
    writeCommand(0xA6); 					// Normal display
    writeCommand(0x2E); 					// Deactivate scroll
    writeCommand(0xAF); 					// Display ON
}


void setBrightness(uint8_t level)
{
    writeCommand(0x81);   
    writeCommand(level); 
}

void writeData(uint8_t* data, uint16_t size) {
    HAL_GPIO_WritePin(OLED_DC_Port, OLED_DC_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(OLED_CS_Port, OLED_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, data, size, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(OLED_CS_Port, OLED_CS_Pin, GPIO_PIN_SET);
}

void setCursor(uint8_t column, uint8_t page) {
    writeCommand(0x21); // Set column address
    writeCommand(column);
    writeCommand(127);
    writeCommand(0x22); // Set page address
    writeCommand(page);
    writeCommand(7);
}


int getFontIndex(char c) {
    if (c < 0x20 || c > 0x7E)
        return -1;
    return c - 0x20;
}

void drawChar(char c, uint8_t x, uint8_t page) {
    int index = getFontIndex(c);
    if (index < 0) return;

    setCursor(x, page);
    writeData((uint8_t *)font_5x7[index], 5);

    uint8_t space = 0x00;
    writeData(&space, 1);
}

void drawString(const char* str, uint8_t x, uint8_t page) {
    while (*str && x < 128) {
        drawChar(*str, x, page);
        x += 6;
        str++;
    }
}

void drawDigit(uint8_t digit, uint8_t x, uint8_t page) {
    if (digit > 9) return;  
    
    int index = digit + 16;  
    setCursor(x, page);
    writeData((uint8_t *)font_5x7[index], 5);
    
    uint8_t space = 0x00;
    writeData(&space, 1);
}

void drawFloat(float value, uint8_t x, uint8_t page) {
    int intPart = (int)value;
    int fracPart = (int)((value - intPart) * 100);  
    
    uint8_t currentX = x;
    
    if (intPart == 0) {
        drawDigit(0, currentX, page);
        currentX += 6;
    } else {
        if (intPart >= 100) {
            drawDigit(intPart / 100, currentX, page);
            currentX += 6;
            intPart %= 100;
        }
        if (intPart >= 10 || (intPart < 10 && currentX > x)) {  
            drawDigit(intPart / 10, currentX, page);
            currentX += 6;
            intPart %= 10;
        }
        drawDigit(intPart, currentX, page);
        currentX += 6;
    }
    
    drawChar('.', currentX, page);
    currentX += 6;
    
    drawDigit(fracPart / 10, currentX, page);
    currentX += 6;
    drawDigit(fracPart % 10, currentX, page);
}

void drawBitmap(const Bitmap& bmp, uint8_t x, uint8_t pageStart) {
    uint16_t byteIndex = 0;
    uint8_t pagesNeeded = (bmp.height + 7) / 8;  

    for (uint8_t page = 0; page < pagesNeeded; page++) {
        setCursor(x, pageStart + page);

        writeData((uint8_t*)&bmp.data[byteIndex], bmp.width);
        byteIndex += bmp.width;
    }
}

void displayError(const char* area, uint8_t errorCode) {
    clear();
    drawString("Error:", 0, 0);
    drawString(area, 0, 1);
    drawString("Code:", 0, 2);
    drawDigit(errorCode, 30, 2);
}

void printf(uint8_t x, uint8_t page, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    uint8_t currentX = x;
    const char* ptr = format;
    char buffer[16]; 
    
    while (*ptr && currentX < DISPLAY_WIDTH) {
        if (*ptr == '%' && *(ptr + 1)) {
            ptr++; // Skip '%'
            
            switch (*ptr) {
                case 'd': { // Decimal integer
                    int value = va_arg(args, int);
                    snprintf(buffer, sizeof(buffer), "%d", value);
                    drawString(buffer, currentX, page);
                    currentX += strlen(buffer) * 6;
                    break;
                }
                case 'u': { // Unsigned integer
                    unsigned int value = va_arg(args, unsigned int);
                    snprintf(buffer, sizeof(buffer), "%u", value);
                    drawString(buffer, currentX, page);
                    currentX += strlen(buffer) * 6;
                    break;
                }
                case 'x': { // Hexadecimal lowercase
                    unsigned int value = va_arg(args, unsigned int);
                    snprintf(buffer, sizeof(buffer), "%x", value);
                    drawString(buffer, currentX, page);
                    currentX += strlen(buffer) * 6;
                    break;
                }
                case 'X': { // Hexadecimal uppercase
                    unsigned int value = va_arg(args, unsigned int);
                    snprintf(buffer, sizeof(buffer), "%X", value);
                    drawString(buffer, currentX, page);
                    currentX += strlen(buffer) * 6;
                    break;
                }
                case 'b': // Binary (custom implementation)
                case 'B': {
                    unsigned int value = va_arg(args, unsigned int);
                    // Convert to binary string
                    char binStr[33] = {0}; // 32 bits + null terminator
                    int index = 0;
                    if (value == 0) {
                        binStr[0] = '0';
                        index = 1;
                    } else {
                        // Find the highest bit position
                        unsigned int temp = value;
                        int bitPos = 0;
                        while (temp > 0) {
                            temp >>= 1;
                            bitPos++;
                        }
                        // Build binary string
                        for (int i = bitPos - 1; i >= 0; i--) {
                            binStr[index++] = ((value >> i) & 1) ? '1' : '0';
                        }
                    }
                    binStr[index] = '\0';
                    drawString(binStr, currentX, page);
                    currentX += strlen(binStr) * 6;
                    break;
                }
                case 'f': { // Float
                    double value = va_arg(args, double);
                    drawFloat((float)value, currentX, page);
                    // Estimate float display width (integer + decimal point + 2 decimal places)
                    int intPart = (int)value;
                    int digits = (intPart == 0) ? 1 : 0;
                    int temp = (intPart < 0) ? -intPart : intPart;
                    while (temp > 0) {
                        digits++;
                        temp /= 10;
                    }
                    currentX += (digits + 3) * 6; // digits + '.' + 2 decimal places
                    break;
                }
                case 'c': { // Character
                    char value = (char)va_arg(args, int);
                    drawChar(value, currentX, page);
                    currentX += 6;
                    break;
                }
                case 's': { // String
                    const char* value = va_arg(args, const char*);
                    drawString(value, currentX, page);
                    currentX += strlen(value) * 6;
                    break;
                }
                case '%': { // Literal '%'
                    drawChar('%', currentX, page);
                    currentX += 6;
                    break;
                }
                default:
                    // Unknown format specifier, just draw it as is
                    drawChar('%', currentX, page);
                    currentX += 6;
                    if (currentX < DISPLAY_WIDTH) {
                        drawChar(*ptr, currentX, page);
                        currentX += 6;
                    }
                    break;
            }
        } else {
            // Regular character
            drawChar(*ptr, currentX, page);
            currentX += 6;
        }
        ptr++;
    }
    
    va_end(args);
}

}
