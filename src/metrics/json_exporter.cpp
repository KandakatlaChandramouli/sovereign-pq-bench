#include "sovereign/metrics/json_exporter.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace sovereign {

bool JsonExporter::export_to_file(const BenchmarkSuite& suite,
                                   std::string_view filepath) const {
    std::ofstream file(filepath.data(), std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Failed to open JSON file: " << filepath << "\n";
        return false;
    }

    std::string content = generate_json_content(suite);
    file << content;
    file.close();

    std::cout << "JSON exported to: " << filepath << "\n";
    return true;
}

bool JsonExporter::export_single_result(const AlgorithmBenchmarkResult& result,
                                         std::string_view filepath) const {
    BenchmarkSuite suite;
    suite.results.push_back(result);
    return export_to_file(suite, filepath);
}

std::string JsonExporter::generate_json_content(const BenchmarkSuite& suite) const {
    std::ostringstream oss;
    oss << "{\n"
        << "  \"suite_timestamp\": \"" << suite.suite_timestamp << "\",\n"
        << "  \"system_description\": \"" << suite.system_description << "\",\n"
        << "  \"results\": [\n";

    for (std::size_t i = 0; i < suite.results.size(); ++i) {
        oss << algorithm_to_json(suite.results[i]);
        if (i < suite.results.size() - 1) {
            oss << ",";
        }
        oss << "\n";
    }

    oss << "  ]\n}";
    return oss.str();
}

std::string JsonExporter::algorithm_to_json(const AlgorithmBenchmarkResult& result) const {
    std::ostringstream oss;
    oss << "    {\n"
        << "      \"algorithm\": \"" << result.algorithm_name << "\",\n"
        << "      \"public_key_bytes\": " << result.public_key_bytes << ",\n"
        << "      \"private_key_bytes\": " << result.private_key_bytes << ",\n"
        << "      \"signature_bytes\": " << result.signature_bytes << ",\n"
        << "      \"keygen\": " << metrics_to_json(result.keygen) << ",\n"
        << "      \"sign_1b\": " << metrics_to_json(result.sign_1b) << ",\n"
        << "      \"sign_1kb\": " << metrics_to_json(result.sign_1kb) << ",\n"
        << "      \"sign_1mb\": " << metrics_to_json(result.sign_1mb) << ",\n"
        << "      \"verify_1b\": " << metrics_to_json(result.verify_1b) << ",\n"
        << "      \"verify_1kb\": " << metrics_to_json(result.verify_1kb) << ",\n"
        << "      \"verify_1mb\": " << metrics_to_json(result.verify_1mb) << ",\n"
        << "      \"cpu_model\": \"" << result.cpu_model << "\",\n"
        << "      \"timestamp\": \"" << result.timestamp << "\"\n"
        << "    }";
    return oss.str();
}

std::string JsonExporter::metrics_to_json(const OperationMetrics& metrics) const {
    std::ostringstream oss;
    oss << "{\n"
        << "        \"mean_ns\": " << std::fixed << std::setprecision(2) << metrics.mean_ns << ",\n"
        << "        \"median_ns\": " << metrics.median_ns << ",\n"
        << "        \"p95_ns\": " << metrics.p95_ns << ",\n"
        << "        \"p99_ns\": " << metrics.p99_ns << ",\n"
        << "        \"stddev_ns\": " << metrics.stddev_ns << ",\n"
        << "        \"min_ns\": " << metrics.min_ns << ",\n"
        << "        \"max_ns\": " << metrics.max_ns << ",\n"
        << "        \"ops_per_sec\": " << metrics.ops_per_sec << ",\n"
        << "        \"mb_per_sec\": " << metrics.mb_per_sec << "\n"
        << "      }";
    return oss.str();
}

}
