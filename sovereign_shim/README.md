# ⚜ Sovereign-Shim — Bare-Metal PQC Proxy Core

## NOT a demo. NOT a simulation. REAL production code.

This is the operational deployment engine for the Sovereign Protocol.
It intercepts real TCP traffic, signs data with actual liboqs PQC algorithms,
and adapts algorithm selection based on genuine Linux PMU hardware counters.

## Prerequisites

```bash
sudo apt install -y build-essential liboqs-dev
sudo sysctl kernel.perf_event_paranoid=-1  # Enable PMU counters
q
quit
