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

extern SPI_HandleTypeDef hspi1;

namespace Display {
    void init();
    void setContrast(uint8_t level);
    void writeCommand(uint8_t cmd);
    void writeData(uint8_t* data, uint16_t size);
    void reset();
    void setCursor(uint8_t column, uint8_t page);
    void drawChar(uint8_t c, uint8_t page, uint8_t col);
    void drawString(const char* str, uint8_t page, uint8_t col);
    void drawBitmap(const Bitmap& bmp, uint8_t x, uint8_t pageStart);
    void clear(void);
}



#endif /* INC_DISPLAY_HPP_ */
