#include <pedals.hpp>
#include <dsp.hpp>

const Bitmap& Pedal::getBitmapForType(PedalType type) {
    switch (type) {
        case PedalType::OVERDRIVE_DISTORTION:
            return overdrive_distortion_bitmap;
        case PedalType::ECHO:
            return echo_bitmap;
        case PedalType::REVERB:
            return reverb_bitmap;
        default:
            return pass_through_bitmap; 
    }
}

const Bitmap& Pedal::getDisabledBitmapForType(PedalType type) {
    switch (type) {
        case PedalType::OVERDRIVE_DISTORTION:
            return overdrive_distortion_disabled_bitmap;  
        case PedalType::ECHO:
            return echo_disabled_bitmap;                  
        case PedalType::REVERB:
            return reverb_disabled_bitmap;             
        default:
            return pass_through_bitmap;    
}
}

const char* Pedal::getNameForType(PedalType type) {
    switch (type) {
        case PedalType::OVERDRIVE_DISTORTION:
            return "Overdrive Distortion";
        case PedalType::ECHO:
            return "Echo";
        case PedalType::REVERB:
            return "Reverb";
        default:
            return "Pass Through"; 
    }
}

Pedal* Pedal::createPedal(PedalType type) {
    switch (type) {
        case PedalType::OVERDRIVE_DISTORTION:
            return new DistortionPedal();
        case PedalType::ECHO:
            return new EchoPedal();
        case PedalType::REVERB:
            return new ReverbPedal();
        case PedalType::PASS_THROUGH:
        default:
            return new PassThroughPedal();
    }
}

void DistortionPedal::setParams(float* params) {
    volume = params[0];
    gain   = params[1];
    tone   = params[2];
    level  = params[3];
}

void DistortionPedal::getParams(float* params) const {
    params[0] = volume;    
    params[1] = gain;     
    params[2] = tone;     
    params[3] = level;     
}

void EchoPedal::setParams(float* params) {
    volume    = params[0];
    delayTime = params[1];
    feedback  = params[2];
    mix       = params[3];
    mod       = params[4];

}

void EchoPedal::getParams(float* params) const {
    params[0] = volume;   
    params[1] = delayTime;  
    params[2] = feedback;  
    params[3] = mix;       
    params[4] = mod;            
}

void ReverbPedal::setParams(float* params) {
    volume = params[0];
    depth  = params[1];
    rate   = params[2];
    mix    = params[3];
}

void ReverbPedal::getParams(float* params) const {
    params[0] = volume; 
    params[1] = depth;  
    params[2] = rate; 
    params[3] = mix;   
}

void PassThroughPedal::setParams(float* params) {
    highs  = params[0];
    mids   = params[1];
    lows   = params[2]; 
    volume = params[3];
}

void PassThroughPedal::getParams(float* params) const {
    params[0] = highs;   
    params[1] = mids;   
    params[2] = lows;  
    params[3] = volume; 
}

void Pedal::process(float* input, float* output, uint16_t length)
{
    memcpy(output, input, length * sizeof(float));
}

void DistortionPedal::process(float* input, float* output, uint16_t length)
{
    DSP::applyOverdrive(input, output, length, gain, tone, level);
    
    for (uint16_t i = 0; i < length; i++)
    {
        output[i] *= volume;
    }
}

void EchoPedal::process(float* input, float* output, uint16_t length)
{
    DSP::applyEcho(input, output, length,
                delayTime,  feedback,  mix,
                 mod);
    
    for (uint16_t i = 0; i < length; i++)
    {
        output[i] *= volume;
    }
}

void ReverbPedal::process(float* input, float* output, uint16_t length)
{
    DSP::applyReverb(input, output, length, depth, rate, mix);
    
    for (uint16_t i = 0; i < length; i++)
    {
        output[i] *= volume;
    }
}

void PassThroughPedal::process(float* input, float* output, uint16_t length)
{
    memcpy(output, input, length * sizeof(float));
}