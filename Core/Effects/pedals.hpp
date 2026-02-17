#include "bitmaps.hpp"
#include "common.hpp"

#include <cstring>

#pragma once 
#ifndef PEDAL_HPP_
#define PEDAL_HPP_

enum class PedalType {
	OVERDRIVE_DISTORTION,
    ECHO,
    REVERB,
    NOISE_GATE,
    COMPRESSOR,
    PASS_THROUGH
};

constexpr uint8_t pedalType_size = static_cast<size_t>(PedalType::PASS_THROUGH) + 1;

class Pedal {
public:
    Pedal(PedalType t)
        : type(t), 
          image(getBitmapForType(t)), 
          disabled_image(getDisabledBitmapForType(t)),
          name(getNameForType(t)) {}

    virtual ~Pedal() {}
    
    float volume = 0.5f;

    const Bitmap& getImage() const { return enabled ? image : disabled_image; }
    PedalType getType() const { return type; }
    const char* getName() const { return name; }
    virtual const char* const* getMemberNames() const { return nullptr; }
    virtual uint8_t getMemberSize() const { return 0; }
    virtual void setParams(float* params) { volume = params[0]; }
    virtual void getParams(float* params) const { params[0] = volume; }
    virtual float getParamValue(uint8_t index) const { return (index == 0) ? volume : 0.0f; }

    static Pedal* createPedal(PedalType type);
    virtual void process(float* input, float* output, uint16_t length);
    virtual bool isEnabled() const { return enabled; }
    virtual void setEnabled(bool state) { enabled = state; } 

protected:
    PedalType type;
    Bitmap image;
    Bitmap disabled_image;
    const char* name;
    bool enabled = true;

private:
    static const Bitmap& getBitmapForType(PedalType type);
    static const Bitmap& getDisabledBitmapForType(PedalType type) ;
    static const char* getNameForType(PedalType type);
};

// Distortion/Overdrive pedals
class DistortionPedal : public Pedal {
public:
    DistortionPedal() : Pedal(PedalType::OVERDRIVE_DISTORTION) {
        volume = 0.5f;
        gain = 0.5f;
        tone = 0.2f;
        level = 0.6f;
    }

    float gain;
    float tone;
    float level;

    static constexpr const char* memberNames[] = {"Vol", "Gain", "Tone", "Level"};
    static constexpr uint8_t member_size = array_size(memberNames);

    const char* const* getMemberNames() const override { return memberNames; }
    uint8_t getMemberSize() const override { return member_size; }

    void setParams(float* params) override;
    void getParams(float* params) const override;

    void process(float* input, float* output, uint16_t length) override;
};

// Echo/Delay pedals
class EchoPedal : public Pedal {
public:
    EchoPedal() : Pedal(PedalType::ECHO) {
        volume = 1.0f;
        delayTime = 0.5f;
        feedback = 0.5f;
        mix = 0.5f;
    }
    
    float delayTime;
    float feedback;
    float mix;

    static constexpr const char* memberNames[] = {"Vol", "Delay", "Fdbck", "Mix"};
    static constexpr uint8_t member_size = array_size(memberNames);

    const char* const* getMemberNames() const override { return memberNames; }
    uint8_t getMemberSize() const override { return member_size; }

    void setParams(float* params) override;
    void getParams(float* params) const override;

    void process(float* input, float* output, uint16_t length) override;
};

// Reverb pedals
class ReverbPedal : public Pedal {
public:
    ReverbPedal() : Pedal(PedalType::REVERB) {
        volume = 1.0f;
        depth  = 0.5f;
        rate   = 0.5f;
        mix    = 0.5f;
    }
    float depth;
    float rate;
    float mix;

    static constexpr const char* memberNames[] = {"Vol", "Depth", "Rate", "Mix"};
    static constexpr uint8_t member_size = array_size(memberNames);

    const char* const* getMemberNames() const override { return memberNames; }
    uint8_t getMemberSize() const override { return member_size; }
    
    void setParams(float* params) override;
    void getParams(float* params) const override;

    void process(float* input, float* output, uint16_t length) override;
};

class NoiseGatePedal : public Pedal {
public:
    NoiseGatePedal() : Pedal(PedalType::NOISE_GATE) {
        volume    = 1.0f;
        threshold = 0.5f;
        hold      = 0.5f;
        release   = 0.5f;
    }
    
    float threshold;
    float hold;
    float release;

    static constexpr const char* memberNames[] = {"Vol", "Trshld", "Hold", "Rel" };
    static constexpr uint8_t member_size = array_size(memberNames);

    const char* const* getMemberNames() const override { return memberNames; }
    uint8_t getMemberSize() const override { return member_size; }

    void setParams(float* params) override;
    void getParams(float* params) const override;

    void process(float* input, float* output, uint16_t length) override;
};

class CompressorPedal : public Pedal {
public:
    CompressorPedal() : Pedal(PedalType::COMPRESSOR) {
        volume    = 1.0f;
        threshold = 0.5f;
        ratio      = 0.5f;
        makeupGain   = 0.5f;
    }
    
    float threshold;
    float ratio;
    float makeupGain;

    static constexpr const char* memberNames[] = {"Vol", "Trshld", "Ratio", "Gain" };
    static constexpr uint8_t member_size = array_size(memberNames);

    const char* const* getMemberNames() const override { return memberNames; }
    uint8_t getMemberSize() const override { return member_size; }

    void setParams(float* params) override;
    void getParams(float* params) const override;

    void process(float* input, float* output, uint16_t length) override;
};

class PassThroughPedal : public Pedal {
public:
    PassThroughPedal() : Pedal(PedalType::PASS_THROUGH) {
        highs  = 1.0f;
        lows   = 1.0f;
        mids   = 1.0f;
        volume = 1.0f;
    }
    
    float highs;
    float mids;
    float lows;

    static constexpr const char* memberNames[] = { "Highs", "Mids", "Lows", "Vol"};
    static constexpr uint8_t member_size = array_size(memberNames);

    const char* const* getMemberNames() const override { return memberNames; }
    uint8_t getMemberSize() const override { return member_size; }
    
    void setParams(float* params) override ;
    void getParams(float* params) const override ;
    
    void process(float* input, float* output, uint16_t length) override;
};

 

#endif // PEDAL_HPP_
