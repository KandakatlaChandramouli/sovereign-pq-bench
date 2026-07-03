#include "sovereign/metrics/metrics_collector.hpp"
#include "sovereign/utils/random_generator.hpp"
#include "sovereign/utils/timer.hpp"
#include <iostream>

namespace sovereign {

MetricsCollector::MetricsCollector() = default;
MetricsCollector::~MetricsCollector() = default;

void MetricsCollector::set_iterations(uint64_t warmup, uint64_t measured) {
    warmup_iterations_ = warmup;
    measured_iterations_ = measured;
}

AlgorithmBenchmarkResult MetricsCollector::run_algorithm_benchmark(SignatureScheme& scheme) {
    AlgorithmBenchmarkResult result;
    result.algorithm = scheme.algorithm();
    result.algorithm_name = std::string(scheme.name());

    collect_keygen_metrics(scheme, result.keygen);

    auto keypair_result = scheme.generate_keypair();
    if (!keypair_result.first) {
        std::cerr << "Key generation failed for " << scheme.name() << "\n";
        return result;
    }

    auto& keypair = *keypair_result.first;
    result.public_key_bytes = keypair.public_key.size();
    result.private_key_bytes = keypair.private_key.size();

    RandomGenerator rng;
    std::vector<std::byte> msg_1b(1);
    std::vector<std::byte> msg_1kb(1024);
    std::vector<std::byte> msg_1mb(1024 * 1024);
    rng.fill_buffer(msg_1b);
    rng.fill_buffer(msg_1kb);
    rng.fill_buffer(msg_1mb);

    auto sig_1b_result = scheme.sign(msg_1b, keypair.private_key);
    if (sig_1b_result.first) {
        result.signature_bytes = sig_1b_result.first->data.size();
    }

    collect_signing_metrics(scheme, msg_1b, keypair.private_key, result.sign_1b, 1);
    collect_signing_metrics(scheme, msg_1kb, keypair.private_key, result.sign_1kb, 1024);
    collect_signing_metrics(scheme, msg_1mb, keypair.private_key, result.sign_1mb, 1024 * 1024);

    if (sig_1b_result.first) {
        collect_verification_metrics(scheme, msg_1b, sig_1b_result.first->data, 
                                      keypair.public_key, result.verify_1b, 1);
    }

    auto sig_1kb_result = scheme.sign(msg_1kb, keypair.private_key);
    if (sig_1kb_result.first) {
        collect_verification_metrics(scheme, msg_1kb, sig_1kb_result.first->data,
                                      keypair.public_key, result.verify_1kb, 1024);
    }

    auto sig_1mb_result = scheme.sign(msg_1mb, keypair.private_key);
    if (sig_1mb_result.first) {
        collect_verification_metrics(scheme, msg_1mb, sig_1mb_result.first->data,
                                      keypair.public_key, result.verify_1mb, 1024 * 1024);
    }

    return result;
}

void MetricsCollector::collect_keygen_metrics(SignatureScheme& scheme,
                                               OperationMetrics& metrics) {
    std::vector<double> latencies;
    latencies.reserve(measured_iterations_);

    for (uint64_t i = 0; i < warmup_iterations_; ++i) {
        scheme.generate_keypair();
    }

    for (uint64_t i = 0; i < measured_iterations_; ++i) {
        HighResolutionTimer timer;
        timer.start();
        scheme.generate_keypair();
        latencies.push_back(static_cast<double>(timer.elapsed_ns()));
    }

    metrics.compute(latencies);
}

void MetricsCollector::collect_signing_metrics(SignatureScheme& scheme,
                                                std::span<const std::byte> message,
                                                std::span<const std::byte> private_key,
                                                OperationMetrics& metrics,
                                                uint64_t message_size) {
    std::vector<double> latencies;
    latencies.reserve(measured_iterations_);

    for (uint64_t i = 0; i < warmup_iterations_; ++i) {
        scheme.sign(message, private_key);
    }

    for (uint64_t i = 0; i < measured_iterations_; ++i) {
        HighResolutionTimer timer;
        timer.start();
        scheme.sign(message, private_key);
        latencies.push_back(static_cast<double>(timer.elapsed_ns()));
    }

    metrics.compute(latencies, message_size * measured_iterations_);
}

void MetricsCollector::collect_verification_metrics(SignatureScheme& scheme,
                                                     std::span<const std::byte> message,
                                                     std::span<const std::byte> signature,
                                                     std::span<const std::byte> public_key,
                                                     OperationMetrics& metrics,
                                                     uint64_t message_size) {
    std::vector<double> latencies;
    latencies.reserve(measured_iterations_);

    for (uint64_t i = 0; i < warmup_iterations_; ++i) {
        scheme.verify(message, signature, public_key);
    }

    for (uint64_t i = 0; i < measured_iterations_; ++i) {
        HighResolutionTimer timer;
        timer.start();
        scheme.verify(message, signature, public_key);
        latencies.push_back(static_cast<double>(timer.elapsed_ns()));
    }

    metrics.compute(latencies, (message_size + signature.size()) * measured_iterations_);
}

BenchmarkSuite MetricsCollector::run_full_suite(
    std::vector<std::unique_ptr<SignatureScheme>>& schemes) {

    BenchmarkSuite suite;
    suite.suite_timestamp = "generated";

    for (auto& scheme : schemes) {
        std::cout << "Benchmarking " << scheme->name() << "...\n";
        auto result = run_algorithm_benchmark(*scheme);
        suite.results.push_back(std::move(result));
    }

    return suite;
}

}
