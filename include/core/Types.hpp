#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace spqb {

enum class SignatureAlgorithm
{
    ECDSA_P256,
    ED25519,
    ML_DSA_44
};

enum class HashAlgorithm
{
    SHA256,
    SHA3_256,
    BLAKE3
};

struct KeyPair
{
    std::vector<std::uint8_t> publicKey;
    std::vector<std::uint8_t> privateKey;
};

struct BenchmarkResult
{
    std::string algorithm;

    double mean_ns = 0.0;
    double median_ns = 0.0;
    double p99_ns = 0.0;

    double throughput = 0.0;

    std::size_t publicKeySize = 0;
    std::size_t signatureSize = 0;
};

} // namespace spqb
