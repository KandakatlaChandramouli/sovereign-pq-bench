#include "sovereign/crypto/signature_scheme.hpp"
#include "sovereign/crypto/ecdsa_engine.hpp"
#include "sovereign/crypto/ml_dsa_engine.hpp"
#include "sovereign/metrics/metrics_collector.hpp"
#include "sovereign/metrics/benchmark_result.hpp"
#include "sovereign/metrics/csv_exporter.hpp"
#include "sovereign/metrics/json_exporter.hpp"
#include "sovereign/metrics/markdown_exporter.hpp"
#include "sovereign/metrics/system_info.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <string>

int main() {
    std::cout << "============================================================\n";
    std::cout << "  SOVEREIGN PROTOCOL - Post-Quantum Migration Truth Engine\n";
    std::cout << "  \"Quantifying the Cost of Sovereignty\"\n";
    std::cout << "============================================================\n\n";

    auto sys_info = sovereign::SystemInfo::collect();
    std::cout << "System Information:\n" << sys_info.to_string() << "\n\n";

    std::vector<std::unique_ptr<sovereign::SignatureScheme>> schemes;
    try {
        std::cout << "Initializing ECDSA-P256 (The Old Standard)...\n";
        schemes.push_back(std::make_unique<sovereign::EcdsaEngine>());
        std::cout << "Initializing ML-DSA-44 (The New Sovereign Standard)...\n";
        schemes.push_back(std::make_unique<sovereign::MlDsaEngine>());
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }

    sovereign::MetricsCollector collector;
    collector.set_iterations(50, 500);
    std::cout << "\nRunning benchmarks (50 warmup, 500 measured)...\n\n";

    auto suite = collector.run_full_suite(schemes);
    suite.suite_timestamp = sys_info.timestamp;
    suite.system_description = sys_info.cpu_model;

    for (const auto& result : suite.results) {
        std::cout << "Algorithm: " << result.algorithm_name << "\n";
        std::cout << "  KeyGen:  " << result.keygen.mean_ns / 1e3 << " us\n";
        std::cout << "  Sign:    " << result.sign_1kb.mean_ns / 1e3 << " us\n";
        std::cout << "  Verify:  " << result.verify_1kb.mean_ns / 1e3 << " us\n";
        std::cout << "  PubKey:  " << result.public_key_bytes << " B\n";
        std::cout << "  Sig:     " << result.signature_bytes << " B\n\n";
    }

    sovereign::CsvExporter csv;
    sovereign::JsonExporter json;
    sovereign::MarkdownExporter md;

    bool ok = csv.export_to_file(suite, "../results/benchmark_results.csv");
    ok &= json.export_to_file(suite, "../results/benchmark_results.json");
    ok &= md.export_to_file(suite, "../results/benchmark_results.md");

    if (ok) {
        std::cout << "Evidence exported to ../results/\n";
    }

    if (suite.results.size() >= 2) {
        const auto& e = suite.results[0];
        const auto& m = suite.results[1];
        double tax = m.sign_1kb.mean_ns / std::max(e.sign_1kb.mean_ns, 1.0);
        std::cout << "\n=== THE QUANTUM TAX ===\n";
        std::cout << "Signing Tax: " << tax << "x\n";
        std::cout << "Key Size Tax: " << static_cast<double>(m.public_key_bytes) / e.public_key_bytes << "x\n";
        std::cout << "Sig Size Tax: " << static_cast<double>(m.signature_bytes) / e.signature_bytes << "x\n";
    }

    return 0;
}
