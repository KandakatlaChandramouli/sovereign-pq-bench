#pragma once

#include "sovereign/metrics/benchmark_result.hpp"
#include <string>
#include <string_view>

namespace sovereign {

class JsonExporter {
public:
    JsonExporter() = default;
    ~JsonExporter() = default;

    JsonExporter(const JsonExporter&) = default;
    JsonExporter& operator=(const JsonExporter&) = default;
    JsonExporter(JsonExporter&&) noexcept = default;
    JsonExporter& operator=(JsonExporter&&) noexcept = default;

    [[nodiscard]] bool export_to_file(const BenchmarkSuite& suite,
                                       std::string_view filepath) const;

    [[nodiscard]] bool export_single_result(const AlgorithmBenchmarkResult& result,
                                             std::string_view filepath) const;

private:
    [[nodiscard]] std::string generate_json_content(const BenchmarkSuite& suite) const;
    [[nodiscard]] std::string algorithm_to_json(const AlgorithmBenchmarkResult& result) const;
    [[nodiscard]] std::string metrics_to_json(const OperationMetrics& metrics) const;
};

}
