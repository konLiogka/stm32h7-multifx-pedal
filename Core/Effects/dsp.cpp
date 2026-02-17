#include "dsp.hpp"
#include "common.hpp"
#include <cmath>

typedef struct {
    float prev_in;
    float prev_out;
} HPF_State;

typedef struct {
    float prev_out;
} LPF_State;


struct CombFilterState {
    std::array<float, 6008> buffer{};
    uint32_t position{};
    float filterState{};
};

struct AllpassFilterState {
    std::array<float, 560> buffer{};
    uint32_t position{};
};


// Echo
constexpr uint32_t BUFFER_SIZE = 32768;
constexpr float SAMPLE_RATE    = 96000.0f;
constexpr float GAMMA          = 0.7f;
constexpr uint8_t NUM_COMBS    = 8;
constexpr uint8_t NUM_ALLPASS  = 4;

std::array<float, BUFFER_SIZE>  delayBuffer{};
static    uint32_t writePos         = 0;
static    float feedbackFilterState = 0.0f;

constexpr float minDelayMs       = 20.0f;
constexpr float maxDelayMs       = 680.0f;

constexpr std::array<uint32_t, NUM_COMBS> combDelays = {
    3163, 3617, 4013, 4409, 4801, 5209, 5597, 6007
};

constexpr std::array<uint32_t, NUM_ALLPASS> allpassDelays = {
    556, 441, 341, 225
};

// Reverb
constexpr float allpassFeedback = 0.5f;

// Noise gate
constexpr float minHoldMs    = 10.0f;
constexpr float maxHoldMs    = 500.0f;
constexpr float sampleRateMs = SAMPLE_RATE * 0.001f;

constexpr float minReleaseMs    = 5.0f;
constexpr float maxReleaseMs    = 500.0f;
constexpr float attackCoeff     = 0.989958f;

static std::array<CombFilterState, NUM_COMBS> combFilters{};
static std::array<AllpassFilterState, NUM_ALLPASS> allpassFilters{};

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

float combFilter(float input, float* buffer, uint32_t BUFFER_SIZE, 
                 uint32_t* position, float* filterState, 
                 float feedback, float damping)
{
    const float delayed = buffer[*position];
    const float filtered = (*filterState) * damping + delayed * (1.0f - damping);
    *filterState = filtered;
    buffer[*position] = input + filtered * feedback;
    
    *position = (*position + 1);
    if (*position >= BUFFER_SIZE) *position = 0;
    
    return delayed;
}

namespace DSP
{
    void applyOverdrive(float *input, float *output, uint16_t length,
                        float gain, float tone, float level)
    {
        static HPF_State hpf_state = {0.0f, 0.0f};
        static LPF_State lpf_state = {0.0f};

        for (uint16_t i = 0; i < length; i++)
        {
            float sample = input[i];

            sample = highPassFilter(sample, &hpf_state);

            sample *= (1.0f + gain * 49.0f);

            if (sample > 1.0f)
            {
                sample = 0.7f + 0.3f * tanhf((sample - 1.0f) * 2.0f);
            }
            else if (sample < -1.0f)
            {
                sample = -0.7f + 0.3f * tanhf((sample + 1.5f) * 1.8f);
            }
            else
            {
                sample = tanhf(sample * 1.5f);
            }

            sample = lowPassFilter(sample, tone, &lpf_state);

            output[i] = sample * level * 1.5f;
        }
    }

    void applyEcho(float *input, float *output, uint16_t length,
                   float delayTime, float feedback, float mix)
    {
        delayTime = clamp(delayTime, 0.0f, 1.0f);
        feedback  = clamp(feedback, 0.0f, 0.95f);
        mix       = clamp(mix, 0.0f, 1.0f);

        float delayMs = minDelayMs + delayTime * (maxDelayMs - minDelayMs);
        const uint32_t delay_samples = std::min(
            static_cast<uint32_t>((delayMs / 1000.0f) * SAMPLE_RATE),
            BUFFER_SIZE - 1);

        for (uint16_t i = 0; i < length; ++i)
        {
            float x_n = input[i];

            uint32_t readPos       = (writePos + BUFFER_SIZE - delay_samples) & (BUFFER_SIZE - 1);
            float    delayedSample = delayBuffer[readPos];

            feedbackFilterState   = feedbackFilterState * GAMMA + delayedSample * (1.0f - GAMMA);
            delayBuffer[writePos] = x_n + feedbackFilterState * feedback;

            writePos = (writePos + 1) & (BUFFER_SIZE - 1);

            output[i] = (1.0f - mix) * x_n + mix * delayedSample;
        }
    }

    void applyReverb(float *input, float *output, uint16_t length,
                     float roomSize, float damping, float mix)
    {
        roomSize = clamp(roomSize, 0.0f, 1.0f);
        damping  = clamp(damping, 0.0f, 1.0f);
        mix      = clamp(mix, 0.0f, 1.0f);

        const float combFeedback = 0.7f + (roomSize * 0.25f);

        for (uint16_t i = 0; i < length; ++i)
        {
            const float x_n    = input[i];
            float reverbSignal = 0.0f;

            for (size_t c = 0; c < NUM_COMBS; ++c)
            {
                auto &comb = combFilters[c];
                const float combOut = combFilter(x_n, comb.buffer.data(), combDelays[c],
                                                 &comb.position, &comb.filterState,
                                                 combFeedback, damping);
                reverbSignal += combOut;
            }

            reverbSignal *= 0.125f;

            for (size_t a = 0; a < NUM_ALLPASS; ++a)
            {
                auto  &allpass             = allpassFilters[a];
                const uint32_t delayLength = allpassDelays[a];

                const float delayed  = allpass.buffer[allpass.position];
                const float input_ap = reverbSignal;

                reverbSignal                     = -input_ap + delayed;
                allpass.buffer[allpass.position] = input_ap + allpassFeedback * delayed;

                allpass.position = (allpass.position + 1) < delayLength ? (allpass.position + 1) : 0;
            }

            output[i] = (1.0f - mix) * x_n + mix * reverbSignal;
        }
    }

    void applyNoiseGate(float *input, float *output, uint16_t length,
                        float threshold, float hold, float release)
    {
        threshold = clamp(threshold, 0.0f, 1.0f);
        hold      = clamp(hold, 0.0f, 1.0f);
        release   = clamp(release, 0.0f, 1.0f);

        float    thresholdAmp = powf(10.0f, -3.0f + threshold * 2.5f);
        float    holdMs       = minHoldMs + hold * (maxHoldMs - minHoldMs);
        uint32_t holdSamples  = (uint32_t)(holdMs * sampleRateMs);

        float releaseMs    = minReleaseMs + release * (maxReleaseMs - minReleaseMs);
        float releaseCoeff = expf(-1000.0f / (releaseMs * SAMPLE_RATE));

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