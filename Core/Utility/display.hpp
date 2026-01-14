/*

For SSD1309 OLED 128x64 display

*/

#include <main.h>
#include <cstdint>
#include "bitmaps.hpp"

#ifndef INC_DISPLAY_HPP_
#define INC_DISPLAY_HPP_

#define OLED_CS_Port    GPIOD
#define OLED_CS_Pin     GPIO_PIN_2
#define OLED_DC_Port    GPIOD
#define OLED_DC_Pin     GPIO_PIN_4
#define OLED_RST_Port   GPIOD
#define OLED_RST_Pin    GPIO_PIN_3

#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  64
#define DISPLAY_PAGES   8
#define FULL_SCREEN     (DISPLAY_WIDTH * DISPLAY_PAGES)

extern SPI_HandleTypeDef hspi1;

namespace Display {
        void init();
        void setBrightness(uint8_t level);
        void writeCommand(uint8_t cmd);
        void writeData(uint8_t* data, uint16_t size);
        void reset();
        void setCursor(uint8_t column, uint8_t page);
        void drawChar(char c, uint8_t x, uint8_t page);
        void drawString(const char* str, uint8_t x, uint8_t page);
        void drawDigit(uint8_t digit, uint8_t x, uint8_t page);
        void drawFloat(float value, uint8_t x, uint8_t page);
        void drawBitmap(const Bitmap& bmp, uint8_t x, uint8_t pageStart);
        void clear(void);

        void printf(const char* format, ...) ;
        void printf(uint8_t x, uint8_t page, const char* format, ...);

        extern uint8_t frameBuffer[FULL_SCREEN];
}

#endif /* INC_DISPLAY_HPP_ */
