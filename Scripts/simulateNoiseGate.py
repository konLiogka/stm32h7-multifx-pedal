#!/usr/bin/env python3
"""
simulateNoiseGate.py
--------------------
Thin driver for the noise gate effect.
All matplotlib logic lives in plotAnalysis.py.
"""

import numpy as np
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
    effect = NoiseGateEffect()
    effect.reset()
    analyzer = NoiseGateAnalyzer(effect)
    analyzer.run()


if __name__ == "__main__":
    main()