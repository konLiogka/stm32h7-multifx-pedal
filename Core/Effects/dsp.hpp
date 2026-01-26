#pragma once

#include <string.h>
#include <stdint.h>

#ifndef DSP_HPP_
#define DSP_HPP_

namespace DSP
{
    void applyOverdrive(float *input, float *output, uint16_t length,
                        float gain, float tone, float level);

    void applyEcho(float *input, float *output, uint16_t length,
                   float delayTime, float feedback, float mix);

    void applyReverb(float* input, float* output, uint16_t length,
                    float roomSize, float damping, float mix);

    void applyNoiseGate(float* input,float* output, uint16_t length,
                    float threshold, float hold, float release);

}

#endif