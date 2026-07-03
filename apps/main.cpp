#include "sovereign/crypto/signature_scheme.hpp"
#include "sovereign/crypto/ecdsa_engine.hpp"
#include "sovereign/crypto/ed25519_engine.hpp"
#include "sovereign/crypto/ml_dsa_engine.hpp"
#include "sovereign/crypto/pq_engine.hpp"
#include "sovereign/metrics/metrics_collector.hpp"
#include "sovereign/metrics/benchmark_result.hpp"
#include "sovereign/metrics/csv_exporter.hpp"
#include "sovereign/metrics/json_exporter.hpp"
#include "sovereign/metrics/markdown_exporter.hpp"
#include "sovereign/metrics/system_info.hpp"
#include <sys/stat.h>
#include <iostream>
#include <memory>
#include <vector>
#include <fstream>

int main() {
    std::cout << "================================================================\n";
    std::cout << "  SOVEREIGN PROTOCOL v2.1 - God-Tier Evidence Engine\n";
    std::cout << "  \"Quantifying the Cost of Sovereignty\"\n";
    std::cout << "================================================================\n\n";

    auto sys_info = sovereign::SystemInfo::collect();
    std::cout << "System: " << sys_info.cpu_model << " | " << sys_info.os_name << "\n";
    std::cout << "Compiler: " << sys_info.compiler_name << " " << sys_info.compiler_version << "\n\n";

    // Configure benchmark
    sovereign::BenchmarkConfig config;
    config.warmup_iterations = 30;
    config.measured_iterations = 200;
    config.message_sizes = {64, 256, 1024, 8192, 65536};

    sovereign::MetricsCollector collector;
    collector.set_config(config);

    std::vector<std::unique_ptr<sovereign::SignatureScheme>> schemes;
    try {
        schemes.push_back(std::make_unique<sovereign::EcdsaEngine>());
        schemes.push_back(std::make_unique<sovereign::Ed25519Engine>());
        schemes.push_back(std::make_unique<sovereign::MlDsa44Engine>());
        schemes.push_back(std::make_unique<sovereign::MlDsa65Engine>());
        schemes.push_back(std::make_unique<sovereign::MlDsa87Engine>());
        schemes.push_back(std::make_unique<sovereign::Falcon512Engine>());
        schemes.push_back(std::make_unique<sovereign::Falcon1024Engine>());
        schemes.push_back(std::make_unique<sovereign::Sphincs128sEngine>());
        schemes.push_back(std::make_unique<sovereign::Sphincs128fEngine>());
        schemes.push_back(std::make_unique<sovereign::Sphincs192sEngine>());
        schemes.push_back(std::make_unique<sovereign::Sphincs256sEngine>());
        std::cout << schemes.size() << " algorithms loaded.\n\n";
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }

    ::mkdir("results", 0755);

    // Full CSV output
    std::ofstream csv_file("results/comprehensive_benchmarks.csv");
    csv_file << "algorithm,msg_size,sign_mean_ns,sign_median_ns,sign_p95_ns,sign_p99_ns,"
             << "sign_ci95_lower,sign_ci95_upper,sign_cv_pct,sign_skewness,"
             << "verify_mean_ns,verify_median_ns,verify_p95_ns,verify_p99_ns,"
             << "verify_ci95_lower,verify_ci95_upper,verify_cv_pct,verify_skewness,"
             << "sign_joules,sign_watts,verify_joules,verify_watts,"
             << "pub_key_bytes,sig_bytes,cpu_model,timestamp\n";

    // Summary table
    std::cout << "=== COMPREHENSIVE RESULTS (256B messages) ===\n\n";
    std::cout << std::left << std::setw(18) << "Algorithm"
              << std::right << std::setw(10) << "Sign(us)"
              << std::setw(10) << "P99(us)"
              << std::setw(20) << "95% CI"
              << std::setw(10) << "Vfy(us)"
              << std::setw(10) << "CV%"
              << std::setw(8) << "Key(B)"
              << std::setw(8) << "Sig(B)"
              << std::setw(12) << "Joules/Sig\n";
    std::cout << std::string(106, '-') << "\n";

    for (auto& scheme : schemes) {
        std::cout << "Benchmarking " << scheme->name() << "...\n";
        auto result = collector.run_comprehensive_benchmark(*scheme);

        // Write CSV rows
        for (auto& sr : result.size_results) {
            csv_file << result.algorithm_name << ","
                     << sr.message_size << ","
                     << std::fixed << std::setprecision(2)
                     << sr.sign_stats.mean_ns << ","
                     << sr.sign_stats.median_ns << ","
                     << sr.sign_stats.p95_ns << ","
                     << sr.sign_stats.p99_ns << ","
                     << sr.sign_stats.ci_95.lower << ","
                     << sr.sign_stats.ci_95.upper << ","
                     << sr.sign_stats.cv_percent << ","
                     << sr.sign_stats.skewness << ","
                     << sr.verify_stats.mean_ns << ","
                     << sr.verify_stats.median_ns << ","
                     << sr.verify_stats.p95_ns << ","
                     << sr.verify_stats.p99_ns << ","
                     << sr.verify_stats.ci_95.lower << ","
                     << sr.verify_stats.ci_95.upper << ","
                     << sr.verify_stats.cv_percent << ","
                     << sr.verify_stats.skewness << ","
                     << sr.sign_energy.joules << ","
                     << sr.sign_energy.avg_watts << ","
                     << sr.verify_energy.joules << ","
                     << sr.verify_energy.avg_watts << ","
                     << result.public_key_bytes << ","
                     << result.signature_bytes << ","
                     << result.cpu_model << ","
                     << result.timestamp << "\n";
        }

        // Print summary (256B message = index 2)
        if (result.size_results.size() > 2) {
            auto& sr = result.size_results[2];
            std::cout << std::left << std::setw(18) << result.algorithm_name
                      << std::right << std::setw(10) << std::fixed << std::setprecision(1) << sr.sign_stats.mean_ns / 1000.0
                      << std::setw(10) << sr.sign_stats.p99_ns / 1000.0
                      << std::setw(20) << sr.sign_stats.ci_95.to_string()
                      << std::setw(10) << sr.verify_stats.mean_ns / 1000.0
                      << std::setw(10) << std::setprecision(1) << sr.sign_stats.cv_percent
                      << std::setw(8) << result.public_key_bytes
                      << std::setw(8) << result.signature_bytes;
            if (sr.sign_energy.joules > 0) {
                std::cout << std::setw(12) << std::setprecision(4) << sr.sign_energy.joules * 1e6 << " µJ";
            } else {
                std::cout << std::setw(12) << "N/A";
            }
            std::cout << "\n";
        }
    }

    csv_file.close();

    std::cout << "\n================================================================\n";
    std::cout << "  EVIDENCE EXPORTED\n";
    std::cout << "  results/comprehensive_benchmarks.csv - God-tier truth data\n";
    std::cout << "================================================================\n";

    return 0;
}
