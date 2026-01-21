#include "dsp.hpp"
#include <cstring>
#include <cmath>

namespace DSP {
    
    void applyOverdrive(float* input, float* output, uint16_t length, 
                       float gain, float tone, float level, float crunch)
  {
        static float hp_prev_in = 0.0f;
        static float hp_prev_out = 0.0f;
        static float lp_prev_out = 0.0f;
        
        for (uint16_t i = 0; i < length; i++)
        {
            float sample = input[i];
            
            float hp_out = sample - hp_prev_in + 0.995f * hp_prev_out;
            hp_prev_in = sample;
            hp_prev_out = hp_out;
            sample = hp_out;
            
            sample *= (1.0f + gain * 9.0f);
            
            if (sample > 0.0f) {
                sample = tanhf(sample * 1.2f) * 0.9f;
            } else {
                sample = tanhf(sample * 1.5f) * 0.85f;
            }
            
            float lp_out = tone * sample + (1.0f - tone) * lp_prev_out;
            lp_prev_out = lp_out;
            
            output[i] = lp_out * level;
        }
    }
    
    void applyEcho(float* input, float* output, uint16_t length,
                  float delayTime, float feedback, float mix)
    {
        memcpy(output, input, length * sizeof(float));
    }
    
    void applyReverb(float* input, float* output, uint16_t length,
                    float depth, float rate, float mix)
    {
        memcpy(output, input, length * sizeof(float));
    }
}