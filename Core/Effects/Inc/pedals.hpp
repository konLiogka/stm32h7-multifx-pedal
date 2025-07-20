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
    
    // Volume and tone controls
    float volume;
    float highs;
    float mids;
    float lows;

    // For distortion pedals
    float gain;
    float tone;
    float level;

    // For delay-based pedals
    float delayTime;
    float feedback;
    float mix;

    // For modulator pedals
    float depth;
    float rate;
    float resonance;

    // For dynamic/filter/compression/range control pedals
    float cutoffFrequency;
    float threshold;
    float ratio;
    float attack;
    float release;

    const Bitmap& getImage() const { return image; }
    PedalType getType() const { return type; }
    const char* getName() const { return name; }


protected:
    PedalType type;
    Bitmap image;
    const char* name;

private:
    static const Bitmap& getBitmapForType(PedalType type);
    static const char* getNameForType(PedalType type);

};

 

#endif // PEDAL_HPP_
