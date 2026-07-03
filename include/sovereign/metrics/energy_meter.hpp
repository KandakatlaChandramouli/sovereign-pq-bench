#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace sovereign {

struct EnergyReading {
    double joules{0.0};
    double avg_watts{0.0};
    double elapsed_sec{0.0};
    std::string source;
};

class EnergyMeter {
public:
    EnergyMeter();
    ~EnergyMeter();
    
    [[nodiscard]] bool is_available() const noexcept;
    
    void start();
    EnergyReading stop();
    
    [[nodiscard]] static std::string detect_available_source();

private:
    bool available_{false};
    std::string source_;
    double start_energy_{0.0};
    uint64_t start_time_ns_{0};
    int fd_{-1};
    
    double read_rapl_energy();
    double read_perf_energy();
    uint64_t read_tsc();
};

}
