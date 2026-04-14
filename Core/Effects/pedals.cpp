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
        case PedalType::NOISE_GATE:
            return noise_gate_bitmap;
        case PedalType::COMPRESSOR:
            return compressor_bitmap;
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
        case PedalType::NOISE_GATE:
            return noise_gate_disabled_bitmap;  
        case PedalType::COMPRESSOR:
            return compressor_disabled_bitmap; 
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
        case PedalType::NOISE_GATE:
            return "Noise Gate";
        case PedalType::COMPRESSOR:
            return "Compressor";
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
        case PedalType::NOISE_GATE:
            return new NoiseGatePedal();
        case PedalType::COMPRESSOR:
            return new CompressorPedal();
        case PedalType::PASS_THROUGH:
        default:
            return new PassThroughPedal();
    }
}

void Pedal::process(float* input, float* output, uint16_t length)
{
    memcpy(output, input, length * sizeof(float));
}

void DistortionPedal::process(float* input, float* output, uint16_t length)
{
    DSP::applyOverdrive(input, output, length, 
                        params.gain, params.tone, params.level);
    
    for (uint16_t i = 0; i < length; i++)
    {
        output[i] *= params.volume;
    }
}

void EchoPedal::process(float* input, float* output, uint16_t length)
{
    DSP::applyEcho(input, output, length, 
                   params.delayTime, params.feedback, params.mix);
    
    for (uint16_t i = 0; i < length; i++)
    {
        output[i] *= params.volume;
    }
}

void ReverbPedal::process(float* input, float* output, uint16_t length)
{
    DSP::applyReverb(input, output, length, 
                     params.roomSize, params.damping, params.mix);
    
    for (uint16_t i = 0; i < length; i++)
    {
        output[i] *= params.volume;
    }
}

void NoiseGatePedal::process(float* input, float* output, uint16_t length)
{
    DSP::applyNoiseGate(input, output, length, 
                        params.threshold, params.attack, params.hold, params.release);
    
    for (uint16_t i = 0; i < length; i++)
    {
        output[i] *= params.volume;
    }
}

void CompressorPedal::process(float* input, float* output, uint16_t length)
{
    DSP::applyCompressor(input, output, length, 
                         params.threshold, params.ratio, params.makeupGain, params.attack, params.release);
    
    for (uint16_t i = 0; i < length; i++)
    {
        output[i] *= params.volume;
    }
}

void PassThroughPedal::process(float* input, float* output, uint16_t length)
{
    memcpy(output, input, length * sizeof(float));
    
    for (uint16_t i = 0; i < length; i++)
    {
        output[i] *= params.volume;
    }
}