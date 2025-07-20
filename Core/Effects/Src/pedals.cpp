#include <pedals.hpp>

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