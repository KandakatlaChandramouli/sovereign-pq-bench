#include "sovereign/metrics/markdown_exporter.hpp"
#include <fstream>
#include <iostream>

namespace sovereign {

bool MarkdownExporter::export_to_file(const BenchmarkSuite& suite,
                                       std::string_view filepath) const {
    std::ofstream file(filepath.data(), std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Failed to open Markdown file: " << filepath << "\n";
        return false;
    }

    std::string content = generate_markdown_content(suite);
    file << content;
    
    file << "\n\n---\n\n";
    file << generate_latex_table(suite);
    
    file.close();

    std::cout << "Markdown exported to: " << filepath << "\n";
    return true;
}

bool MarkdownExporter::export_single_result(const AlgorithmBenchmarkResult& result,
                                             std::string_view filepath) const {
    BenchmarkSuite suite;
    suite.results.push_back(result);
    return export_to_file(suite, filepath);
}

std::string MarkdownExporter::generate_markdown_content(const BenchmarkSuite& suite) const {
    return suite.to_markdown();
}

std::string MarkdownExporter::generate_latex_table(const BenchmarkSuite& suite) const {
    return suite.to_latex_table();
}

}
