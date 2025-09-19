#include <cstdint>

#ifndef BITMAPS_HPP_
#define BITMAPS_HPP_

constexpr uint16_t FULL_SCREEN_WIDTH  = 128;
constexpr uint16_t FULL_SCREEN_HEIGHT = 64;
constexpr uint32_t FULL_SCREEN 		  = FULL_SCREEN_WIDTH * FULL_SCREEN_HEIGHT;

constexpr uint16_t  PEDAL_WIDTH  = 24;
constexpr uint16_t  PEDAL_HEIGHT = 34;
constexpr uint16_t  PEDAL_SIZE   = (PEDAL_WIDTH * PEDAL_HEIGHT);

constexpr uint8_t VOLUME_BAR_WIDTH  = 5;
constexpr uint8_t VOLUME_BAR_HEIGHT = 32;
constexpr uint16_t VOLUME_BAR_SIZE   = (VOLUME_BAR_WIDTH * VOLUME_BAR_HEIGHT);

constexpr uint8_t MOD_BAR_WIDTH  = 33;
constexpr uint8_t MOD_BAR_HEIGHT = 36;
constexpr uint16_t MOD_BAR_SIZE = (MOD_BAR_WIDTH * MOD_BAR_HEIGHT);

constexpr uint8_t  PEDAL_PAGE_START   = 10;
constexpr uint8_t  PEDAL1_X_POS_START = 13;
constexpr uint8_t  PEDAL2_X_POS_START = 39;
constexpr uint8_t  PEDAL3_X_POS_START = 65;
constexpr uint8_t  PEDAL4_X_POS_START = 91;

struct Bitmap {
    const uint8_t* data;
    uint16_t width;
    uint16_t height;
    const char* name;
};

const int pedalPositionsX[] = {
    PEDAL1_X_POS_START,
    PEDAL2_X_POS_START,
    PEDAL3_X_POS_START,
    PEDAL4_X_POS_START
};

extern const uint8_t font_5x7[][5];

extern const uint8_t base_chain[FULL_SCREEN];
extern const Bitmap  base_chain_bitmap;

extern const uint8_t startup_screen[FULL_SCREEN];
extern const Bitmap  startup_screen_bitmap;

extern const uint8_t edit_pedal[FULL_SCREEN];
extern const Bitmap  edit_pedal_bitmap;

extern const uint8_t mod_pedal[FULL_SCREEN];
extern const Bitmap  mod_pedal_bitmap;

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

extern const Bitmap vol_0_bitmap;
extern const Bitmap vol_1_bitmap;
extern const Bitmap vol_2_bitmap;
extern const Bitmap vol_3_bitmap;
extern const Bitmap vol_4_bitmap;
extern const Bitmap vol_5_bitmap;
extern const Bitmap vol_6_bitmap;
extern const Bitmap vol_7_bitmap;

extern const uint8_t arrow_right[8*16];
extern const Bitmap arrow_right_bitmap;

extern const uint8_t arrow_left[8*16];
extern const Bitmap arrow_left_bitmap;

extern const uint8_t indicator[32];
extern const Bitmap indicator_bitmap;

extern const uint8_t mod_pedal_val_0[MOD_BAR_SIZE];
extern const uint8_t mod_pedal_val_1[MOD_BAR_SIZE];
extern const uint8_t mod_pedal_val_2[MOD_BAR_SIZE];
extern const uint8_t mod_pedal_val_3[MOD_BAR_SIZE];
extern const uint8_t mod_pedal_val_4[MOD_BAR_SIZE];
extern const uint8_t mod_pedal_val_5[MOD_BAR_SIZE];
extern const uint8_t mod_pedal_val_6[MOD_BAR_SIZE];
extern const uint8_t mod_pedal_val_7[MOD_BAR_SIZE];
extern const uint8_t mod_pedal_val_8[MOD_BAR_SIZE];
extern const uint8_t mod_pedal_val_9[MOD_BAR_SIZE];
extern const uint8_t mod_pedal_val_10[MOD_BAR_SIZE];

extern const Bitmap mod_pedal_val_bitmap_0;
extern const Bitmap mod_pedal_val_bitmap_1;
extern const Bitmap mod_pedal_val_bitmap_2;
extern const Bitmap mod_pedal_val_bitmap_3;
extern const Bitmap mod_pedal_val_bitmap_4;
extern const Bitmap mod_pedal_val_bitmap_5;
extern const Bitmap mod_pedal_val_bitmap_6;
extern const Bitmap mod_pedal_val_bitmap_7;
extern const Bitmap mod_pedal_val_bitmap_8;
extern const Bitmap mod_pedal_val_bitmap_9;
extern const Bitmap mod_pedal_val_bitmap_10;

extern const Bitmap pedals[4];

extern const int pedalPositionsX[];


#endif /* INC_BITMAPS_HPP_ */
