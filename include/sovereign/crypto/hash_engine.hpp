#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <memory>

namespace sovereign {

struct HashResult {
    std::vector<std::byte> digest;
    double latency_ns;
    double throughput_mbps;
};

enum class HashAlgorithm {
    SHA_256,
    SHA_512,
    SHA3_256,
    SHA3_512,
    BLAKE3_256
};

class HashEngine {
public:
    explicit HashEngine(HashAlgorithm algo);
    ~HashEngine();
    
    [[nodiscard]] std::string_view name() const noexcept;
    [[nodiscard]] std::size_t digest_size() const noexcept;
    [[nodiscard]] HashResult hash(std::span<const std::byte> data);

private:
    HashAlgorithm algo_;
};

}
