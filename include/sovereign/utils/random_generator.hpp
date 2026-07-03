#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace sovereign {

class RandomGenerator {
public:
    RandomGenerator();
    explicit RandomGenerator(uint64_t seed);
    ~RandomGenerator();

    RandomGenerator(const RandomGenerator&) = delete;
    RandomGenerator& operator=(const RandomGenerator&) = delete;
    RandomGenerator(RandomGenerator&&) noexcept = default;
    RandomGenerator& operator=(RandomGenerator&&) noexcept = default;

    [[nodiscard]] std::vector<std::byte> generate_bytes(std::size_t count);
    
    void fill_buffer(std::span<std::byte> buffer);

    [[nodiscard]] uint64_t next_uint64();
    
    void reseed(uint64_t seed);

private:
    uint64_t state_[4];
};

}
