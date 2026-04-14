#!/usr/bin/env python3
"""
simulateCompression.py
----------------------
Thin driver for the compressor effect.
All matplotlib logic lives in plotAnalysis.py.
"""

import numpy as np
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
    effect   = CompressorEffect()
    analyzer = CompressorAnalyzer(effect)
    analyzer.run()


if __name__ == "__main__":
    main()