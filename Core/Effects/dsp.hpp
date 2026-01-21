#pragma once

#include <cstdint>

namespace DSP {
    void applyOverdrive(float* input, float* output, uint16_t length, 
                       float gain, float tone, float level, float crunch);
    
    void applyEcho(float* input, float* output, uint16_t length,
                  float delayTime, float feedback, float mix);
    
    void applyReverb(float* input, float* output, uint16_t length,
                    float depth, float rate, float mix);
    
    void applyPassThrough(float* input, float* output, uint16_t length,
                         float highs, float mids, float lows, float volume);
}