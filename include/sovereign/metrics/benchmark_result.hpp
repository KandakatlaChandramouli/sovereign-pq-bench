#pragma once

#include "sovereign/crypto/signature_scheme.hpp"
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace sovereign {

struct OperationMetrics {
    double mean_ns{0.0};
    double median_ns{0.0};
    double p95_ns{0.0};
    double p99_ns{0.0};
    double stddev_ns{0.0};
    double min_ns{0.0};
    double max_ns{0.0};
    uint64_t total_operations{0};
    uint64_t total_bytes{0};
    double ops_per_sec{0.0};
    double mb_per_sec{0.0};

    void compute(const std::vector<double>& latencies_ns, uint64_t bytes_processed = 0);
};

struct AlgorithmBenchmarkResult {
    Algorithm algorithm;
    std::string algorithm_name;
    
    OperationMetrics keygen;
    OperationMetrics sign_1b;
    OperationMetrics sign_1kb;
    OperationMetrics sign_1mb;
    OperationMetrics verify_1b;
    OperationMetrics verify_1kb;
    OperationMetrics verify_1mb;
    
    std::size_t public_key_bytes{0};
    std::size_t private_key_bytes{0};
    std::size_t signature_bytes{0};
    
    std::string cpu_model;
    std::string os_info;
    std::string compiler_version;
    std::string git_commit;
    std::string timestamp;

    [[nodiscard]] std::string to_csv_row() const;
    [[nodiscard]] std::string to_json() const;
    [[nodiscard]] std::string to_markdown() const;
};

struct BenchmarkSuite {
    std::vector<AlgorithmBenchmarkResult> results;
    std::string suite_timestamp;
    std::string system_description;

    [[nodiscard]] std::string to_csv() const;
    [[nodiscard]] std::string to_json() const;
    [[nodiscard]] std::string to_markdown() const;
    [[nodiscard]] std::string to_latex_table() const;
};

}
