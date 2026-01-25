#pragma once

#include <cstdint>

#ifndef DSP_HPP_
#define DSP_HPP_

typedef struct {
    float prev_in;
    float prev_out;
} HPF_State;

typedef struct {
    float prev_out;
} LPF_State;

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