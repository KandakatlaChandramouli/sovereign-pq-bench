#pragma once

#include "sovereign/metrics/benchmark_result.hpp"
#include "sovereign/metrics/statistical_analysis.hpp"
#include "sovereign/metrics/energy_meter.hpp"
#include "sovereign/crypto/signature_scheme.hpp"
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <vector>
#include <string>

namespace sovereign {

struct PerfCounters {
    uint64_t instructions{0};
    uint64_t cycles{0};
    uint64_t cache_misses{0};
    uint64_t cache_references{0};
    uint64_t branch_misses{0};
    uint64_t branch_instructions{0};
    double ipc{0.0};
    double cache_miss_rate{0.0};
    double branch_miss_rate{0.0};
};

struct BenchmarkConfig {
    uint64_t warmup_iterations{50};
    uint64_t measured_iterations{500};
    uint64_t repetition_count{3};
    std::vector<std::size_t> message_sizes{1, 64, 256, 1024, 8192, 65536, 1048576};
};

struct ComprehensiveResult {
    std::string algorithm_name;
    Algorithm algorithm;
    
    // Per-message-size results
    struct SizeResult {
        std::size_t message_size;
        StatisticalResult keygen_stats;
        StatisticalResult sign_stats;
        StatisticalResult verify_stats;
        EnergyReading sign_energy;
        EnergyReading verify_energy;
        PerfCounters sign_perf;
        PerfCounters verify_perf;
    };
    std::vector<SizeResult> size_results;
    
    // Aggregate
    StatisticalResult aggregate_keygen;
    StatisticalResult aggregate_sign;
    StatisticalResult aggregate_verify;
    double avg_sign_joules{0.0};
    double avg_verify_joules{0.0};
    double avg_sign_watts{0.0};
    double avg_verify_watts{0.0};
    
    // Key sizes
    std::size_t public_key_bytes{0};
    std::size_t private_key_bytes{0};
    std::size_t signature_bytes{0};
    
    // System provenance
    std::string cpu_model;
    std::string os_info;
    std::string compiler_version;
    std::string git_commit;
    std::string timestamp;
    
    [[nodiscard]] std::string to_csv() const;
    [[nodiscard]] std::string to_json() const;
    [[nodiscard]] std::string to_markdown_row() const;
};

class MetricsCollector {
public:
    MetricsCollector();
    ~MetricsCollector();
    
    void set_config(const BenchmarkConfig& config);
    
    [[nodiscard]] ComprehensiveResult run_comprehensive_benchmark(SignatureScheme& scheme);
    
    [[nodiscard]] static PerfCounters read_perf_counters();
    
private:
    BenchmarkConfig config_;
    StatisticalAnalyzer analyzer_;
    
    [[nodiscard]] std::vector<double> collect_latencies(
        const std::function<void()>& operation,
        uint64_t iterations);
    
    [[nodiscard]] std::vector<std::byte> generate_message(std::size_t size);
};

}
