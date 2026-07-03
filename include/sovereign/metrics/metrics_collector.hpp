#pragma once

#include "sovereign/metrics/benchmark_result.hpp"
#include "sovereign/crypto/signature_scheme.hpp"
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <vector>

namespace sovereign {

class MetricsCollector {
public:
    MetricsCollector();
    ~MetricsCollector();

    MetricsCollector(const MetricsCollector&) = delete;
    MetricsCollector& operator=(const MetricsCollector&) = delete;
    MetricsCollector(MetricsCollector&&) noexcept = default;
    MetricsCollector& operator=(MetricsCollector&&) noexcept = default;

    void set_iterations(uint64_t warmup, uint64_t measured);
    
    AlgorithmBenchmarkResult run_algorithm_benchmark(SignatureScheme& scheme);

    void collect_keygen_metrics(SignatureScheme& scheme,
                                 OperationMetrics& metrics);

    void collect_signing_metrics(SignatureScheme& scheme,
                                  std::span<const std::byte> message,
                                  std::span<const std::byte> private_key,
                                  OperationMetrics& metrics,
                                  uint64_t message_size);

    void collect_verification_metrics(SignatureScheme& scheme,
                                       std::span<const std::byte> message,
                                       std::span<const std::byte> signature,
                                       std::span<const std::byte> public_key,
                                       OperationMetrics& metrics,
                                       uint64_t message_size);

    [[nodiscard]] BenchmarkSuite run_full_suite(std::vector<std::unique_ptr<SignatureScheme>>& schemes);

private:
    uint64_t warmup_iterations_{100};
    uint64_t measured_iterations_{1000};

    [[nodiscard]] std::vector<double> measure_latency(
        const std::function<void()>& operation,
        uint64_t iterations);
};

}
