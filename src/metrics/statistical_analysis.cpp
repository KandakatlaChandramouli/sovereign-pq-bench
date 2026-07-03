#include "sovereign/metrics/statistical_analysis.hpp"
#include <random>
#include <stdexcept>

namespace sovereign {

void StatisticalResult::compute(std::vector<double>& latencies_ns, uint64_t bytes_per_op) {
    if (latencies_ns.empty()) return;
    
    sample_count = latencies_ns.size();
    total_bytes_processed = bytes_per_op * sample_count;
    
    std::sort(latencies_ns.begin(), latencies_ns.end());
    
    min_ns = latencies_ns.front();
    max_ns = latencies_ns.back();
    
    double sum = std::accumulate(latencies_ns.begin(), latencies_ns.end(), 0.0);
    mean_ns = sum / static_cast<double>(sample_count);
    
    std::size_t mid = sample_count / 2;
    median_ns = (sample_count % 2 == 0) ? (latencies_ns[mid-1] + latencies_ns[mid]) / 2.0 : latencies_ns[mid];
    
    double sq_sum = 0.0;
    for (double v : latencies_ns) {
        double diff = v - mean_ns;
        sq_sum += diff * diff;
    }
    variance_ns = sq_sum / static_cast<double>(sample_count);
    stddev_ns = std::sqrt(variance_ns);
    
    cv_percent = (mean_ns > 0) ? (stddev_ns / mean_ns) * 100.0 : 0.0;
    
    p50_ns = percentile(latencies_ns, 50.0);
    p90_ns = percentile(latencies_ns, 90.0);
    p95_ns = percentile(latencies_ns, 95.0);
    p99_ns = percentile(latencies_ns, 99.0);
    p999_ns = percentile(latencies_ns, 99.9);
    
    skewness = compute_skewness(latencies_ns, mean_ns, stddev_ns);
    kurtosis = compute_kurtosis(latencies_ns, mean_ns, stddev_ns);
    
    ci_95 = bootstrap_ci(latencies_ns, 0.95);
    ci_99 = bootstrap_ci(latencies_ns, 0.99);
    
    ops_per_sec = (mean_ns > 0) ? 1e9 / mean_ns : 0.0;
    mb_per_sec = (mean_ns > 0 && bytes_per_op > 0) ? (static_cast<double>(bytes_per_op) / mean_ns) * 1e3 : 0.0;
}

StatisticalResult StatisticalAnalyzer::analyze(std::vector<double> latencies_ns, uint64_t bytes_per_op) {
    StatisticalResult result;
    result.compute(latencies_ns, bytes_per_op);
    return result;
}

double StatisticalAnalyzer::percentile(const std::vector<double>& sorted, double pct) {
    if (sorted.empty()) return 0.0;
    double idx = (pct / 100.0) * static_cast<double>(sorted.size() - 1);
    std::size_t lo = static_cast<std::size_t>(std::floor(idx));
    std::size_t hi = static_cast<std::size_t>(std::ceil(idx));
    if (lo == hi) return sorted[lo];
    double frac = idx - static_cast<double>(lo);
    return sorted[lo] * (1.0 - frac) + sorted[hi] * frac;
}

ConfidenceInterval StatisticalAnalyzer::bootstrap_ci(const std::vector<double>& data, double confidence, int bootstrap_samples) {
    std::mt19937 rng(42);
    std::uniform_int_distribution<std::size_t> dist(0, data.size() - 1);
    
    std::vector<double> means(bootstrap_samples);
    for (int i = 0; i < bootstrap_samples; ++i) {
        double sum = 0.0;
        for (std::size_t j = 0; j < data.size(); ++j) {
            sum += data[dist(rng)];
        }
        means[i] = sum / static_cast<double>(data.size());
    }
    
    std::sort(means.begin(), means.end());
    double alpha = (1.0 - confidence) / 2.0;
    std::size_t lo_idx = static_cast<std::size_t>(alpha * bootstrap_samples);
    std::size_t hi_idx = static_cast<std::size_t>((1.0 - alpha) * bootstrap_samples);
    
    double mean = std::accumulate(means.begin(), means.end(), 0.0) / bootstrap_samples;
    double moe = (means[hi_idx] - means[lo_idx]) / 2.0;
    
    return {means[lo_idx], means[hi_idx], mean, moe};
}

double StatisticalAnalyzer::compute_skewness(const std::vector<double>& data, double mean, double stddev) {
    if (stddev < 1e-9 || data.size() < 3) return 0.0;
    double n = static_cast<double>(data.size());
    double m3 = 0.0;
    for (double v : data) {
        double diff = (v - mean) / stddev;
        m3 += diff * diff * diff;
    }
    return (n / ((n-1)*(n-2))) * m3;
}

double StatisticalAnalyzer::compute_kurtosis(const std::vector<double>& data, double mean, double stddev) {
    if (stddev < 1e-9 || data.size() < 4) return 0.0;
    double n = static_cast<double>(data.size());
    double m4 = 0.0;
    for (double v : data) {
        double diff = (v - mean) / stddev;
        m4 += diff * diff * diff * diff;
    }
    double k = (n*(n+1))/((n-1)*(n-2)*(n-3)) * m4 - (3.0*(n-1)*(n-1))/((n-2)*(n-3));
    return k;
}

}
