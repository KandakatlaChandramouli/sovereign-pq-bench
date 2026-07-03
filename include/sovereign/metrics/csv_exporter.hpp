#pragma once

#include "sovereign/metrics/benchmark_result.hpp"
#include <string>
#include <string_view>

namespace sovereign {

class CsvExporter {
public:
    CsvExporter() = default;
    ~CsvExporter() = default;

    CsvExporter(const CsvExporter&) = default;
    CsvExporter& operator=(const CsvExporter&) = default;
    CsvExporter(CsvExporter&&) noexcept = default;
    CsvExporter& operator=(CsvExporter&&) noexcept = default;

    [[nodiscard]] bool export_to_file(const BenchmarkSuite& suite,
                                       std::string_view filepath) const;

    [[nodiscard]] bool export_single_result(const AlgorithmBenchmarkResult& result,
                                             std::string_view filepath) const;

private:
    [[nodiscard]] std::string generate_csv_content(const BenchmarkSuite& suite) const;
};

}
