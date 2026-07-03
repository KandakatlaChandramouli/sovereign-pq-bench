# Sovereign Protocol: Post-Quantum Migration Truth Engine

> **"Quantifying the Cost of Sovereignty"**

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-green.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![OpenSSL](https://img.shields.io/badge/OpenSSL-3.x-red.svg)](https://www.openssl.org/)
[![liboqs](https://img.shields.io/badge/liboqs-0.10.0-orange.svg)](https://openquantumsafe.org/)

## Mission

We are building the **Truth Engine** that empirically measures the "Hardware Tax" of switching a nation's critical infrastructure from Legacy Cryptography (ECDSA-P256) to Post-Quantum Cryptography (ML-DSA-44/Dilithium).

This is not a theoretical exercise. This is the undeniable evidence required for Gulf Governments to make their $100B infrastructure decisions.

## The Contenders

| Algorithm | Type | Standard | Role |
|-----------|------|----------|------|
| **ECDSA-P256** | Classical | FIPS 186-4 | The Old Standard |
| **ML-DSA-44** | Post-Quantum | FIPS 204 | The New Sovereign Standard |

## Architecture

sovereign-pq-bench/
├── include/sovereign/
│ ├── crypto/ # Signature scheme abstractions
│ │ ├── signature_scheme.hpp
│ │ ├── ecdsa_engine.hpp
│ │ └── ml_dsa_engine.hpp
│ ├── metrics/ # Benchmarking & export
│ │ ├── benchmark_result.hpp
│ │ ├── metrics_collector.hpp
│ │ ├── csv_exporter.hpp
│ │ ├── json_exporter.hpp
│ │ └── markdown_exporter.hpp
│ └── utils/ # Timer, RNG
├── src/ # Implementation files
├── apps/ # CLI entry point
├── benchmarks/ # Google Benchmark harness
├── tests/ # Unit & integration tests
├── scripts/ # Python visualization
└── results/ # Evidence artifacts output
q
quit
