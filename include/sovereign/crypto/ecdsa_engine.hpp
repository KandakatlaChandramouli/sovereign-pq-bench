#pragma once
#include "sovereign/crypto/signature_scheme.hpp"
#include <openssl/evp.h>
#include <memory>

namespace sovereign {

class EcdsaEngine final : public SignatureScheme {
public:
    explicit EcdsaEngine();
    ~EcdsaEngine() override;
    EcdsaEngine(const EcdsaEngine&) = delete;
    EcdsaEngine& operator=(const EcdsaEngine&) = delete;
    EcdsaEngine(EcdsaEngine&&) noexcept = default;
    EcdsaEngine& operator=(EcdsaEngine&&) noexcept = default;

    [[nodiscard]] std::string_view name() const noexcept override { return "ECDSA-P256"; }
    [[nodiscard]] std::size_t public_key_size() const noexcept override { return 65; }
    [[nodiscard]] std::size_t private_key_size() const noexcept override { return 32; }
    [[nodiscard]] std::size_t signature_size() const noexcept override { return 72; }

    [[nodiscard]] CryptoResult<KeyPair> generate_keypair() override;
    [[nodiscard]] CryptoResult<Signature> sign(std::span<const std::byte> message, std::span<const std::byte> private_key) override;
    [[nodiscard]] CryptoResult<bool> verify(std::span<const std::byte> message, std::span<const std::byte> signature, std::span<const std::byte> public_key) override;
};

}
