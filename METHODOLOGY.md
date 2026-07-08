# Experimental Methodology

- **Hardware:** AMD EPYC 7763 64‑Core @ 2.45 GHz, 256 GB RAM
- **OS:** Ubuntu 24.04 LTS, Linux 6.8.0
- **Compiler:** GCC 13.3.0, flags: `-O3 -march=native -Wall -Wextra -Wpedantic`
- **Libraries:** OpenSSL 3.0.13, liboqs 0.10.0 (built with AVX2)
- **CPU state:** Frequency locked, Turbo Boost off, SMT enabled
- **Warmup:** 100 iterations per algorithm, discarded
- **Measurement:** 1000 timed iterations; high‑resolution clock
- **Statistics:** Bootstrap 95% CI (10k resamples), median, P95, P99, CV
- **PMU:** Not available in this virtualised environment (recorded as N/A)
- **Energy:** Not measured (host energy monitoring unavailable)
- **Memory:** Peak heap, allocation count recorded where possible; stack usage estimated
- **Reproducibility:** `./reproduce.sh` regenerates all results and figures

# Threats to Validity

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
