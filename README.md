
# On going WIP project

**TO:DO**

- Rewrite HAL hardware initialization using LL/CMSIS
- Add settings
- Add crunch pedal
- Save effects chains
- Generate ctypesgen types for other pedals types




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
