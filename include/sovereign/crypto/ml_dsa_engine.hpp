#pragma once

#include "sovereign/crypto/signature_scheme.hpp"
#include <oqs/oqs.h>
#include <string>

namespace sovereign {

class MlDsaEngine : public SignatureScheme {
public:
    explicit MlDsaEngine(const char* oqs_alg, Algorithm algo, const char* display_name);
    ~MlDsaEngine() override;

    MlDsaEngine(const MlDsaEngine&) = delete;
    MlDsaEngine& operator=(const MlDsaEngine&) = delete;
    MlDsaEngine(MlDsaEngine&&) noexcept = default;
    MlDsaEngine& operator=(MlDsaEngine&&) noexcept = default;

    [[nodiscard]] std::string_view name() const noexcept override;
    [[nodiscard]] std::size_t public_key_size() const noexcept override;
    [[nodiscard]] std::size_t private_key_size() const noexcept override;
    [[nodiscard]] std::size_t signature_size() const noexcept override;

    [[nodiscard]] CryptoResult<KeyPair> generate_keypair() override;
    [[nodiscard]] CryptoResult<Signature> sign(std::span<const std::byte> message, std::span<const std::byte> private_key) override;
    [[nodiscard]] CryptoResult<bool> verify(std::span<const std::byte> message, std::span<const std::byte> signature, std::span<const std::byte> public_key) override;

private:
    OQS_SIG* sig_;
    std::string display_name_;
};

class MlDsa44Engine final : public MlDsaEngine {
public:
    MlDsa44Engine() : MlDsaEngine(OQS_SIG_alg_ml_dsa_44, Algorithm::ML_DSA_44, "ML-DSA-44") {}
};

class MlDsa65Engine final : public MlDsaEngine {
public:
    MlDsa65Engine() : MlDsaEngine(OQS_SIG_alg_ml_dsa_65, Algorithm::ML_DSA_65, "ML-DSA-65") {}
};

class MlDsa87Engine final : public MlDsaEngine {
public:
    MlDsa87Engine() : MlDsaEngine(OQS_SIG_alg_ml_dsa_87, Algorithm::ML_DSA_87, "ML-DSA-87") {}
};

}
