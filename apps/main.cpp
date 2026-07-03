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

int main() {
    std::cout << "================================================================\n";
    std::cout << "  SOVEREIGN PROTOCOL v2.0 - Full PQ Migration Matrix\n";
    std::cout << "  \"Quantifying the Cost of Sovereignty\"\n";
    std::cout << "================================================================\n\n";

    auto sys_info = sovereign::SystemInfo::collect();
    std::cout << "System: " << sys_info.cpu_model << " | " << sys_info.os_name << "\n\n";

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

    sovereign::MetricsCollector collector;
    collector.set_iterations(10, 50);
    auto suite = collector.run_full_suite(schemes);
    suite.suite_timestamp = sys_info.timestamp;
    suite.system_description = sys_info.cpu_model;

    for (const auto& r : suite.results) {
        std::cout << r.algorithm_name << ": Sign=" << r.sign_1kb.mean_ns/1e3 
                  << "us Verify=" << r.verify_1kb.mean_ns/1e3 
                  << "us Pub=" << r.public_key_bytes << "B Sig=" << r.signature_bytes << "B\n";
    }

    ::mkdir("results", 0755);
    sovereign::CsvExporter csv;
    sovereign::JsonExporter json;
    sovereign::MarkdownExporter md;
    csv.export_to_file(suite, "results/benchmark_results.csv");
    json.export_to_file(suite, "results/benchmark_results.json");
    md.export_to_file(suite, "results/benchmark_results.md");
    std::cout << "\nEvidence exported to results/\n";
    return 0;
}
