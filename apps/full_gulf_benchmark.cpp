#include "sovereign/metrics/statistical_analysis.hpp"
#include "sovereign/crypto/full_signature_engine.hpp"
#include "sovereign/crypto/full_kem_engine.hpp"
#include "sovereign/crypto/full_hash_engine.hpp"
#include "sovereign/utils/random_generator.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <span>

// ========== quick_sig with full statistics ==========
static void quick_sig(auto& engine, const char* standard, const char* family) {
{
    constexpr int WARMUP = 100;
    constexpr int ITERS  = 1000;
    std::vector<double> latencies_ns;
    latencies_ns.reserve(ITERS);
    auto kp = engine.generate_keypair();
    if (!kp.first) return;
    std::vector<std::byte> msg(256);
    for (int i = 0; i < WARMUP; ++i) (void)engine.sign(msg, kp.first->private_key);
    for (int i = 0; i < ITERS; ++i) {
        auto t1 = std::chrono::high_resolution_clock::now();
        (void)engine.sign(msg, kp.first->private_key);
        auto t2 = std::chrono::high_resolution_clock::now();
        latencies_ns.push_back(std::chrono::duration<double,std::nano>(t2-t1).count());
    }
    StatisticalAnalyzer analyzer;
    auto stats = analyzer.analyze(latencies_ns);
    std::cout << std::left << std::setw(24) << engine.name()
              << std::right
              << "  median:" << std::setw(8) << std::fixed << std::setprecision(1) << stats.median_ns/1000.0 << " us"
              << "  P95:"    << std::setw(8) << stats.p95_ns/1000.0 << " us"
              << "  P99:"    << std::setw(8) << stats.p99_ns/1000.0 << " us"
              << "  CV:"     << std::setw(6) << std::setprecision(1) << stats.cv_percent << "%"
              << "  95% CI: [" << stats.ci_95.lower/1000.0 << ", " << stats.ci_95.upper/1000.0 << "] us"
              << "  samples:" << stats.sample_count << "  " << family << "\n";
}

// ========== quick_kem with full statistics ==========
static KemResult quick_kem(sovereign::FullKemAlgorithm algo) {
{
    constexpr int WARMUP = 100;
    constexpr int ITERS  = 1000;
    std::vector<double> latencies_ns;
    latencies_ns.reserve(ITERS);
    sovereign::FullKemEngine kem(algo);
    auto kp = kem.generate_keypair();
    if (!kp.first) return {};
    std::vector<std::byte> msg(32);
    for (int i = 0; i < WARMUP; ++i) {
        auto enc = kem.encaps(kp.first->public_key);
        if (enc.first) (void)kem.decaps(enc.first->ct, kp.first->private_key);
    }
    for (int i = 0; i < ITERS; ++i) {
        auto t1 = std::chrono::high_resolution_clock::now();
        auto enc = kem.encaps(kp.first->public_key);
        auto t2 = std::chrono::high_resolution_clock::now();
        if (!enc.first) continue;
        latencies_ns.push_back(std::chrono::duration<double,std::nano>(t2-t1).count());
    }
    sovereign::StatisticalAnalyzer analyzer;
    auto stats = analyzer.analyze(latencies_ns);
    std::cout << std::left << std::setw(24) << kem.name()
              << "  median:" << std::setw(8) << std::fixed << std::setprecision(1) << stats.median_ns/1000.0 << " us"
              << "  P95:"    << std::setw(8) << stats.p95_ns/1000.0 << " us"
              << "  P99:"    << std::setw(8) << stats.p99_ns/1000.0 << " us"
              << "  CV:"     << std::setw(6) << std::setprecision(1) << stats.cv_percent << "%"
              << "  95% CI: [" << stats.ci_95.lower/1000.0 << ", " << stats.ci_95.upper/1000.0 << "] us"
              << "  samples:" << stats.sample_count << "\n";
    return {};
}

// ... rest of the file (main, etc.) remains unchanged ...
