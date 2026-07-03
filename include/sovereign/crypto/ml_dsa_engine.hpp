#pragma once

#include "sovereign/crypto/signature_scheme.hpp"
#include <oqs/oqs.h>
#include <memory>
#include <string>

namespace sovereign {

class MlDsaEngine final : public SignatureScheme {
public:
    explicit MlDsaEngine();
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

    [[nodiscard]] CryptoResult<Signature>
    sign(std::span<const std::byte> message,
         std::span<const std::byte> private_key) override;

    [[nodiscard]] CryptoResult<bool>
    verify(std::span<const std::byte> message,
           std::span<const std::byte> signature,
           std::span<const std::byte> public_key) override;

    [[nodiscard]] CryptoResult<std::vector<std::byte>>
    export_public_key_pem(std::span<const std::byte> public_key) override;

    [[nodiscard]] CryptoResult<std::vector<std::byte>>
    export_private_key_pem(std::span<const std::byte> private_key) override;

    [[nodiscard]] CryptoResult<std::vector<std::byte>>
    import_public_key_pem(std::span<const std::byte> pem_data) override;

    [[nodiscard]] CryptoResult<std::vector<std::byte>>
    import_private_key_pem(std::span<const std::byte> pem_data) override;

private:
    OQS_SIG* sig_;
    std::string algorithm_name_;
};

}
