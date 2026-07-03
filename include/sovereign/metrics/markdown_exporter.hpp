#pragma once

#include "sovereign/metrics/benchmark_result.hpp"
#include <string>
#include <string_view>

namespace sovereign {

class MarkdownExporter {
public:
    MarkdownExporter() = default;
    ~MarkdownExporter() = default;

    MarkdownExporter(const MarkdownExporter&) = default;
    MarkdownExporter& operator=(const MarkdownExporter&) = default;
    MarkdownExporter(MarkdownExporter&&) noexcept = default;
    MarkdownExporter& operator=(MarkdownExporter&&) noexcept = default;

    [[nodiscard]] bool export_to_file(const BenchmarkSuite& suite,
                                       std::string_view filepath) const;

    [[nodiscard]] bool export_single_result(const AlgorithmBenchmarkResult& result,
                                             std::string_view filepath) const;

private:
    [[nodiscard]] std::string generate_markdown_content(const BenchmarkSuite& suite) const;
    [[nodiscard]] std::string generate_latex_table(const BenchmarkSuite& suite) const;
};

}
