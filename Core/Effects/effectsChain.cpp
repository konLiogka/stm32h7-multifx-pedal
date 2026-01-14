#include "effectsChain.hpp"
#include "display.hpp"

EffectsChain::EffectsChain() {
    pedals[0] = Pedal::createPedal(PedalType::PASS_THROUGH);
    pedals[1] = Pedal::createPedal(PedalType::PASS_THROUGH);
    pedals[2] = Pedal::createPedal(PedalType::PASS_THROUGH);
    pedals[3] = Pedal::createPedal(PedalType::PASS_THROUGH);
}

EffectsChain::~EffectsChain() {
    for (uint8_t i = 0; i < 4; ++i) {
        delete pedals[i];
    }
}

void EffectsChain::clear() {
    for (uint8_t i = 0; i < 4; ++i) {
        delete pedals[i];
        pedals[i] = Pedal::createPedal(PedalType::PASS_THROUGH);
    }
}

void EffectsChain::setPedal(uint8_t index, PedalType type) {
    if (index < 0 || index >= 4) return;

    delete pedals[index]; 
    pedals[index] = Pedal::createPedal(type);
}

Pedal* EffectsChain::getPedal(uint8_t index) const {
    return (index >= 0 && index < 4) ? pedals[index] : nullptr;
}

void EffectsChain::draw() const {
    for (uint8_t i = 0; i < 4; ++i) {
        if (pedals[i]) {
            const Bitmap& bmp = pedals[i]->getImage();
            Display::drawBitmap(bmp, pedalPositionsX[i], PEDAL_PAGE_START);
        }
    }
}
