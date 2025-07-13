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
        : type(t), image(getBitmapForType(t)) {}

    virtual ~Pedal() {}
    
    // Volume and tone controls
    float volume;
    float highs;
    float lows;

    // For distortion pedals
    float gain;
    float tone;
    float level;

    // For delay-based pedals
    float delayTime;
    float feedback;
    float mix;

    // For filter pedals
    float cutoffFrequency;
    float resonance;

    // For modulator pedals
    float depth;
    float rate;

    // For dynamic range control pedals
    float threshold;
    float ratio;
    float attack;
    float release;

    const Bitmap& getImage() const { return image; }
    PedalType getType() const { return type; }

protected:
    PedalType type;
    Bitmap image;

private:
    static const Bitmap& getBitmapForType(PedalType type);
};

 

#endif // PEDAL_HPP_
