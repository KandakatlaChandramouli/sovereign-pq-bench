#!/bin/bash
# ╔══════════════════════════════════════════════════════════════╗
# ║  SOVEREIGN PROTOCOL — One-Command Full Evidence Pipeline    ║
# ║  Builds everything, runs all benchmarks, generates figures   ║
# ╚══════════════════════════════════════════════════════════════╝
set -e
echo "⚜ Sovereign Protocol — Full Pipeline"
echo "====================================="
cd "$(dirname "$0")"

# 1. Build all engines
echo "[1/6] Building C++ benchmarking engines..."
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF 2>&1 | tail -3
cmake --build . -j$(nproc) --target gulf_bench hardware_bench 2>&1 | tail -5

# 2. Run Gulf Bench
echo "[2/6] Running Gulf Bench (20 algorithms)..."
./gulf_bench 2>&1 | head -30

# 3. Run Hardware Bench
echo "[3/6] Running Hardware Bench (PMU + MTU)..."
./hardware_bench 2>&1 | head -25

# 4. Run tests
echo "[4/6] Running unit tests..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DCMAKE_CXX_FLAGS="-I$(pwd)/_deps/liboqs-build/include" 2>&1 | tail -3
cmake --build . -j$(nproc) --target test_all 2>&1 | tail -3
./tests/test_all 2>&1 | tail -10

# 5. Generate figures
echo "[5/6] Generating publication figures..."
cd ..
python3 scripts/gulf_complete_visualizer.py 2>&1 | grep "Figure"
python3 scripts/gulf_100_visualizer.py 2>&1 | grep "Figure"

# 6. Build WRAITH tools
echo "[6/6] Building WRAITH field tools..."
cd wraith_ebpf
make clean 2>/dev/null; make 2>&1 | tail -5
cd ..

echo ""
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  ✅ FULL PIPELINE COMPLETE                                  ║"
echo "║  Results: results/                                          ║"
echo "║  Figures: results/figures/                                  ║"
echo "║  WRAITH:  wraith_ebpf/                                      ║"
echo "╚══════════════════════════════════════════════════════════════╝"
