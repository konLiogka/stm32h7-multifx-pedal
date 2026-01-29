#pragma once

#ifndef DSP_HPP_
#define DSP_HPP_

#include <string.h>
#include <stdint.h>

#ifdef _WIN32
    #ifdef DSP_EXPORT
        #define DSP_API __declspec(dllexport)
    #else
        #define DSP_API __declspec(dllimport)
    #endif
#else
    #ifdef DSP_EXPORT
        #define DSP_API __attribute__((visibility("default")))
    #else
        #define DSP_API
    #endif
#endif


namespace DSP
{
    #ifdef __cplusplus
    extern "C" {
    #endif

    DSP_API void applyOverdrive(float *input, float *output, uint16_t length,
                        float gain, float tone, float level);

    DSP_API void applyEcho(float *input, float *output, uint16_t length,
                   float delayTime, float feedback, float mix);

    DSP_API void applyReverb(float* input, float* output, uint16_t length,
                    float roomSize, float damping, float mix);

    DSP_API void applyNoiseGate(float* input,float* output, uint16_t length,
                    float threshold, float hold, float release);

    #ifdef __cplusplus
    }
    #endif

}

#endif