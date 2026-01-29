#!/usr/bin/env python3


import numpy as np
import matplotlib.pyplot as plt
import sys
import os
from ctypes import c_float, POINTER, cast, c_uint8, c_uint16, Structure, c_void_p
from enum import IntEnum
import dsp_bindings as dsp


class PedalType(IntEnum):
    OVERDRIVE_DISTORTION = 0
    ECHO = 1
    REVERB = 2
    NOISE_GATE = 3
    PASS_THROUGH = 4


class OverdriveEffect:
    
    def __init__(self):
        self.lib = dsp
        self.volume = 0.1
        self.gain = 0.1
        self.tone = 0.1
        self.level = 0.1
        
    def set_params(self, volume=None, gain=None, tone=None, level=None):
        if volume is not None:
            self.volume = max(0.0, min(1.0, volume))
        if gain is not None:
            self.gain = max(0.0, min(1.0, gain))
        if tone is not None:
            self.tone = max(0.0, min(1.0, tone))
        if level is not None:
            self.level = max(0.0, min(1.0, level))
    
    def process(self, input_signal, gain=None, tone=None, level=None):

        if gain is not None:
            self.gain = gain
        if tone is not None:
            self.tone = tone
        if level is not None:
            self.level = level
        
        input_signal = np.asarray(input_signal, dtype=np.float32)
        length = len(input_signal)
        
        output_signal = np.zeros(length, dtype=np.float32)
        
        input_ptr = input_signal.ctypes.data_as(POINTER(c_float))
        output_ptr = output_signal.ctypes.data_as(POINTER(c_float))
        
        if hasattr(self.lib, 'applyOverdrive'):
            self.lib.applyOverdrive(
                input_ptr,
                output_ptr,
                length,
                self.gain,
                self.tone,
                self.level
            )
        
        return output_signal
    
    def get_parameter_names(self):
        return ["Vol", "Gain", "Tone", "Level"]
    
    def get_parameters(self):
        return [self.volume, self.gain, self.tone, self.level]


def generate_sine_wave(fs, duration, freq, amplitude=0.5):
    """Generate a sine wave test signal"""
    t = np.arange(0, duration, 1/fs, dtype=np.float32)
    signal = amplitude * np.sin(2 * np.pi * freq * t)
    return t, signal


def calculate_thd(signal, fundamental_freq, fs, num_harmonics=5):
    """Calculate Total Harmonic Distortion"""
    N = len(signal)
    fft = np.fft.fft(signal)
    freqs = np.fft.fftfreq(N, 1/fs)
    
    # Find fundamental
    fund_idx = int(fundamental_freq * N / fs)
    fundamental_power = np.abs(fft[fund_idx]) ** 2
    
    # Sum harmonic powers
    harmonic_power = 0
    for h in range(2, num_harmonics + 1):
        harm_idx = int(h * fundamental_freq * N / fs)
        if harm_idx < N // 2:
            harmonic_power += np.abs(fft[harm_idx]) ** 2
    
    thd = 100 * np.sqrt(harmonic_power / fundamental_power) if fundamental_power > 0 else 0
    return thd


def analyze_harmonic_content(signal, fundamental_freq, fs, max_harmonics=10):
    """Analyze harmonic content of signal"""
    N = len(signal)
    fft = np.fft.fft(signal)
    
    harmonics = {}
    for h in range(1, max_harmonics + 1):
        freq = h * fundamental_freq
        idx = int(freq * N / fs)
        if idx < N // 2:
            magnitude = np.abs(fft[idx]) / N
            phase = np.angle(fft[idx])
            harmonics[h] = {
                'frequency': freq,
                'magnitude': magnitude,
                'magnitude_db': 20 * np.log10(magnitude + 1e-10),
                'phase': phase
            }
    
    return harmonics


def plot_analysis(t, input_signal, output_signal, effect, fs, fundamental_freq=220):
    """Create comprehensive DSP analysis plots"""
    
    params = effect.get_parameters()
    param_names = effect.get_parameter_names()
    
    fig = plt.figure(figsize=(16, 10))
    fig.suptitle('DSP Guitar Overdrive Effect Analysis', fontsize=16, fontweight='bold')
    
    # 1. Time domain comparison
    ax1 = plt.subplot(3, 3, 1)
    ax1.plot(t * 1000, input_signal, 'b-', linewidth=1.5, label='Input', alpha=0.7)
    ax1.plot(t * 1000, output_signal, 'r-', linewidth=1.5, label='Output')
    ax1.set_xlabel('Time (ms)')
    ax1.set_ylabel('Amplitude')
    ax1.set_title('Input vs Output Waveform')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    ax1.set_ylim([-1.5, 1.5])
    
    # 2. Zoomed waveform (first 5 cycles)
    ax2 = plt.subplot(3, 3, 2)
    zoom_samples = min(int(5 * fs / fundamental_freq), len(t))
    ax2.plot(t[:zoom_samples] * 1000, input_signal[:zoom_samples], 
             'b-', linewidth=2, label='Input', alpha=0.7)
    ax2.plot(t[:zoom_samples] * 1000, output_signal[:zoom_samples], 
             'r-', linewidth=2, label='Output')
    ax2.set_xlabel('Time (ms)')
    ax2.set_ylabel('Amplitude')
    ax2.set_title('Zoomed View (5 cycles)')
    ax2.legend()
    ax2.grid(True, alpha=0.3)
    
    # 3. Transfer characteristic (clipping curve)
    ax3 = plt.subplot(3, 3, 3)
    test_input = np.linspace(-2, 2, 1000, dtype=np.float32)
    test_output = effect.process(test_input)
    ax3.plot(test_input, test_output, 'r-', linewidth=2.5, label='Overdrive')
    ax3.plot([-2, 2], [-2, 2], 'k--', linewidth=1, label='Linear', alpha=0.5)
    ax3.set_xlabel('Input Level')
    ax3.set_ylabel('Output Level')
    ax3.set_title('Transfer Characteristic')
    ax3.legend()
    ax3.grid(True, alpha=0.3)
    ax3.set_xlim([-2, 2])
    ax3.set_ylim([-1.5, 1.5])
    ax3.axhline(y=0, color='gray', linestyle='-', linewidth=0.5, alpha=0.5)
    ax3.axvline(x=0, color='gray', linestyle='-', linewidth=0.5, alpha=0.5)
    
    # 4. Input spectrum
    ax4 = plt.subplot(3, 3, 4)
    N = len(input_signal)
    freqs = np.fft.fftfreq(N, 1/fs)[:N//2]
    input_fft = np.abs(np.fft.fft(input_signal))[:N//2] / N
    input_fft[input_fft < 1e-10] = 1e-10
    ax4.plot(freqs, 20 * np.log10(input_fft), 'b-', linewidth=1.5)
    ax4.set_xlabel('Frequency (Hz)')
    ax4.set_ylabel('Magnitude (dB)')
    ax4.set_title('Input Spectrum')
    ax4.set_xlim([0, 5000])
    ax4.set_ylim([-100, 0])
    ax4.grid(True, alpha=0.3)
    
    # 5. Output spectrum
    ax5 = plt.subplot(3, 3, 5)
    output_fft = np.abs(np.fft.fft(output_signal))[:N//2] / N
    output_fft[output_fft < 1e-10] = 1e-10
    ax5.plot(freqs, 20 * np.log10(output_fft), 'r-', linewidth=1.5)
    ax5.set_xlabel('Frequency (Hz)')
    ax5.set_ylabel('Magnitude (dB)')
    ax5.set_title('Output Spectrum')
    ax5.set_xlim([0, 5000])
    ax5.set_ylim([-100, 0])
    ax5.grid(True, alpha=0.3)
    
    # Mark harmonics
    for h in range(1, 6):
        harm_freq = h * fundamental_freq
        if harm_freq < 5000:
            ax5.axvline(x=harm_freq, color='orange', linestyle='--', 
                       linewidth=1, alpha=0.5)
            ax5.text(harm_freq, -95, f'H{h}', fontsize=8, ha='center')
    
    # 6. Harmonic content bar chart
    ax6 = plt.subplot(3, 3, 6)
    harmonics = analyze_harmonic_content(output_signal, fundamental_freq, fs, 10)
    harmonic_nums = list(harmonics.keys())
    harmonic_dbs = [harmonics[h]['magnitude_db'] for h in harmonic_nums]
    
    colors = ['darkred' if h == 1 else 'orangered' for h in harmonic_nums]
    bars = ax6.bar(harmonic_nums, harmonic_dbs, color=colors, alpha=0.7, edgecolor='black')
    ax6.set_xlabel('Harmonic Number')
    ax6.set_ylabel('Magnitude (dB)')
    ax6.set_title('Harmonic Content')
    ax6.set_xticks(harmonic_nums)
    ax6.grid(True, alpha=0.3, axis='y')
    ax6.set_ylim([-100, 0])
    
    # 7. Difference signal (added distortion)
    ax7 = plt.subplot(3, 3, 7)
    scaled_input = input_signal * (np.max(np.abs(output_signal)) / np.max(np.abs(input_signal)))
    difference = output_signal - scaled_input
    ax7.plot(t * 1000, difference, 'g-', linewidth=1.5)
    ax7.set_xlabel('Time (ms)')
    ax7.set_ylabel('Amplitude')
    ax7.set_title('Difference Signal')
    ax7.grid(True, alpha=0.3)
    
    # 8. Lissajous plot
    ax8 = plt.subplot(3, 3, 8)
    ax8.plot(input_signal[:zoom_samples], output_signal[:zoom_samples], 
             'purple', linewidth=2, alpha=0.7)
    ax8.set_xlabel('Input')
    ax8.set_ylabel('Output')
    ax8.set_title('Input vs Output')
    ax8.grid(True, alpha=0.3)
    ax8.set_aspect('equal')
    
    # 9. Statistics panel
    ax9 = plt.subplot(3, 3, 9)
    ax9.axis('off')
    
    thd = calculate_thd(output_signal, fundamental_freq, fs)
    input_rms = np.sqrt(np.mean(input_signal**2))
    output_rms = np.sqrt(np.mean(output_signal**2))
    gain_db = 20 * np.log10(output_rms / input_rms) if input_rms > 0 else 0

    param_str = " Effect Parameters:\n"
    for name, value in zip(param_names, params):
        param_str += f" {name}: {value:.3f}\n"
    
    stats_text = f"""╔══════════════════════════════╗
{param_str}╠══════════════════════════════╣
Signal Analysis:           
╠══════════════════════════════╣
 Input Peak:    {np.max(np.abs(input_signal)):.4f}       
 Output Peak:   {np.max(np.abs(output_signal)):.4f}       
 Input RMS:     {input_rms:.4f}       
 Output RMS:    {output_rms:.4f}       
 Gain (dB):     {gain_db:+.2f} dB      
 THD:           {thd:.2f}%          
╠══════════════════════════════╣
 Signal Info:                
╠══════════════════════════════╣
 Sample Rate:   {fs} Hz       
 Duration:      {len(t)/fs*1000:.1f} ms        
 Samples:       {len(t)}          
 Fundamental:   {fundamental_freq} Hz          
╚══════════════════════════════╝
    """
    
    ax9.text(0.05, 0.5, stats_text, family='monospace', 
             fontsize=9, verticalalignment='center',
             bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.3))
    
    plt.tight_layout()
    return fig, thd, harmonics


def main():
    fs = 44100
    duration = 0.1
    fundamental_freq = 220
    amplitude = 0.5
    
    effect = OverdriveEffect()
    
    effect.set_params(
        volume=0.5,   
        gain=0.1,     
        tone=0.1,    
        level=0.1     
    )
    
    params = effect.get_parameters()
    param_names = effect.get_parameter_names()
    
    t, input_signal = generate_sine_wave(fs, duration, fundamental_freq, amplitude)
    
    output_signal = effect.process(input_signal)
    
    thd = calculate_thd(output_signal, fundamental_freq, fs)
    harmonics = analyze_harmonic_content(output_signal, fundamental_freq, fs)
    
    fig, _, _ = plot_analysis(t, input_signal, output_signal, effect, fs, fundamental_freq)
    
    plt.show()


if __name__ == "__main__":
    main()