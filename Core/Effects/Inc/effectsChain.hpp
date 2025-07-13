#include <array>
#include <memory>
#include <cstring>
#include "PedalType.hpp"
#include "display.hpp"

#ifndef EFFECTS_EFFECTSCHAIN_HPP_
#define EFFECTS_EFFECTSCHAIN_HPP_


#pragma once
class EffectsChain {
public:
    EffectsChain();
    ~EffectsChain();
    
    void setPedal(int index, PedalType type);
    Pedal* getPedal(int index) const;
    void draw() const;



private:
    Pedal* pedals[4];
};



#endif /* EFFECTS_EFFECTSCHAIN_HPP_ */

