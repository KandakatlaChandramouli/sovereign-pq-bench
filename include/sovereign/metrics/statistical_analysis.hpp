#pragma once

#include <cmath>
#include <cstdint>
#include <numeric>
#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

namespace sovereign {

struct ConfidenceInterval {
    double lower;
    double upper;
    double mean;
    double margin_of_error;
    
    [[nodiscard]] std::string to_string() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << mean << " ± " << margin_of_error;
        return oss.str();
    }
};

struct StatisticalResult {
    uint64_t sample_count{0};
    double mean_ns{0.0};
    double median_ns{0.0};
    double stddev_ns{0.0};
    double variance_ns{0.0};
    double min_ns{0.0};
    double max_ns{0.0};
    double p50_ns{0.0};
    double p90_ns{0.0};
    double p95_ns{0.0};
    double p99_ns{0.0};
    double p999_ns{0.0};
    double skewness{0.0};
    double kurtosis{0.0};
    double cv_percent{0.0};
    ConfidenceInterval ci_95;
    ConfidenceInterval ci_99;
    double ops_per_sec{0.0};
    double mb_per_sec{0.0};
    uint64_t total_bytes_processed{0};
    
    void compute(std::vector<double>& latencies_ns, uint64_t bytes_per_op = 0);
};

class StatisticalAnalyzer {
public:
    StatisticalAnalyzer() = default;
    [[nodiscard]] StatisticalResult analyze(std::vector<double> latencies_ns, uint64_t bytes_per_op = 0);
    [[nodiscard]] static double percentile(const std::vector<double>& sorted, double pct);
    [[nodiscard]] static ConfidenceInterval bootstrap_ci(const std::vector<double>& data, double confidence, int bootstrap_samples = 10000);
    [[nodiscard]] static double compute_skewness(const std::vector<double>& data, double mean, double stddev);
    [[nodiscard]] static double compute_kurtosis(const std::vector<double>& data, double mean, double stddev);
};

}
