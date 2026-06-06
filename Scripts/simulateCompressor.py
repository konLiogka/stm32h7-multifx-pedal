#!/usr/bin/env python3

import numpy as np
import argparse
from ctypes import c_float, POINTER
import dsp_bindings as dsp

from plotanalysis import CompressorAnalyzer


class CompressorEffect:

    def __init__(self):
        self.lib        = dsp
        self.threshold  = 0.2
        self.ratio      = 0.1
        self.makeup_gain = 0.0
        self.attack     = 0.1
        self.release    = 0.1

    def set_params(self, threshold=None, ratio=None, makeup_gain=None,
                   attack=None, release=None):
        if threshold   is not None: self.threshold   = max(0.0, min(1.0, threshold))
        if ratio       is not None: self.ratio       = max(0.0, min(1.0, ratio))
        if makeup_gain is not None: self.makeup_gain = max(0.0, min(1.0, makeup_gain))
        if attack      is not None: self.attack      = max(0.0, min(1.0, attack))
        if release     is not None: self.release     = max(0.0, min(1.0, release))

    def process(self, input_signal):
        input_signal = np.asarray(input_signal, dtype=np.float32)
        output_signal = np.zeros(len(input_signal), dtype=np.float32)

        input_ptr  = input_signal.ctypes.data_as(POINTER(c_float))
        output_ptr = output_signal.ctypes.data_as(POINTER(c_float))

        if hasattr(self.lib, "applyCompressor"):
            self.lib.applyCompressor(
                input_ptr, output_ptr, len(input_signal),
                self.threshold, self.ratio, self.makeup_gain,
                self.attack, self.release,
            )
        return output_signal


def main():
    parser = argparse.ArgumentParser(description='Simulate compressor effect on WAV file')
    parser.add_argument('--wav', type=str, help='Path to input WAV file')
    parser.add_argument('--threshold', type=float, default=0.2, help='Threshold parameter (0-1)')
    parser.add_argument('--ratio', type=float, default=0.1, help='Ratio parameter (0-1)')
    parser.add_argument('--makeup_gain', type=float, default=0.0, help='Makeup gain parameter (0-1)')
    parser.add_argument('--attack', type=float, default=0.1, help='Attack parameter (0-1)')
    parser.add_argument('--release', type=float, default=0.1, help='Release parameter (0-1)')
    parser.add_argument('--duration', type=float, default=None, help='Duration in seconds to analyze (default: entire file)')
    parser.add_argument('--save', action='store_true', help='Save processed output to WAV file')
    parser.add_argument('--output', type=str, default='compressed_output.wav', help='Path to save processed output WAV file')
    
    args = parser.parse_args()
    
    effect = CompressorEffect()
    effect.set_params(
        threshold=args.threshold,
        ratio=args.ratio,
        makeup_gain=args.makeup_gain,
        attack=args.attack,
        release=args.release
    )
    
    # Create analyzer with WAV file support
    analyzer = CompressorAnalyzer(effect, wav_file=args.wav, duration=args.duration)
    
    if args.save and args.wav:
        # Save the processed audio
        from plotanalysis import save_processed_audio
        save_processed_audio(analyzer.input_signal, analyzer.output_signal, 
                            analyzer.fs, args.output)
        print(f"Saved processed audio to: {args.output}")
    
    analyzer.run()


if __name__ == "__main__":
    main()