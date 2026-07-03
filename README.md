# Sovereign-PQ-Bench: High-Assurance Cryptographic Profiling Framework

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Standard: C++23](https://img.shields.io/badge/Standard-C%2B%2B23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Precision: Nanosecond](https://img.shields.io/badge/Precision-Nanosecond-purple)]()

## 🏛️ Abstract
This framework provides a standardized methodology for measuring the **computational, energy, and latency penalties** of migrating Critical Infrastructure (CI) to Post-Quantum Cryptography (PQC). It specifically targets **resource-constrained edge environments** (UAV Swarms, SCADA, IoT) relevant to National Data Sovereignty mandates.

## ⚡ Key Capabilities
*   **Hybrid Toggling:** Hot-swapping between `ECDSA-P256` and `ML-DSA-44` (Dilithium).
*   **Energy Profiling:** Direct Joules/Op measurement for battery-critical systems.
*   **Network Simulation:** High-fidelity packet-loss modeling (0-50%) for desert/hostile environments.
*   **IEEE-Compliant Output:** Auto-generation of Vector PDF charts for publication.
