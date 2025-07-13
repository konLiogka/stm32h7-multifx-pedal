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

    ~Pedal() {}

    const Bitmap& getImage() const { return image; }
    PedalType getType() const { return type; }

protected:
    PedalType type;
    Bitmap image;

private:
    static const Bitmap& getBitmapForType(PedalType type);
};



#endif // PEDAL_HPP_
