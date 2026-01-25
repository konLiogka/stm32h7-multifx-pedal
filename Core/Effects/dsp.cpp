#include "dsp.hpp"
#include <cstring>
#include <cmath>

typedef struct {
    float prev_in;
    float prev_out;
} HPF_State;

typedef struct {
    float prev_out;
} LPF_State;


// For Delay effects
static float delayBuffer[32768] = {0};
static const uint32_t bufferSize = 32768;
static uint32_t writePos = 0;

static float modPhase = 0.0f;

static float feedbackFilterState = 0.0f;

const float sampleRate = 96000.0f;          
const float modPhaseIncrement = 0.000005208f; // 0.5 / 96kHz 
const float gamma_val = 0.7f; 

float lowPassFilter(float sample, float tone, LPF_State* state)
{
    float lp_out = tone * sample + (1.0f - tone) * state->prev_out;
    state->prev_out = lp_out;

    float highs = sample - lp_out;

    float presence = tone * 0.4f;

    return lp_out + highs * presence;
}

float highPassFilter(float sample, HPF_State* state)
{
    float hp_out = sample - state->prev_in + 0.995f * state->prev_out;

    state->prev_in  = sample;
    state->prev_out = hp_out;

    return hp_out;
}

namespace DSP {
   void applyOverdrive(float* input, float* output, uint16_t length,
                    float gain, float tone, float level) 
{
    static HPF_State hpf_state = {0.0f, 0.0f};
    static LPF_State lpf_state = {0.0f};
    
    for (uint16_t i = 0; i < length; i++) {
        float sample = input[i];
        
        sample = highPassFilter(sample, &hpf_state);
        
        sample *= (1.0f + gain * 49.0f);
        
        if (sample > 1.0f) {
            sample = 0.7f + 0.3f * tanhf((sample - 1.0f) * 2.0f);
        } else if (sample < -1.0f) {
            sample = -0.7f + 0.3f * tanhf((sample + 1.5f) * 1.8f);
        } else {
            sample = tanhf(sample * 1.5f);
        }
        
        sample = lowPassFilter(sample, tone, &lpf_state);
        
        output[i] = sample * level * 1.5f; 
    }
}

    
void applyEcho(float* input, float* output, uint16_t length,
               float delayTime, float feedback, float mix, float mod)
{
   
    
    uint32_t D0 = (uint32_t)((delayTime * 0.001f) * sampleRate);
    
    float A = fminf(mod * 96.0f, (float)(D0 - 1));
    
    for (uint16_t i = 0; i < length; i++)
    {
        float x_n = input[i];
        
        float sinValue = sinf(2.0f * 3.14159265359f * modPhase);
        float D_n = (float)D0 + A * sinValue;
        
        if (D_n < 1.0f) D_n = 1.0f;
        if (D_n > (float)(bufferSize - 2)) D_n = (float)(bufferSize - 2);
        
        uint32_t D_floor = (uint32_t)D_n;    
        float alpha = D_n - (float)D_floor ;
        
        uint32_t readPos1 = (writePos + bufferSize - D_floor) & (bufferSize - 1);     
        uint32_t readPos2 = (writePos + bufferSize - D_floor - 1) & (bufferSize - 1); 
        
        float y_delayed = delayBuffer[readPos1] * (1.0f - alpha) + 
                          delayBuffer[readPos2] * alpha;
        
        float h_n = gamma_val * feedbackFilterState + (1.0f - gamma_val) * y_delayed;
        feedbackFilterState = h_n; 
        
        delayBuffer[writePos] = x_n + feedback * h_n;
        
        output[i] = (1.0f - mix) * x_n + mix * y_delayed;
        
        writePos = (writePos + 1) & (bufferSize - 1);
        
        modPhase += modPhaseIncrement;
        if (modPhase >= 1.0f) modPhase -= 1.0f;
    }
}

    void applyReverb(float* input, float* output, uint16_t length,
                    float depth, float rate, float mix)
    {
        memcpy(output, input, length * sizeof(float));
    }
}