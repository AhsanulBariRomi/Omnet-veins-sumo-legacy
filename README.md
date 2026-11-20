# Legacy Veins with TraCI TCP Socket Baseline

> **Master's Thesis Research Project**  
> **Author:** Md Ahsanul Bari ([md_ahsanul.bari@mailbox.tu-dresden.de](mailto:md_ahsanul.bari@mailbox.tu-dresden.de))  
> **Institution:** Technische Universität Dresden — Faculty of Computer Science / Chair of Networked Systems Modeling (CMS Labs)  
> **Academic Year:** 2026  
> **Remote Repository:** [GitLab TU Dresden](https://gitlab.mn.tu-dresden.de/mdba207f/thesis-veins-legacy)

---

## Overview

This repository contains the baseline implementation of the **Veins (Vehicular Network Simulation)** framework coupled with **Eclipse SUMO** via the standard **TraCI (Traffic Control Interface)** protocol over a TCP socket connection.

### Role in the Master's Thesis
In this research, the standard TraCI socket coupling serves as the **empirical reference baseline**. It is rigorously benchmarked against two modernized co-simulation architectures:
1. **`libsumo` (Direct In-Memory C++ Integration):** Embedding SUMO directly in OMNeT++ process memory to eliminate socket latency.
2. **`gRPC` (RPC / Microservices Architecture):** Decoupled, schema-driven serialization via Protocol Buffers over high-performance HTTP/2 streams.

All three systems are evaluated across communication round-trip latency (RTT), network payload, simulation throughput, CPU/memory footprints, and kinematic physical equivalence.

---


## Thesis Modifications & Instrumentation

To enable an objective, scientific comparison against the `libsumo` and `gRPC` architectures, this legacy codebase incorporates custom high-resolution logging probes:

1. **Microsecond Step Latency & Wire Payload Logger (`TraCIScenarioManager.cc:666–685`)**
   - Measures exact socket round-trip time ($RTT_{\mu s}$) for every simulation timestep using `std::chrono::high_resolution_clock`.
   - Records timestep, RTT ($\mu s$), active vehicle count, and socket wire payload size (bytes) to `Traci_RTT_<sim-time>.csv`.
2. **Kinematic Physical Validation Logger (`TraCIScenarioManager.cc:1192–1208`)**
   - Logs timestamp, vehicle ID, 3D coordinates $(X, Y, Z)$, speed, acceleration, vehicle dimensions, and signals to `Legacy_physics.csv`.
   - Used in post-processing to verify that vehicle kinematics across TraCI, Libsumo, and gRPC remain identical down to 14 significant digits.
3. **Dedicated Benchmark Parameter Sweeps (`omnetpp.ini`)**
   - Configured with identical scenario parameters, seed sets, and run limits across all three projects (`Throughput_Evaluation`, `Evaluation_Frequency`, and `Evaluation_Seeds`).

---

## Prerequisites & System Requirements

The framework is developed and tested on **Ubuntu Linux (20.04 / 22.04 / 24.04 LTS)** or **Windows Subsystem for Linux 2 (WSL2)**.

### 1. System Packages & Build Tools
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    bison \
    flex \
    perl \
    tcl-dev \
    tk-dev \
    blt \
    libxml2-dev \
    zlib1g-dev \
    python3 \
    python3-pip
```

### 2. Simulation Engines
- **OMNeT++:** Version `6.2.0` (compiled with `setenv` sourced)
- **Eclipse SUMO:** Version `1.20.0+` with TraCI support (`sumo` available in `PATH`)

---

## Build Instructions

Open your WSL/Ubuntu terminal and navigate to the Veins root directory:

### 1. Navigate to the Veins root directory
```bash
cd "F:/VEINS/veins-master"
# Or from WSL:
# cd "/mnt/f/VEINS/veins-master"
```

### 2. Activate the OMNeT++ Environment
```bash
source /path/to/omnetpp-6.2.0/setenv
# Example local path:
# source /mnt/f/Omnet/omnetpp-6.2.0/setenv
```

### 3. Compile the Veins Framework
```bash
./configure
make MODE=release -j$(nproc)
```

---

## Running Simulations

The standard TraCI coupling requires the SUMO launch daemon (`sumo-launchd.py`) to run in the background or a dedicated terminal.

### Step 1: Start the TraCI Launch Daemon (Terminal 1)
```bash
cd "veins-master"
python3 ./sumo-launchd.py -vv -c sumo
```
*The daemon will listen on TCP port `9999` and spawn SUMO on demand.*

---

### Step 2: Execute the Simulation (Terminal 2)

Navigate to the scenario directory:
```bash
cd "veins-master/examples/veins"
source /path/to/omnetpp-6.2.0/setenv
```

#### Headless / Command-line Mode (Fastest for Benchmarking)
```bash
opp_run -u Cmdenv -l ../../src/veins -n .:../../src/veins omnetpp.ini
```

#### GUI Visualizer Mode (Interactive Inspection)
```bash
opp_run -u Qtenv -l ../../src/veins -n .:../../src/veins omnetpp.ini
```

#### Throughput Evaluation Benchmark
Runs the dedicated throughput scenario and pipes the execution wall-clock time to CSV:
```bash
opp_run -u Cmdenv -l ../../src/veins -n .:../../src/veins omnetpp.ini -c Throughput_Evaluation | awk '/100% completed/ { gsub(/t=/, "", $4); gsub(/s/, "", $6); print $4 "," $6 }' > traci_throughput.csv
```

#### Frequency & Scaling Benchmark
Executes repetitions across vehicle traffic frequencies and extracts completion times:
```bash
opp_run -u Cmdenv -l ../../src/veins -n .:../../src/veins omnetpp.ini -c Evaluation_Frequency | awk 'BEGIN { print "SEED,FREQUENCY,TIME" } /Scenario:/ { for(i=1;i<=NF;i++) { if($i ~ /\$frequency=/) { f=$i; gsub(/\$frequency=|,/, "", f); } if($i ~ /\$repetition=/) { s=$i; gsub(/\$repetition=|,/, "", s); } } } /100% completed/ { gsub(/s/, "", $6); print s "," f "," $6; fflush(); }' > traci_frequency.csv
```

---

## Generated Metrics & Validation Logs

When running simulations, the following evaluation data files are generated in `examples/veins/`:

| Output File | Contents & Purpose |
| :--- | :--- |
| **`Legacy_physics.csv`** | High-precision timestep trajectory records (ID, $(X, Y, Z)$, Speed, Acceleration, Signals, Dimensions) used to validate physical equivalence against Libsumo and gRPC. |
| **`Traci_RTT_<sim-time>.csv`** | Timestep-by-timestep socket round-trip latency ($RTT_{\mu s}$), active vehicle counts, and TCP payload byte sizes. |
| **`traci_throughput.csv`** | Macro simulation speed (simulated seconds per real elapsed second) across scenario duration. |
| **`traci_frequency.csv`** | Wall-clock execution time under varying vehicle injection rates and seed repetitions. |

---

## Repository Structure

```
Legacy_Veins_Traci/
├── .gitignore                     # Git ignore rules (*.csv, git.txt, build junk, pcaps)
├── git.txt                        # Step-by-step GitLab deployment guide
├── README.md                      # Project documentation and deployment guide
└── veins-master/                  # Veins source code with TraCI baseline
    ├── configure                  # OMNeT++ build configuration script
    ├── Makefile                   # Top-level makefile
    ├── sumo-launchd.py            # Python TraCI TCP proxy daemon
    ├── bin/                       # Compiled helper binaries (veins_launchd)
    ├── src/veins/modules/mobility/traci/
    │   ├── TraCIScenarioManager.cc# TraCI scenario manager with RTT & validation probes
    │   ├── TraCIScenarioManager.h
    │   ├── TraCIConnection.cc     # TCP socket abstraction layer
    │   ├── TraCIConnection.h
    │   ├── TraCICommandInterface.cc
    │   └── TraCIMobility.cc       # Vehicle mobility and kinematic state tracking
    └── examples/veins/            # Simulation scenario configuration
        ├── omnetpp.ini            # Simulation configurations & evaluation parameter sweeps
        ├── erlangen.sumo.cfg      # SUMO scenario configuration
        ├── erlangen.net.xml       # Road network definition
        ├── erlangen.rou.xml       # Vehicle route demand definitions
        ├── erlangen.poly.xml      # Scenario obstacle polygons
        └── erlangen.launchd.xml   # Launch daemon configuration
```

---

## License & Attribution

Veins is an open-source vehicular network simulation framework licensed under **GPL-2.0-or-later** or **CC-BY-SA-4.0**. TraCI baseline instrumentation and benchmarking configurations were developed as part of Master's Thesis research at Technische Universität Dresden.
