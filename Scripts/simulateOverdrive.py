#!/usr/bin/env python3
"""
simulateOverdrive.py
--------------------
Thin driver for the overdrive/distortion effect.
All matplotlib logic lives in plotAnalysis.py.
"""

import numpy as np
from ctypes import c_float, POINTER
import dsp_bindings as dsp

from plotanalysis import OverdriveAnalyzer


class OverdriveEffect:

    def __init__(self):
        self.lib    = dsp
        self.gain   = 0.1
        self.tone   = 0.1
        self.level  = 0.1

    def set_params(self, gain=None, tone=None, level=None):
        if gain   is not None: self.gain   = max(0.0, min(1.0, gain))
        if tone   is not None: self.tone   = max(0.0, min(1.0, tone))
        if level  is not None: self.level  = max(0.0, min(1.0, level))

    def process(self, input_signal):
        input_signal  = np.asarray(input_signal, dtype=np.float32)
        output_signal = np.zeros(len(input_signal), dtype=np.float32)

        input_ptr  = input_signal.ctypes.data_as(POINTER(c_float))
        output_ptr = output_signal.ctypes.data_as(POINTER(c_float))

        if hasattr(self.lib, "applyOverdrive"):
            self.lib.applyOverdrive(
                input_ptr, output_ptr, len(input_signal),
                self.gain, self.tone, self.level,
            )
        return output_signal


def main():
    effect = OverdriveEffect()
    effect.set_params(gain=0.3, tone=0.1, level=0.3)

    analyzer = OverdriveAnalyzer(effect)
    analyzer.run()


if __name__ == "__main__":
    main()