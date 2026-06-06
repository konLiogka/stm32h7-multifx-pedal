#!/usr/bin/env python3


import numpy as np
import argparse
from ctypes import c_float, POINTER
import dsp_bindings as dsp

from plotanalysis import NoiseGateAnalyzer


class NoiseGateEffect:

    def __init__(self):
        self.lib        = dsp
        self.threshold  = 0.3
        self.attack     = 0.1
        self.hold       = 0.5
        self.release    = 0.1

    def set_params(self, threshold=None, attack=None,
                   hold=None, release=None):
        if threshold is not None: self.threshold = max(0.0, min(1.0, threshold))
        if attack is not None: self.attack = max(0.0, min(1.0, attack))
        if hold is not None: self.hold = max(0.0, min(1.0, hold))
        if release is not None: self.release = max(0.0, min(1.0, release))

    def reset(self):
        """Reset the internal state by processing silence with fastest settings"""
        # Store original params
        orig_thresh = self.threshold
        orig_attack = self.attack
        orig_hold = self.hold
        orig_release = self.release
        
        # Use fastest possible settings to reset
        self.threshold = 1.0  # Gate stays closed
        self.attack = 0.0
        self.hold = 0.0
        self.release = 0.0
        
        # Process enough silence to let envelope decay to zero
        # At 96kHz, 50ms = 4800 samples should be enough
        silence_samples = int(96000 * 0.05)  # 50ms at 96kHz
        silence = np.zeros(silence_samples, dtype=np.float32)
        self._process(silence)
        
        # Restore original params
        self.threshold = orig_thresh
        self.attack = orig_attack
        self.hold = orig_hold
        self.release = orig_release
        
        # One more quick process to ensure gate is ready with original settings
        silence = np.zeros(100, dtype=np.float32)
        self._process(silence)
    
    def _process(self, input_signal):
        """Internal processing method"""
        input_signal = np.asarray(input_signal, dtype=np.float32)
        output_signal = np.zeros(len(input_signal), dtype=np.float32)
        
        input_ptr = input_signal.ctypes.data_as(POINTER(c_float))
        output_ptr = output_signal.ctypes.data_as(POINTER(c_float))
        
        if hasattr(self.lib, "applyNoiseGate"):
            self.lib.applyNoiseGate(
                input_ptr, output_ptr, len(input_signal),
                self.threshold, self.attack, self.hold, self.release
            )
        
        return output_signal
    
    def process(self, input_signal):
        return self._process(input_signal)


def main():
    parser = argparse.ArgumentParser(description='Simulate noise gate effect on WAV file')
    parser.add_argument('--wav', type=str, help='Path to input WAV file')
    parser.add_argument('--threshold', type=float, default=0.3, help='Threshold parameter (0-1)')
    parser.add_argument('--attack', type=float, default=0.1, help='Attack parameter (0-1)')
    parser.add_argument('--hold', type=float, default=0.5, help='Hold parameter (0-1)')
    parser.add_argument('--release', type=float, default=0.1, help='Release parameter (0-1)')
    parser.add_argument('--duration', type=float, default=None, help='Duration in seconds to analyze (default: entire file)')
    parser.add_argument('--save', action='store_true', help='Save processed output to WAV file')
    parser.add_argument('--output', type=str, default='noisegate_output.wav', help='Path to save processed output WAV file')
    
    args = parser.parse_args()
    
    effect = NoiseGateEffect()
    effect.set_params(
        threshold=args.threshold,
        attack=args.attack,
        hold=args.hold,
        release=args.release
    )
    effect.reset()
    
    # Create analyzer with WAV file support
    analyzer = NoiseGateAnalyzer(effect, wav_file=args.wav, duration=args.duration)
    
    if args.save and args.wav:
        # Save the processed audio
        from plotanalysis import save_processed_audio
        save_processed_audio(analyzer.input_signal, analyzer.output_signal, 
                            analyzer.fs, args.output)
        print(f"Saved processed audio to: {args.output}")
    
    analyzer.run()


if __name__ == "__main__":
    main()