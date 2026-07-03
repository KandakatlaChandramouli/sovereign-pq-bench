<p align="center">
  <img src="https://img.shields.io/badge/SOVEREIGN-PROTOCOL-black?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/C++20-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white"/>
  <img src="https://img.shields.io/badge/11_Algorithms-NIST_PQC-FF6F00?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/IEEE-Artifact_Ready-00629B?style=for-the-badge&logo=ieee&logoColor=white"/>
</p>

# Sovereign Protocol v2.0

> **"Quantifying the Cost of Sovereignty"**
>
> A Post-Quantum Migration Truth Engine for Critical Infrastructure

---

## Abstract

National infrastructure faces an existential cryptographic migration. The transition from classical signature schemes (ECDSA, Ed25519) to NIST-standardized post-quantum cryptography (ML-DSA, Falcon, SPHINCS+) imposes a **hardware tax** on resource-constrained systems — drone swarms, oil pipeline sensors, and SCADA controllers.

**The Sovereign Protocol** is an open-source, production-grade benchmarking framework that empirically quantifies this tax across the **full NIST PQC portfolio** of 11 algorithms.

---

## The Contenders

| Class | Algorithm | Security | Sign (us) | Verify (us) | Pub Key (B) | Sig (B) |
|:---:|:---|:---:|:---:|:---:|:---:|:---:|
| Classical | **Ed25519** | 128-bit | 131 | 171 | 32 | 64 |
| PQ | **ML-DSA-44** | 128-bit | 100 | 39 | 1,312 | 2,420 |
| PQ | ML-DSA-65 | 192-bit | 171 | 68 | 1,952 | 3,309 |
| PQ | ML-DSA-87 | 256-bit | 214 | 98 | 2,592 | 4,627 |
| PQ | **Falcon-512** | 128-bit | 352 | 63 | 897 | 656 |
| PQ | Falcon-1024 | 256-bit | 736 | 130 | 1,793 | 1,270 |
| PQ | SPHINCS+-128s | 128-bit | 228,243 | 362 | 32 | 7,856 |
| PQ | SPHINCS+-256s | 256-bit | 400,696 | 758 | 64 | 29,792 |

---

## Key Findings

### The Quantum Tax

| Metric | Winner | Value |
|:---|:---|:---:|
| **Fastest Verification** | ML-DSA-44 | 39 us |
| **Smallest On-Wire (PQ)** | Falcon-512 | 1,553 bytes total |
| **Smallest Public Key** | SPHINCS+ | 32 bytes |
| **Maximum Assurance** | SPHINCS+-256s | Hash-based, stateless |
| **Tax Factor** | 0.76x — 3,048x | Depends on algorithm choice |

> **Conclusion:** ML-DSA-44 is the best general-purpose PQ signature (faster than Ed25519). Falcon-512 wins for bandwidth-constrained channels. SPHINCS+ is for conservative, long-term sovereign key attestation.

---

## Quick Start

```bash
git clone https://github.com/KandakatlaChandramouli/sovereign-pq-bench.git
cd sovereign-pq-bench
sudo apt install -y build-essential cmake libssl-dev libgtest-dev libbenchmark-dev
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
./sovereign_cli
./tests/test_all
python3 ../scripts/visualize_results.py
```

---

## Citation

```bibtex
@software{sovereign_protocol_2026,
  author = {Kandakatla, Chandramouli},
  title = {Sovereign Protocol: Quantifying the Cost of Sovereignty},
  year = {2026},
  version = {2.0.0},
  url = {https://github.com/KandakatlaChandramouli/sovereign-pq-bench},
  note = {IEEE Artifact Evaluation Ready. 11 NIST PQC algorithms benchmarked.}
}
```

---

> *"You cannot negotiate with physics. But you can measure it."*
>
> Made with dedication by **Chandramouli Kandakatla** • Parul University • 2026
