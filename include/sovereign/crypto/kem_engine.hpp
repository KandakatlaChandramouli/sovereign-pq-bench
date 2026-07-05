#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <system_error>
#include <memory>

namespace sovereign {

enum class KemAlgorithm {
    ECDH_P256,
    ECDH_X25519,
    ML_KEM_512,
    ML_KEM_768,
    ML_KEM_1024,
    CLASSIC_MCELIECE_348864,
    HQC_128,
    BIKE_L1,
    FRODO_976_AES
};

struct KemKeyPair {
    std::vector<std::byte> public_key;
    std::vector<std::byte> private_key;
};

struct KemCiphertext {
    std::vector<std::byte> data;
    std::vector<std::byte> shared_secret;
    double encaps_ns{0.0};
    double decaps_ns{0.0};
};

class KemEngine {
public:
    explicit KemEngine(KemAlgorithm algo);
    ~KemEngine();

    KemEngine(const KemEngine&) = delete;
    KemEngine& operator=(const KemEngine&) = delete;
    KemEngine(KemEngine&&) noexcept = default;
    KemEngine& operator=(KemEngine&&) noexcept = default;

    [[nodiscard]] std::string_view name() const noexcept;
    [[nodiscard]] std::size_t public_key_size() const noexcept;
    [[nodiscard]] std::size_t private_key_size() const noexcept;
    [[nodiscard]] std::size_t ciphertext_size() const noexcept;
    [[nodiscard]] std::size_t shared_secret_size() const noexcept;

    [[nodiscard]] std::pair<std::optional<KemKeyPair>, std::error_code> generate_keypair();
    [[nodiscard]] std::pair<std::optional<KemCiphertext>, std::error_code> encapsulate(std::span<const std::byte> public_key);
    [[nodiscard]] std::pair<std::optional<KemCiphertext>, std::error_code> decapsulate(std::span<const std::byte> ciphertext, std::span<const std::byte> private_key);

private:
    KemAlgorithm algo_;
    void* kem_;  // OQS_KEM*
    std::string name_;
};

}
