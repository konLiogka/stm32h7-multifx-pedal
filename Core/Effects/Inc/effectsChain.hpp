#include <pedals.hpp>
#include "display.hpp"

#pragma once
#ifndef EFFECTSCHAIN_HPP_
#define EFFECTSCHAIN_HPP_

class EffectsChain {
public:
    EffectsChain();
    ~EffectsChain();

    uint8_t selectedPedal;
    
    void setPedal(int index, PedalType type);
    Pedal* getPedal(int index) const;
    void draw() const;

private:
    Pedal* pedals[4];
};



#endif /* EFFECTS_EFFECTSCHAIN_HPP_ */

