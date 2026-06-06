# MultiFX Guitar Pedal
*International Hellenic University, Department of Information and Electornic Engineering*

This is the main repository that includes the code for the STM32H743 firmware of my thesis, a multiFX guitar pedal. 
<img width="auto" height="600" alt="20260605_201607" src="https://github.com/user-attachments/assets/5100c489-ce6c-40aa-8c51-74102210a967" />

The project is located within the Core directory, separated by 4 directories. 
- The **Effects/** directory includes are pedal related files, classes, libraries and of course the Digital Signal Processing (DSP) library which includes all the virtual analog effect models. The DSP library utilizes known mathematical algorithms that describe an effect pedal, like the Schroeder Reverberator described in the 1960s.
- The **Modules/** directory includes all the core part of the interface, hardware initialisation and views.
- The **Startup/** directory includes the standard startup assembly init for the stm32h743 and also the animation that plays in the beginning.
- Lastly, the **Utility/** directory includes all the common files, dirvers and functions/symbols used by all the other directories, such as the display drivers.


## Simulating DSP library in Python

This part describes the process of running the DSP static library through the PC, without the need to use the STM32.

### Export macros

```bash
export DSP_PROJECT_ROOT="$(pwd)" 
export DSP_BUILD_DIR="$DSP_PROJECT_ROOT/Build" 
export DSP_SCRIPTS_DIR="$DSP_PROJECT_ROOT/Scripts" 
export DSP_CORE_DIR="$DSP_PROJECT_ROOT/Core/Effects"
```

### Build DSP

```bash
cmake -B "$DSP_BUILD_DIR" -DCMAKE_BUILD_TYPE=Release 
cmake --build "$DSP_BUILD_DIR"  --parallel $(nproc) 
``` 

### Generate ctypesgen

```bash
ctypesgen "$DSP_CORE_DIR/dsp.hpp" -l "$DSP_BUILD_DIR/libdsp_overdrive.so" -o "$DSP_SCRIPTS_DIR/dsp_bindings.py" --no-macro-warnings --cpp="g++ -E"
```

### Run Python script

```bash
python "$DSP_SCRIPTS_DIR/simulateOverdrive.py"
```
