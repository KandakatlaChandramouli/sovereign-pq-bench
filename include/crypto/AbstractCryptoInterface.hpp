#pragma once

#include <memory>
#include <span>
#include <string>
#include <vector>

#include "core/Types.hpp"

namespace spqb
{

class AbstractCryptoInterface
{
public:
    virtual ~AbstractCryptoInterface() = default;

    [[nodiscard]]
    virtual std::string name() const noexcept = 0;

    [[nodiscard]]
    virtual SignatureAlgorithm algorithm() const noexcept = 0;

    [[nodiscard]]
    virtual KeyPair generateKeyPair() = 0;

    [[nodiscard]]
    virtual std::vector<std::uint8_t>
    sign(std::span<const std::uint8_t> message) = 0;

    [[nodiscard]]
    virtual bool
    verify(
        std::span<const std::uint8_t> message,
        std::span<const std::uint8_t> signature,
        std::span<const std::uint8_t> publicKey) = 0;

    [[nodiscard]]
    virtual std::size_t publicKeySize() const noexcept = 0;

    [[nodiscard]]
    virtual std::size_t privateKeySize() const noexcept = 0;

    [[nodiscard]]
    virtual std::size_t signatureSize() const noexcept = 0;
};

using CryptoPtr = std::unique_ptr<AbstractCryptoInterface>;

} // namespace spqb
