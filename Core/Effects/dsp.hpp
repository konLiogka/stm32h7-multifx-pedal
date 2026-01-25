#pragma once

#include <string.h>
#include <stdint.h>

#ifndef DSP_HPP_
#define DSP_HPP_


namespace DSP {
    void applyOverdrive(float* input, float* output, uint16_t length, 
                       float gain, float tone, float level);
    
    void applyEcho(float* input, float* output, uint16_t length,
                float delayTime, float feedback, float mix,
                float mod);
    
    void applyReverb(float* input, float* output, uint16_t length,
                    float depth, float rate, float mix);
    
    void applyPassThrough(float* input, float* output, uint16_t length,
                         float highs, float mids, float lows, float volume);
}

#endif