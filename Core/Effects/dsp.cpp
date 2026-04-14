#include "dsp.hpp"
#include "common.hpp"
#include <cmath>
 


namespace Config
{
    constexpr float    SAMPLE_RATE     = 96000.0f;
    constexpr float    SAMPLE_RATE_MS  = SAMPLE_RATE * 0.001f;

    constexpr uint32_t ECHO_BUFFER_SIZE = 32768;
    constexpr float    ECHO_FEEDBACK_GAMMA = 0.7f;
    constexpr float    ECHO_MIN_DELAY_MS   = 20.0f;
    constexpr float    ECHO_MAX_DELAY_MS   = 680.0f;

    constexpr uint8_t  NUM_COMBS    = 8;
    constexpr uint8_t  NUM_ALLPASS  = 4;
    constexpr float    ALLPASS_FEEDBACK = 0.5f;

    constexpr std::array<uint32_t, NUM_COMBS> COMB_DELAYS = {
        3163, 3617, 4013, 4409, 4801, 5209, 5597, 6007
    };
    constexpr std::array<uint32_t, NUM_ALLPASS> ALLPASS_DELAYS = {
        556, 441, 341, 225
    };

    constexpr float MIN_HOLD_MS    = 10.0f;
    constexpr float MAX_HOLD_MS    = 500.0f;
    constexpr float MIN_ATTACK_MS  = 1.0f;    
    constexpr float MAX_ATTACK_MS  = 100.0f;   
    constexpr float MIN_RELEASE_MS = 10.0f;   
    constexpr float MAX_RELEASE_MS = 1000.0f;
    
    constexpr float ATTACK_COEFF = 0.989958f;
    
    constexpr float COMPRESSOR_ATTACK_COEFF = 0.999f;
    constexpr float COMPRESSOR_RELEASE_COEFF = 0.9998f;
}


struct HpfState
{
    float prevIn = 0.0f;
    float prevOut = 0.0f;

    float process(float sample, float tone = 0.995f)
    {
        float hp_out = sample - prevIn + tone * prevOut;
        prevIn = sample;
        prevOut = hp_out;
        return hp_out;
    }
};

struct LpfState
{
    float prevOut = 0.0f;

    float process(float sample, float tone)
    {
        float lp_out = tone * sample + (1.0f - tone) * prevOut;
        prevOut = lp_out;
        
        float highs = sample - lp_out;
        float presence = tone * 0.4f;
        
        return lp_out + highs * presence;
    }
};

struct CombFilterState
{
    std::array<float, 6008> buffer{};
    uint32_t position = 0;
    float filterState = 0.0f;

    float process(float input, uint32_t delayLength, float feedback, float damping)
    {
        const float delayed = buffer[position];
        const float filtered = filterState * damping + delayed * (1.0f - damping);
        filterState = filtered;
        buffer[position] = input + filtered * feedback;
        
        position = (position + 1);
        if (position >= delayLength) position = 0;
        
        return delayed;
    }
    
};

struct AllpassFilterState
{
    std::array<float, 560> buffer{};
    uint32_t position = 0;

    float process(float input, uint32_t delayLength, float feedback)
    {
        const float delayed = buffer[position];
        const float input_ap = input;
        
        float output = -input_ap + delayed;
        buffer[position] = input_ap + feedback * delayed;
        
        position = (position + 1);
        if (position >= delayLength) position = 0;
        
        return output;
    }
    
   
};

struct EchoState
{
    std::array<float, Config::ECHO_BUFFER_SIZE> buffer{};
    uint32_t writePos = 0;
    float feedbackFilterState = 0.0f;

    float process(float input, uint32_t delaySamples, float feedback, float gamma, float mix)
    {
        uint32_t readPos = (writePos + Config::ECHO_BUFFER_SIZE - delaySamples) & (Config::ECHO_BUFFER_SIZE - 1);
        float delayedSample = buffer[readPos];
        
        feedbackFilterState = feedbackFilterState * gamma + delayedSample * (1.0f - gamma);

        buffer[writePos] = input + feedbackFilterState * feedback;
        
        writePos = (writePos + 1) & (Config::ECHO_BUFFER_SIZE - 1);
        
        return (1.0f - mix) * input + mix * delayedSample;
    }
    
    void reset()
    {
        buffer.fill(0.0f);
        writePos = 0;
        feedbackFilterState = 0.0f;
    }
};

struct NoiseGateState
{
    float envelope = 0.0f;
    float gainSmooth = 0.0f;
    uint32_t holdCounter = 0;
    
    float process(float input, float thresholdAmp, uint32_t holdSamples, 
                  float attackCoeff, float releaseCoeff)
    {
        float rectified = fabsf(input);
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
        
        return input * gainSmooth;
    }
    
    void reset()
    {
        envelope = 0.0f;
        gainSmooth = 0.0f;
        holdCounter = 0;
    }
};

 
struct CompressorState
{
    float envelope = 0.0f;
    float gainSmooth = 1.0f;
    
    float process(float input, float thresholdLin, float compression, 
                  float makeup, float attackCoeff, float releaseCoeff)
    {
        float rectified = fabsf(input);
        
        if (rectified > envelope)
            envelope = envelope * attackCoeff + rectified * (1.0f - attackCoeff);
        else
            envelope = envelope * releaseCoeff + rectified * (1.0f - releaseCoeff);
        
        float targetGain = 1.0f;
        
        if (envelope > thresholdLin)
        {
            float overshoot = (envelope - thresholdLin) * (1.0f / thresholdLin);
            float reduction = 1.0f + overshoot * compression;
            targetGain = 1.0f / reduction;
        }
        
        if (targetGain < gainSmooth)
            gainSmooth = gainSmooth * attackCoeff + targetGain * (1.0f - attackCoeff);
        else
            gainSmooth = gainSmooth * releaseCoeff + targetGain * (1.0f - releaseCoeff);
        
        return input * gainSmooth * makeup;
    }
    
    void reset()
    {
        envelope = 0.0f;
        gainSmooth = 1.0f;
    }
};
    


static std::array<CombFilterState, Config::NUM_COMBS> combFilters{};
static std::array<AllpassFilterState, Config::NUM_ALLPASS> allpassFilters{};
static EchoState echoState{};
static NoiseGateState noiseGateState{};
static CompressorState compressorState{};

namespace DSP
{
    void applyOverdrive(float *input, float *output, uint16_t length,
                        float gain, float tone, float level)
    {
        static HpfState hpfState{};
        static LpfState lpfState{};

        for (uint16_t i = 0; i < length; i++)
        {
            float sample = input[i];

            sample = hpfState.process(sample, 0.995f);
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

            sample = lpfState.process(sample, tone);

            output[i] = sample * level * 1.5f;
        }
    }

    void applyEcho(float *input, float *output, uint16_t length,
                   float delayTime, float feedback, float mix)
    {
        delayTime = clamp(delayTime, 0.0f, 1.0f);
        feedback  = clamp(feedback, 0.0f, 0.95f);
        mix       = clamp(mix, 0.0f, 1.0f);

        float delayMs = Config::ECHO_MIN_DELAY_MS + delayTime * (Config::ECHO_MAX_DELAY_MS - Config::ECHO_MIN_DELAY_MS);
        const uint32_t delaySamples = std::min(
            static_cast<uint32_t>((delayMs / 1000.0f) * Config::SAMPLE_RATE),
            Config::ECHO_BUFFER_SIZE - 1);

        for (uint16_t i = 0; i < length; ++i)
        {
            output[i] = echoState.process(input[i], delaySamples, feedback, 
                                         Config::ECHO_FEEDBACK_GAMMA, mix);
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
            const float x_n = input[i];
            float reverbSignal = 0.0f;

            for (size_t c = 0; c < Config::NUM_COMBS; ++c)
            {
                reverbSignal += combFilters[c].process(x_n, Config::COMB_DELAYS[c], 
                                                       combFeedback, damping);
            }

            reverbSignal *= 0.125f;

            for (size_t a = 0; a < Config::NUM_ALLPASS; ++a)
            {
                reverbSignal = allpassFilters[a].process(reverbSignal, Config::ALLPASS_DELAYS[a], 
                                                        Config::ALLPASS_FEEDBACK);
            }

            output[i] = (1.0f - mix) * x_n + mix * reverbSignal;
        }
    }

    void applyNoiseGate(float *input, float *output, uint16_t length,
                        float threshold, float attack, float hold, float release)
    {
        threshold = clamp(threshold, 0.0f, 1.0f);
        attack    = clamp(attack, 0.0f, 1.0f);
        hold      = clamp(hold, 0.0f, 1.0f);
        release   = clamp(release, 0.0f, 1.0f);

        float thresholdAmp = powf(10.0f, -3.0f + threshold * 2.5f);
        
        float attackMs = Config::MIN_ATTACK_MS + attack * (Config::MAX_ATTACK_MS - Config::MIN_ATTACK_MS);
        float attackCoeff = expf(-1000.0f / (attackMs * Config::SAMPLE_RATE));
        
        float holdMs = Config::MIN_HOLD_MS + hold * (Config::MAX_HOLD_MS - Config::MIN_HOLD_MS);
        uint32_t holdSamples = static_cast<uint32_t>(holdMs * Config::SAMPLE_RATE_MS);
        
        float releaseMs = Config::MIN_RELEASE_MS + release * (Config::MAX_RELEASE_MS - Config::MIN_RELEASE_MS);
        float releaseCoeff = expf(-1000.0f / (releaseMs * Config::SAMPLE_RATE));

        for (uint16_t i = 0; i < length; ++i)
        {
            output[i] = noiseGateState.process(input[i], thresholdAmp, holdSamples,
                                            attackCoeff, releaseCoeff);
        }
    }

    void applyCompressor(float *input, float *output, uint16_t length,
                        float threshold, float ratio, float makeupGain, 
                        float attack, float release)
    {
        threshold  = clamp(threshold, 0.0f, 1.0f);
        ratio      = clamp(ratio, 0.0f, 1.0f);
        makeupGain = clamp(makeupGain, 0.0f, 1.0f);
        attack     = clamp(attack, 0.0f, 1.0f);
        release    = clamp(release, 0.0f, 1.0f);

        float thresholdLin = 0.001f + threshold * 0.3f;
        float compression = ratio * 2.5f;
        float makeup = 1.0f + makeupGain * 8.0f;
        
        float attackMs = Config::MIN_ATTACK_MS + attack * (Config::MAX_ATTACK_MS - Config::MIN_ATTACK_MS);
        float releaseMs = Config::MIN_RELEASE_MS + release * (Config::MAX_RELEASE_MS - Config::MIN_RELEASE_MS);
        float attackCoeff = expf(-1000.0f / (attackMs * Config::SAMPLE_RATE));
        float releaseCoeff = expf(-1000.0f / (releaseMs * Config::SAMPLE_RATE));

        for (uint16_t i = 0; i < length; ++i)
        {
            output[i] = compressorState.process(input[i], thresholdLin, compression, makeup,
                                            attackCoeff, releaseCoeff);
        }
    }
    
   
}