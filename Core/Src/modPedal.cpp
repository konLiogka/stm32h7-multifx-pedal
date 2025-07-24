#include "modPedal.hpp" 
#include <cstring>

void drawValBar(float val, uint8_t x)
{
    if(val<0.1f)
        Display::drawBitmap(mod_pedal_val_bitmap_0, x, 11);
    else if(val<0.2f)
        Display::drawBitmap(mod_pedal_val_bitmap_1, x, 11);
    else if(val<0.3f)
        Display::drawBitmap(mod_pedal_val_bitmap_2, x, 11);
    else if(val<0.4f)
        Display::drawBitmap(mod_pedal_val_bitmap_3, x, 11);
    else if(val<0.5f)
        Display::drawBitmap(mod_pedal_val_bitmap_4, x, 11);
    else if(val<0.6f)
        Display::drawBitmap(mod_pedal_val_bitmap_5, x, 11);
    else if(val<0.7f)
        Display::drawBitmap(mod_pedal_val_bitmap_6, x, 11);
    else if(val<0.8f)
        Display::drawBitmap(mod_pedal_val_bitmap_7, x, 11);
    else if(val<0.9f)
        Display::drawBitmap(mod_pedal_val_bitmap_8, x, 11);
    else if(val<1.0f)
        Display::drawBitmap(mod_pedal_val_bitmap_9, x, 11);
    else
        Display::drawBitmap(mod_pedal_val_bitmap_10, x, 11);
}



void displayPedalSettings(Pedal *pedal, uint8_t page)
{
    if (pedal == nullptr) return;



    const char* const* names = pedal->getMemberNames();
    float* params = new float[pedal->getMemberSize()];
    pedal->getParams(params);
    uint8_t numParams = pedal->getMemberSize();
    uint8_t char_width = 6;
    uint8_t index = 0;
    if (page == 1)
    {
        index = 3;
    }

    for (uint8_t i = 0; i < 3; i++) {
        if (i + index >= numParams) break;  
        uint8_t bitmap_x = (i % 3) * 40 + 10;
        uint8_t string_width = strlen(names[i + index]) * char_width;
        uint8_t string_x = bitmap_x + (30 - string_width) / 2;

        drawValBar(params[i + index], bitmap_x);
        Display::drawString(names[i + index], string_x, 11);
        Display::printf(bitmap_x + 8 , 12, "%d", (int)(params[i + index]*100));

     }
    if (page == 0)
    {
        Display::drawBitmap(arrow_right_bitmap, 120, 3);
    }
    else
    {
        Display::drawBitmap(arrow_left_bitmap, 0, 3);
    }
    delete[] params;

}



void changePedalSettings(Pedal *pedal, uint8_t page, uint32_t potValues[3])
{
    if (pedal == nullptr) return;

    uint8_t index = 0;
    if (page == 1)
    {
        index = 3;
    }

    float* params = new float[pedal->getMemberSize()];
    pedal->getParams(params);
    uint8_t numParams = pedal->getMemberSize();

    for (uint8_t i = 0; i < 3; i++) {
        if (i + index >= numParams) break;
        params[i + index] = potValues[i] / 100.0f;
    }

    pedal->setParams(params);
    delete[] params;
    displayPedalSettings(pedal, page);
}