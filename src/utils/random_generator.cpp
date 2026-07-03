#include "sovereign/utils/random_generator.hpp"
#include <cstring>
#include <random>
#include <chrono>

namespace sovereign {

RandomGenerator::RandomGenerator() {
    auto seed = static_cast<uint64_t>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()
    );
    reseed(seed);
}

RandomGenerator::RandomGenerator(uint64_t seed) {
    reseed(seed);
}

RandomGenerator::~RandomGenerator() = default;

std::vector<std::byte> RandomGenerator::generate_bytes(std::size_t count) {
    std::vector<std::byte> buffer(count);
    fill_buffer(buffer);
    return buffer;
}

void RandomGenerator::fill_buffer(std::span<std::byte> buffer) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned char> dist(0, 255);

    for (auto& byte : buffer) {
        byte = static_cast<std::byte>(dist(gen));
    }
}

uint64_t RandomGenerator::next_uint64() {
    state_[0] = state_[0] * 6364136223846793005ULL + 1442695040888963407ULL;
    uint64_t x = state_[0];
    x ^= x >> 33;
    x *= 0xFF51AFD7ED558CCDULL;
    x ^= x >> 33;
    x *= 0xC4CEB9FE1A85EC53ULL;
    x ^= x >> 33;
    return x;
}

void RandomGenerator::reseed(uint64_t seed) {
    std::mt19937_64 gen(seed);
    for (int i = 0; i < 4; ++i) {
        state_[i] = gen();
    }
}

}
