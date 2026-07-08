#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <atomic>

#ifdef __linux__
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <unistd.h>
#endif

namespace sovereign {

struct MicroArchMetrics {
    uint64_t cache_misses{0};
    uint64_t cache_references{0};
    uint64_t branch_misses{0};
    uint64_t branch_instructions{0};
    uint64_t instructions{0};
    uint64_t cycles{0};
    uint64_t context_switches{0};
    uint64_t cpu_migrations{0};
    uint64_t page_faults{0};
    
    double cache_miss_rate() const { return cache_references > 0 ? (double)cache_misses / cache_references * 100.0 : 0.0; }
    double branch_miss_rate() const { return branch_instructions > 0 ? (double)branch_misses / branch_instructions * 100.0 : 0.0; }
    double ipc() const { return cycles > 0 ? (double)instructions / cycles : 0.0; }
    
    std::string to_csv_header() const;
    std::string to_csv_row() const;
};

class MicroArchProfiler {
public:
    MicroArchProfiler();
    ~MicroArchProfiler();
    
    bool is_available() const noexcept { return available_; }
    
    void start();
    MicroArchMetrics stop();

private:
    bool available_{false};
    
#ifdef __linux__
    int fd_cache_misses_{-1};
    int fd_cache_refs_{-1};
    int fd_branch_misses_{-1};
    int fd_branch_instr_{-1};
    int fd_instructions_{-1};
    int fd_cycles_{-1};
    int fd_ctx_switches_{-1};
    int fd_cpu_migrations_{-1};
    int fd_page_faults_{-1};
    
    static long perf_event_open(struct perf_event_attr* hw_event, pid_t pid,
                                int cpu, int group_fd, unsigned long flags) {
        return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
    }
    
    int open_counter(uint32_t type, uint64_t config);
    uint64_t read_counter(int fd);
#endif
};

} // namespace sovereign
