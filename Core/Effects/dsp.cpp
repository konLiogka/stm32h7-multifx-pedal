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



// For Delay based effects
static float delayBuffer[32768]  = {0};
static const uint32_t bufferSize = 32768;
static uint32_t writePos         = 0;
static float feedbackFilterState = 0.0f;
const  float sampleRate          = 96000.0f;
const  float gamma_val           = 0.7f;

constexpr uint8_t NUM_COMBS   = 8;
constexpr uint8_t NUM_ALLPASS = 4;

static const uint32_t combDelays[NUM_COMBS] = {
    3163, 3617, 4013, 4409, 4801, 5209, 5597, 6007
};

static const uint32_t allpassDelays[NUM_ALLPASS] = {
    556, 441, 341, 225
};

constexpr float clamp(float x, float min, float max) {
    return (x < min) ? min : ((x > max) ? max : x);
}
 
static float combBuffers[NUM_COMBS][6008] = {0}; 
static float allpassBuffers[NUM_ALLPASS][560] = {0};
static uint32_t combPositions[NUM_COMBS]      = {0};
static uint32_t allpassPositions[NUM_ALLPASS] = {0};
static float combFilterStates[NUM_COMBS]      = {0};


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


float combFilter(float input, float* buffer, uint32_t bufferSize, 
                 uint32_t* position, float* filterState, 
                 float feedback, float damping)
{
    
    float delayed = buffer[*position];
        
    float filtered = (*filterState) * damping + delayed * (1.0f - damping);
        *filterState = filtered;
        
    buffer[*position] = input + filtered * feedback;
        
    *position = (*position + 1);
    if (*position >= bufferSize) *position = 0;
    
    return delayed;
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
               float delayTime, float feedback, float mix)
{
    delayTime = clamp(delayTime, 0.0f, 1.0f);
    feedback  = clamp(feedback, 0.0f, 0.95f);
    mix       = clamp(mix, 0.0f, 1.0f);
        
    const    float minDelayMs = 20.0f;
    const    float maxDelayMs = 680.0f;
    float    delayMs          = minDelayMs + delayTime * (maxDelayMs - minDelayMs);
    uint32_t delaySamples     = (uint32_t)((delayMs / 1000.0f) * sampleRate);
    
    if (delaySamples >= bufferSize) {
        delaySamples = bufferSize - 1;
    }
    
    for (uint16_t i = 0; i < length; ++i)
    {
        float x_n = input[i];
        
        uint32_t readPos       = (writePos + bufferSize - delaySamples) & (bufferSize - 1);
        float    delayedSample = delayBuffer[readPos];

        feedbackFilterState   = feedbackFilterState * gamma_val + delayedSample * (1.0f - gamma_val);
        delayBuffer[writePos] = x_n + feedbackFilterState * feedback;

        writePos = (writePos + 1) & (bufferSize - 1);

        output[i] = (1.0f - mix) * x_n + mix * delayedSample;
    }
}


void applyReverb(float* input, float* output, uint16_t length,
                float roomSize, float damping, float mix)
{
    roomSize = clamp(roomSize, 0.0f, 1.0f);
    damping = clamp(damping, 0.0f, 1.0f);
    mix = clamp(mix, 0.0f, 1.0f);
    
    float combFeedback = 0.7f + (roomSize * 0.25f);
    const float allpassFeedback = 0.5f;
    
    for (uint16_t i = 0; i < length; ++i)
    {
        float x_n = input[i];
        float reverbSignal = 0.0f;
        
        for (uint8_t c = 0; c < NUM_COMBS; ++c)
        {
            float combOut = combFilter(x_n, combBuffers[c], combDelays[c],
                                       &combPositions[c], &combFilterStates[c],
                                       combFeedback, damping);
            reverbSignal += combOut;
        }
        
        reverbSignal *= 0.125f;  
        
        for (uint8_t a = 0; a < NUM_ALLPASS; ++a)
        {
            uint32_t delayLength = allpassDelays[a];
            uint32_t readPos     = allpassPositions[a];
            
            float delayed  = allpassBuffers[a][readPos];
            float input_ap = reverbSignal;
            
            reverbSignal               = -input_ap + delayed;
            allpassBuffers[a][readPos] = input_ap + allpassFeedback * delayed;
            
            allpassPositions[a] = (readPos + 1) < delayLength ? (readPos + 1) : 0;
        }
        
        output[i] = (1.0f - mix) * x_n + mix * reverbSignal;
    }
}

void applyNoiseGate(float* input, float* output, uint16_t length,
                   float threshold, float hold, float release)
{
    threshold = clamp(threshold, 0.0f, 1.0f);
    hold      = clamp(hold, 0.0f, 1.0f);
    release   = clamp(release, 0.0f, 1.0f);

    float thresholdAmp = powf(10.0f, -3.0f + threshold * 2.5f);
    
    const float minHoldMs    = 10.0f;
    const float maxHoldMs    = 500.0f;
    const float sampleRateMs = sampleRate * 0.001f;
    float holdMs             = minHoldMs + hold * (maxHoldMs - minHoldMs);

    uint32_t holdSamples        = (uint32_t)(holdMs * sampleRateMs);
    const    float minReleaseMs = 5.0f;
    const    float maxReleaseMs = 500.0f;
    float    releaseMs          = minReleaseMs + release * (maxReleaseMs - minReleaseMs);
    float    releaseCoeff       = expf(-1000.0f / (releaseMs * sampleRate));
    
    const float attackMs    = 1.0f;
    const float attackCoeff = 0.989958f;
    
    static float envelope       = 0.0f;
    static float gainSmooth     = 0.0f;
    static uint32_t holdCounter = 0;
    
    for (uint16_t i = 0; i < length; ++i)
    {
        float x_n = input[i];
        
        float rectified = fabsf(x_n);
        envelope = envelope * 0.999f + rectified * 0.001f;
        
        float targetGain = 0.0f;
        
        if (envelope > thresholdAmp)
        {
            holdCounter = holdSamples;
            targetGain = 1.0f;
        }
        else
        {
            if (holdCounter > 0)
            {
                holdCounter--;
                targetGain = 1.0f;
            }
            else
            {
                targetGain = 0.0f;
            }
        }
        
        if (targetGain > gainSmooth)
        {
            gainSmooth = gainSmooth * attackCoeff + targetGain * (1.0f - attackCoeff);
        }
        else
        {
            gainSmooth = gainSmooth * releaseCoeff + targetGain * (1.0f - releaseCoeff);
        }
        
        output[i] = x_n * gainSmooth;
    }
}

}