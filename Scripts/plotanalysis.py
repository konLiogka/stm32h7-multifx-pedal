#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Arc, Circle
from matplotlib.widgets import Button, RadioButtons
from scipy.signal import spectrogram as scipy_spectrogram
import librosa
import librosa.display
import soundfile as sf
from abc import ABC, abstractmethod

# ---------------------------------------------------------------------------
# Shared colour palette (Catppuccin Mocha)
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

_KNOB_COLORS = [
    COLORS["blue"], COLORS["peach"], COLORS["yellow"],
    COLORS["teal"], COLORS["lavender"], COLORS["green"], COLORS["mauve"],
    COLORS["red"],
]

_ANGLE_MIN_DEG =  225.0
_ANGLE_MAX_DEG = -45.0

# ---------------------------------------------------------------------------
# Knob widget
# ---------------------------------------------------------------------------
class Knob:
    def __init__(self, ax, label, vmin=0.0, vmax=1.0, valinit=0.5, color=None, step=0.05):
        self.ax    = ax
        self.label = label
        self.vmin  = float(vmin)
        self.vmax  = float(vmax)
        self.step  = float(step)
        self.val   = float(self._quantize(np.clip(valinit, vmin, vmax)))
        self.color = color or COLORS["blue"]
        self._cbs  = []

        self._dragging    = False
        self._drag_y0     = 0.0
        self._drag_val0   = self.val

        ax.set_facecolor(COLORS["mantle"])
        ax.axis("off")

        self._draw()

        fig = ax.figure
        self._cids = [
            fig.canvas.mpl_connect("button_press_event",   self._on_press),
            fig.canvas.mpl_connect("button_release_event", self._on_release),
            fig.canvas.mpl_connect("motion_notify_event",  self._on_motion),
            fig.canvas.mpl_connect("scroll_event",         self._on_scroll),
        ]

    def _quantize(self, value):
        if self.step <= 0:
            return value
        steps = round((value - self.vmin) / self.step)
        quantized = self.vmin + steps * self.step
        return np.clip(quantized, self.vmin, self.vmax)

    def _draw(self):
        ax = self.ax
        ax.cla()
        ax.set_xlim(-1.5, 1.5)
        ax.set_ylim(-1.75, 1.4)
        ax.set_aspect("equal")
        ax.axis("off")
        ax.set_facecolor(COLORS["mantle"])

        norm  = self._norm()
        angle = self._angle_deg(norm)
        rad   = np.deg2rad(angle)

        groove = Arc((0, 0), 2.1, 2.1, angle=0,
                     theta1=_ANGLE_MAX_DEG, theta2=_ANGLE_MIN_DEG,
                     color=COLORS["surface2"], lw=5, zorder=1)
        ax.add_patch(groove)

        self._draw_anchor_lines(ax, radius=1.02)

        if norm > 0.001:
            fill = Arc((0, 0), 2.1, 2.1, angle=0,
                       theta1=angle, theta2=_ANGLE_MIN_DEG,
                       color=self.color, lw=5, zorder=2, alpha=0.90)
            ax.add_patch(fill)

        shadow = Circle((0.045, -0.045), 0.84, color=COLORS["crust"], zorder=3, alpha=0.55)
        ax.add_patch(shadow)

        body = Circle((0, 0), 0.84, color=COLORS["surface1"], zorder=4)
        ax.add_patch(body)

        inner = Circle((0, 0), 0.74, color=COLORS["surface0"], zorder=5)
        ax.add_patch(inner)

        hl = Circle((-0.20, 0.24), 0.28, color=COLORS["surface2"], alpha=0.32, zorder=6)
        ax.add_patch(hl)

        tip_x = 0.54 * np.cos(rad)
        tip_y = 0.54 * np.sin(rad)
        ax.plot([0, tip_x], [0, tip_y], color=COLORS["mantle"], lw=4.0, solid_capstyle="round", zorder=7)
        ax.plot([0, tip_x], [0, tip_y], color=self.color, lw=3, solid_capstyle="round", zorder=8)

        dot = Circle((tip_x * 1.18, tip_y * 1.18), 0.105, color=self.color, zorder=9)
        ax.add_patch(dot)

        ax.text(0, -1.28, self.label.upper(), ha="center", va="center",
                color=COLORS["text"], fontsize=8.5, fontfamily="monospace", fontweight="bold", zorder=10)
        ax.text(0, -1.58, f"{self.val:.2f}", ha="center", va="center",
                color=self.color, fontsize=8, fontfamily="monospace", zorder=10)

    def _draw_anchor_lines(self, ax, radius=0.98):
        if self.step <= 0:
            return
        num_steps = int((self.vmax - self.vmin) / self.step) + 1
        max_ticks = 12
        step_skip = max(1, num_steps // max_ticks)
        
        for i in range(0, num_steps, step_skip):
            value = self.vmin + i * self.step
            if value > self.vmax:
                break
            angle_deg = self._value_to_angle(value)
            angle_rad = np.deg2rad(angle_deg)
            is_major = (i % max(2, step_skip * 2) == 0)
            tick_length = 0.12 if is_major else 0.07
            start_x = radius * np.cos(angle_rad)
            start_y = radius * np.sin(angle_rad)
            end_x = (radius - tick_length) * np.cos(angle_rad)
            end_y = (radius - tick_length) * np.sin(angle_rad)
            ax.plot([start_x, end_x], [start_y, end_y],
                   color=COLORS["overlay2"], lw=0.8 if is_major else 0.5,
                   alpha=0.6, zorder=2)

    def _norm(self):
        return (self.val - self.vmin) / (self.vmax - self.vmin)

    def _angle_deg(self, norm=None):
        if norm is None:
            norm = self._norm()
        return _ANGLE_MIN_DEG + norm * (_ANGLE_MAX_DEG - _ANGLE_MIN_DEG)
    
    def _value_to_angle(self, value):
        norm = (value - self.vmin) / (self.vmax - self.vmin)
        return self._angle_deg(norm)

    def _hit(self, event):
        return event.inaxes is self.ax

    def _on_press(self, event):
        if not self._hit(event) or event.button != 1:
            return
        self._dragging  = True
        self._drag_y0   = event.y
        self._drag_val0 = self.val

    def _on_release(self, event):
        self._dragging = False

    def _on_motion(self, event):
        if not self._dragging:
            return
        sensitivity = (self.vmax - self.vmin) / 200.0
        raw_val = np.clip(
            self._drag_val0 + (event.y - self._drag_y0) * sensitivity,
            self.vmin, self.vmax,
        )
        new_val = self._quantize(raw_val)
        self._set_val(new_val)

    def _on_scroll(self, event):
        if not self._hit(event):
            return
        new_val = np.clip(self.val + self.step * event.step, self.vmin, self.vmax)
        new_val = self._quantize(new_val)
        self._set_val(new_val)

    def _set_val(self, val):
        val = self._quantize(val)
        if abs(val - self.val) < 0.0001:
            return
        self.val = float(val)
        self._draw()
        self.ax.figure.canvas.draw_idle()
        for cb in self._cbs:
            cb(self.val)

    def on_changed(self, func):
        self._cbs.append(func)

    def set_val(self, val):
        self._set_val(self._quantize(np.clip(val, self.vmin, self.vmax)))

    def disconnect(self):
        for cid in self._cids:
            self.ax.figure.canvas.mpl_disconnect(cid)

# ---------------------------------------------------------------------------
# Shared plot primitives
# ---------------------------------------------------------------------------

def _style_ax(ax):
    ax.set_facecolor(COLORS["surface0"])
    for spine in ax.spines.values():
        spine.set_edgecolor(COLORS["surface1"])
    ax.tick_params(colors=COLORS["subtext"], labelsize=8)
    ax.xaxis.label.set_color(COLORS["subtext"])
    ax.yaxis.label.set_color(COLORS["subtext"])
    ax.title.set_color(COLORS["text"])

def draw_waveform(ax, t, in_sig, out_sig=None, *, label_in="Είσοδος", label_out="Έξοδος"):
    """
    Draw waveform plot. Can show one or two signals.
    
    Parameters:
    -----------
    ax : matplotlib axis
        The axis to draw on
    t : array
        Time array
    in_sig : array
        First signal (always shown, typically input)
    out_sig : array or None
        Second signal (optional, typically output)
    label_in : str
        Label for the first signal
    label_out : str
        Label for the second signal (only used if out_sig is provided)
    """
    ax.clear()
    
    # Always plot the first signal (input)
    ax.plot(t * 1000, in_sig, color=COLORS["blue"], lw=1.5, 
            label=label_in, alpha=0.85)
    
    # Plot second signal only if provided
    if out_sig is not None:
        ax.plot(t * 1000, out_sig, color=COLORS["red"], lw=1.5, 
                label=label_out, alpha=0.9)
    
    ax.set_xlabel("Χρόνος (ms)")
    ax.set_ylabel("Πλάτος")
    ax.set_title("Κυματομορφή")
    
    # Only show legend if there are labels to show
    if out_sig is not None:
        ax.legend(fontsize=8)
    
    ax.grid(True, alpha=0.18, color=COLORS["surface2"])
    _style_ax(ax)

def draw_frequency_spectrum(ax, signal, fs, title, color):
    """Draw frequency spectrum (FFT) of a signal"""
    ax.clear()
    N = len(signal)
    freqs = np.fft.rfftfreq(N, 1/fs)
    mag = np.abs(np.fft.rfft(signal)) / N
    mag_db = 20 * np.log10(mag + 1e-10)
    
    # Limit frequency range for better visualization (up to 10kHz or Nyquist)
    max_freq = min(10000, fs/2)
    mask = freqs <= max_freq
    
    ax.plot(freqs[mask], mag_db[mask], color=color, lw=1.5)
    ax.set_xlabel("Συχνότητα (Hz)")
    ax.set_ylabel("Ένταση Στάθμης (dB)")
    ax.set_title(title)
    ax.grid(True, alpha=0.18, color=COLORS["surface2"])
    _style_ax(ax)

def draw_chromagram(ax, signal, fs, title):
    """Draw chromagram showing pitch class distribution over time"""
    ax.clear()
    
    try:
        # Compute chromagram using librosa
        hop_length = 512
        n_fft = 2048
        
        # Ensure signal is float32
        signal_float = signal.astype(np.float32)
        
        # Compute chromagram
        chromagram = librosa.feature.chroma_stft(
            y=signal_float, 
            sr=fs, 
            n_fft=n_fft, 
            hop_length=hop_length
        )
        
        # Display chromagram
        img = librosa.display.specshow(
            chromagram, 
            x_axis='time', 
            y_axis='chroma', 
            sr=fs, 
            hop_length=hop_length,
            ax=ax,
            cmap='magma'
        )
        
        ax.set_title(title)
        ax.set_ylabel("Τονικό Ύψος")
        ax.set_xlabel("Χρόνος (S)")
        
        # Add colorbar
        cbar = plt.colorbar(img, ax=ax, fraction=0.046, pad=0.04)
        cbar.set_label('Ένταση', color=COLORS["text"])
        cbar.ax.yaxis.set_tick_params(color=COLORS["text"])
        plt.setp(plt.getp(cbar.ax.axes, 'yticklabels'), color=COLORS["text"])
        
    except Exception as e:
        # Fallback if librosa fails
        ax.text(0.5, 0.5, f"Chromagram unavailable\n{str(e)}", 
                ha='center', va='center', color=COLORS["text"],
                transform=ax.transAxes)
        ax.set_title(title)
    
    _style_ax(ax)

def draw_spectrogram(ax, signal, fs, *, title="Spectrogram", max_freq=5000):
    ax.clear()
    nperseg = min(256, len(signal) // 10)
    f, t_seg, Sxx = scipy_spectrogram(signal, fs, nperseg=nperseg,
                                      noverlap=nperseg // 2)
    mask = f <= max_freq
    im = ax.pcolormesh(t_seg, f[mask], 10 * np.log10(Sxx[mask] + 1e-10),
                       shading="gouraud", cmap="magma", vmin=-80, vmax=0)
    ax.set_xlabel("Χρόνος (S)")
    ax.set_ylabel("Συχνότητα (Hz)")
    ax.set_ylim(0, max_freq)
    ax.set_title(title)
    ax.grid(True, alpha=0.08, color=COLORS["surface2"])
    ax.set_facecolor(COLORS["base"])
    ax.tick_params(colors=COLORS["subtext"], labelsize=8)
    ax.xaxis.label.set_color(COLORS["subtext"])
    ax.yaxis.label.set_color(COLORS["subtext"])
    ax.title.set_color(COLORS["text"])
    for spine in ax.spines.values():
        spine.set_edgecolor(COLORS["surface1"])
    return im

def draw_harmonic_spectrum(ax, in_sig, out_sig, fs, fundamental_hz=200):
    """Draw harmonic spectrum with both input and output"""
    ax.clear()
    N = len(in_sig)
    freqs = np.fft.rfftfreq(N, 1/fs)
    mask = freqs <= 8000

    mag_in = np.abs(np.fft.rfft(in_sig)) / N
    mag_out = np.abs(np.fft.rfft(out_sig)) / N

    ax.fill_between(freqs[mask], 1e-10, mag_out[mask] + 1e-10,
                    color=COLORS["peach"], alpha=0.15)
    ax.semilogy(freqs[mask], mag_in[mask] + 1e-10,
                color=COLORS["blue"], lw=1.2, alpha=0.85, label="Input")
    ax.semilogy(freqs[mask], mag_out[mask] + 1e-10,
                color=COLORS["peach"], lw=1.4, label="Output")

    for h in range(1, 9):
        hf = h * fundamental_hz
        if hf < 8000:
            ax.axvline(hf, color=COLORS["overlay0"],
                       lw=0.7, ls="--", alpha=0.55)

    ax.set_xlabel("Συχνότητα (Hz)")
    ax.set_ylabel("Ένταησ Στάθμης (dB)")
    ax.set_title("Αρμονικό Φάσμα")
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.18, color=COLORS["surface2"])
    _style_ax(ax)

 

# ---------------------------------------------------------------------------
# Core analyzer base class with view selection
# ---------------------------------------------------------------------------

class EffectAnalyzer(ABC):
    """
    Base class for interactive DSP effect analyzers with multiple view modes.
    """
    
    FS             = 44_100
    DURATION       = 0.1
    FUNDAMENTAL_HZ = 200
    AMPLITUDE      = 1

    # Layout constants
    _PLOT_RIGHT = 0.67
    _PANEL_L    = 0.695
    _PANEL_W    = 0.285
    _KNOB_W     = 0.082
    _KNOB_H     = 0.195
    _ROW_TOP    = 0.68
    _ROW_STEP   = 0.245

    def __init__(self, wav_file=None, duration=None):
        """
        Initialize analyzer.
        
        Parameters:
        -----------
        wav_file : str, optional
            Path to WAV file to load. If None, generates test sine wave.
        duration : float, optional
            Duration in seconds to load from WAV file. If None, loads entire file.
        """
        self.wav_file = wav_file
        self.duration = duration
        
        # Load or generate signal
        if wav_file:
            self.t, self.input_signal, self.fs = self._load_wav(wav_file, duration)
            # Update sample rate to match loaded file
            self.FS = self.fs
        else:
            self.t, self.input_signal = self._make_signal()
            self.fs = self.FS
        
        # Initialize output signal (will be computed by child class)
        self.output_signal = None
        
        # View mode: 'standard', 'freq_response', 'chromagram'
        self.view_mode = 'standard'
        
        self._spectrogram_cache = {}
        self._redraw_counter = 0
        self._last_params_hash = None
        self._update_timer = None
        self._pending_update = False
        self._last_update_time = 0 
        
        # Build the figure
        self._build_figure()
        
        # Now compute initial output signal (after figure is built)
        self.output_signal = self.apply_effect(self.input_signal)
        
        # Add file info if WAV was loaded
        if wav_file:
            self._add_file_info()
        
        # Initial redraw
        self._redraw()

    def _load_wav(self, wav_file, duration=None):
        """Load a WAV file and prepare it for analysis."""
        try:
            # Load WAV file
            signal, fs = sf.read(wav_file)
            
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
            
            # Convert to float32
            signal = signal.astype(np.float32)
            
            # Create time array
            t = np.arange(len(signal)) / fs
            
            print(f"Loaded WAV file: {wav_file}")
            print(f"  Sample rate: {fs} Hz")
            print(f"  Duration: {len(signal)/fs:.2f} seconds")
            print(f"  Samples: {len(signal)}")
            print(f"  Peak amplitude: {np.max(np.abs(signal)):.3f}")
            
            return t, signal, fs
            
        except Exception as e:
            print(f"Error loading WAV file: {e}")
            print("Falling back to generated sine wave...")
            return self._make_signal()

    def _make_signal(self):
        """Generate test sine wave (fallback if no WAV file)"""
        t = np.arange(0, self.DURATION, 1 / self.FS, dtype=np.float32)
        n = len(t)
        s = n // 2
        env = np.zeros(n, dtype=np.float32)
        env[:s]    = 0.15
        env[s:]  = 0.8
        sig = (env * self.AMPLITUDE
               * np.sin(2 * np.pi * self.FUNDAMENTAL_HZ * t)).astype(np.float32)
        return t, sig

    def _add_file_info(self):
        """Add file information to the control panel"""
        import os
        filename = os.path.basename(self.wav_file)
        
        # Find the panel axes
        panel = None
        for ax in self.fig.axes:
            bbox = ax.get_position()
            if bbox.x0 == self._PANEL_L and bbox.width == self._PANEL_W:
                panel = ax
                break
        
        if panel:
            # Add file info text
            panel.text(0.5, 0.78, f"FILE: {filename[:20]}",
                       transform=panel.transAxes,
                       ha="center", va="top",
                       color=COLORS["green"], fontsize=7,
                       fontfamily="monospace")
            panel.text(0.5, 0.75, f"{len(self.input_signal)/self.fs:.1f}s @ {self.fs}Hz",
                       transform=panel.transAxes,
                       ha="center", va="top",
                       color=COLORS["subtext"], fontsize=7,
                       fontfamily="monospace")

    @abstractmethod
    def effect_subplots(self) -> list:
        """Return the 4 subplot definitions for standard view"""
        pass

    @abstractmethod
    def slider_definitions(self) -> list:
        pass

    @abstractmethod
    def apply_effect(self, in_sig: np.ndarray) -> np.ndarray:
        pass

    def _build_figure(self):
        subplot_defs = self.effect_subplots()
        assert len(subplot_defs) == 4, "effect_subplots() must return exactly 4 entries"
        slider_defs = self.slider_definitions()

        # Create figure
        self.fig = plt.figure(figsize=(18, 11), facecolor=COLORS["mantle"])

        # Create axes for plots (will be reconfigured based on view mode)
        self.axes = [plt.subplot(2, 2, i + 1) for i in range(4)]
        self.fig.subplots_adjust(
            left=0.055, right=self._PLOT_RIGHT,
            top=0.92,   bottom=0.08,
            hspace=0.42, wspace=0.34,
        )
        for ax in self.axes:
            _style_ax(ax)

        # Control panel background
        panel = self.fig.add_axes([self._PANEL_L, 0.03, self._PANEL_W, 0.94])
        panel.set_facecolor(COLORS["mantle"])
        panel.axis("off")
        for spine in panel.spines.values():
            spine.set_visible(True)
            spine.set_edgecolor(COLORS["surface1"])
            spine.set_linewidth(1.2)

        effect_name = type(self).__name__.replace("Analyzer", "")
        panel.text(0.5, 0.972, effect_name.upper(),
                   transform=panel.transAxes,
                   ha="center", va="top",
                   color=COLORS["text"], fontsize=11,
                   fontfamily="monospace", fontweight="bold")
        panel.text(0.5, 0.948, "─" * 20,
                   transform=panel.transAxes,
                   ha="center", va="top",
                   color=COLORS["surface2"], fontsize=9,
                   fontfamily="monospace")

        # View selector radio buttons
        self._add_view_selector(panel)

        # Figure title
        self.fig.text(
            (0.055 + self._PLOT_RIGHT) / 2, 0.965,
            f"{effect_name} Effect Analyzer",
            ha="center", va="top",
            color=COLORS["text"], fontsize=13,
            fontfamily="monospace", fontweight="bold",
        )

        # Knobs
        n = len(slider_defs)
        n_cols = min(3, n)
        pad = 0.012
        col_xs = [
            self._PANEL_L + pad + c * (self._PANEL_W - 2 * pad - self._KNOB_W) / max(n_cols - 1, 1)
            for c in range(n_cols)
        ]
        if n_cols == 1:
            col_xs = [self._PANEL_L + (self._PANEL_W - self._KNOB_W) / 2]

        self.knobs = {}
        for idx, sdef in enumerate(slider_defs):
            col_i = idx % n_cols
            row_i = idx // n_cols
            x = col_xs[col_i]
            y = self._ROW_TOP - row_i * self._ROW_STEP

            color = sdef.get("color", _KNOB_COLORS[idx % len(_KNOB_COLORS)])
            ax_k = self.fig.add_axes([x, y, self._KNOB_W, self._KNOB_H])

            knob = Knob(ax_k,
                       label=sdef["label"],
                       vmin=sdef["min"],
                       vmax=sdef["max"],
                       valinit=sdef["init"],
                       color=color)
            knob.on_changed(self._on_knob_change)
            self.knobs[sdef["attr"]] = knob

        # Reset button
        n_rows = (n + n_cols - 1) // n_cols
        btn_y = self._ROW_TOP - n_rows * self._ROW_STEP + 0.06
        btn_cx = self._PANEL_L + self._PANEL_W / 2
        ax_btn = self.fig.add_axes([btn_cx - 0.048, btn_y, 0.096, 0.038])
        self.reset_button = Button(ax_btn, "RESET",
                                   color=COLORS["surface1"],
                                   hovercolor=COLORS["surface2"])
        self.reset_button.label.set_color(COLORS["text"])
        self.reset_button.label.set_fontfamily("monospace")
        self.reset_button.label.set_fontsize(9)
        self.reset_button.on_clicked(self._on_reset)

        self._subplot_defs = subplot_defs
        self._slider_defs = slider_defs

    def _add_view_selector(self, panel):
        """Add radio buttons to select visualization mode"""
        # Position for radio buttons below the title
        radio_ax = self.fig.add_axes([self._PANEL_L + 0.05, 0.85, 0.18, 0.12])
        radio_ax.set_facecolor(COLORS["surface0"])
        
        self.view_radio = RadioButtons(
            radio_ax,
            ('Standard', 'Freq Response', 'Chromagram', 'Κρουστική Απόκριση'),  # Changed here
            active=0
        )
        
        # Style the radio buttons - handle different matplotlib versions
        try:
            for circle in self.view_radio.circles:
                circle.set_edgecolor(COLORS["text"])
                circle.set_facecolor(COLORS["blue"])
        except AttributeError:
            try:
                for rect in self.view_radio.rectangles:
                    rect.set_edgecolor(COLORS["text"])
                    rect.set_facecolor(COLORS["blue"])
            except AttributeError:
                pass
        
        # Style the labels
        for label in self.view_radio.labels:
            label.set_color(COLORS["text"])
            label.set_fontsize(7)
            label.set_fontfamily("monospace")
        
        self.view_radio.on_clicked(self._on_view_change)
        
        # Add description text
        panel.text(0.5, 0.82, "VIEW MODE",
                transform=panel.transAxes,
                ha="center", va="top",
                color=COLORS["subtext"], fontsize=7,
                fontfamily="monospace")

    def _on_view_change(self, label):
        """Handle view mode change"""
        if label == 'Standard':
            self.view_mode = 'standard'
        elif label == 'Freq Response':
            self.view_mode = 'freq_response'
        elif label == 'Chromagram':
            self.view_mode = 'chromagram'
        elif label == 'Κρουστική Απόκριση':  # Changed here
            self.view_mode = 'impulse_response'
        
        # Reconfigure subplot layout based on view mode
        self._reconfigure_layout()
        self._redraw()
    def _reconfigure_layout(self):
        """Reconfigure the subplot layout based on current view mode"""
        # Clear existing axes
        for ax in self.axes:
            ax.remove()
        
        if self.view_mode == 'standard':
            # 2x2 grid for standard view (4 panels)
            self.axes = []
            for i in range(4):
                ax = plt.subplot(2, 2, i + 1)
                self.axes.append(ax)
                _style_ax(ax)
            self.fig.subplots_adjust(
                left=0.055, right=self._PLOT_RIGHT,
                top=0.92, bottom=0.08,
                hspace=0.42, wspace=0.34
            )
        elif self.view_mode == 'impulse_response':  # Changed here
            # Single plot for impulse response view (1 panel)
            self.axes = []
            ax = plt.subplot(1, 1, 1)
            self.axes.append(ax)
            _style_ax(ax)
            self.fig.subplots_adjust(
                left=0.08, right=self._PLOT_RIGHT,
                top=0.92, bottom=0.08,
            )
        else:
            # 1x2 grid for frequency response and chromagram views (2 panels)
            self.axes = []
            for i in range(2):
                ax = plt.subplot(1, 2, i + 1)
                self.axes.append(ax)
                _style_ax(ax)
            self.fig.subplots_adjust(
                left=0.055, right=self._PLOT_RIGHT,
                top=0.92, bottom=0.08,
                hspace=0.3, wspace=0.3
            )
    def _on_knob_change(self, _val):
        changed = False
        for attr, knob in self.knobs.items():
            if hasattr(self, "effect") and hasattr(self.effect, attr):
                new_val = knob.val
                old_val = getattr(self.effect, attr)
                if abs(new_val - old_val) > 0.0001:
                    setattr(self.effect, attr, new_val)
                    changed = True
        
        if changed:
            if self._update_timer is not None:
                self._update_timer.stop()
                self._update_timer = None
            
            self._update_timer = self.fig.canvas.new_timer(50)
            self._update_timer.single_shot = True
            self._update_timer.add_callback(self._update_plots)
            self._update_timer.start()

    def _update_plots(self):
        """Update the plots based on current view mode"""
        self.output_signal = self.apply_effect(self.input_signal)
        
        if self.view_mode == 'standard':
            self._redraw_standard()
        elif self.view_mode == 'freq_response':
            self._redraw_freq_response()
        elif self.view_mode == 'chromagram':
            self._redraw_chromagram()
        elif self.view_mode == 'impulse_response':  # Changed here
            self._redraw_impulse_response()
        
        self.fig.canvas.draw_idle()
        self._update_timer = None
    def _redraw_standard(self):
        """Redraw with standard 4-panel view"""
        for ax, sdef in zip(self.axes, self._subplot_defs):
            ax.clear()
            sdef["draw"](ax,
                        in_sig=self.input_signal,
                        out_sig=self.output_signal,
                        t=self.t,
                        fs=self.fs,
                        effect=getattr(self, "effect", None))

    def _redraw_freq_response(self):
        """Redraw with frequency response view (2 panels: input FFT, output FFT)"""
        # Left panel: Input frequency spectrum
        draw_frequency_spectrum(self.axes[0], self.input_signal, self.fs, 
                               "Φάσμα Συχνοτήτων Εισόδου", COLORS["blue"])
        
        # Right panel: Output frequency spectrum
        draw_frequency_spectrum(self.axes[1], self.output_signal, self.fs,
                               "Φάσμα Συχνοτήτων Εξόδου", COLORS["red"])

    def _redraw_chromagram(self):
        """Redraw with chromagram view (2 panels: input chromagram, output chromagram)"""
        # Left panel: Input chromagram
        draw_chromagram(self.axes[0], self.input_signal, self.fs,
                       "Χρωμόγραμμα Εισόδου")
        
        # Right panel: Output chromagram
        draw_chromagram(self.axes[1], self.output_signal, self.fs,
                       "Χρωμόγραμμα Εξόδου")
        

    def _redraw_impulse_response(self):
        """Redraw with impulse response view (SINGLE plot showing impulse response)"""
        ax = self.axes[0]  # Single axis
        ax.clear()
        
        # Create impulse signal
        duration = 0.5  # 500ms impulse response
        impulse_len = int(self.fs * duration)
        impulse_sig = np.zeros(impulse_len, dtype=np.float32)
        impulse_sig[100] = 1.0  # Single impulse at 100 samples
        
        # Process the impulse through the effect
        if hasattr(self, 'effect') and hasattr(self.effect, 'process'):
            # Store original mix if it exists
            original_mix = None
            if hasattr(self.effect, 'mix'):
                original_mix = self.effect.mix
                self.effect.mix = 1.0  # 100% wet for impulse response
            
            impulse_response = self.effect.process(impulse_sig)
            
            # Restore original mix
            if original_mix is not None:
                self.effect.mix = original_mix
        else:
            impulse_response = impulse_sig
        
        # Plot impulse response
        time_ms = np.arange(len(impulse_response)) / self.fs * 1000
        ax.plot(time_ms, impulse_response, color=COLORS["teal"], lw=1.5, 
            alpha=0.9, label="Κρουστική Απόκριση")
        
        

        
        # Add parameter info
        info_text = ""
        if hasattr(self, 'effect'):
            if hasattr(self.effect, 'get_delay_time_ms'):  # Echo effect
                delay_ms = self.effect.get_delay_time_ms()
                feedback = self.effect.feedback
                mix = self.effect.mix
                info_text = f"Καθυστέρηση: {delay_ms:.1f} ms\nΑνάδραση: {feedback:.2f}\nΜίξη: {mix:.2f}"
                
                # Add vertical lines for echo repeats
                max_time = time_ms[-1]
                current_time = delay_ms
                repeat_count = 1
                while current_time < max_time:
                    ax.axvline(current_time, color=COLORS["surface2"], 
                            linestyle=':', alpha=0.3, lw=0.8)
                    # Add repeat number
                    if current_time + 50 < max_time:
                        ax.text(current_time, 0.8, f"{repeat_count}", 
                            ha='center', va='bottom', fontsize=7,
                            color=COLORS["surface2"], alpha=0.5)
                    current_time += delay_ms
                    repeat_count += 1
                    
            elif hasattr(self.effect, 'roomSize'):  # Reverb effect
                room_size = self.effect.roomSize
                damping = self.effect.damping
                mix = self.effect.mix
                info_text = f"Μέγεθος Δωματίου: {room_size:.2f}\nΑπόσβεση: {damping:.2f}\nΜίξη: {mix:.2f}"
                
                # Add RT60 estimate
                envelope_db = 20 * np.log10(envelope_smoothed + 1e-10)
                peak_db = np.max(envelope_db[:len(envelope_db)//4])
                rt60_time = 0
                for i, db in enumerate(envelope_db):
                    if db <= peak_db - 60:
                        rt60_time = time_ms[i]
                        break
                if rt60_time > 0:
                    ax.text(0.98, 0.85, f"RT60: {rt60_time:.0f} ms",
                        transform=ax.transAxes,
                        ha="right", va="top",
                        color=COLORS["yellow"],
                        fontsize=7,
                        family="monospace",
                        bbox=dict(boxstyle="round", facecolor=COLORS["surface0"], alpha=0.8))
        
        # Add info box
        if info_text:
            ax.text(0.98, 0.95, info_text,
                transform=ax.transAxes,
                ha="right", va="top",
                color=COLORS["subtext"],
                fontsize=8,
                family="monospace",
                bbox=dict(boxstyle="round", facecolor=COLORS["surface0"], alpha=0.8))
        
        ax.set_xlabel("Χρόνος (ms)")
        ax.set_ylabel("Πλάτος")
        ax.set_title("Κρουστική Απόκριση Εφέ")
        ax.legend(fontsize=7, loc='upper left')
        ax.grid(True, alpha=0.18, color=COLORS["surface2"])
        _style_ax(ax)

    def _on_reset(self, _event):
        for sdef in self._slider_defs:
            knob = self.knobs.get(sdef["attr"])
            if knob is not None:
                knob.set_val(sdef["init"])
        self._on_knob_change(None)

    def _redraw(self):
        """Redraw based on current view mode"""
        if self.view_mode == 'standard':
            self._redraw_standard()
        elif self.view_mode == 'freq_response':
            self._redraw_freq_response()
        elif self.view_mode == 'chromagram':
            self._redraw_chromagram()
        elif self.view_mode == 'impulse_response':  # Changed here
            self._redraw_impulse_response()
        self.fig.canvas.draw_idle()
    def run(self):
        plt.show()



# ---------------------------------------------------------------------------
# OverdriveAnalyzer
# ---------------------------------------------------------------------------

class OverdriveAnalyzer(EffectAnalyzer):

    def __init__(self, effect, wav_file=None, duration=None):
        self.effect = effect
        # Pass wav_file and duration to parent class
        super().__init__(wav_file=wav_file, duration=duration)

    def effect_subplots(self):
        return [
            {"draw": lambda ax, **kw: draw_waveform(ax, kw['t'], kw['in_sig'], kw['out_sig'])},
            {"draw": lambda ax, **kw: draw_harmonic_spectrum(ax, kw['in_sig'], kw['out_sig'], 
                                                             kw['fs'], self.FUNDAMENTAL_HZ)},
            {"draw": lambda ax, **kw: draw_spectrogram(ax, kw['in_sig'], kw['fs'], title="Φασματογράφημα Εισόδου")},
            {"draw": lambda ax, **kw: draw_spectrogram(ax, kw['out_sig'], kw['fs'], title="Φασματογράφημα Εξόδου")},
        ]

    def slider_definitions(self):
        e = self.effect
        return [
            {"label": "Gain",   "min": 0.0, "max": 1.0,
             "init": getattr(e, "gain",   0.1), "attr": "gain",   "color": COLORS["red"]},
            {"label": "Tone",   "min": 0.0, "max": 1.0,
             "init": getattr(e, "tone",   0.1), "attr": "tone",   "color": COLORS["peach"]},
            {"label": "Level",  "min": 0.0, "max": 1.0,
             "init": getattr(e, "level",  0.1), "attr": "level",  "color": COLORS["yellow"]},
        ]

    def apply_effect(self, in_sig):
        return self.effect.process(in_sig)
    
# ---------------------------------------------------------------------------
# CompressorAnalyzer (Updated with Greek labels and WAV support)
# ---------------------------------------------------------------------------

class CompressorAnalyzer(EffectAnalyzer):

    def __init__(self, effect, wav_file=None, duration=None):
        self.effect = effect
        # Pass wav_file and duration to parent class
        super().__init__(wav_file=wav_file, duration=duration)

    def effect_subplots(self):
        return [
            {"draw": lambda ax, **kw: draw_waveform(ax, kw['t'], kw['in_sig'], kw['out_sig'])},
            {"draw": lambda ax, **kw: self._draw_compression_curve(ax, **kw)},
            {"draw": lambda ax, **kw: draw_spectrogram(ax, kw['in_sig'], kw['fs'], title="Φασματογράφημα Εισόδου")},
            {"draw": lambda ax, **kw: draw_spectrogram(ax, kw['out_sig'], kw['fs'], title="Φασματογράφημα Εξόδου")},
        ]

    def _draw_compression_curve(self, ax, **kw):
        """Draw the compression transfer curve"""
        ax.clear()
        levels = np.linspace(0, 1, 200, dtype=np.float32)
        compressed = self.effect.process(levels)
        
        # Draw unity line
        ax.plot(levels, levels, "--", color=COLORS["text"], 
               alpha=0.35, label="Γραμμή Μονάδας")
        
        # Draw compression curve
        ax.plot(levels, compressed, color=COLORS["red"], lw=2, 
               label="Συμπίεση", alpha=0.9)
        
        # Mark threshold
        if hasattr(self.effect, 'threshold'):
            threshold = self.effect.threshold
            ax.axvline(threshold, color=COLORS["yellow"], 
                      linestyle='--', alpha=0.7, label=f'Κατώφλι: {threshold:.2f}')
            
            # Fill compressed region
            compressed_at_threshold = np.interp([threshold], levels, compressed)[0]
            ax.fill_between([threshold, 1], [threshold, 1], [compressed_at_threshold, 1],
                           color=COLORS["red"], alpha=0.15)
        
        ax.set_xlabel("Στάθμη Εισόδου")
        ax.set_ylabel("Στάθμη Εξόδου")
        ax.set_title("Καμπύλη Συμπίεσης")
        ax.legend(fontsize=8)
        ax.grid(True, alpha=0.18, color=COLORS["surface2"])
        _style_ax(ax)
        
        # Add info text
        if hasattr(self.effect, 'ratio') and hasattr(self.effect, 'makeup_gain'):
            ratio_val = self.effect.ratio
            makeup_val = self.effect.makeup_gain
            ax.text(0.98, 0.02, 
                   f"Λόγος: {ratio_val:.2f} | Ενίσχυση: {makeup_val:.2f}",
                   transform=ax.transAxes,
                   ha="right", va="bottom",
                   color=COLORS["subtext"],
                   fontsize=7,
                   family="monospace",
                   bbox=dict(boxstyle="round", facecolor=COLORS["surface0"], alpha=0.8))

    def slider_definitions(self):
        e = self.effect
        return [
            {"label": "Κατώφλι",  "min": 0.0, "max": 1.0,
             "init": e.threshold,   "attr": "threshold",   "color": COLORS["blue"]},
            {"label": "Λόγος",   "min": 0.0, "max": 1.0,
             "init": e.ratio,       "attr": "ratio",       "color": COLORS["peach"]},
            {"label": "Makeup",  "min": 0.0, "max": 1.0,
             "init": e.makeup_gain, "attr": "makeup_gain", "color": COLORS["yellow"]},
            {"label": "Επίθεση",  "min": 0.0, "max": 1.0,
             "init": e.attack,      "attr": "attack",      "color": COLORS["teal"]},
            {"label": "Απελευθ.", "min": 0.0, "max": 1.0,
             "init": e.release,     "attr": "release",     "color": COLORS["lavender"]},
        ]

    def apply_effect(self, in_sig):
        return self.effect.process(in_sig)
# ---------------------------------------------------------------------------
# EchoAnalyzer (Updated with signal vs trail visualization)
# ---------------------------------------------------------------------------

def draw_echo_tail(ax, effect, fs, duration=0.5):
    """Visualize the echo tail decay over time"""
    ax.clear()
    
    # Create an impulse signal to see the echo tail clearly
    impulse_len = int(fs * duration)
    impulse_sig = np.zeros(impulse_len, dtype=np.float32)
    impulse_sig[int(impulse_len * 0.1)] = 1.0  # Single impulse at 10% mark
    
    # Process the impulse through the echo
    if effect:
        tail_sig = effect.process(impulse_sig)
    else:
        tail_sig = impulse_sig
    
    # Plot the echo tail
    time_ms = np.arange(len(tail_sig)) / fs * 1000
    ax.plot(time_ms, tail_sig, color=COLORS["teal"], lw=1.5, label="Ουρά Ηχούς")
    
    # Mark the original impulse
    impulse_time = time_ms[int(impulse_len * 0.1)]
    ax.axvline(impulse_time, color=COLORS["yellow"], 
              linestyle='--', alpha=0.5, label='Αρχικός Παλμός')
    
    # Add info text
    if effect and hasattr(effect, 'get_delay_time_ms'):
        delay_ms = effect.get_delay_time_ms()
        feedback = effect.feedback
        mix = effect.mix
        
        ax.text(0.98, 0.95,
               f"Καθυστέρηση: {delay_ms:.1f} ms\n"
               f"Ανάδραση: {feedback:.2f}\n"
               f"Μίξη: {mix:.2f}",
               transform=ax.transAxes,
               ha="right", va="top",
               color=COLORS["subtext"],
               fontsize=7,
               family="monospace",
               bbox=dict(boxstyle="round", facecolor=COLORS["surface0"], alpha=0.8))
    
    ax.set_xlabel("Χρόνος (ms)")
    ax.set_ylabel("Πλάτος")
    ax.set_title("Κρουστική Απόκριση Ηχούς")
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.18, color=COLORS["surface2"])
    _style_ax(ax)


def draw_signal_vs_trail(ax, in_sig, out_sig, t, fs, effect):
    """Draw initial signal and echo trail in different colors"""
    ax.clear()
    
    # Calculate time array in milliseconds
    time_ms = t * 1000
    
    # Plot initial (dry) signal in blue
    ax.plot(time_ms, in_sig, color=COLORS["blue"], lw=1.5, 
           alpha=0.85, label="Αρχικό Σήμα (Dry)")
    
    # Plot echo trail (wet signal minus dry) in orange/red
    # For echo, the trail is the delayed/repeated parts
    if effect and hasattr(effect, 'process'):
        # Get just the wet signal (echo only) by processing with mix=1.0
        original_mix = effect.mix
        effect.mix = 1.0  # 100% wet
        wet_only = effect.process(in_sig)
        effect.mix = original_mix  # Restore original mix
        
        # Plot the trail (echo repeats)
        ax.plot(time_ms, wet_only, color=COLORS["peach"], lw=1.5, 
               alpha=0.9, label="Ουρά Ηχούς (Wet)")
        
        # Add vertical lines to show delay times
        if hasattr(effect, 'get_delay_time_ms'):
            delay_ms = effect.get_delay_time_ms()
            # Draw lines at delay intervals
            max_time = time_ms[-1]
            current_time = delay_ms
            while current_time < max_time:
                ax.axvline(current_time, color=COLORS["surface2"], 
                          linestyle=':', alpha=0.3, lw=0.8)
                current_time += delay_ms
            
            # Add text annotation
            ax.text(0.02, 0.95, f"Καθυστέρηση: {delay_ms:.0f} ms",
                   transform=ax.transAxes,
                   ha="left", va="top",
                   color=COLORS["subtext"],
                   fontsize=8,
                   family="monospace",
                   bbox=dict(boxstyle="round", facecolor=COLORS["surface0"], alpha=0.8))
    
    ax.set_xlabel("Χρόνος (ms)")
    ax.set_ylabel("Πλάτος")
    ax.set_title("Αρχικό Σήμα vs Ουρά Ηχούς")
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.18, color=COLORS["surface2"])
    _style_ax(ax)


class EchoAnalyzer(EffectAnalyzer):
    
    FS = 48000
    DURATION = 0.5 
    FUNDAMENTAL_HZ = 50
    
    def __init__(self, effect, wav_file=None, duration=None):
        self.effect = effect
        # Pass wav_file and duration to parent class
        super().__init__(wav_file=wav_file, duration=duration)
    
    def effect_subplots(self):
        return [
            {"draw": lambda ax, **kw: draw_waveform(ax, kw['t'], kw['in_sig'])},
            {"draw": lambda ax, **kw: draw_waveform(ax, kw['t'], kw['out_sig'])},
            {"draw": lambda ax, **kw: draw_spectrogram(ax, kw['in_sig'], kw['fs'], title="Φασματογράφημα Εισόδου")},
            {"draw": lambda ax, **kw: draw_spectrogram(ax, kw['out_sig'], kw['fs'], title="Φασματογράφημα Εξόδου")},
        ]
    
    def slider_definitions(self):
        e = self.effect
        return [
            {"label": "Καθυστ.",    "min": 0.0, "max": 1.0,
             "init": e.delayTime, "attr": "delayTime", "color": COLORS["teal"]},
            {"label": "Ανάδραση", "min": 0.0, "max": 0.95,
             "init": e.feedback,  "attr": "feedback",  "color": COLORS["peach"]},
            {"label": "Μίξη",      "min": 0.0, "max": 1.0,
             "init": e.mix,       "attr": "mix",       "color": COLORS["lavender"]},
        ]
    
    def apply_effect(self, in_sig):
        return self.effect.process(in_sig)
    
# ---------------------------------------------------------------------------
# NoiseGateAnalyzer (Updated with Greek labels and WAV support)
# ---------------------------------------------------------------------------

def draw_noisegate_curve(ax, effect):
    """Draw the noise gate transfer curve"""
    ax.clear()
    
    test_levels = np.linspace(0, 1, 500, dtype=np.float32)
    
    gated = np.zeros_like(test_levels)
    if effect and hasattr(effect, 'threshold'):
        thresh = effect.threshold
        for i, level in enumerate(test_levels):
            if level > thresh:
                gated[i] = level  
            else:
                gated[i] = 0.0     
    else:
        gated = test_levels
    
    # Draw unity line
    ax.plot(test_levels, test_levels, '--', color=COLORS["text"], 
           alpha=0.35, label="Γραμμή Μονάδας (Χωρίς Πύλη)")
    
    # Draw noise gate curve
    ax.plot(test_levels, gated, color=COLORS["red"], lw=2, 
           label="Πύλη Θορύβου (Σταθερή Κατάσταση)", alpha=0.9)
    
    # Fill gated region
    ax.fill_between(test_levels, 0, gated, where=(gated > 0),
                   color=COLORS["red"], alpha=0.15)
    
    # Mark threshold
    if effect and hasattr(effect, 'threshold'):
        ax.axvline(effect.threshold, color=COLORS["yellow"], 
                  linestyle='--', alpha=0.7, label=f'Κατώφλι: {effect.threshold:.2f}')
    
    ax.set_xlabel("Στάθμη Εισόδου")
    ax.set_ylabel("Στάθμη Εξόδου")
    ax.set_title("Καμπύλη Πύλης Θορύβου")
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.18, color=COLORS["surface2"])
    _style_ax(ax)
    
    # Add info text
    if effect and hasattr(effect, 'threshold'):
        ax.text(0.98, 0.02, 
               f"Η πύλη κλείνει κάτω από {effect.threshold:.2f}",
               transform=ax.transAxes,
               ha="right", va="bottom",
               color=COLORS["subtext"],
               fontsize=7,
               family="monospace",
               bbox=dict(boxstyle="round", facecolor=COLORS["surface0"], alpha=0.8))


def draw_gate_activity(ax, in_sig, out_sig, fs, effect):
    """Draw gate activity/state over time"""
    ax.clear()
    
    # Calculate gate state (simplified)
    window_size = int(fs / 100)  # 10ms windows
    num_windows = len(in_sig) // window_size
    
    gate_state = []
    times = []
    
    for i in range(num_windows):
        start = i * window_size
        end = (i + 1) * window_size
        window_in = in_sig[start:end]
        window_out = out_sig[start:end]
        
        # Gate is "active" when output is significantly different from input
        rms_in = np.sqrt(np.mean(window_in**2))
        rms_out = np.sqrt(np.mean(window_out**2))
        
        # If output is much lower than input, gate is reducing signal
        if rms_out < rms_in * 0.5 and rms_in > 0.01:
            state = 1.0  # Gate closed/active
        else:
            state = 0.0  # Gate open/inactive
        
        gate_state.append(state)
        times.append(i * window_size / fs)
    
    # Plot gate state
    ax.fill_between(times, gate_state, 0, color=COLORS["teal"], alpha=0.5, label="Πύλη Ενεργή")
    ax.plot(times, gate_state, color=COLORS["teal"], lw=1.5)
    
    ax.set_xlabel("Χρόνος (s)")
    ax.set_ylabel("Κατάσταση Πύλης")
    ax.set_title("Δραστηριότητα Πύλης Θορύβου")
    ax.set_ylim(-0.1, 1.1)
    ax.set_yticks([0, 1])
    ax.set_yticklabels(['Ανοιχτή', 'Κλειστή'])
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.18, color=COLORS["surface2"])
    _style_ax(ax)


class NoiseGateAnalyzer(EffectAnalyzer):
    
    FS = 48000  # Noise gate works well at 48kHz
    DURATION = 0.5
    
    def __init__(self, effect, wav_file=None, duration=None):
        self.effect = effect
        # Pass wav_file and duration to parent class
        super().__init__(wav_file=wav_file, duration=duration)
        if hasattr(effect, 'reset'):
            effect.reset()

    def apply_effect(self, in_sig):
        if hasattr(self.effect, 'reset'):
            self.effect.reset()
        return self.effect.process(in_sig)

    def effect_subplots(self):
        return [
            {"draw": lambda ax, **kw: draw_waveform(ax, kw['t'], kw['in_sig'], kw['out_sig'])},
            {"draw": lambda ax, **kw: draw_noisegate_curve(ax, kw['effect'])},
            {"draw": lambda ax, **kw: draw_spectrogram(ax, kw['in_sig'], kw['fs'], title="Φασματογράφημα Εισόδου")},
            {"draw": lambda ax, **kw: draw_spectrogram(ax, kw['out_sig'], kw['fs'], title="Φασματογράφημα Εξόδου")},
        ]

    def slider_definitions(self):
        e = self.effect
        return [
            {"label": "Κατώφλι",   "min": 0.0, "max": 1.0,
             "init": e.threshold,  "attr": "threshold", "color": COLORS["peach"]},
            {"label": "Επίθεση",   "min": 0.0, "max": 1.0,
             "init": e.attack,     "attr": "attack",    "color": COLORS["yellow"]},
            {"label": "Συγκράτηση","min": 0.0, "max": 1.0,
             "init": e.hold,       "attr": "hold",      "color": COLORS["teal"]},
            {"label": "Απελευθ.",  "min": 0.0, "max": 1.0,
             "init": e.release,    "attr": "release",   "color": COLORS["lavender"]},
        ]
    
# ---------------------------------------------------------------------------
# ReverbAnalyzer (Updated with signal vs reverb trail visualization)
# ---------------------------------------------------------------------------

def draw_comb_filter_response(ax, effect, fs):
    """Draw the frequency response of the Schroeder reverb comb filters"""
    ax.clear()
    
    if effect and hasattr(effect, 'get_comb_delays'):
        delays = effect.get_comb_delays()  # In samples
        feedback = effect.get_comb_feedback()
        
        freqs = np.linspace(0, fs / 2, 2000)
        combined_magnitude = np.ones(len(freqs))
        
        for delay_samples, fb in zip(delays, feedback):
            tau = delay_samples / fs
            omega = 2 * np.pi * freqs
            magnitude = 1.0 / np.sqrt(1 + fb**2 - 2 * fb * np.cos(omega * tau))
            combined_magnitude *= magnitude
        
        combined_magnitude_db = 20 * np.log10(combined_magnitude + 1e-10)
        
        ax.plot(freqs, combined_magnitude_db, color=COLORS["teal"], lw=1.5, label="Συνδυασμένη Απόκριση")
        
        # Plot individual comb filters (first 3 only for clarity)
        for delay_samples, fb in zip(delays[:3], feedback[:3]):
            tau = delay_samples / fs
            magnitude = 1.0 / np.sqrt(1 + fb**2 - 2 * fb * np.cos(2 * np.pi * freqs * tau))
            ax.plot(freqs, 20 * np.log10(magnitude + 1e-10), 
                color=COLORS["overlay0"], alpha=0.15, lw=0.8)
        
        delay_ms = [d / fs * 1000 for d in delays[:4]]
        ax.text(0.98, 0.95, 
            f"Φίλτρα Comb: {len(delays)}\n"
            f"Ανάδραση: {feedback[0]:.3f}",
            transform=ax.transAxes,
            ha="right", va="top",
            color=COLORS["subtext"],
            fontsize=7,
            family="monospace",
            bbox=dict(boxstyle="round", facecolor=COLORS["surface0"], alpha=0.8))
        
    else:
        roomSize = effect.roomSize if effect and hasattr(effect, 'roomSize') else 0.5
        delay_ms = 10.0 + roomSize * 90.0
        delay_samples = int(fs * delay_ms / 1000.0)
        feedback = 0.5 + roomSize * 0.4
        
        freqs = np.linspace(0, fs / 2, 1000)
        tau = delay_ms / 1000.0
        magnitude = 1.0 / np.sqrt(1 + feedback**2 - 2 * feedback * np.cos(2 * np.pi * freqs * tau))
        ax.plot(freqs, 20 * np.log10(magnitude + 1e-10), color=COLORS["teal"], lw=2)
        
        ax.text(0.98, 0.95,
            f"Καθυστέρηση: {delay_ms:.0f} ms\n"
            f"Ανάδραση: {feedback:.2f}",
            transform=ax.transAxes,
            ha="right", va="top",
            color=COLORS["subtext"],
            fontsize=7,
            family="monospace",
            bbox=dict(boxstyle="round", facecolor=COLORS["surface0"], alpha=0.8))
    
    ax.set_xlabel("Συχνότητα (Hz)")
    ax.set_ylabel("Ένταση (dB)")
    ax.set_title("Απόκριση Συχνότητας Αντήχησης (Φίλτρα Comb)")
    ax.grid(True, alpha=0.18, color=COLORS["surface2"])
    ax.set_ylim(-30, 30)
    ax.set_xlim(0, min(8000, fs / 2))
    _style_ax(ax)

 

class ReverbAnalyzer(EffectAnalyzer):
    
    FS = 48000  # Reverb often sounds better at 48kHz
    DURATION = 0.5
    
    def __init__(self, effect, wav_file=None, duration=None):
        self.effect = effect
        # Pass wav_file and duration to parent class
        super().__init__(wav_file=wav_file, duration=duration)
    
    def effect_subplots(self):
        return [
            {"draw": lambda ax, **kw: draw_waveform(ax, kw['t'], kw['in_sig'])},
            {"draw": lambda ax, **kw: draw_waveform(ax, kw['t'], kw['out_sig'])},
             {"draw": lambda ax, **kw: draw_spectrogram(ax, kw['in_sig'], kw['fs'], title="Φασματογράφημα Εισόδου")},
            {"draw": lambda ax, **kw: draw_spectrogram(ax, kw['out_sig'], kw['fs'], title="Φασματογράφημα Εξόδου")},
        ]
    
    def slider_definitions(self):
        e = self.effect
        return [
            {"label": "Μέγεθος", "min": 0.0, "max": 1.0,
             "init": e.roomSize, "attr": "roomSize", "color": COLORS["teal"]},
            {"label": "Απόσβεση",   "min": 0.0, "max": 1.0,
             "init": e.damping,   "attr": "damping",  "color": COLORS["peach"]},
            {"label": "Μίξη",       "min": 0.0, "max": 1.0,
             "init": e.mix,       "attr": "mix",      "color": COLORS["lavender"]},
        ]
    
    def apply_effect(self, in_sig):
        return self.effect.process(in_sig)