#!/usr/bin/env python3
"""
simulateReverb.py
-----------------
Thin driver for the reverb effect.
All matplotlib logic lives in plotAnalysis.py.
"""

import numpy as np
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
    effect   = ReverbEffect()
    analyzer = ReverbAnalyzer(effect)
    analyzer.run()


if __name__ == "__main__":
    main()