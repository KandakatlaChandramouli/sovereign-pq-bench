#include "sovereign/utils/timer.hpp"
#include <iostream>

namespace sovereign {

HighResolutionTimer::HighResolutionTimer() noexcept
    : start_time_(std::chrono::high_resolution_clock::now()) {}

void HighResolutionTimer::start() noexcept {
    start_time_ = std::chrono::high_resolution_clock::now();
}

uint64_t HighResolutionTimer::elapsed_ns() const noexcept {
    auto now = std::chrono::high_resolution_clock::now();
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_time_).count()
    );
}

double HighResolutionTimer::elapsed_us() const noexcept {
    return static_cast<double>(elapsed_ns()) / 1000.0;
}

double HighResolutionTimer::elapsed_ms() const noexcept {
    return static_cast<double>(elapsed_ns()) / 1e6;
}

void HighResolutionTimer::reset() noexcept {
    start_time_ = std::chrono::high_resolution_clock::now();
}

uint64_t HighResolutionTimer::now_ns() noexcept {
    auto now = std::chrono::high_resolution_clock::now();
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()
        ).count()
    );
}

ScopedTimer::ScopedTimer(std::string_view label) noexcept
    : label_(label) {
    timer_.start();
}

ScopedTimer::~ScopedTimer() {
    uint64_t elapsed = timer_.elapsed_ns();
    std::cout << "[TIMER] " << label_ << ": " 
              << static_cast<double>(elapsed) / 1e6 << " ms\n";
}

uint64_t ScopedTimer::elapsed_ns() const noexcept {
    return timer_.elapsed_ns();
}

}
