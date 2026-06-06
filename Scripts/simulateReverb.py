#!/usr/bin/env python3

import numpy as np
import argparse
from ctypes import c_float, POINTER
import dsp_bindings as dsp

from plotanalysis import ReverbAnalyzer


class ReverbEffect:

    def __init__(self):
        self.lib        = dsp
        self.roomSize   = 0.5
        self.damping    = 0.5
        self.mix        = 0.3

    def get_comb_delays(self):
        """Return comb filter delays from C++"""
        if hasattr(self.lib, "getCombDelays"):
            return self.lib.getCombDelays()
        # Fallback to typical Schroeder values
        return [3163, 3617, 4013, 4409, 4801, 5209, 5597, 6007]
    
    def get_comb_feedback(self):
        """Return feedback based on roomSize"""
        fb = self.roomSize * 0.9
        return [fb] * 8

    def set_params(self, roomSize=None, damping=None, mix=None):
        if roomSize is not None: self.roomSize = max(0.0, min(1.0, roomSize))
        if damping is not None: self.damping = max(0.0, min(1.0, damping))
        if mix is not None: self.mix = max(0.0, min(1.0, mix))

    def process(self, input_signal):
        input_signal = np.asarray(input_signal, dtype=np.float32)
        output_signal = np.zeros(len(input_signal), dtype=np.float32)

        input_ptr  = input_signal.ctypes.data_as(POINTER(c_float))
        output_ptr = output_signal.ctypes.data_as(POINTER(c_float))

        if hasattr(self.lib, "applyReverb"):
            self.lib.applyReverb(
                input_ptr, output_ptr, len(input_signal),
                self.roomSize, self.damping, self.mix
            )
        return output_signal


def main():
    parser = argparse.ArgumentParser(description='Simulate reverb effect on WAV file')
    parser.add_argument('--wav', type=str, help='Path to input WAV file')
    parser.add_argument('--roomSize', type=float, default=0.5, help='Room size parameter (0-1)')
    parser.add_argument('--damping', type=float, default=0.5, help='Damping parameter (0-1)')
    parser.add_argument('--mix', type=float, default=0.3, help='Mix parameter (0-1)')
    parser.add_argument('--duration', type=float, default=None, help='Duration in seconds to analyze (default: entire file)')
    parser.add_argument('--save', action='store_true', help='Save processed output to WAV file')
    parser.add_argument('--output', type=str, default='reverb_output.wav', help='Path to save processed output WAV file')
    
    args = parser.parse_args()
    
    effect = ReverbEffect()
    effect.set_params(
        roomSize=args.roomSize,
        damping=args.damping,
        mix=args.mix
    )
    
    # Create analyzer with WAV file support
    analyzer = ReverbAnalyzer(effect, wav_file=args.wav, duration=args.duration)
    
    if args.save and args.wav:
        # Save the processed audio
        from plotanalysis import save_processed_audio
        save_processed_audio(analyzer.input_signal, analyzer.output_signal, 
                            analyzer.fs, args.output)
        print(f"Saved processed audio to: {args.output}")
    
    analyzer.run()


if __name__ == "__main__":
    main()