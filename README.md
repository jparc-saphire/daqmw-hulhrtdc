# daqmw-hulhrtdc

DAQ-Middleware system for HUL-HRTDC readout, with setup scripts for AlmaLinux 10.

## System Overview

```
MznHRTdcReader1 → Merger → BestEffortDispatcher → DAQLogger
                                                 → Monitor
```

## Environment Setup

Tested on AlmaLinux 10.0 (Purple Lion) x86_64.

```bash
bash setup/setup.sh 2>&1 | tee /tmp/setup_log.txt
```

This script automatically builds and installs all dependencies:
omniORB 4.2.5, OpenRTM-aist 1.2.2, DAQ-Middleware libraries, TBB 2020.3, and all DAQ components.

## Usage

For detailed information and usage of this DAQ-MW system, refer to `daq_work/README`.

```bash
cd daq_work
run.py -l config_all.xml
```
