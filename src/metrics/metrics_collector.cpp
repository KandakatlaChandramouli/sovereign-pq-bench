#include "sovereign/metrics/metrics_collector.hpp"
#include "sovereign/utils/random_generator.hpp"
#include "sovereign/utils/timer.hpp"
#include "sovereign/metrics/system_info.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace sovereign {

MetricsCollector::MetricsCollector() = default;
MetricsCollector::~MetricsCollector() = default;

void MetricsCollector::set_config(const BenchmarkConfig& config) { config_ = config; }

PerfCounters MetricsCollector::read_perf_counters() {
    PerfCounters pc;
    return pc;
}

ComprehensiveResult MetricsCollector::run_comprehensive_benchmark(SignatureScheme& scheme) {
    ComprehensiveResult result;
    result.algorithm_name = std::string(scheme.name());
    result.algorithm = scheme.algorithm();
    
    auto sys = SystemInfo::collect();
    result.cpu_model = sys.cpu_model;
    result.os_info = sys.os_name;
    result.compiler_version = sys.compiler_name + " " + sys.compiler_version;
    result.timestamp = sys.timestamp;
    
    RandomGenerator rng;
    
    // Keygen benchmark
    std::vector<double> keygen_latencies;
    keygen_latencies.reserve(config_.measured_iterations);
    for (uint64_t i = 0; i < config_.warmup_iterations; ++i) scheme.generate_keypair();
    for (uint64_t i = 0; i < config_.measured_iterations; ++i) {
        HighResolutionTimer timer;
        timer.start();
        scheme.generate_keypair();
        keygen_latencies.push_back(static_cast<double>(timer.elapsed_ns()));
    }
    result.aggregate_keygen = analyzer_.analyze(keygen_latencies);
    
    // Generate working keypair
    auto kp_result = scheme.generate_keypair();
    if (!kp_result.first) return result;
    auto& kp = *kp_result.first;
    result.public_key_bytes = kp.public_key.size();
    result.private_key_bytes = kp.private_key.size();
    
    EnergyMeter energy;
    
    // Test each message size
    for (auto msg_size : config_.message_sizes) {
        ComprehensiveResult::SizeResult sr;
        sr.message_size = msg_size;
        auto msg = generate_message(msg_size);
        
        // Signing
        std::vector<double> sign_latencies;
        sign_latencies.reserve(config_.measured_iterations);
        for (uint64_t i = 0; i < config_.warmup_iterations; ++i) scheme.sign(msg, kp.private_key);
        if (energy.is_available()) energy.start();
        for (uint64_t i = 0; i < config_.measured_iterations; ++i) {
            HighResolutionTimer timer;
            timer.start();
            scheme.sign(msg, kp.private_key);
            sign_latencies.push_back(static_cast<double>(timer.elapsed_ns()));
        }
        if (energy.is_available()) sr.sign_energy = energy.stop();
        sr.sign_stats = analyzer_.analyze(sign_latencies, msg_size);
        
        // Get signature
        auto sig_result = scheme.sign(msg, kp.private_key);
        if (!sig_result.first) continue;
        if (result.signature_bytes == 0) result.signature_bytes = sig_result.first->data.size();
        
        // Verification
        std::vector<double> verify_latencies;
        verify_latencies.reserve(config_.measured_iterations);
        for (uint64_t i = 0; i < config_.warmup_iterations; ++i)
            scheme.verify(msg, sig_result.first->data, kp.public_key);
        if (energy.is_available()) energy.start();
        for (uint64_t i = 0; i < config_.measured_iterations; ++i) {
            HighResolutionTimer timer;
            timer.start();
            scheme.verify(msg, sig_result.first->data, kp.public_key);
            verify_latencies.push_back(static_cast<double>(timer.elapsed_ns()));
        }
        if (energy.is_available()) sr.verify_energy = energy.stop();
        sr.verify_stats = analyzer_.analyze(verify_latencies, msg_size + sig_result.first->data.size());
        
        result.size_results.push_back(std::move(sr));
    }
    
    return result;
}

std::vector<double> MetricsCollector::collect_latencies(const std::function<void()>& op, uint64_t iters) {
    std::vector<double> l; l.reserve(iters);
    for (uint64_t i = 0; i < iters; ++i) {
        HighResolutionTimer t; t.start(); op();
        l.push_back(static_cast<double>(t.elapsed_ns()));
    }
    return l;
}

std::vector<std::byte> MetricsCollector::generate_message(std::size_t size) {
    RandomGenerator rng;
    return rng.generate_bytes(size);
}

}
