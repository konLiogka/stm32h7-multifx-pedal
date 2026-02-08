#include "effectsChain.hpp"
#include "display.hpp"
#include <array>

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

void EffectsChain::process(uint16_t* input, uint16_t* output, uint16_t startIdx, uint16_t length)
{
    static std::array<float, 1024> buffer_a;
    static std::array<float, 1024> buffer_b;
    
    for (uint16_t i = 0; i < length; ++i)
    {
        buffer_a[i] = (static_cast<float>(input[startIdx + i]) - 2048.0f) / 2048.0f;
    }
    
    float* in_buf = buffer_a.data();
    float* out_buf = buffer_b.data();
    
    for (uint8_t i = 0; i < 4; ++i)
    {
        if (pedals[i] != nullptr && pedals[i]->isEnabled())
        {
            pedals[i]->process(in_buf, out_buf, length);
            
            float* temp = in_buf;
            in_buf = out_buf;
            out_buf = temp;
        }
        else
        {
            if (in_buf != out_buf)
            {
                memcpy(out_buf, in_buf, length * sizeof(float));
                float* temp = in_buf;
                in_buf = out_buf;
                out_buf = temp;
            }
        }
    }
    
    for (uint16_t i = 0; i < length; ++i)
    {
        const float sample = clamp(in_buf[i], -1.0f, 1.0f);
        const int32_t value = clamp(
            static_cast<int32_t>(sample * 2048.0f) + 2048,
            0, 4095
        );
        output[startIdx + i] = static_cast<uint16_t>(value);
    }
}