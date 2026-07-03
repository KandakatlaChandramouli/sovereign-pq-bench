#include "sovereign/metrics/benchmark_result.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <sstream>
#include <iomanip>

namespace sovereign {

void OperationMetrics::compute(const std::vector<double>& latencies_ns, uint64_t bytes_processed) {
    if (latencies_ns.empty()) return;

    total_operations = latencies_ns.size();
    total_bytes = bytes_processed;

    auto sorted = latencies_ns;
    std::sort(sorted.begin(), sorted.end());

    min_ns = sorted.front();
    max_ns = sorted.back();

    double sum = std::accumulate(sorted.begin(), sorted.end(), 0.0);
    mean_ns = sum / static_cast<double>(sorted.size());

    std::size_t mid = sorted.size() / 2;
    if (sorted.size() % 2 == 0) {
        median_ns = (sorted[mid - 1] + sorted[mid]) / 2.0;
    } else {
        median_ns = sorted[mid];
    }

    std::size_t p95_idx = static_cast<std::size_t>(std::ceil(0.95 * static_cast<double>(sorted.size()))) - 1;
    std::size_t p99_idx = static_cast<std::size_t>(std::ceil(0.99 * static_cast<double>(sorted.size()))) - 1;
    
    p95_idx = std::min(p95_idx, sorted.size() - 1);
    p99_idx = std::min(p99_idx, sorted.size() - 1);
    
    p95_ns = sorted[p95_idx];
    p99_ns = sorted[p99_idx];

    double sq_sum = 0.0;
    for (double val : sorted) {
        double diff = val - mean_ns;
        sq_sum += diff * diff;
    }
    stddev_ns = std::sqrt(sq_sum / static_cast<double>(sorted.size()));

    if (mean_ns > 0.0) {
        ops_per_sec = 1e9 / mean_ns;
    }

    if (bytes_processed > 0 && mean_ns > 0.0) {
        double bytes_per_ns = static_cast<double>(bytes_processed) / mean_ns;
        mb_per_sec = bytes_per_ns * 1e3;
    }
}

std::string AlgorithmBenchmarkResult::to_csv_row() const {
    std::ostringstream oss;
    oss << algorithm_name << ","
        << keygen.mean_ns << "," << keygen.median_ns << "," << keygen.p95_ns << "," << keygen.p99_ns << ","
        << sign_1b.mean_ns << "," << sign_1b.median_ns << "," << sign_1b.p95_ns << "," << sign_1b.p99_ns << ","
        << sign_1kb.mean_ns << "," << sign_1kb.median_ns << "," << sign_1kb.p95_ns << "," << sign_1kb.p99_ns << ","
        << sign_1mb.mean_ns << "," << sign_1mb.median_ns << "," << sign_1mb.p95_ns << "," << sign_1mb.p99_ns << ","
        << verify_1b.mean_ns << "," << verify_1b.median_ns << "," << verify_1b.p95_ns << "," << verify_1b.p99_ns << ","
        << verify_1kb.mean_ns << "," << verify_1kb.median_ns << "," << verify_1kb.p95_ns << "," << verify_1kb.p99_ns << ","
        << verify_1mb.mean_ns << "," << verify_1mb.median_ns << "," << verify_1mb.p95_ns << "," << verify_1mb.p99_ns << ","
        << public_key_bytes << "," << private_key_bytes << "," << signature_bytes << ","
        << cpu_model << "," << timestamp;
    return oss.str();
}

std::string AlgorithmBenchmarkResult::to_json() const {
    std::ostringstream oss;
    oss << "{\n"
        << "  \"algorithm\": \"" << algorithm_name << "\",\n"
        << "  \"public_key_bytes\": " << public_key_bytes << ",\n"
        << "  \"private_key_bytes\": " << private_key_bytes << ",\n"
        << "  \"signature_bytes\": " << signature_bytes << ",\n"
        << "  \"cpu_model\": \"" << cpu_model << "\",\n"
        << "  \"timestamp\": \"" << timestamp << "\"\n"
        << "}";
    return oss.str();
}

std::string AlgorithmBenchmarkResult::to_markdown() const {
    std::ostringstream oss;
    oss << "| " << std::setw(15) << algorithm_name
        << " | " << std::setw(10) << std::fixed << std::setprecision(2) << keygen.mean_ns
        << " | " << std::setw(10) << sign_1b.mean_ns
        << " | " << std::setw(10) << sign_1kb.mean_ns
        << " | " << std::setw(10) << verify_1b.mean_ns
        << " | " << std::setw(10) << verify_1kb.mean_ns
        << " | " << std::setw(10) << public_key_bytes
        << " | " << std::setw(10) << signature_bytes << " |";
    return oss.str();
}

std::string BenchmarkSuite::to_csv() const {
    std::ostringstream oss;
    oss << "algorithm,keygen_mean,keygen_median,keygen_p95,keygen_p99,"
        << "sign_1b_mean,sign_1b_median,sign_1b_p95,sign_1b_p99,"
        << "sign_1kb_mean,sign_1kb_median,sign_1kb_p95,sign_1kb_p99,"
        << "sign_1mb_mean,sign_1mb_median,sign_1mb_p95,sign_1mb_p99,"
        << "verify_1b_mean,verify_1b_median,verify_1b_p95,verify_1b_p99,"
        << "verify_1kb_mean,verify_1kb_median,verify_1kb_p95,verify_1kb_p99,"
        << "verify_1mb_mean,verify_1mb_median,verify_1mb_p95,verify_1mb_p99,"
        << "pub_key_bytes,priv_key_bytes,sig_bytes,cpu_model,timestamp\n";

    for (const auto& result : results) {
        oss << result.to_csv_row() << "\n";
    }

    return oss.str();
}

std::string BenchmarkSuite::to_json() const {
    std::ostringstream oss;
    oss << "{\n"
        << "  \"suite_timestamp\": \"" << suite_timestamp << "\",\n"
        << "  \"system_description\": \"" << system_description << "\",\n"
        << "  \"results\": [\n";

    for (std::size_t i = 0; i < results.size(); ++i) {
        oss << results[i].to_json();
        if (i < results.size() - 1) oss << ",";
        oss << "\n";
    }

    oss << "  ]\n}";
    return oss.str();
}

std::string BenchmarkSuite::to_markdown() const {
    std::ostringstream oss;
    oss << "# Post-Quantum Migration Benchmark Results\n\n";
    oss << "**Suite Timestamp:** " << suite_timestamp << "\n\n";
    oss << "**System:** " << system_description << "\n\n";

    oss << "| Algorithm       | KeyGen (ns) | Sign 1B (ns) | Sign 1KB (ns) | "
        << "Verify 1B (ns) | Verify 1KB (ns) | Pub Key (B) | Sig (B) |\n";
    oss << "|-----------------|-------------|--------------|---------------|"
        << "----------------|-----------------|-------------|----------|\n";

    for (const auto& result : results) {
        oss << result.to_markdown() << "\n";
    }

    return oss.str();
}

std::string BenchmarkSuite::to_latex_table() const {
    std::ostringstream oss;
    oss << "\\begin{table}[h]\n"
        << "\\centering\n"
        << "\\begin{tabular}{|l|r|r|r|r|}\n"
        << "\\hline\n"
        << "Algorithm & KeyGen (ns) & Sign (ns) & Verify (ns) & Sig Size (B) \\\\\n"
        << "\\hline\n";

    for (const auto& result : results) {
        oss << result.algorithm_name << " & "
            << std::fixed << std::setprecision(0) << result.keygen.mean_ns << " & "
            << result.sign_1kb.mean_ns << " & "
            << result.verify_1kb.mean_ns << " & "
            << result.signature_bytes << " \\\\\n";
    }

    oss << "\\hline\n"
        << "\\end{tabular}\n"
        << "\\caption{Post-Quantum Signature Performance Comparison}\n"
        << "\\end{table}\n";

    return oss.str();
}

}
