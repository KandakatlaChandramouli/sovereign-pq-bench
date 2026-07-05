#include "sovereign/crypto/hash_engine.hpp"
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <cstring>
#include <chrono>

namespace sovereign {

HashEngine::HashEngine(HashAlgorithm algo) : algo_(algo) {}
HashEngine::~HashEngine() = default;

std::string_view HashEngine::name() const noexcept {
    switch (algo_) {
        case HashAlgorithm::SHA_256: return "SHA-256";
        case HashAlgorithm::SHA_512: return "SHA-512";
        case HashAlgorithm::SHA3_256: return "SHA3-256";
        case HashAlgorithm::SHA3_512: return "SHA3-512";
        case HashAlgorithm::BLAKE3_256: return "BLAKE3-256";
    }
    return "Unknown";
}

std::size_t HashEngine::digest_size() const noexcept {
    switch (algo_) {
        case HashAlgorithm::SHA_256: return 32;
        case HashAlgorithm::SHA_512: return 64;
        case HashAlgorithm::SHA3_256: return 32;
        case HashAlgorithm::SHA3_512: return 64;
        case HashAlgorithm::BLAKE3_256: return 32;
    }
    return 0;
}

HashResult HashEngine::hash(std::span<const std::byte> data) {
    HashResult result;
    result.digest.resize(digest_size());
    
    auto start = std::chrono::high_resolution_clock::now();
    
    if (algo_ == HashAlgorithm::SHA_256) {
        SHA256(reinterpret_cast<const unsigned char*>(data.data()), data.size(),
               reinterpret_cast<unsigned char*>(result.digest.data()));
    } else if (algo_ == HashAlgorithm::SHA_512) {
        SHA512(reinterpret_cast<const unsigned char*>(data.data()), data.size(),
               reinterpret_cast<unsigned char*>(result.digest.data()));
    } else if (algo_ == HashAlgorithm::SHA3_256) {
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        EVP_DigestInit_ex(ctx, EVP_sha3_256(), nullptr);
        EVP_DigestUpdate(ctx, data.data(), data.size());
        EVP_DigestFinal_ex(ctx, reinterpret_cast<unsigned char*>(result.digest.data()), nullptr);
        EVP_MD_CTX_free(ctx);
    } else if (algo_ == HashAlgorithm::SHA3_512) {
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        EVP_DigestInit_ex(ctx, EVP_sha3_512(), nullptr);
        EVP_DigestUpdate(ctx, data.data(), data.size());
        EVP_DigestFinal_ex(ctx, reinterpret_cast<unsigned char*>(result.digest.data()), nullptr);
        EVP_MD_CTX_free(ctx);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.latency_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    result.throughput_mbps = (data.size() / (result.latency_ns / 1e9)) / (1024.0 * 1024.0);
    
    return result;
}

}
