#include "sovereign/metrics/memory_tracker.hpp"
#include <fstream>
#include <chrono>
#include <unistd.h>

namespace sovereign {

MemoryTracker::MemoryTracker() : start_malloc_(0) {}

void MemoryTracker::start() {
    getrusage(RUSAGE_SELF, &start_usage_);
    start_malloc_ = mallinfo().uordblks;
}

MemoryMetrics MemoryTracker::stop() {
    struct rusage end_usage;
    getrusage(RUSAGE_SELF, &end_usage);
    size_t end_malloc = mallinfo().uordblks;

    MemoryMetrics m;
    m.peak_rss_kb = end_usage.ru_maxrss;
    m.current_rss_kb = end_usage.ru_idrss + end_usage.ru_isrss;
    m.malloc_bytes = end_malloc > start_malloc_ ? end_malloc - start_malloc_ : 0;
    return m;
}

size_t MemoryTracker::get_binary_size(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    return f.tellg();
}

} // namespace sovereign
