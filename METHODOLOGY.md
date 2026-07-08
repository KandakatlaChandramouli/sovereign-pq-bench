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
- **Reproducibility:** `./reproduce.sh` regenerates all results and figures
