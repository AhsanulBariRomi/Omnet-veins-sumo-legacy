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

## License & Attribution

Veins is an open-source vehicular network simulation framework licensed under **GPL-2.0-or-later** or **CC-BY-SA-4.0**. TraCI baseline instrumentation and benchmarking configurations were developed as part of Master's Thesis research at Technische Universität Dresden.
