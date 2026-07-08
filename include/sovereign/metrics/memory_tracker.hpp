#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <sys/resource.h>
#include <malloc.h>

namespace sovereign {

struct MemoryMetrics {
    size_t peak_rss_kb = 0;       // Peak resident set size
    size_t current_rss_kb = 0;    // Current RSS
    size_t malloc_bytes = 0;      // Total allocated via malloc (glibc)
    size_t binary_size_kb = 0;    // Size of the benchmark binary
    size_t library_size_kb = 0;   // liboqs + libcrypto combined
    double alloc_rate_mb_s = 0;   // Allocation rate during benchmark
};

class MemoryTracker {
public:
    MemoryTracker();
    void start();
    MemoryMetrics stop();
    static size_t get_binary_size(const std::string& path);
private:
    struct rusage start_usage_;
    size_t start_malloc_;
};

} // namespace sovereign
