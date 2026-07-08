

## Internal Validity
- All benchmarks run on a single machine; background processes were minimised but not eliminated.
- Virtualisation overhead may affect timing precision; PMU counters are unavailable.
- Compiler optimisations are fixed to `-O3 -march=native`; results may differ with other compilers.

## External Validity
- Results are specific to AMD EPYC 7763; other microarchitectures (Intel, ARM, Apple Silicon) may exhibit different performance profiles.
- Only liboqs 0.10.0 was tested; future updates may alter algorithm implementations.
- Network MTU analysis assumes standard 1500‑byte Ethernet; other link layers may differ.

## Construct Validity
- Latency is measured as wall‑clock time, which includes library overhead, not solely the cryptographic operation.
- Throughput calculations assume full CPU utilisation; real‑world workloads will vary.
- Memory metrics rely on `mallinfo()` which is deprecated and may not capture all allocations.

## Reproducibility
- All raw data, figures, and build scripts are included in the repository.
- One‑command reproduction is provided (`./reproduce.sh`).
- Exact environment can be recreated using the provided Dockerfile.
