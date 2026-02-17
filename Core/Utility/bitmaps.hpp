#include <cstdint>

#ifndef BITMAPS_HPP_
#define BITMAPS_HPP_

constexpr uint16_t MAX_WIDTH      = 128;
constexpr uint16_t MAX_HEIGHT     = 64;
constexpr uint32_t MAX_SIZE		  = MAX_WIDTH * MAX_HEIGHT;

constexpr uint16_t  PEDAL_WIDTH  = 24;
constexpr uint16_t  PEDAL_HEIGHT = 34;
constexpr uint16_t  PEDAL_SIZE   = (PEDAL_WIDTH * PEDAL_HEIGHT);

constexpr uint8_t VOLUME_BAR_WIDTH  = 5;
constexpr uint8_t VOLUME_BAR_HEIGHT = 32;
constexpr uint16_t VOLUME_BAR_SIZE  = (VOLUME_BAR_WIDTH * VOLUME_BAR_HEIGHT);

constexpr uint8_t MOD_BAR_WIDTH  = 33;
constexpr uint8_t MOD_BAR_HEIGHT = 36;
constexpr uint16_t MOD_BAR_SIZE  = (MOD_BAR_WIDTH * MOD_BAR_HEIGHT);

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

extern const Bitmap  base_chain_bitmap;
extern const Bitmap  startup_screen_bitmap;
extern const Bitmap  edit_pedal_bitmap;
extern const Bitmap  mod_pedal_bitmap;

extern const Bitmap  overdrive_distortion_bitmap;
extern const Bitmap  overdrive_distortion_disabled_bitmap;

extern const Bitmap  echo_bitmap;
extern const Bitmap  echo_disabled_bitmap;

extern const Bitmap  reverb_bitmap;
extern const Bitmap  reverb_disabled_bitmap;

extern const Bitmap  noise_gate_bitmap;
extern const Bitmap  noise_gate_disabled_bitmap;

extern const Bitmap  compressor_bitmap;
extern const Bitmap  compressor_disabled_bitmap;

extern const Bitmap  pass_through_bitmap;


extern const Bitmap vol_0_bitmap;
extern const Bitmap vol_1_bitmap;
extern const Bitmap vol_2_bitmap;
extern const Bitmap vol_3_bitmap;
extern const Bitmap vol_4_bitmap;
extern const Bitmap vol_5_bitmap;
extern const Bitmap vol_6_bitmap;
extern const Bitmap vol_7_bitmap;

extern const Bitmap arrow_right_bitmap;
extern const Bitmap arrow_left_bitmap;
extern const Bitmap indicator_bitmap;


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
