#include "startup_animation.hpp"
#include "display.hpp"

void startupAnimation()
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