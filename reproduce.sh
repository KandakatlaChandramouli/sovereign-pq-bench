#!/bin/bash
set -e
echo "=== Sovereign PQ Bench Reproducibility Pipeline ==="
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
cmake --build . -j$(nproc) --target gulf_bench hardware_bench
cd ..
mkdir -p results
./build/gulf_bench | tee results/gulf_bench.txt
./build/hardware_bench | tee results/hardware_bench.txt
python3 scripts/gulf_complete_visualizer.py
python3 scripts/gulf_100_visualizer.py
python3 scripts/pareto_frontiers.py
python3 scripts/case_study_impact.py
tar -czf sovereign_evidence_$(date +%Y%m%d).tar.gz results/
echo "✅ Evidence package created."
