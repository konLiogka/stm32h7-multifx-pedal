#include "bitmaps.hpp"

#pragma once 
#ifndef PEDAL_HPP_
#define PEDAL_HPP_

enum class PedalType {
	OVERDRIVE_DISTORTION,
    ECHO,
    REVERB,
    PASS_THROUGH
};

class Pedal {
public:
    Pedal(PedalType t)
        : type(t), image(getBitmapForType(t)), name(getNameForType(t)) {}

    virtual ~Pedal() {}
    
    float volume = 0.5f;

    const Bitmap& getImage() const { return image; }
    PedalType getType() const { return type; }
    const char* getName() const { return name; }
    virtual const char* const* getMemberNames() const { return nullptr; }
    virtual uint8_t getMemberSize() const { return 0; }
    virtual void setParams(float* params) { volume = params[0]; }
    virtual void getParams(float* params) const { params[0] = volume; }
    virtual float getParamValue(uint8_t index) const { return (index == 0) ? volume : 0.0f; }

    static Pedal* createPedal(PedalType type);

protected:
    PedalType type;
    Bitmap image;
    const char* name;

private:
    static const Bitmap& getBitmapForType(PedalType type);
    static const char* getNameForType(PedalType type);
};

// Distortion/Overdrive pedals
class DistortionPedal : public Pedal {
public:
    DistortionPedal() : Pedal(PedalType::OVERDRIVE_DISTORTION) {
        volume = 0.1f;
        gain = 0.1f;
        tone = 0.2f;
        level = 0.3f;
        crunch = 0.4f;
    }

    float gain;
    float tone;
    float level;
    float crunch;

    const uint8_t member_size = 5; 
    static constexpr const char* memberNames[5] = {"Vol", "Gain", "Tone", "Level", "Crnch"};
    const char* const* getMemberNames() const override { return memberNames; }
    uint8_t getMemberSize() const override { return member_size; }

    void setParams(float* params) override;
    void getParams(float* params) const override;
};

// Echo/Delay pedals
class EchoPedal : public Pedal {
public:
    EchoPedal() : Pedal(PedalType::ECHO) {
        volume = 0.5f;
        delayTime = 0.5f;
        feedback = 0.5f;
        mix = 0.5f;
    }
    
    float delayTime;
    float feedback;
    float mix;
    const uint8_t member_size = 4; 
    static constexpr const char* memberNames[4] = {"Vol", "Delay", "Fdbck", "Mix"};
    const char* const* getMemberNames() const override { return memberNames; }
    uint8_t getMemberSize() const override { return member_size; }

    void setParams(float* params) override;
    void getParams(float* params) const override;
};

// Reverb pedals
class ReverbPedal : public Pedal {
public:
    ReverbPedal() : Pedal(PedalType::REVERB) {
        volume = 0.5f;
        depth = 0.5f;
        rate = 0.5f;
        mix = 0.5f;
    }
    float depth;
    float rate;
    float mix;
    const uint8_t member_size = 4; 
    static constexpr const char* memberNames[4] = {"Vol", "Depth", "Rate", "Mix"};
    const char* const* getMemberNames() const override { return memberNames; }
    uint8_t getMemberSize() const override { return member_size; }
    
    void setParams(float* params) override;
    void getParams(float* params) const override;
};

// Pass-through pedals (EQ-style controls)
class PassThroughPedal : public Pedal {
public:
    PassThroughPedal() : Pedal(PedalType::PASS_THROUGH) {
    }
    
    float highs;
    float mids;
    float lows;
    const uint8_t member_size = 4; 
    static constexpr const char* memberNames[4] = {"Vol", "Highs", "Mids", "Lows"};
    const char* const* getMemberNames() const override { return memberNames; }
    uint8_t getMemberSize() const override { return member_size; }
    
    void setParams(float* params) override ;
    void getParams(float* params) const override ;
};

 

#endif // PEDAL_HPP_
