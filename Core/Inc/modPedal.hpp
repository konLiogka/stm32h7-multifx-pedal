#include "display.hpp"

#ifndef IN_MODPEDAL_HPP_
#define IN_MODPEDAL_HPP_

void displayPedalSettings(Pedal *pedal, uint8_t page);

void changePedalSettings(Pedal *pedal, uint8_t page, uint32_t potValues[3]);

#endif