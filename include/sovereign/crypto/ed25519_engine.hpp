#pragma once
#include "sovereign/crypto/signature_scheme.hpp"
#include <openssl/evp.h>
#include <memory>

namespace sovereign {
class Ed25519Engine final : public SignatureScheme {
public:
    explicit Ed25519Engine();
    ~Ed25519Engine() override;
    Ed25519Engine(const Ed25519Engine&) = delete;
    Ed25519Engine& operator=(const Ed25519Engine&) = delete;
    Ed25519Engine(Ed25519Engine&&) noexcept = default;
    Ed25519Engine& operator=(Ed25519Engine&&) noexcept = default;
    [[nodiscard]] std::string_view name() const noexcept override { return "Ed25519"; }
    [[nodiscard]] std::size_t public_key_size() const noexcept override { return 32; }
    [[nodiscard]] std::size_t private_key_size() const noexcept override { return 32; }
    [[nodiscard]] std::size_t signature_size() const noexcept override { return 64; }
    [[nodiscard]] CryptoResult<KeyPair> generate_keypair() override;
    [[nodiscard]] CryptoResult<Signature> sign(std::span<const std::byte> message, std::span<const std::byte> private_key) override;
    [[nodiscard]] CryptoResult<bool> verify(std::span<const std::byte> message, std::span<const std::byte> signature, std::span<const std::byte> public_key) override;
};
}
