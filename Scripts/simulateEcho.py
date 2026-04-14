#!/usr/bin/env python3
"""
simulateEcho.py
---------------
Thin driver for the echo effect.
All matplotlib logic lives in plotAnalysis.py.
"""
import numpy as np
from ctypes import c_float, POINTER
import dsp_bindings as dsp
from plotanalysis import EchoAnalyzer

class EchoEffect:
    def __init__(self):
        self.lib       = dsp
        self.delayTime = 0.3
        self.feedback  = 0.5
        self.mix       = 0.5
    
    def get_delay_samples(self, fs=48000):
        """Return delay in samples based on delayTime parameter"""
        min_delay_ms = 20.0
        max_delay_ms = 680.0
        delay_ms = min_delay_ms + self.delayTime * (max_delay_ms - min_delay_ms)
        return int((delay_ms / 1000.0) * fs)
    
    def get_delay_time_ms(self):
        """Return delay time in milliseconds"""
        min_delay_ms = 20.0
        max_delay_ms = 680.0
        return min_delay_ms + self.delayTime * (max_delay_ms - min_delay_ms)
    
    def set_params(self, delayTime=None, feedback=None, mix=None):
        if delayTime is not None: self.delayTime = max(0.0, min(1.0, delayTime))
        if feedback is not None: self.feedback = max(0.0, min(1.0, feedback))
        if mix is not None: self.mix = max(0.0, min(1.0, mix))
    
    def process(self, input_signal):
        input_signal = np.asarray(input_signal, dtype=np.float32)
        output_signal = np.zeros(len(input_signal), dtype=np.float32)
        
        input_ptr  = input_signal.ctypes.data_as(POINTER(c_float))
        output_ptr = output_signal.ctypes.data_as(POINTER(c_float))
        
        if hasattr(self.lib, "applyEcho"):
            self.lib.applyEcho(
                input_ptr, output_ptr, len(input_signal),
                self.delayTime, self.feedback, self.mix
            )
        
        return output_signal

def main():
    effect   = EchoEffect()
    analyzer = EchoAnalyzer(effect)
    analyzer.run()

if __name__ == "__main__":
    main()