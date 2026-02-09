
# Currently on-going WIP project

**Milestones**

- Create user interface using bitmaps.
- Setup peripherals such as the display, buttons and QSPI.
- Enable ADC/DAC with periph-to-mem and mem-to-periph DMA support.
- Create effects (Distortion, Reverb, Echo and Noise gate).
- Add configurations for each pedal class.
- Create digital twin for testing the DSP library.
- Design circuit for pre-amplifying and filtering the signal.
- Design PCB.
- Design 3D casing.

**TO:DO**

- Rewrite HAL hardware initialization using LL/CMSIS.
- Add settings view.
- Add more effects.
- Save an effects chain.
- Generate ctypesgen types for other pedals types.


# Using DSP lib in Python

## Export macros

```bash
export DSP_PROJECT_ROOT="$(pwd)" \
export DSP_BUILD_DIR="$DSP_PROJECT_ROOT/Build" \
export DSP_SCRIPTS_DIR="$DSP_PROJECT_ROOT/Scripts" \
export DSP_CORE_DIR="$DSP_PROJECT_ROOT/Core/Effects"
```

## Build DSP

```bash
cmake -B "$DSP_BUILD_DIR" -DCMAKE_BUILD_TYPE=Release \
cmake --build "$DSP_BUILD_DIR"  --parallel $(nproc) 
``` 

## Generate ctypesgen

```bash
ctypesgen "$DSP_CORE_DIR/dsp.hpp" -l "$DSP_BUILD_DIR/libdsp_overdrive.so" -o "$DSP_SCRIPTS_DIR/dsp_bindings.py" --no-macro-warnings
```

## Run Python script

```bash
python "$DSP_SCRIPTS_DIR/simulateOverdrive.py"
```
