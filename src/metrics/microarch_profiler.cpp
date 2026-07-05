#include "sovereign/metrics/microarch_profiler.hpp"
#include <iostream>
#include <stdexcept>

namespace sovereign {

#ifdef __linux__
int MicroArchProfiler::open_counter(uint32_t type, uint64_t config) {
    struct perf_event_attr pe;
    std::memset(&pe, 0, sizeof(pe));
    pe.type = type;
    pe.size = sizeof(pe);
    pe.config = config;
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;
    pe.read_format = 0;
    
    int fd = perf_event_open(&pe, 0, -1, -1, 0);
    if (fd < 0) return -1;
    return fd;
}

uint64_t MicroArchProfiler::read_counter(int fd) {
    if (fd < 0) return 0;
    uint64_t val = 0;
    ::read(fd, &val, sizeof(val));
    return val;
}
#endif

MicroArchProfiler::MicroArchProfiler() {
#ifdef __linux__
    fd_cache_misses_ = open_counter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES);
    fd_cache_refs_ = open_counter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES);
    fd_branch_misses_ = open_counter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES);
    fd_branch_instr_ = open_counter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_INSTRUCTIONS);
    fd_instructions_ = open_counter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS);
    fd_cycles_ = open_counter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES);
    fd_ctx_switches_ = open_counter(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CONTEXT_SWITCHES);
    fd_cpu_migrations_ = open_counter(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_MIGRATIONS);
    fd_page_faults_ = open_counter(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS);
    
    available_ = (fd_cycles_ >= 0 && fd_instructions_ >= 0);
    
    if (!available_) {
        std::cerr << "[MicroArchProfiler] PMU counters not available (non-root or virtualized). "
                  << "Micro-architectural metrics will be zero. "
                  << "Run with: sudo sysctl kernel.perf_event_paranoid=-1\n";
    }
#endif
}

MicroArchProfiler::~MicroArchProfiler() {
#ifdef __linux__
    auto close_fd = [](int& fd) { if (fd >= 0) close(fd); fd = -1; };
    close_fd(fd_cache_misses_); close_fd(fd_cache_refs_);
    close_fd(fd_branch_misses_); close_fd(fd_branch_instr_);
    close_fd(fd_instructions_); close_fd(fd_cycles_);
    close_fd(fd_ctx_switches_); close_fd(fd_cpu_migrations_); close_fd(fd_page_faults_);
#endif
}

void MicroArchProfiler::start() {
#ifdef __linux__
    auto reset_enable = [](int fd) {
        if (fd >= 0) {
            ioctl(fd, PERF_EVENT_IOC_RESET, 0);
            ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
        }
    };
    reset_enable(fd_cache_misses_); reset_enable(fd_cache_refs_);
    reset_enable(fd_branch_misses_); reset_enable(fd_branch_instr_);
    reset_enable(fd_instructions_); reset_enable(fd_cycles_);
    reset_enable(fd_ctx_switches_); reset_enable(fd_cpu_migrations_); reset_enable(fd_page_faults_);
#endif
}

MicroArchMetrics MicroArchProfiler::stop() {
    MicroArchMetrics m;
#ifdef __linux__
    auto disable_read = [](int fd) -> uint64_t {
        if (fd < 0) return 0;
        ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
        uint64_t val = 0;
        ::read(fd, &val, sizeof(val));
        return val;
    };
    m.cache_misses = disable_read(fd_cache_misses_);
    m.cache_references = disable_read(fd_cache_refs_);
    m.branch_misses = disable_read(fd_branch_misses_);
    m.branch_instructions = disable_read(fd_branch_instr_);
    m.instructions = disable_read(fd_instructions_);
    m.cycles = disable_read(fd_cycles_);
    m.context_switches = disable_read(fd_ctx_switches_);
    m.cpu_migrations = disable_read(fd_cpu_migrations_);
    m.page_faults = disable_read(fd_page_faults_);
#endif
    return m;
}

}
