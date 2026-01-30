#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import sys
import os
from ctypes import c_float, POINTER, cast, c_uint8, c_uint16, Structure, c_void_p
from enum import IntEnum
import dsp_bindings as dsp
import matplotlib.pyplot as plt
from scipy.signal import spectrogram


COLORS = {
    "blue"    : "#89b4fa",
    "red"     : "#f38ba8",
    "peach"   : "#fab387",
    "yellow"  : "#f9e2af",
    "teal"    : "#94e2d5",
    "lavender": "#b4befe",
    "text"    : "#cdd6f4",
    "surface0": "#313244",
    "base"    : "#1e1e2e",
    "overlay2": "#9399b2",
}


class PedalType(IntEnum):
    OVERDRIVE_DISTORTION = 0 # TODO: Add rest of the effects 
    ECHO                 = 1
    REVERB               = 2
    NOISE_GATE           = 3
    PASS_THROUGH         = 4


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

        if hasattr(self.lib, "applyOverdrive"):
            self.lib.applyOverdrive(
                input_ptr, output_ptr, length, self.gain, self.tone, self.level
            )

        return output_signal

    def get_parameter_names(self):
        return ["Vol", "Gain", "Tone", "Level"]

    def get_parameters(self):
        return [self.volume, self.gain, self.tone, self.level]


def generate_sine_wave(fs, duration, freq, amplitude=0.5):
    t = np.arange(0, duration, 1 / fs, dtype=np.float32)
    signal = amplitude * np.sin(2 * np.pi * freq * t)
    return t, signal


def calculate_thd(signal, fundamental_freq, fs, num_harmonics=5):
    N = len(signal)
    fft = np.fft.fft(signal)
    freqs = np.fft.fftfreq(N, 1 / fs)

    fund_idx = int(fundamental_freq * N / fs)
    fundamental_power = np.abs(fft[fund_idx]) ** 2

    harmonic_power = 0
    for h in range(2, num_harmonics + 1):
        harm_idx = int(h * fundamental_freq * N / fs)
        if harm_idx < N // 2:
            harmonic_power += np.abs(fft[harm_idx]) ** 2

    thd = (
        100 * np.sqrt(harmonic_power / fundamental_power)
        if fundamental_power > 0
        else 0
    )
    return thd


def analyze_harmonic_content(signal, fundamental_freq, fs, max_harmonics=10):
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
                "frequency": freq,
                "magnitude": magnitude,
                "magnitude_db": 20 * np.log10(magnitude + 1e-10),
                "phase": phase,
            }

    return harmonics


def calc_rms(signal):
    return np.sqrt(np.mean(signal**2))


def calc_gain_db(input_sig, output_sig):
    input_rms = calc_rms(input_sig)
    output_rms = calc_rms(output_sig)
    return 20 * np.log10(output_rms / input_rms) if input_rms > 0 else 0


def safe_fft(signal, N):
    fft = np.abs(np.fft.rfft(signal[:N])) / N
    return np.maximum(fft, 1e-10)


def plot_waveform(ax, t, in_sig, out_sig, fs, freq, colors):
    cycles = 5
    samples = min(int(cycles * fs / freq), len(t))

    ax.plot(
        t[:samples] * 1000,
        in_sig[:samples],
        color=colors["blue"],
        lw=1.5,
        label="Input",
        alpha=0.8,
    )
    ax.plot(
        t[:samples] * 1000,
        out_sig[:samples],
        color=colors["red"],
        lw=1.5,
        label="Output",
    )

    ax.set_xlabel("Time (ms)")
    ax.set_ylabel("Amplitude")
    ax.set_title("Waveform (5 cycles)")
    ax.legend()
    ax.grid(True, alpha=0.2)
    ax.set_facecolor(colors["surface0"])


def plot_spectrum(ax, in_sig, out_sig, fs, freq, colors):
    N = min(len(in_sig), len(out_sig))
    freqs = np.fft.rfftfreq(N, 1 / fs)

    in_fft = safe_fft(in_sig, N)
    out_fft = safe_fft(out_sig, N)

    ax.plot(
        freqs,
        20 * np.log10(in_fft),
        color=colors["blue"],
        lw=1.2,
        alpha=0.7,
        label="Input",
    )
    ax.plot(
        freqs,
        20 * np.log10(out_fft),
        color=colors["red"],
        lw=1.2,
        alpha=0.7,
        label="Output",
    )

    for h in range(1, 6):
        harm_freq = h * freq
        if harm_freq < 5000:
            ax.axvline(harm_freq, color=colors["peach"], ls=":", lw=0.8, alpha=0.5)

    ax.set(
        xlabel="Frequency (Hz)",
        ylabel="Magnitude (dB)",
        xlim=[0, 5000],
        ylim=[-100, 5],
        title="Spectrum Comparison",
    )
    ax.legend()
    ax.grid(True, alpha=0.2)
    ax.set_facecolor(colors["surface0"])


def plot_spectrogram(ax, signal, fs, colors):
    nperseg = min(256, len(signal) // 10)
    f, t_seg, Sxx = spectrogram(signal, fs, nperseg=nperseg, noverlap=nperseg // 2)

    mask = f <= 5000
    f, Sxx = f[mask], Sxx[mask, :]

    im = ax.pcolormesh(
        t_seg,
        f,
        10 * np.log10(Sxx + 1e-10),
        shading="gouraud",
        cmap="magma",
        vmin=-80,
        vmax=0,
    )

    ax.set(
        xlabel="Time (s)",
        ylabel="Frequency (Hz)",
        ylim=[0, 5000],
        title="Output Spectrogram",
    )
    ax.grid(True, alpha=0.1)
    ax.set_facecolor(colors["base"])

    return im


def plot_harmonics(ax, signal, freq, fs, colors):
    harmonics = analyze_harmonic_content(signal, freq, fs, 10)
    harm_nums = list(harmonics.keys())
    harm_dbs = [harmonics[h]["magnitude_db"] for h in harm_nums]

    color_map = lambda h: {
        1: colors["red"],
        2: colors["peach"],
        3: colors["yellow"],
    }.get(h, colors["teal"] if h % 2 == 0 else colors["lavender"])

    colors_bars = [color_map(h) for h in harm_nums]
    bars = ax.bar(
        harm_nums, harm_dbs, color=colors_bars, alpha=0.8, edgecolor=colors["text"]
    )

    for bar, val in zip(bars, harm_dbs):
        ax.text(
            bar.get_x() + bar.get_width() / 2,
            bar.get_height() - 5,
            f"{val:.0f}",
            ha="center",
            va="top",
            fontsize=8,
            color=colors["text"],
        )

    ax.set(
        xlabel="Harmonic Number",
        ylabel="Magnitude (dB)",
        ylim=[-100, 0],
        title="Harmonic Distribution",
    )
    ax.set_xticks(harm_nums)
    ax.grid(True, alpha=0.2, axis="y")
    ax.set_facecolor(colors["surface0"])


def add_stats_panel(ax, effect, stats, fs, t, freq, colors):
    ax.axis("off")

    params = effect.get_parameters()
    param_names = effect.get_parameter_names()
    param_text = "\n".join(f"{n}: {v:.3f}" for n, v in zip(param_names, params))

    signal_text = f"""
Effect Parameters:
{param_text}

Signal Analysis:
Input Peak:    {stats['input_peak']:.4f}
Output Peak:   {stats['output_peak']:.4f}
Input RMS:     {stats['input_rms']:.4f}
Output RMS:    {stats['output_rms']:.4f}
Gain (dB):     {stats['gain_db']:+.2f} dB
THD:           {stats['thd']:.2f}%

Signal Info:
Sample Rate:   {fs} Hz
Duration:      {len(t)/fs*1000:.1f} ms
Fundamental:   {freq} Hz
    """

    ax.text(
        0.05,
        0.5,
        signal_text,
        family="monospace",
        fontsize=9,
        va="center",
        color=colors["text"],
    )


def plot_analysis(t, input_signal, output_signal, effect, fs, fundamental_freq=220):

    stats = {
        "input_peak": np.max(np.abas(input_signal)),
        "output_peak": np.max(np.abs(output_signal)),
        "input_rms": calc_rms(input_signal),
        "output_rms": calc_rms(output_signal),
        "gain_db": calc_gain_db(input_signal, output_signal),
        "thd": calculate_thd(output_signal, fundamental_freq, fs),
    }

    fig = plt.figure(figsize=(14, 10), facecolor=COLORS["overlay2"])

    # 1. Waveform
    ax1 = plt.subplot(2, 2, 1)
    plot_waveform(ax1, t, input_signal, output_signal, fs, fundamental_freq, COLORS)

    # 2. Spectrum
    ax2 = plt.subplot(2, 2, 2)
    plot_spectrum(ax2, input_signal, output_signal, fs, fundamental_freq, COLORS)

    # 3. Spectrogram
    ax3 = plt.subplot(2, 2, 3)
    im = plot_spectrogram(ax3, output_signal, fs, COLORS)
    plt.colorbar(im, ax=ax3, label="dB")

    # 4. Harmonics
    ax4 = plt.subplot(2, 2, 4)
    plot_harmonics(ax4, output_signal, fundamental_freq, fs, COLORS)

    # 5. Stats panel
    ax5 = plt.subplot(2, 3, 4)
    add_stats_panel(ax5, effect, stats, fs, t, fundamental_freq, COLORS)

    plt.tight_layout()
    return fig

# TODO: Create CLI toolkit for selecting which effect to simulate and give it default values
def main():
    fs = 44100
    duration = 0.1
    fundamental_freq = 220
    amplitude = 0.5

    effect = OverdriveEffect()

    effect.set_params(volume=1.0, gain=0.3, tone=0.1, level=0.3)

    t, input_signal = generate_sine_wave(fs, duration, fundamental_freq, amplitude)

    output_signal = effect.process(input_signal)

    thd = calculate_thd(output_signal, fundamental_freq, fs)
    harmonics = analyze_harmonic_content(output_signal, fundamental_freq, fs)

    fig = plot_analysis(t, input_signal, output_signal, effect, fs, fundamental_freq)

    plt.show()


if __name__ == "__main__":
    main()
