#include <pedals.hpp>
#include "display.hpp"

#pragma once
#ifndef EFFECTSCHAIN_HPP_
#define EFFECTSCHAIN_HPP_

class EffectsChain {
public:
    EffectsChain();
    ~EffectsChain();

    uint8_t selectedPedal = 0;
    
    void setPedal(uint8_t index, PedalType type);
    Pedal* getPedal(uint8_t index) const;
    void draw() const;
    void clear();

private:
    Pedal* pedals[4];
};



#endif /* EFFECTS_EFFECTSCHAIN_HPP_ */

