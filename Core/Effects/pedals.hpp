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


template<typename T>
void setParams(T& params, float* values) {
    float* ptr = reinterpret_cast<float*>(&params);
    for (size_t i = 0; i < sizeof(T) / sizeof(float); ++i) {
        ptr[i] = values[i];
    }
}

template<typename T>
void getParams(const T& params, float* values) {
    const float* ptr = reinterpret_cast<const float*>(&params);
    for (size_t i = 0; i < sizeof(T) / sizeof(float); ++i) {
        values[i] = ptr[i];
    }
}

class Pedal {
public:
    Pedal(PedalType t)
        : type(t), 
          image(getBitmapForType(t)), 
          image_disabled(getDisabledBitmapForType(t)),
          name(getNameForType(t)) {}

    virtual ~Pedal() {}
    
    const Bitmap& getImage() const { return enabled ? image : image_disabled; }
    PedalType getType() const { return type; }
    const char* getName() const { return name; }
    virtual const char* const* getMemberNames() const { return nullptr; }
    virtual uint8_t getMemberSize() const { return 0; }
    
    virtual void set(float* values) = 0;
    virtual void get(float* values) const = 0;

    static Pedal* createPedal(PedalType type);
    virtual void process(float* input, float* output, uint16_t length) = 0;
    virtual bool isEnabled() const { return enabled; }
    virtual void setEnabled(bool state) { enabled = state; } 

protected:
    PedalType type;
    Bitmap image;
    Bitmap image_disabled;
    const char* name;
    bool enabled = true;

private:
    static const Bitmap& getBitmapForType(PedalType type);
    static const Bitmap& getDisabledBitmapForType(PedalType type);
    static const char* getNameForType(PedalType type);
};

class DistortionPedal : public Pedal {
public:
    DistortionPedal() : Pedal(PedalType::OVERDRIVE_DISTORTION) {
        params.volume = 0.5f;
        params.gain = 0.5f;
        params.tone = 0.2f;
        params.level = 0.6f;
    }

    struct {
        float volume;
        float gain;
        float tone;
        float level;
    } params;

    static constexpr const char* memberNames[] = {"Vol", "Gain", "Tone", "Level"};
    static constexpr uint8_t member_size = array_size(memberNames);

    const char* const* getMemberNames() const override { return memberNames; }
    uint8_t getMemberSize() const override { return member_size; }

    void set(float* values) override { setParams(params, values); }
    void get(float* values) const override { getParams(params, values); }

    void process(float* input, float* output, uint16_t length) override;
};

class EchoPedal : public Pedal {
public:
    EchoPedal() : Pedal(PedalType::ECHO) {
        params.volume = 1.0f;
        params.delayTime = 0.3f;
        params.feedback = 0.4f;
        params.mix = 0.5f;
    }
    
    struct {
        float volume;
        float delayTime;
        float feedback;
        float mix;
    } params;

    static constexpr const char* memberNames[] = {"Vol", "Delay", "Fdbck", "Mix"};
    static constexpr uint8_t member_size = array_size(memberNames);

    const char* const* getMemberNames() const override { return memberNames; }
    uint8_t getMemberSize() const override { return member_size; }

    void set(float* values) override { setParams(params, values); }
    void get(float* values) const override { getParams(params, values); }

    void process(float* input, float* output, uint16_t length) override;
};

// Reverb pedals
class ReverbPedal : public Pedal {
public:
    ReverbPedal() : Pedal(PedalType::REVERB) {
        params.volume = 1.0f;
        params.roomSize = 0.5f;
        params.damping = 0.5f;
        params.mix = 0.5f;
    }
    
    struct {
        float volume;
        float roomSize;
        float damping;
        float mix;
    } params;

    static constexpr const char* memberNames[] = {"Vol", "Depth", "Rate", "Mix"};
    static constexpr uint8_t member_size = array_size(memberNames);

    const char* const* getMemberNames() const override { return memberNames; }
    uint8_t getMemberSize() const override { return member_size; }
    
    void set(float* values) override { setParams(params, values); }
    void get(float* values) const override { getParams(params, values); }

    void process(float* input, float* output, uint16_t length) override;
};

class NoiseGatePedal : public Pedal {
public:
    NoiseGatePedal() : Pedal(PedalType::NOISE_GATE) {
        params.volume = 1.0f;
        params.threshold = 0.5f;
        params.attack = 0.5f;
        params.hold = 0.5f;
        params.release = 0.5f;
    }
    
    struct {
        float volume;
        float threshold;
        float attack;
        float hold;
        float release;
    } params;

    static constexpr const char* memberNames[] = {"Vol", "Trshld", "Atk", "Hold", "Rel"};
    static constexpr uint8_t member_size = array_size(memberNames);

    const char* const* getMemberNames() const override { return memberNames; }
    uint8_t getMemberSize() const override { return member_size; }

    void set(float* values) override { setParams(params, values); }
    void get(float* values) const override { getParams(params, values); }

    void process(float* input, float* output, uint16_t length) override;
};

class CompressorPedal : public Pedal {
public:
    CompressorPedal() : Pedal(PedalType::COMPRESSOR) {
        params.volume = 1.0f;
        params.threshold = 0.5f;
        params.ratio = 0.5f;
        params.makeupGain = 0.5f;
        params.attack = 0.5f;
        params.release = 0.5f;
    }
    
    struct {
        float volume;
        float threshold;
        float ratio;
        float makeupGain;
        float attack;
        float release;
    } params;

    static constexpr const char* memberNames[] = {"Vol", "Trshld", "Ratio","Gain", "Atk", "Rel"};
    static constexpr uint8_t member_size = array_size(memberNames);

    const char* const* getMemberNames() const override { return memberNames; }
    uint8_t getMemberSize() const override { return member_size; }

    void set(float* values) override { setParams(params, values); }
    void get(float* values) const override { getParams(params, values); }

    void process(float* input, float* output, uint16_t length) override;
};

class PassThroughPedal : public Pedal {
public:
    PassThroughPedal() : Pedal(PedalType::PASS_THROUGH) {
        params.highs = 1.0f;
        params.mids = 1.0f;
        params.lows = 1.0f;
        params.volume = 1.0f;
    }
    
    struct {
        float highs;
        float mids;
        float lows;
        float volume;
    } params;

    static constexpr const char* memberNames[] = {"Highs", "Mids", "Lows", "Vol"};
    static constexpr uint8_t member_size = array_size(memberNames);

    const char* const* getMemberNames() const override { return memberNames; }
    uint8_t getMemberSize() const override { return member_size; }
    
    void set(float* values) override { setParams(params, values); }
    void get(float* values) const override { getParams(params, values); }
    
    void process(float* input, float* output, uint16_t length) override;
};

 

#endif // PEDAL_HPP_
