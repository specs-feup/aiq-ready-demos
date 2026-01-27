# Co-Simulation with Socsim Generator

This guide explains how to run the MAC-NVISION test on an external high-level simulator that simulates the behaviour of NVISION's MAC accelerator.

## Prerequisites

- Java Runtime Environment (JRE) 11 or later
- Verilator
- RISC-V toolchain (`rv32imfc`)
- Python 3.8+ (for virtual environment)

## Setup

### 1. Clone X-Heep Repository

Clone the X-Heep repository (tested with commit 7078bf6):
```bash
git clone https://github.com/esl-epfl/x-heep.git
cd x-heep
git checkout 7078bf6
``` 

Follow [X-Heep's setup guide](https://x-heep.readthedocs.io/en/latest/GettingStarted/Setup.html#manual-setup) to configure the SoC (**Note:** Do not use Docker for this setup).

### 2. Clone Socsim Libs:

In a separate folder:

```bash
git clone https://github.com/specs-feup/socsim-libs
```

Follow the repository's instructions to install the needed libraries.

### 3. Clone Socsim Generator:

In a third folder:

```bash
git clone https://github.com/specs-feup/socsim-generator.git
cd socsim-generator
```

### 4. Configure the JSON File

Create or modify the configuration file for XIF (eXtension InterFace). The configuration for the MAC_NVISION accelerator is provided in this folder (`config.json`).

**Important:** Update `x_heep_dir` and `output_dir` to match your local paths.

### 5. Run Socsim Generator

```bash
./gradlew build
./gradlew run
```

After running, you should see the following structure:
```
socsim-generator/
└── examples/
    └── nvision-mac/
        ├── base_nvision_mac.hpp
        ├── CMakeLists.txt
        ├── nvision_mac.cpp
        └── README.md
```

Replace the generated `nvision_mac.cpp` and `base_nvision_mac.h` with the provided implementations from this demo folder.

In your X-Heep, there should be two new folders:
```
x-heep/
├── hw/
│   └── ip_examples/
│       └── nvision_mac/
└── sw/
    └── applications/
        └── nvision_mac/
```

Replace the generated `sw/applications/nvision_mac/main.c` with the provided `main.c` file provided in this demo folder.

You will also need to link some libraries (`libcipc`, `libcmsg`, `libzmq`, `libmsgpackc`) when building the simulation. 

Open the X-Heep `Makefile` and locate the `verilator-sim` target. Replace it with the following to include the required linker flags:
```makefile
verilator-sim:
    $(FUSESOC) --cores-root . run --no-export --target=sim --tool=verilator $(FUSESOC_FLAGS) --build openhwgroup.org:systems:core-v-mini-mcu ${FUSESOC_PARAM} --verilator_options "-LDFLAGS \"-lcipc -lcmsg -lzmq -lmsgpackc\"" 2>&1 | tee buildsim.log
```

**Note:** Use tabs (not spaces) for the indentation in Makefiles.


### 6. Run the High-Level Simulator

Follow the instructions in `examples/nvision_mac/README.md` to start the simulator.

### 7. Build and Run on X-Heep

Navigate to your X-Heep directory and run:

```bash
# Optional: Create and activate a virtual environment
python3 -m venv venv
source venv/bin/activate

# Generate the MCU configuration
make mcu-gen CPU=cv32e40px MEMORY_BANKS=12 MEMORY_BANKS_IL=4 BUS=NtoM MCU_CFG=mcu_cfg.hjson

# Build the Verilator simulation with X_EXT enabled
make verilator-sim FUSESOC_PARAM="--X_EXT=1"

# Compile the NVISION application
make app PROJECT=nvision_mac ARCH=rv32imfc

# Run the simulation
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator && ./Vtestharness +firmware=../../../sw/build/main.hex
```

## Expected Output

You should see the MAC operation results printed in the terminal, confirming successful co-simulation between X-Heep and the NVISION accelerator. This process might take some time.

## Documentation
- [Socsim Generator Documentation](https://github.com/specs-feup/socsim-generator/blob/0c229876ae9c8d18f4dab54c18105a7fb9d9de9e/README.md)
- [X-Heep Documentation](https://github.com/esl-epfl/x-heep)