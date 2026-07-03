#pragma once

#include <chrono>
#include <cstdint>
#include <string_view>

namespace sovereign {

class HighResolutionTimer {
public:
    HighResolutionTimer() noexcept;
    ~HighResolutionTimer() = default;

    HighResolutionTimer(const HighResolutionTimer&) = default;
    HighResolutionTimer& operator=(const HighResolutionTimer&) = default;
    HighResolutionTimer(HighResolutionTimer&&) noexcept = default;
    HighResolutionTimer& operator=(HighResolutionTimer&&) noexcept = default;

    void start() noexcept;
    
    [[nodiscard]] uint64_t elapsed_ns() const noexcept;
    [[nodiscard]] double elapsed_us() const noexcept;
    [[nodiscard]] double elapsed_ms() const noexcept;
    
    void reset() noexcept;

    [[nodiscard]] static uint64_t now_ns() noexcept;
    
    [[nodiscard]] static constexpr std::string_view name() noexcept {
        return "HighResolutionTimer";
    }

private:
    std::chrono::high_resolution_clock::time_point start_time_;
};

class ScopedTimer {
public:
    explicit ScopedTimer(std::string_view label) noexcept;
    ~ScopedTimer();

    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;
    ScopedTimer(ScopedTimer&&) = delete;
    ScopedTimer& operator=(ScopedTimer&&) = delete;

    [[nodiscard]] uint64_t elapsed_ns() const noexcept;

private:
    HighResolutionTimer timer_;
    std::string_view label_;
};

}
