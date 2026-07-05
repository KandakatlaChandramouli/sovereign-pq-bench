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
#include <chrono>

namespace sovereign {

enum class FullKemAlgorithm {
    ECDH_P256, ECDH_X25519,
    ML_KEM_512, ML_KEM_768, ML_KEM_1024,
    CLASSIC_MCELIECE_348864, CLASSIC_MCELIECE_460896,
    HQC_128, HQC_192, HQC_256,
    BIKE_L1, BIKE_L3,
    FRODO_976_AES, FRODO_1344_AES,
    NTRU_HPS_2048509
};

struct FullKemKeyPair { std::vector<std::byte> public_key, private_key; };
struct FullKemResult { std::vector<std::byte> ct, shared_secret; double enc_us, dec_us; };

class FullKemEngine {
public:
    explicit FullKemEngine(FullKemAlgorithm algo);
    ~FullKemEngine();
    [[nodiscard]] std::string_view name() const noexcept;
    [[nodiscard]] std::size_t pk_size() const, sk_size() const, ct_size() const, ss_size() const;
    [[nodiscard]] std::pair<std::optional<FullKemKeyPair>, std::error_code> keygen();
    [[nodiscard]] std::pair<std::optional<FullKemResult>, std::error_code> encaps(std::span<const std::byte> pk);
    [[nodiscard]] std::pair<std::optional<FullKemResult>, std::error_code> decaps(std::span<const std::byte> ct, std::span<const std::byte> sk);
private:
    FullKemAlgorithm algo_; void* kem_; std::string name_;
};

// Implementation inline since it's heavily templated
inline FullKemEngine::FullKemEngine(FullKemAlgorithm algo) : algo_(algo), kem_(nullptr) {
    auto init = [&](const char* oqs_name, const char* display) {
        kem_ = OQS_KEM_new(oqs_name);
        if (!kem_) name_ = std::string("N/A: ") + display;
        else name_ = display;
    };
    switch (algo) {
        case FullKemAlgorithm::ML_KEM_512: init(OQS_KEM_alg_ml_kem_512, "ML-KEM-512"); break;
        case FullKemAlgorithm::ML_KEM_768: init(OQS_KEM_alg_ml_kem_768, "ML-KEM-768"); break;
        case FullKemAlgorithm::ML_KEM_1024: init(OQS_KEM_alg_ml_kem_1024, "ML-KEM-1024"); break;
        case FullKemAlgorithm::CLASSIC_MCELIECE_348864: init(OQS_KEM_alg_classic_mceliece_348864, "Classic-McEliece-348864"); break;
        case FullKemAlgorithm::HQC_128: init(OQS_KEM_alg_hqc_128, "HQC-128"); break;
        case FullKemAlgorithm::BIKE_L1: init(OQS_KEM_alg_bike_l1, "BIKE-L1"); break;
        case FullKemAlgorithm::FRODO_976_AES: init(OQS_KEM_alg_frodokem_976_aes, "FrodoKEM-976-AES"); break;
        default: name_ = "Unsupported"; break;
    }
}
inline FullKemEngine::~FullKemEngine() { if (kem_) OQS_KEM_free(static_cast<OQS_KEM*>(kem_)); }
inline std::string_view FullKemEngine::name() const noexcept { return name_; }
inline std::size_t FullKemEngine::pk_size() const { return kem_ ? static_cast<OQS_KEM*>(kem_)->length_public_key : 0; }
inline std::size_t FullKemEngine::ct_size() const { return kem_ ? static_cast<OQS_KEM*>(kem_)->length_ciphertext : 0; }

inline std::pair<std::optional<FullKemKeyPair>, std::error_code> FullKemEngine::keygen() {
    if (!kem_) return {std::nullopt, std::make_error_code(std::errc::operation_not_permitted)};
    auto* k = static_cast<OQS_KEM*>(kem_);
    FullKemKeyPair kp;
    kp.public_key.resize(k->length_public_key);
    kp.private_key.resize(k->length_secret_key);
    if (OQS_KEM_keypair(k, (uint8_t*)kp.public_key.data(), (uint8_t*)kp.private_key.data()) != OQS_SUCCESS)
        return {std::nullopt, std::make_error_code(std::errc::operation_not_permitted)};
    return {std::move(kp), std::error_code{}};
}
inline std::pair<std::optional<FullKemResult>, std::error_code> FullKemEngine::encaps(std::span<const std::byte> pk) {
    if (!kem_) return {std::nullopt, std::make_error_code(std::errc::operation_not_permitted)};
    auto* k = static_cast<OQS_KEM*>(kem_);
    FullKemResult r;
    r.ct.resize(k->length_ciphertext); r.shared_secret.resize(k->length_shared_secret);
    auto t1 = std::chrono::high_resolution_clock::now();
    if (OQS_KEM_encaps(k, (uint8_t*)r.ct.data(), (uint8_t*)r.shared_secret.data(), (const uint8_t*)pk.data()) != OQS_SUCCESS)
        return {std::nullopt, std::make_error_code(std::errc::operation_not_permitted)};
    r.enc_us = std::chrono::duration<double,std::micro>(std::chrono::high_resolution_clock::now() - t1).count();
    return {std::move(r), std::error_code{}};
}
inline std::pair<std::optional<FullKemResult>, std::error_code> FullKemEngine::decaps(std::span<const std::byte> ct, std::span<const std::byte> sk) {
    if (!kem_) return {std::nullopt, std::make_error_code(std::errc::operation_not_permitted)};
    auto* k = static_cast<OQS_KEM*>(kem_);
    FullKemResult r; r.shared_secret.resize(k->length_shared_secret);
    auto t1 = std::chrono::high_resolution_clock::now();
    if (OQS_KEM_decaps(k, (uint8_t*)r.shared_secret.data(), (const uint8_t*)ct.data(), (const uint8_t*)sk.data()) != OQS_SUCCESS)
        return {std::nullopt, std::make_error_code(std::errc::operation_not_permitted)};
    r.dec_us = std::chrono::duration<double,std::micro>(std::chrono::high_resolution_clock::now() - t1).count();
    return {std::move(r), std::error_code{}};
}

}
