#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <system_error>

namespace sovereign {

enum class Algorithm {
    ECDSA_P256,
    ED25519,
    ML_DSA_44,
    ML_DSA_65,
    ML_DSA_87,
    FALCON_512,
    FALCON_1024,
    SPHINCS_128S,
    SPHINCS_128F,
    SPHINCS_192S,
    SPHINCS_192F,
    SPHINCS_256S,
    SPHINCS_256F
};

struct KeyPair {
    std::vector<std::byte> public_key;
    std::vector<std::byte> private_key;
    KeyPair() = default;
    KeyPair(std::vector<std::byte> pub, std::vector<std::byte> priv)
        : public_key(std::move(pub)), private_key(std::move(priv)) {}
    [[nodiscard]] bool is_valid() const noexcept {
        return !public_key.empty() && !private_key.empty();
    }
};

struct Signature {
    std::vector<std::byte> data;
    Algorithm algorithm;
    Signature() = default;
    Signature(std::vector<std::byte> sig, Algorithm algo)
        : data(std::move(sig)), algorithm(algo) {}
    [[nodiscard]] bool is_valid() const noexcept { return !data.empty(); }
};

enum class ErrorCode {
    SUCCESS = 0,
    KEY_GENERATION_FAILED,
    SIGNING_FAILED,
    VERIFICATION_FAILED,
    INVALID_KEY,
    INVALID_SIGNATURE,
    INVALID_MESSAGE,
    SERIALIZATION_FAILED,
    DESERIALIZATION_FAILED,
    UNSUPPORTED_ALGORITHM,
    INTERNAL_ERROR
};

std::error_code make_error_code(ErrorCode ec) noexcept;

template<typename T>
using CryptoResult = std::pair<std::optional<T>, std::error_code>;

class SignatureScheme {
public:
    explicit SignatureScheme(Algorithm algo);
    virtual ~SignatureScheme();
    SignatureScheme(const SignatureScheme&) = delete;
    SignatureScheme& operator=(const SignatureScheme&) = delete;
    SignatureScheme(SignatureScheme&&) noexcept;
    SignatureScheme& operator=(SignatureScheme&&) noexcept;

    [[nodiscard]] Algorithm algorithm() const noexcept { return algo_; }
    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    [[nodiscard]] virtual std::size_t public_key_size() const noexcept = 0;
    [[nodiscard]] virtual std::size_t private_key_size() const noexcept = 0;
    [[nodiscard]] virtual std::size_t signature_size() const noexcept = 0;

    [[nodiscard]] virtual CryptoResult<KeyPair> generate_keypair() = 0;
    [[nodiscard]] virtual CryptoResult<Signature> sign(std::span<const std::byte> message, std::span<const std::byte> private_key) = 0;
    [[nodiscard]] virtual CryptoResult<bool> verify(std::span<const std::byte> message, std::span<const std::byte> signature, std::span<const std::byte> public_key) = 0;

protected:
    Algorithm algo_;
};

}
