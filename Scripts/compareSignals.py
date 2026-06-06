#!/usr/bin/env python3
"""
compare_wavs.py
---------------
Compare two WAV files with the same styling as plotAnalysis.py.
Generates 4 separate figures, each with 2 subplots (one for each file):
1. Waveforms (amplitude vs time) - 2 subplots
2. Spectrograms - 2 subplots  
3. Chromagrams - 2 subplots
4. Impulse responses - 2 subplots
"""

import numpy as np
import matplotlib.pyplot as plt
from scipy.signal import spectrogram as scipy_spectrogram
import librosa
import librosa.display
import soundfile as sf
import argparse
import os

# ---------------------------------------------------------------------------
# Shared colour palette (Catppuccin Mocha) - Same as plotAnalysis
# ---------------------------------------------------------------------------
COLORS = {
    "blue"    : "#89b4fa",
    "red"     : "#f38ba8",
    "peach"   : "#fab387",
    "yellow"  : "#f9e2af",
    "teal"    : "#94e2d5",
    "lavender": "#b4befe",
    "green"   : "#a6e3a1",
    "mauve"   : "#cba6f7",
    "text"    : "#cdd6f4",
    "subtext" : "#a6adc8",
    "surface0": "#313244",
    "surface1": "#45475a",
    "surface2": "#585b70",
    "base"    : "#1e1e2e",
    "mantle"  : "#181825",
    "overlay0": "#6c7086",
    "overlay2": "#9399b2",
    "crust"   : "#11111b",
}

def _style_ax(ax):
    """Apply consistent styling to axes"""
    ax.set_facecolor(COLORS["surface0"])
    for spine in ax.spines.values():
        spine.set_edgecolor(COLORS["surface1"])
    ax.tick_params(colors=COLORS["subtext"], labelsize=8)
    ax.xaxis.label.set_color(COLORS["subtext"])
    ax.yaxis.label.set_color(COLORS["subtext"])
    ax.title.set_color(COLORS["text"])

def load_wav(filepath, duration=None):
    """
    Load a WAV file and prepare it for analysis.
    
    Parameters:
    -----------
    filepath : str
        Path to WAV file
    duration : float, optional
        Duration in seconds to load (starting from beginning)
    
    Returns:
    --------
    signal : ndarray
        Audio signal normalized to [-1, 1]
    fs : int
        Sample rate
    t : ndarray
        Time array in seconds
    """
    try:
        # Load WAV file
        signal, fs = sf.read(filepath)
        
        # Convert to mono if stereo
        if len(signal.shape) > 1:
            signal = np.mean(signal, axis=1)
        
        # Normalize to [-1, 1] if not already
        max_val = np.max(np.abs(signal))
        if max_val > 0:
            signal = signal / max_val
        
        # Truncate to specified duration
        if duration is not None:
            samples = int(duration * fs)
            signal = signal[:samples]
        
        # Create time array
        t = np.arange(len(signal)) / fs
        
        print(f"Loaded: {os.path.basename(filepath)}")
        print(f"  Sample rate: {fs} Hz")
        print(f"  Duration: {len(signal)/fs:.2f} seconds")
        print(f"  Samples: {len(signal)}")
        print(f"  Peak amplitude: {np.max(np.abs(signal)):.3f}")
        
        return signal, fs, t
        
    except Exception as e:
        print(f"Error loading {filepath}: {e}")
        raise

def draw_waveform_figure(signal1, fs1, t1, signal2, fs2, t2, name1, name2):
    """Figure 1: Waveforms - two separate subplots (one for each file)"""
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(14, 8), facecolor=COLORS["mantle"])
    
    # Plot signal 1
    ax1.plot(t1 * 1000, signal1, color=COLORS["blue"], lw=1.5, alpha=0.85)
    ax1.set_xlabel("Χρόνος (ms)")
    ax1.set_ylabel("Πλάτος")
    ax1.set_title(f"Κυματομορφή - Καθαρό Σήμα")
    ax1.grid(True, alpha=0.18, color=COLORS["surface2"])
    _style_ax(ax1)
    
    # Plot signal 2
    ax2.plot(t2 * 1000, signal2, color=COLORS["red"], lw=1.5, alpha=0.85)
    ax2.set_xlabel("Χρόνος (ms)")
    ax2.set_ylabel("Πλάτος")
    ax2.set_title(f"Κυματομορφή - Σήμα Εξόδου")
    ax2.grid(True, alpha=0.18, color=COLORS["surface2"])
    _style_ax(ax2)
    
    plt.tight_layout()
    return fig

def draw_spectrogram_figure(signal1, fs1, signal2, fs2, name1, name2):
    """Figure 2: Spectrograms - two separate subplots (one for each file)"""
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(14, 8), facecolor=COLORS["mantle"])
    
    # Spectrogram for signal 1
    nperseg = min(256, len(signal1) // 10)
    f1, t_seg1, Sxx1 = scipy_spectrogram(signal1, fs1, nperseg=nperseg,
                                          noverlap=nperseg // 2)
    max_freq = min(10000, fs1/2)
    mask1 = f1 <= max_freq
    im1 = ax1.pcolormesh(t_seg1, f1[mask1], 10 * np.log10(Sxx1[mask1] + 1e-10),
                         shading="gouraud", cmap="magma", vmin=-80, vmax=0)
    ax1.set_xlabel("Χρόνος (s)")
    ax1.set_ylabel("Συχνότητα (Hz)")
    ax1.set_ylim(0, max_freq)
    ax1.set_title(f"Φασματογράφημα - Καθαρό Σήμα")
    ax1.grid(True, alpha=0.08, color=COLORS["surface2"])
    cbar1 = plt.colorbar(im1, ax=ax1, fraction=0.046, pad=0.04)
    cbar1.set_label('Ένταση (dB)', color=COLORS["text"])
    cbar1.ax.yaxis.set_tick_params(color=COLORS["text"])
    
    # Spectrogram for signal 2
    nperseg = min(256, len(signal2) // 10)
    f2, t_seg2, Sxx2 = scipy_spectrogram(signal2, fs2, nperseg=nperseg,
                                          noverlap=nperseg // 2)
    max_freq = min(10000, fs2/2)
    mask2 = f2 <= max_freq
    im2 = ax2.pcolormesh(t_seg2, f2[mask2], 10 * np.log10(Sxx2[mask2] + 1e-10),
                         shading="gouraud", cmap="magma", vmin=-80, vmax=0)
    ax2.set_xlabel("Χρόνος (s)")
    ax2.set_ylabel("Συχνότητα (Hz)")
    ax2.set_ylim(0, max_freq)
    ax2.set_title(f"Φασματογράφημα - Σήμα Εξόδου")
    ax2.grid(True, alpha=0.08, color=COLORS["surface2"])
    cbar2 = plt.colorbar(im2, ax=ax2, fraction=0.046, pad=0.04)
    cbar2.set_label('Ένταση (dB)', color=COLORS["text"])
    cbar2.ax.yaxis.set_tick_params(color=COLORS["text"])
    
    # Style both axes
    for ax in [ax1, ax2]:
        ax.set_facecolor(COLORS["base"])
        ax.tick_params(colors=COLORS["subtext"], labelsize=8)
        ax.xaxis.label.set_color(COLORS["subtext"])
        ax.yaxis.label.set_color(COLORS["subtext"])
        ax.title.set_color(COLORS["text"])
        for spine in ax.spines.values():
            spine.set_edgecolor(COLORS["surface1"])
    
    plt.tight_layout()
    return fig

def draw_chromagram_figure(signal1, fs1, signal2, fs2, name1, name2):
    """Figure 3: Chromagrams - two separate subplots (one for each file)"""
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(14, 8), facecolor=COLORS["mantle"])
    
    try:
        # Chromagram for signal 1
        hop_length = 512
        n_fft = 2048
        
        chromagram1 = librosa.feature.chroma_stft(
            y=signal1.astype(np.float32), 
            sr=fs1, 
            n_fft=n_fft, 
            hop_length=hop_length
        )
        
        img1 = librosa.display.specshow(
            chromagram1, 
            x_axis='time', 
            y_axis='chroma', 
            sr=fs1, 
            hop_length=hop_length,
            ax=ax1,
            cmap='magma'
        )
        ax1.set_title(f"Χρωμόγραμμα - Καθαρό Σήμα")
        ax1.set_ylabel("Τονικό Ύψος")
        ax1.set_xlabel("Χρόνος (s)")
        cbar1 = plt.colorbar(img1, ax=ax1, fraction=0.046, pad=0.04)
        cbar1.set_label('Ένταση', color=COLORS["text"])
        cbar1.ax.yaxis.set_tick_params(color=COLORS["text"])
        
        # Chromagram for signal 2
        chromagram2 = librosa.feature.chroma_stft(
            y=signal2.astype(np.float32), 
            sr=fs2, 
            n_fft=n_fft, 
            hop_length=hop_length
        )
        
        img2 = librosa.display.specshow(
            chromagram2, 
            x_axis='time', 
            y_axis='chroma', 
            sr=fs2, 
            hop_length=hop_length,
            ax=ax2,
            cmap='magma'
        )
        ax2.set_title(f"Χρωμόγραμμα - Σήμα Εξόδου")
        ax2.set_ylabel("Τονικό Ύψος")
        ax2.set_xlabel("Χρόνος (s)")
        cbar2 = plt.colorbar(img2, ax=ax2, fraction=0.046, pad=0.04)
        cbar2.set_label('Ένταση', color=COLORS["text"])
        cbar2.ax.yaxis.set_tick_params(color=COLORS["text"])
        
        # Style both axes
        for ax in [ax1, ax2]:
            ax.tick_params(colors=COLORS["subtext"], labelsize=8)
            ax.xaxis.label.set_color(COLORS["subtext"])
            ax.yaxis.label.set_color(COLORS["subtext"])
            ax.title.set_color(COLORS["text"])
            _style_ax(ax)
            
    except Exception as e:
        # Fallback if librosa fails
        ax1.text(0.5, 0.5, f"Chromagram unavailable\n{str(e)}", 
                ha='center', va='center', color=COLORS["text"],
                transform=ax1.transAxes)
        ax1.set_title(f"Χρωμόγραμμα - Καθαρό Σήμα")
        
        ax2.text(0.5, 0.5, f"Chromagram unavailable\n{str(e)}", 
                ha='center', va='center', color=COLORS["text"],
                transform=ax2.transAxes)
        ax2.set_title(f"Χρωμόγραμμα - Σήμα Εξόδου")
        _style_ax(ax1)
        _style_ax(ax2)
    
    plt.tight_layout()
    return fig

def draw_impulse_response_figure(signal1, fs1, signal2, fs2, name1, name2):
    """Figure 4: Impulse responses - two separate subplots (one for each file)"""
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(14, 8), facecolor=COLORS["mantle"])
    
    # Time arrays
    t1 = np.arange(len(signal1)) / fs1 * 1000
    t2 = np.arange(len(signal2)) / fs2 * 1000
    
    # Plot signal 1 as time-domain response
    ax1.plot(t1[:min(len(t1), 5000)], signal1[:min(len(signal1), 5000)], 
            color=COLORS["teal"], lw=1.5, alpha=0.85)
 
    
    ax1.set_xlabel("Χρόνος (ms)")
    ax1.set_ylabel("Πλάτος")
    ax1.set_title(f"Κρουστική Απόκριση - Καθαρό Σήμα")
    ax1.grid(True, alpha=0.18, color=COLORS["surface2"])
    ax1.legend(fontsize=8)
    _style_ax(ax1)
    
    # Plot signal 2 as time-domain response
    ax2.plot(t2[:min(len(t2), 5000)], signal2[:min(len(signal2), 5000)], 
            color=COLORS["peach"], lw=1.5, alpha=0.85)
    
   
    
    ax2.set_xlabel("Χρόνος (ms)")
    ax2.set_ylabel("Πλάτος")
    ax2.set_title(f"Κρουστική Απόκριση - Σήμα Εξόδου")
    ax2.grid(True, alpha=0.18, color=COLORS["surface2"])
    ax2.legend(fontsize=8)
    _style_ax(ax2)
    
    plt.tight_layout()
    return fig

def save_figure(fig, filename, output_dir):
    """Save figure to output directory"""
    if output_dir:
        os.makedirs(output_dir, exist_ok=True)
        filepath = os.path.join(output_dir, filename)
        fig.savefig(filepath, dpi=150, facecolor=COLORS["mantle"], bbox_inches='tight')
        print(f"Saved: {filepath}")

def main():
    parser = argparse.ArgumentParser(description='Compare two WAV files with separate plots for each file')
    parser.add_argument('file1', type=str, help='Path to first WAV file')
    parser.add_argument('file2', type=str, help='Path to second WAV file')
    parser.add_argument('--duration', type=float, default=None, 
                       help='Duration in seconds to analyze (default: min length of both files)')
    parser.add_argument('--output', type=str, default=None,
                       help='Output directory to save figures (optional)')
    parser.add_argument('--show', action='store_true', default=True,
                       help='Show figures (default: True)')
    
    args = parser.parse_args()
    
    # Get base names for labels
    name1 = os.path.splitext(os.path.basename(args.file1))[0]
    name2 = os.path.splitext(os.path.basename(args.file2))[0]
    
    # Load both WAV files
    print("\n" + "="*50)
    print("Loading WAV files...")
    print("="*50)
    signal1, fs1, t1 = load_wav(args.file1, args.duration)
    signal2, fs2, t2 = load_wav(args.file2, args.duration)
    
    # Ensure same length for comparison (trim to shortest)
    min_len = min(len(signal1), len(signal2))
    if len(signal1) != len(signal2):
        print(f"\nWarning: Files have different lengths. Truncating to {min_len} samples.")
        signal1 = signal1[:min_len]
        signal2 = signal2[:min_len]
        t1 = t1[:min_len]
        t2 = t2[:min_len]
    

    
    # Figure 1: Waveforms (2 subplots)
    fig1 = draw_waveform_figure(signal1, fs1, t1, signal2, fs2, t2, name1, name2)
    
    # Figure 2: Spectrograms (2 subplots)
    fig2 = draw_spectrogram_figure(signal1, fs1, signal2, fs2, name1, name2)
    
    # Figure 3: Chromagrams (2 subplots)
    fig3 = draw_chromagram_figure(signal1, fs1, signal2, fs2, name1, name2)
    
    # Figure 4: Impulse responses (2 subplots)
    fig4 = draw_impulse_response_figure(signal1, fs1, signal2, fs2, name1, name2)
    
    # Save figures if output directory specified
    if args.output:
        save_figure(fig1, f"{name2}_waveforms.png", args.output)
        save_figure(fig2, f"{name2}_spectrograms.png", args.output)
        save_figure(fig3, f"{name2}_chromagrams.png", args.output)
        save_figure(fig4, f"{name2}_impulse_responses.png", args.output)
    
    # Show figures
    if args.show:
        print("\nDisplaying figures...")
        plt.show()
    else:
        plt.close('all')
    
 

if __name__ == "__main__":
    main()