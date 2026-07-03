#include "sovereign/metrics/energy_meter.hpp"
#include <fstream>
#include <chrono>
#include <iostream>

#ifdef __linux__
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#endif

namespace sovereign {

EnergyMeter::EnergyMeter() {
    source_ = detect_available_source();
    available_ = !source_.empty();
    
#ifdef __linux__
    if (source_ == "RAPL" || source_ == "perf") {
        // Try to open RAPL energy counter
        fd_ = open("/sys/class/powercap/intel-rapl:0/energy_uj", O_RDONLY);
        if (fd_ < 0) {
            fd_ = open("/sys/class/powercap/intel-rapl:0:0/energy_uj", O_RDONLY);
        }
        if (fd_ < 0) available_ = false;
    }
#endif
}

EnergyMeter::~EnergyMeter() {
#ifdef __linux__
    if (fd_ >= 0) close(fd_);
#endif
}

bool EnergyMeter::is_available() const noexcept { return available_; }

void EnergyMeter::start() {
    if (!available_) return;
    start_energy_ = read_rapl_energy();
    start_time_ns_ = std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

EnergyReading EnergyMeter::stop() {
    EnergyReading reading;
    if (!available_) return reading;
    
    double end_energy = read_rapl_energy();
    auto end_time = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    
    reading.elapsed_sec = static_cast<double>(end_time - start_time_ns_) / 1e9;
    reading.joules = (end_energy - start_energy_) / 1e6;  // uJ to J
    reading.avg_watts = reading.elapsed_sec > 0 ? reading.joules / reading.elapsed_sec : 0.0;
    reading.source = source_;
    
    return reading;
}

double EnergyMeter::read_rapl_energy() {
#ifdef __linux__
    if (fd_ >= 0) {
        char buf[64];
        lseek(fd_, 0, SEEK_SET);
        ssize_t n = read(fd_, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            return std::stod(buf);
        }
    }
#endif
    return 0.0;
}

double EnergyMeter::read_perf_energy() { return 0.0; }
uint64_t EnergyMeter::read_tsc() {
#ifdef __linux__
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return (static_cast<uint64_t>(hi) << 32) | lo;
#endif
    return 0;
}

std::string EnergyMeter::detect_available_source() {
#ifdef __linux__
    std::ifstream f("/sys/class/powercap/intel-rapl:0/energy_uj");
    if (f.good()) return "RAPL";
    f.close();
    f.open("/sys/class/powercap/intel-rapl:0:0/energy_uj");
    if (f.good()) return "RAPL";
#endif
    return "";
}

}
