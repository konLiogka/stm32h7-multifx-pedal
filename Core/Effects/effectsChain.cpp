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

void EffectsChain::process(uint16_t* input, uint16_t* output, uint16_t startIdx, uint16_t length)
{
    static float buffer_a[1024];
    static float buffer_b[1024];
    
    for (uint16_t i = 0; i < length; i++)
    {
        buffer_a[i] = ((float)input[startIdx + i] - 2048.0f) / 2048.0f;
    }
    
    float* in_buf = buffer_a;
    float* out_buf = buffer_b;
    
    for (uint8_t i = 0; i < 4; i++)
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
    
    for (uint16_t i = 0; i < length; i++)
    {
        float sample = in_buf[i];
        
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        
        int32_t value = (int32_t)(sample * 2048.0f) + 2048;
        if (value > 4095) value = 4095;
        if (value < 0) value = 0;
        
        output[startIdx + i] = (uint16_t)value;
    }
}
