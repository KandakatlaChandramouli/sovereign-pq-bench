#pragma once

#include "crypto/AbstractCryptoInterface.hpp"

namespace spqb
{

class EcdsaAdapter final : public AbstractCryptoInterface
{
public:
    EcdsaAdapter() = default;
    ~EcdsaAdapter() override = default;

    [[nodiscard]]
    std::string name() const noexcept override;

    [[nodiscard]]
    SignatureAlgorithm algorithm() const noexcept override;

    [[nodiscard]]
    KeyPair generateKeyPair() override;

    [[nodiscard]]
    std::vector<std::uint8_t>
    sign(std::span<const std::uint8_t> message) override;

    [[nodiscard]]
    bool verify(
        std::span<const std::uint8_t> message,
        std::span<const std::uint8_t> signature,
        std::span<const std::uint8_t> publicKey) override;

    [[nodiscard]]
    std::size_t publicKeySize() const noexcept override;

    [[nodiscard]]
    std::size_t privateKeySize() const noexcept override;

    [[nodiscard]]
    std::size_t signatureSize() const noexcept override;
};

} // namespace spqb
