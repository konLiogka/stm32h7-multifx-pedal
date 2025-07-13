/*
 * bitmaps.hpp
 *
 *  Created on: May 30, 2025
 *      Author: kliogka
 */

#ifndef INC_BITMAPS_HPP_
#define INC_BITMAPS_HPP_

#include <cstdint>

constexpr uint16_t FULL_SCREEN_WIDTH  = 128;
constexpr uint16_t FULL_SCREEN_HEIGHT = 64;
constexpr uint32_t FULL_SCREEN 		  = FULL_SCREEN_WIDTH * FULL_SCREEN_HEIGHT;

constexpr uint16_t  PEDAL_WIDTH  = 24;
constexpr uint16_t  PEDAL_HEIGHT = 34;
constexpr uint16_t  PEDAL_SIZE   = (PEDAL_WIDTH * PEDAL_HEIGHT);

constexpr uint16_t VOLUME_BAR_WIDTH  = 5;
constexpr uint16_t VOLUME_BAR_HEIGHT = 32;
constexpr uint16_t VOLUME_BAR_SIZE   = (VOLUME_BAR_WIDTH * VOLUME_BAR_HEIGHT);

constexpr uint16_t  PEDAL_PAGE_START   = 10;
constexpr uint16_t  PEDAL1_X_POS_START = 13;
constexpr uint16_t  PEDAL2_X_POS_START = 39;
constexpr uint16_t  PEDAL3_X_POS_START = 65;
constexpr uint16_t PEDAL4_X_POS_START  = 91;

struct Bitmap {
    const uint8_t* data;
    uint16_t width;
    uint16_t height;
    const char* name;
};

extern const uint8_t font_5x7[][5];

extern const uint8_t base_chain[FULL_SCREEN];
extern const Bitmap  base_chain_bitmap;

extern const uint8_t overdrive_distortion_data[PEDAL_SIZE];
extern const Bitmap  overdrive_distortion_bitmap;

extern const uint8_t echo_data[PEDAL_SIZE];
extern const Bitmap  echo_bitmap;

extern const uint8_t pass_through_data[PEDAL_SIZE];
extern const Bitmap  pass_through_bitmap;

extern const uint8_t reverb_data[PEDAL_SIZE];
extern const Bitmap  reverb_bitmap;

extern const uint8_t vol_0[VOLUME_BAR_SIZE];
extern const uint8_t vol_1[VOLUME_BAR_SIZE];
extern const uint8_t vol_2[VOLUME_BAR_SIZE];
extern const uint8_t vol_3[VOLUME_BAR_SIZE];
extern const uint8_t vol_4[VOLUME_BAR_SIZE];
extern const uint8_t vol_5[VOLUME_BAR_SIZE];
extern const uint8_t vol_6[VOLUME_BAR_SIZE];
extern const uint8_t vol_7[VOLUME_BAR_SIZE];

extern const Bitmap pedals[4];

extern const int pedalPositionsX[];


#endif /* INC_BITMAPS_HPP_ */
