#!/bin/bash
#
# Sovereign Protocol - Full Evidence Pipeline
# "Quantifying the Cost of Sovereignty"
#
# This script executes the complete end-to-end pipeline:
# 1. Clean build
# 2. Compile with Release flags
# 3. Run the Truth Engine (benchmarks)
# 4. Run the unit tests
# 5. Run Google microbenchmarks
# 6. Generate publication-ready figures
# 7. Export all evidence artifacts
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"
RESULTS_DIR="$PROJECT_DIR/results"
FIGURES_DIR="$RESULTS_DIR/figures"

echo "============================================================"
echo "  SOVEREIGN PROTOCOL - Full Evidence Pipeline"
echo "  \"Quantifying the Cost of Sovereignty\""
echo "============================================================"
echo ""
echo "Project: $PROJECT_DIR"
echo "Build:   $BUILD_DIR"
echo "Results: $RESULTS_DIR"
echo ""

mkdir -p "$RESULTS_DIR" "$FIGURES_DIR"

echo "[1/6] Configuring CMake..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DBUILD_BENCHMARKS=ON
echo ""

echo "[2/6] Building the Truth Engine..."
cmake --build . -j$(nproc) --target sovereign_cli sovereign_bench test_ecdsa test_ml_dsa test_integration
echo ""

echo "[3/6] Running the Truth Engine (Main Benchmarks)..."
./sovereign_cli
echo ""

echo "[4/6] Running Unit Tests..."
ctest --output-on-failure
echo ""

echo "[5/6] Running Google Microbenchmarks..."
./sovereign_bench --benchmark_out="$RESULTS_DIR/microbenchmarks.json" --benchmark_out_format=json
echo ""

echo "[6/6] Generating Publication Figures..."
cd "$PROJECT_DIR"
python3 scripts/visualize_results.py "$RESULTS_DIR/benchmark_results.csv" "$FIGURES_DIR"
echo ""

echo "============================================================"
echo "  PIPELINE COMPLETE"
echo "============================================================"
echo ""
echo "Evidence artifacts generated:"
echo "  $RESULTS_DIR/benchmark_results.csv   - Raw Truth Data"
echo "  $RESULTS_DIR/benchmark_results.json  - Structured Evidence"
echo "  $RESULTS_DIR/benchmark_results.md    - Publication Tables"
echo "  $RESULTS_DIR/microbenchmarks.json    - Google Benchmarks"
echo "  $FIGURES_DIR/fig_*.svg               - Vector Graphics"
echo "  $FIGURES_DIR/fig_*.png               - Raster Graphics"
echo "  $FIGURES_DIR/fig_*.pdf               - PDF Graphics"
echo ""
echo "The Evidence is complete. The Quantum Tax is quantified."
echo "============================================================"
