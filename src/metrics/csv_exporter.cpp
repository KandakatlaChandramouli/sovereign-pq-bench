#include "sovereign/metrics/csv_exporter.hpp"
#include <fstream>
#include <iostream>

namespace sovereign {

bool CsvExporter::export_to_file(const BenchmarkSuite& suite,
                                  std::string_view filepath) const {
    std::ofstream file(filepath.data(), std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Failed to open CSV file: " << filepath << "\n";
        return false;
    }

    std::string content = generate_csv_content(suite);
    file << content;
    file.close();

    std::cout << "CSV exported to: " << filepath << "\n";
    return true;
}

bool CsvExporter::export_single_result(const AlgorithmBenchmarkResult& result,
                                        std::string_view filepath) const {
    BenchmarkSuite suite;
    suite.results.push_back(result);
    return export_to_file(suite, filepath);
}

std::string CsvExporter::generate_csv_content(const BenchmarkSuite& suite) const {
    return suite.to_csv();
}

}
