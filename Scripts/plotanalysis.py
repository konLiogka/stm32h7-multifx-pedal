#!/usr/bin/env python3
"""
plotAnalysis.py
---------------
Reusable matplotlib analyzer framework for DSP effects.

Usage
-----
Subclass `EffectAnalyzer` and implement:
    - `effect_subplots(self)  -> list[dict]`   — describe the 4 subplot panels
    - `slider_definitions(self) -> list[dict]`  — describe the parameter knobs

Then call `EffectAnalyzer.run()` or instantiate and call `plt.show()` yourself.

See `CompressorAnalyzer` and `OverdriveAnalyzer` at the bottom for examples.
"""

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Arc, Circle
from matplotlib.widgets import Button
from scipy.signal import spectrogram as scipy_spectrogram
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

# Knob sweep: 270° of travel.
# In matplotlib, angles are CCW from east (0° = right).
# Minimum is bottom-left (~225°), maximum is bottom-right (~-45° = 315°).
_ANGLE_MIN_DEG =  225.0   # value = vmin
_ANGLE_MAX_DEG = -45.0    # value = vmax  (= 315° CCW, but stored as negative)


# ---------------------------------------------------------------------------
# Knob widget  (pure matplotlib, no Slider dependency)
# ---------------------------------------------------------------------------
class Knob:
    """
    Circular potentiometer knob rendered inside a small matplotlib Axes.

    Interaction
    -----------
    - Click-drag up/down to change value.
    - Scroll wheel for fine adjustment.
    """

    def __init__(self, ax, label, vmin=0.0, vmax=1.0, valinit=0.5, color=None, step=0.05):
        self.ax    = ax
        self.label = label
        self.vmin  = float(vmin)
        self.vmax  = float(vmax)
        self.step  = float(step)  # Step increment
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

    # ------------------------------------------------------------------
    # Quantization helper
    # ------------------------------------------------------------------
    
    def _quantize(self, value):
        """Round value to nearest step increment"""
        if self.step <= 0:
            return value
        # Calculate number of steps from vmin
        steps = round((value - self.vmin) / self.step)
        quantized = self.vmin + steps * self.step
        # Clamp to valid range
        return np.clip(quantized, self.vmin, self.vmax)

    # ------------------------------------------------------------------
    # Drawing
    # ------------------------------------------------------------------

    def _norm(self):
        return (self.val - self.vmin) / (self.vmax - self.vmin)

    def _angle_deg(self, norm=None):
        """Map normalised value → matplotlib angle (CCW from east, degrees)."""
        if norm is None:
            norm = self._norm()
        return _ANGLE_MIN_DEG + norm * (_ANGLE_MAX_DEG - _ANGLE_MIN_DEG)
    
    def _value_to_angle(self, value):
        """Convert a raw value to angle in degrees"""
        norm = (value - self.vmin) / (self.vmax - self.vmin)
        return self._angle_deg(norm)

    def _draw_anchor_lines(self, ax, radius=0.98):
        """Draw tick marks at each quantization step"""
        if self.step <= 0:
            return
        
        # Calculate number of steps
        num_steps = int((self.vmax - self.vmin) / self.step) + 1
        
        # Don't draw too many ticks (max 12 for readability)
        max_ticks = 12
        step_skip = max(1, num_steps // max_ticks)
        
        # Draw ticks at each step (or every nth step if too many)
        for i in range(0, num_steps, step_skip):
            value = self.vmin + i * self.step
            if value > self.vmax:
                break
            
            # Get angle for this value
            angle_deg = self._value_to_angle(value)
            angle_rad = np.deg2rad(angle_deg)
            
            # Tick length: longer for major steps (every 2nd or 5th)
            is_major = (i % max(2, step_skip * 2) == 0)
            tick_length = 0.12 if is_major else 0.07
            
            # Calculate start and end points of tick mark
            start_x = radius * np.cos(angle_rad)
            start_y = radius * np.sin(angle_rad)
            end_x = (radius - tick_length) * np.cos(angle_rad)
            end_y = (radius - tick_length) * np.sin(angle_rad)
            
            # Draw tick line
            ax.plot([start_x, end_x], [start_y, end_y],
                   color=COLORS["overlay2"], lw=0.8 if is_major else 0.5,
                   alpha=0.6, zorder=2)
            
          
                 

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

        # ── groove arc (full travel range, dark) ──────────────────────
        groove = Arc((0, 0), 2.1, 2.1, angle=0,
                     theta1=_ANGLE_MAX_DEG, theta2=_ANGLE_MIN_DEG,
                     color=COLORS["surface2"], lw=5, zorder=1,
                     )
        ax.add_patch(groove)

        # ── anchor lines (tick marks) ─────────────────────────────────
        self._draw_anchor_lines(ax, radius=1.02)

        # ── coloured fill arc (min → current position) ────────────────
        if norm > 0.001:
            fill = Arc((0, 0), 2.1, 2.1, angle=0,
                       theta1=angle, theta2=_ANGLE_MIN_DEG,
                       color=self.color, lw=5, zorder=2,
                       alpha=0.90)
            ax.add_patch(fill)

        # ── drop shadow ───────────────────────────────────────────────
        shadow = Circle((0.045, -0.045), 0.84,
                         color=COLORS["crust"], zorder=3, alpha=0.55)
        ax.add_patch(shadow)

        # ── knob body ─────────────────────────────────────────────────
        body = Circle((0, 0), 0.84, color=COLORS["surface1"], zorder=4)
        ax.add_patch(body)

        inner = Circle((0, 0), 0.74, color=COLORS["surface0"], zorder=5)
        ax.add_patch(inner)

        # ── specular highlight (top-left) ─────────────────────────────
        hl = Circle((-0.20, 0.24), 0.28,
                     color=COLORS["surface2"], alpha=0.32, zorder=6)
        ax.add_patch(hl)

        # ── indicator line + tip dot ──────────────────────────────────
        tip_x = 0.54 * np.cos(rad)
        tip_y = 0.54 * np.sin(rad)
        # dark halo (makes it pop on any bg)
        ax.plot([0, tip_x], [0, tip_y],
                color=COLORS["mantle"], lw=4.0,
                solid_capstyle="round", zorder=7)
        # coloured line
        ax.plot([0, tip_x], [0, tip_y],
                color=self.color, lw=3,
                solid_capstyle="round", zorder=8)
        # tip dot
        dot = Circle((tip_x * 1.18, tip_y * 1.18), 0.105,
                      color=self.color, zorder=9)
        ax.add_patch(dot)

        # ── label ─────────────────────────────────────────────────────
        ax.text(0, -1.28, self.label.upper(),
                ha="center", va="center",
                color=COLORS["text"], fontsize=8.5,
                fontfamily="monospace", fontweight="bold", zorder=10)

        # ── value readout ─────────────────────────────────────────────
        ax.text(0, -1.58, f"{self.val:.2f}",
                ha="center", va="center",
                color=self.color, fontsize=8,
                fontfamily="monospace", zorder=10)

    # ------------------------------------------------------------------
    # Interaction helpers
    # ------------------------------------------------------------------

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
        # Apply quantization
        new_val = self._quantize(raw_val)
        self._set_val(new_val)

    def _on_scroll(self, event):
        if not self._hit(event):
            return
        # Use step for scroll wheel
        new_val = np.clip(self.val + self.step * event.step, self.vmin, self.vmax)
        new_val = self._quantize(new_val)  # Ensure quantization
        self._set_val(new_val)

    def _set_val(self, val):
        # Already quantized, but double-check
        val = self._quantize(val)
        if abs(val - self.val) < 0.0001:  # Skip if effectively same
            return
        self.val = float(val)
        self._draw()
        self.ax.figure.canvas.draw_idle()
        for cb in self._cbs:
            cb(self.val)

    # ------------------------------------------------------------------
    # Public API
    # ------------------------------------------------------------------

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


def draw_waveform(ax, t, in_sig, out_sig, *,
                  label_in="Input", label_out="Output"):
    ax.clear()
    ax.plot(t * 1000, in_sig,  color=COLORS["blue"],  lw=1.5,
            label=label_in, alpha=0.85)
    ax.plot(t * 1000, out_sig, color=COLORS["red"],   lw=1.5,
            label=label_out)
    ax.set_xlabel("Time (ms)")
    ax.set_ylabel("Amplitude")
    ax.set_title("Waveform")
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.18, color=COLORS["surface2"])
    _style_ax(ax)


def draw_spectrogram(ax, signal, fs, *, title="Spectrogram", max_freq=5000):
    ax.clear()
    nperseg = min(256, len(signal) // 10)
    f, t_seg, Sxx = scipy_spectrogram(signal, fs, nperseg=nperseg,
                                      noverlap=nperseg // 2)
    mask = f <= max_freq
    im = ax.pcolormesh(t_seg, f[mask], 10 * np.log10(Sxx[mask] + 1e-10),
                       shading="gouraud", cmap="magma", vmin=-80, vmax=0)
    ax.set_xlabel("Time (s)")
    ax.set_ylabel("Frequency (Hz)")
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


def draw_io_curve(ax, x_levels, y_levels, *, title="I/O Curve",
                  xlabel="Input Level", ylabel="Output Level"):
    ax.clear()
    ax.plot(x_levels, x_levels, "--", color=COLORS["text"],
            alpha=0.35, label="Unity")
    ax.plot(x_levels, y_levels, color=COLORS["red"], lw=2, label="Effect")
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.set_title(title)
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.18, color=COLORS["surface2"])
    _style_ax(ax)


def draw_spectrum(ax, signal, fs, *, title="Frequency Spectrum",
                  color=None, max_freq=None):
    ax.clear()
    color   = color or COLORS["blue"]
    N       = len(signal)
    fft_mag = np.abs(np.fft.rfft(signal)) / N
    freqs   = np.fft.rfftfreq(N, 1 / fs)
    if max_freq:
        mask    = freqs <= max_freq
        freqs   = freqs[mask]
        fft_mag = fft_mag[mask]
    ax.semilogy(freqs, fft_mag + 1e-10, color=color, lw=1.2)
    ax.set_xlabel("Frequency (Hz)")
    ax.set_ylabel("Magnitude")
    ax.set_title(title)
    ax.grid(True, alpha=0.18, color=COLORS["surface2"])
    _style_ax(ax)


# ---------------------------------------------------------------------------
# Core analyzer base class
# ---------------------------------------------------------------------------

class EffectAnalyzer(ABC):
    """
    Base class for interactive DSP effect analyzers.

    Subclasses must implement
    --------------------------
    effect_subplots() -> list[dict]  (exactly 4 entries)
        Each dict: { "draw": callable(ax, *, in_sig, out_sig, t, fs, effect) }

    slider_definitions() -> list[dict]
        Each dict:
            "label" : str
            "min"   : float
            "max"   : float
            "init"  : float
            "attr"  : str    — attribute on self.effect to sync
            "color" : str    (optional)

    apply_effect(in_sig) -> np.ndarray
    """

    FS             = 44_100
    DURATION       = 0.1
    FUNDAMENTAL_HZ = 200
    AMPLITUDE      = 1

    # Layout constants (figure-fraction units)
    _PLOT_RIGHT = 0.67
    _PANEL_L    = 0.695
    _PANEL_W    = 0.285
    _KNOB_W     = 0.082
    _KNOB_H     = 0.195
    _ROW_TOP    = 0.68
    _ROW_STEP   = 0.245

    def __init__(self):
        self.t, self.input_signal = self._make_signal()
        self.output_signal = self.apply_effect(self.input_signal)
        
        self._spectrogram_cache = {}
        self._redraw_counter = 0
        self._last_params_hash = None
        self._update_timer = None
        self._pending_update = False  # NEW: track pending updates
        self._last_update_time = 0 
        
        self._build_figure()

    # ------------------------------------------------------------------
    # Abstract interface
    # ------------------------------------------------------------------

    @abstractmethod
    def effect_subplots(self) -> list:
        pass

    @abstractmethod
    def slider_definitions(self) -> list:
        pass

    @abstractmethod
    def apply_effect(self, in_sig: np.ndarray) -> np.ndarray:
        pass

    # ------------------------------------------------------------------
    # Signal generation
    # ------------------------------------------------------------------

    def _make_signal(self):
        t = np.arange(0, self.DURATION, 1 / self.FS, dtype=np.float32)
        n = len(t)
        s = n // 2
        env = np.zeros(n, dtype=np.float32)
        env[:s]    = 0.15
        env[s:]  = 0.8
        sig = (env * self.AMPLITUDE
               * np.sin(2 * np.pi * self.FUNDAMENTAL_HZ * t)).astype(np.float32)
        return t, sig

    # ------------------------------------------------------------------
    # Figure construction
    # ------------------------------------------------------------------

    def _build_figure(self):
        subplot_defs = self.effect_subplots()
        assert len(subplot_defs) == 4, "effect_subplots() must return exactly 4 entries"
        slider_defs = self.slider_definitions()

        # ── figure ────────────────────────────────────────────────────
        self.fig = plt.figure(figsize=(18, 11), facecolor=COLORS["mantle"])

        # ── 2×2 plot grid ─────────────────────────────────────────────
        self.axes = [plt.subplot(2, 2, i + 1) for i in range(4)]
        self.fig.subplots_adjust(
            left=0.055, right=self._PLOT_RIGHT,
            top=0.92,   bottom=0.08,
            hspace=0.42, wspace=0.34,
        )
        for ax in self.axes:
            _style_ax(ax)

        # ── control panel background ──────────────────────────────────
        panel = self.fig.add_axes(
            [self._PANEL_L, 0.03, self._PANEL_W, 0.94]
        )
        panel.set_facecolor(COLORS["mantle"])
        panel.axis("off")
        for spine in panel.spines.values():
            spine.set_visible(True)
            spine.set_edgecolor(COLORS["surface1"])
            spine.set_linewidth(1.2)

        # panel heading
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

        # ── figure title ──────────────────────────────────────────────
        self.fig.text(
            (0.055 + self._PLOT_RIGHT) / 2, 0.965,
            f"{effect_name} Effect Analyzer",
            ha="center", va="top",
            color=COLORS["text"], fontsize=13,
            fontfamily="monospace", fontweight="bold",
        )

        # ── knobs: up to 3 per row ────────────────────────────────────
        n      = len(slider_defs)
        n_cols = min(3, n)
        # evenly space columns inside the panel
        pad    = 0.012
        col_xs = [
            self._PANEL_L + pad + c * (self._PANEL_W - 2 * pad - self._KNOB_W) / max(n_cols - 1, 1)
            for c in range(n_cols)
        ]
        # if only 1 knob, centre it
        if n_cols == 1:
            col_xs = [self._PANEL_L + (self._PANEL_W - self._KNOB_W) / 2]

        self.knobs = {}
        for idx, sdef in enumerate(slider_defs):
            col_i = idx % n_cols
            row_i = idx // n_cols
            x = col_xs[col_i]
            y = self._ROW_TOP - row_i * self._ROW_STEP

            color = sdef.get("color", _KNOB_COLORS[idx % len(_KNOB_COLORS)])
            ax_k  = self.fig.add_axes([x, y, self._KNOB_W, self._KNOB_H])

            knob  = Knob(ax_k,
                         label=sdef["label"],
                         vmin=sdef["min"],
                         vmax=sdef["max"],
                         valinit=sdef["init"],
                         color=color)
            knob.on_changed(self._on_knob_change)
            self.knobs[sdef["attr"]] = knob

        # ── reset button ──────────────────────────────────────────────
        n_rows  = (n + n_cols - 1) // n_cols
        btn_y   = self._ROW_TOP - n_rows * self._ROW_STEP + 0.06
        btn_cx  = self._PANEL_L + self._PANEL_W / 2
        ax_btn  = self.fig.add_axes([btn_cx - 0.048, btn_y, 0.096, 0.038])
        self.reset_button = Button(ax_btn, "RESET",
                                   color=COLORS["surface1"],
                                   hovercolor=COLORS["surface2"])
        self.reset_button.label.set_color(COLORS["text"])
        self.reset_button.label.set_fontfamily("monospace")
        self.reset_button.label.set_fontsize(9)
        self.reset_button.on_clicked(self._on_reset)

        # ── store descriptors ─────────────────────────────────────────
        self._subplot_defs = subplot_defs
        self._slider_defs  = slider_defs

        # ── initial draw ──────────────────────────────────────────────
        self._redraw()

        

    # ------------------------------------------------------------------
    # Event handlers
    # ------------------------------------------------------------------
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
            # Cancel any pending timer
            if self._update_timer is not None:
                self._update_timer.stop()
                self._update_timer = None
            
            # Use a shorter delay for responsiveness
            self._update_timer = self.fig.canvas.new_timer(1000)  # 50ms delay
            self._update_timer.single_shot = True
            self._update_timer.add_callback(self._update_plots)
            self._update_timer.start()

    def _update_plots(self):
        """Update the plots - called from main thread by matplotlib timer"""
        # Recompute only what changed
        self.output_signal = self.apply_effect(self.input_signal)
        
        # Update only the plots that depend on the effect
        for ax, sdef in zip(self.axes, self._subplot_defs):
            ax.clear()  # Clear but don't redraw everything
            sdef["draw"](ax,
                        in_sig=self.input_signal,
                        out_sig=self.output_signal,
                        t=self.t,
                        fs=self.FS,
                        effect=getattr(self, "effect", None))
        
        self.fig.canvas.draw_idle()
        self._update_timer = None
    def _update_plots(self):
            """Update the plots - called from main thread by matplotlib timer"""
            self.output_signal = self.apply_effect(self.input_signal)
            self._redraw()
            self._update_timer = None

    def _on_reset(self, _event):
        for sdef in self._slider_defs:
            knob = self.knobs.get(sdef["attr"])
            if knob is not None:
                knob.set_val(sdef["init"])
        
        self._on_knob_change(None)

    # ------------------------------------------------------------------
    # Drawing with optimizations
    # ------------------------------------------------------------------

    def _get_spectrogram(self, signal, fs, title):
        """Cached spectrogram computation"""
        cache_key = (id(signal), fs, title)
        if cache_key not in self._spectrogram_cache:
            nperseg = min(256, len(signal) // 10)
            f, t_seg, Sxx = scipy_spectrogram(signal, fs, nperseg=nperseg,
                                              noverlap=nperseg // 2)
            self._spectrogram_cache[cache_key] = (f, t_seg, Sxx)
        return self._spectrogram_cache[cache_key]

    def _redraw(self):
        """Original redraw method - used for initial draw"""
        for ax, sdef in zip(self.axes, self._subplot_defs):
            sdef["draw"](ax,
                         in_sig=self.input_signal,
                         out_sig=self.output_signal,
                         t=self.t,
                         fs=self.FS,
                         effect=getattr(self, "effect", None))
        self.fig.canvas.draw_idle()

    # ------------------------------------------------------------------
    # Entry point
    # ------------------------------------------------------------------

    def run(self):
        plt.show()

# ---------------------------------------------------------------------------
# Pre-built subplot factories
# ---------------------------------------------------------------------------

def subplot_waveform():
    def draw(ax, *, in_sig, out_sig, t, fs, effect):
        draw_waveform(ax, t, in_sig, out_sig)
    return {"draw": draw}


def subplot_spectrogram_input():
    def draw(ax, *, in_sig, out_sig, t, fs, effect):
        draw_spectrogram(ax, in_sig, fs, title="Input Spectrogram")
    return {"draw": draw}


def subplot_spectrogram_output():
    def draw(ax, *, in_sig, out_sig, t, fs, effect):
        draw_spectrogram(ax, out_sig, fs, title="Output Spectrogram")
    return {"draw": draw}


def subplot_compression_curve():
    def draw(ax, *, in_sig, out_sig, t, fs, effect):
        levels     = np.linspace(0, 1, 200, dtype=np.float32)
        compressed = effect.process(levels) if effect else levels
        draw_io_curve(ax, levels, compressed, title="Compression Curve")
    return {"draw": draw}


def subplot_harmonic_spectrum(fundamental_hz=200):
    def draw(ax, *, in_sig, out_sig, t, fs, effect):
        ax.clear()
        N     = len(in_sig)
        freqs = np.fft.rfftfreq(N, 1 / fs)
        mask  = freqs <= 8000

        mag_in  = np.abs(np.fft.rfft(in_sig))  / N
        mag_out = np.abs(np.fft.rfft(out_sig)) / N

        ax.fill_between(freqs[mask], 1e-10, mag_out[mask] + 1e-10,
                        color=COLORS["peach"], alpha=0.15)
        ax.semilogy(freqs[mask], mag_in[mask]  + 1e-10,
                    color=COLORS["blue"],  lw=1.2, alpha=0.85, label="Input")
        ax.semilogy(freqs[mask], mag_out[mask] + 1e-10,
                    color=COLORS["peach"], lw=1.4, label="Output")

        for h in range(1, 9):
            hf = h * fundamental_hz
            if hf < 8000:
                ax.axvline(hf, color=COLORS["overlay0"],
                           lw=0.7, ls="--", alpha=0.55)

        ax.set_xlabel("Frequency (Hz)")
        ax.set_ylabel("Magnitude")
        ax.set_title("Harmonic Spectrum")
        ax.legend(fontsize=8)
        ax.grid(True, alpha=0.18, color=COLORS["surface2"])
        _style_ax(ax)
    return {"draw": draw}


def subplot_gain_reduction():
    def draw(ax, *, in_sig, out_sig, t, fs, effect):
        ax.clear()
        window = max(1, fs // 500)
        steps  = len(in_sig) // window
        times, gr = [], []
        for i in range(steps):
            sl      = slice(i * window, (i + 1) * window)
            rms_in  = np.sqrt(np.mean(in_sig[sl] ** 2))  + 1e-10
            rms_out = np.sqrt(np.mean(out_sig[sl] ** 2)) + 1e-10
            gr.append(20 * np.log10(rms_out / rms_in))
            times.append(t[i * window] * 1000)
        gr = np.array(gr)
        ax.fill_between(times, gr, 0, where=gr < 0,
                        color=COLORS["red"], alpha=0.35, label="GR")
        ax.plot(times, gr, color=COLORS["red"], lw=1.4)
        ax.axhline(0, color=COLORS["text"], lw=0.8, ls="--", alpha=0.35)
        ax.set_xlabel("Time (ms)")
        ax.set_ylabel("Gain (dB)")
        ax.set_title("Gain Reduction")
        ax.legend(fontsize=8)
        ax.grid(True, alpha=0.18, color=COLORS["surface2"])
        _style_ax(ax)
    return {"draw": draw}


def subplot_transfer_curve(title="Transfer Curve",
                           xlabel="Input", ylabel="Output"):
    def draw(ax, *, in_sig, out_sig, t, fs, effect):
        ax.clear()
        lo, hi = float(np.min(in_sig)), float(np.max(in_sig))
        ref    = np.linspace(lo, hi, 300, dtype=np.float32)
        ax.plot(ref, ref, "--", color=COLORS["text"], alpha=0.35, label="Unity")
        step = max(1, len(in_sig) // 2000)
        ax.scatter(in_sig[::step], out_sig[::step],
                   s=1.8, color=COLORS["peach"], alpha=0.55, label="Effect")
        ax.set_xlabel(xlabel)
        ax.set_ylabel(ylabel)
        ax.set_title(title)
        ax.legend(fontsize=8, markerscale=6)
        ax.grid(True, alpha=0.18, color=COLORS["surface2"])
        _style_ax(ax)
    return {"draw": draw}

def subplot_noisegate_curve():
    def draw(ax, *, in_sig, out_sig, t, fs, effect):
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
        
        if effect and hasattr(effect, 'threshold'):
            ax.axvline(effect.threshold, color=COLORS["yellow"], 
                      linestyle='--', alpha=0.7, label=f'Threshold: {effect.threshold:.2f}')
        
        ax.plot(test_levels, test_levels, '--', color=COLORS["text"], 
               alpha=0.35, label="Unity (No Gate)")
        ax.plot(test_levels, gated, color=COLORS["red"], lw=2, 
               label="Noise Gate (steady state)", alpha=0.9)
        
        ax.fill_between(test_levels, 0, gated, where=(gated > 0),
                       color=COLORS["red"], alpha=0.15)
        
        ax.set_xlabel("Input Level")
        ax.set_ylabel("Output Level")
        ax.set_title("Noise Gate Transfer Curve")
        ax.legend(fontsize=8)
        ax.grid(True, alpha=0.18, color=COLORS["surface2"])
        _style_ax(ax)
        
        if effect and hasattr(effect, 'threshold'):
            ax.text(0.98, 0.02, 
                   f"Gate closes below {effect.threshold:.2f}",
                   transform=ax.transAxes,
                   ha="right", va="bottom",
                   color=COLORS["subtext"],
                   fontsize=7,
                   family="monospace",
                   bbox=dict(boxstyle="round", facecolor=COLORS["surface0"], alpha=0.8))
    
    return {"draw": draw}


# ---------------------------------------------------------------------------
# CompressorAnalyzer
# ---------------------------------------------------------------------------

class CompressorAnalyzer(EffectAnalyzer):

    def __init__(self, effect):
        self.effect = effect
        super().__init__()

    def effect_subplots(self):
        return [
            subplot_waveform(),
            subplot_compression_curve(),
            subplot_spectrogram_input(),
            subplot_spectrogram_output(),
        ]

    def slider_definitions(self):
        e = self.effect
        return [
            {"label": "Thresh",  "min": 0.0, "max": 1.0,
             "init": e.threshold,   "attr": "threshold",   "color": COLORS["blue"]},
            {"label": "Ratio",   "min": 0.0, "max": 1.0,
             "init": e.ratio,       "attr": "ratio",       "color": COLORS["peach"]},
            {"label": "Makeup",  "min": 0.0, "max": 1.0,
             "init": e.makeup_gain, "attr": "makeup_gain", "color": COLORS["yellow"]},
            {"label": "Attack",  "min": 0.0, "max": 1.0,
             "init": e.attack,      "attr": "attack",      "color": COLORS["teal"]},
            {"label": "Release", "min": 0.0, "max": 1.0,
             "init": e.release,     "attr": "release",     "color": COLORS["lavender"]},
        ]

    def apply_effect(self, in_sig):
        return self.effect.process(in_sig)


# ---------------------------------------------------------------------------
# OverdriveAnalyzer
# ---------------------------------------------------------------------------

class OverdriveAnalyzer(EffectAnalyzer):

    def __init__(self, effect):
        self.effect = effect
        super().__init__()

    def effect_subplots(self):
        return [
            subplot_waveform(),
            subplot_harmonic_spectrum(self.FUNDAMENTAL_HZ),
            subplot_spectrogram_input(),
            subplot_spectrogram_output(),
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
# ReverbAnalyzer
# ---------------------------------------------------------------------------

def subplot_comb_filter_response():
        """Draw the frequency response of the Schroeder reverb comb filters"""
        def draw(ax, *, in_sig, out_sig, t, fs, effect):
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
                
                ax.plot(freqs, combined_magnitude_db, color=COLORS["teal"], lw=1.5)
                
                for delay_samples, fb in zip(delays[:3], feedback[:3]):
                    tau = delay_samples / fs
                    magnitude = 1.0 / np.sqrt(1 + fb**2 - 2 * fb * np.cos(2 * np.pi * freqs * tau))
                    ax.plot(freqs, 20 * np.log10(magnitude + 1e-10), 
                        color=COLORS["overlay0"], alpha=0.15, lw=0.8)
                
                delay_ms = [d / fs * 1000 for d in delays[:4]]
                ax.text(0.98, 0.95, 
                    f"Comb Filters: {len(delays)}\n"
                    f"Feedback: {fb:.3f}",
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
            
            ax.set_xlabel("Frequency (Hz)")
            ax.set_ylabel("Magnitude (dB)")
            ax.set_title("Reverb Frequency Response (Comb Filters)")
            ax.grid(True, alpha=0.18, color=COLORS["surface2"])
            ax.set_ylim(-30, 30)
            ax.set_xlim(0, min(8000, fs / 2))
            _style_ax(ax)
        
        return {"draw": draw}

class ReverbAnalyzer(EffectAnalyzer):
    
    FS = 48000  # Reverb often sounds better at 48kHz
    
    def __init__(self, effect):
        self.effect = effect
        super().__init__()
    
    def effect_subplots(self):
        return [
            subplot_waveform(),
            subplot_comb_filter_response(),  # Instead of compression curve
            subplot_spectrogram_input(),       # Show reverb decay
            subplot_spectrogram_output(),     # Show output spectrogram
        ]
    
    def slider_definitions(self):
        e = self.effect
        return [
            {"label": "Room Size", "min": 0.0, "max": 1.0,
             "init": e.roomSize, "attr": "roomSize", "color": COLORS["teal"]},
            {"label": "Damping",   "min": 0.0, "max": 1.0,
             "init": e.damping,   "attr": "damping",  "color": COLORS["peach"]},
            {"label": "Mix",       "min": 0.0, "max": 1.0,
             "init": e.mix,       "attr": "mix",      "color": COLORS["lavender"]},
        ]
    
    def apply_effect(self, in_sig):
        return self.effect.process(in_sig)
    
# ---------------------------------------------------------------------------
# NoiseGateAnalyzer
# ---------------------------------------------------------------------------

class NoiseGateAnalyzer(EffectAnalyzer):

    def __init__(self, effect):
        self.effect = effect
        super().__init__()
        if hasattr(effect, 'reset'):
            effect.reset()

    def apply_effect(self, in_sig):
        if hasattr(self.effect, 'reset'):
            self.effect.reset()
        return self.effect.process(in_sig)

    def effect_subplots(self):
        return [
            subplot_waveform(),
            subplot_noisegate_curve(),
            subplot_spectrogram_input(),
            subplot_spectrogram_output(),
        ]

    def slider_definitions(self):
        e = self.effect
        return [

            {"label": "Thresh",   "min": 0.0, "max": 1.0,
             "init": e.threshold,  "attr": "threshold", "color": COLORS["peach"]},
            {"label": "Attack",   "min": 0.0, "max": 1.0,
             "init": e.attack,     "attr": "attack",    "color": COLORS["yellow"]},
            {"label": "Hold",     "min": 0.0, "max": 1.0,
             "init": e.hold,       "attr": "hold",      "color": COLORS["teal"]},
            {"label": "Release",  "min": 0.0, "max": 1.0,
             "init": e.release,    "attr": "release",   "color": COLORS["lavender"]},
        ]
# ---------------------------------------------------------------------------
# EchoAnalyzer
# ---------------------------------------------------------------------------

def subplot_echo_tail():
    """Visualize the echo tail decay over time"""
    def draw(ax, *, in_sig, out_sig, t, fs, effect):
        ax.clear()
        
        # Create an impulse signal to see the echo tail clearly
        impulse_sig = np.zeros(len(in_sig), dtype=np.float32)
        impulse_sig[len(impulse_sig) // 4] = 1.0  # Single impulse
        
        # Process the impulse through the echo
        if effect:
            tail_sig = effect.process(impulse_sig)
        else:
            tail_sig = impulse_sig
        
        # Plot the echo tail
        time_ms = t * 1000
        ax.plot(time_ms, tail_sig, color=COLORS["teal"], lw=1.5, label="Echo Tail")
        
        # Mark the original impulse
        impulse_time = time_ms[len(impulse_sig) // 4]
        ax.axvline(impulse_time, color=COLORS["yellow"], 
                  linestyle='--', alpha=0.5, label='Original Impulse')
        
        # Add info text
        if effect and hasattr(effect, 'get_delay_time_ms'):
            delay_ms = effect.get_delay_time_ms()
            feedback = effect.feedback
            
            ax.text(0.98, 0.95,
                   f"Delay: {delay_ms:.1f} ms\n"
                   f"Feedback: {feedback:.2f}",
                   transform=ax.transAxes,
                   ha="right", va="top",
                   color=COLORS["subtext"],
                   fontsize=7,
                   family="monospace",
                   bbox=dict(boxstyle="round", facecolor=COLORS["surface0"], alpha=0.8))
        
        ax.set_xlabel("Time (ms)")
        ax.set_ylabel("Amplitude")
        ax.set_title("Echo Impulse Response")
        ax.legend(fontsize=8)
        ax.grid(True, alpha=0.18, color=COLORS["surface2"])
        _style_ax(ax)
    
    return {"draw": draw}


class EchoAnalyzer(EffectAnalyzer):
    
    FS = 48000
    DURATION = 0.5 
    FUNDAMENTAL_HZ = 50
    
    def __init__(self, effect):
        self.effect = effect
        super().__init__()
    
    def effect_subplots(self):
        return [
            subplot_waveform(),
            subplot_echo_tail(),
            subplot_spectrogram_input(),
            subplot_spectrogram_output(),
        ]
    
    def slider_definitions(self):
        e = self.effect
        return [
            {"label": "Delay",    "min": 0.0, "max": 1.0,
             "init": e.delayTime, "attr": "delayTime", "color": COLORS["teal"]},
            {"label": "Feedback", "min": 0.0, "max": 0.95,
             "init": e.feedback,  "attr": "feedback",  "color": COLORS["peach"]},
            {"label": "Mix",      "min": 0.0, "max": 1.0,
             "init": e.mix,       "attr": "mix",       "color": COLORS["lavender"]},
        ]
    
    def apply_effect(self, in_sig):
        return self.effect.process(in_sig)