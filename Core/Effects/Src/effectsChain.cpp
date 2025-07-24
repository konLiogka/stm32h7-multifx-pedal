#include "effectsChain.hpp"
#include "display.hpp"

EffectsChain::EffectsChain() {
    pedals[0] = new Pedal(PedalType::PASS_THROUGH);
    pedals[1] = new Pedal(PedalType::PASS_THROUGH);
    pedals[2] = new Pedal(PedalType::PASS_THROUGH);
    pedals[3] = new Pedal(PedalType::PASS_THROUGH);
}

EffectsChain::~EffectsChain() {
    for (int i = 0; i < 4; ++i) {
        delete pedals[i];
    }
}

void EffectsChain::setPedal(int index, PedalType type) {
    if (index < 0 || index >= 4) return;

    delete pedals[index]; 
    pedals[index] = new Pedal(type);

    // Set default values for each pedal type
    switch (type) {
        case PedalType::OVERDRIVE_DISTORTION:
            pedals[index]->volume = 0.5f;
            pedals[index]->gain = 0.7f;
            pedals[index]->tone = 0.5f;
            pedals[index]->level = 0.5f;
            break;
        case PedalType::ECHO:
            pedals[index]->volume = 0.5f;       
            pedals[index]->delayTime = 0.5f;
            pedals[index]->feedback = 0.5f;
            pedals[index]->mix = 0.5f;  
            break;
        case PedalType::REVERB:
            pedals[index]->volume = 0.5f;   
            pedals[index]->depth = 0.5f;
            pedals[index]->rate = 0.5f;
            pedals[index]->mix = 0.5f;
            break;
        case PedalType::PASS_THROUGH:
            pedals[index]->volume = 1.0f; // Full volume for pass-through
            pedals[index]->highs = 0.5f;
            pedals[index]->mids = 0.5f;
            pedals[index]->lows = 0.5f;
            break;
    }
}

Pedal* EffectsChain::getPedal(int index) const {
    return (index >= 0 && index < 4) ? pedals[index] : nullptr;
}

void EffectsChain::draw() const {
    for (size_t i = 0; i < 4; ++i) {
        if (pedals[i]) {
            const Bitmap& bmp = pedals[i]->getImage();
            Display::drawBitmap(bmp, pedalPositionsX[i], PEDAL_PAGE_START);
        }
    }
}
