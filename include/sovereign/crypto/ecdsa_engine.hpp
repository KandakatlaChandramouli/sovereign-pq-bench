#pragma once

#include "sovereign/crypto/signature_scheme.hpp"
#include <openssl/evp.h>
#include <memory>

namespace sovereign {

struct EvpKeyDeleter {
    void operator()(EVP_PKEY* key) const noexcept {
        if (key) EVP_PKEY_free(key);
    }
};

struct EvpCtxDeleter {
    void operator()(EVP_PKEY_CTX* ctx) const noexcept {
        if (ctx) EVP_PKEY_CTX_free(ctx);
    }
};

using EvpKeyPtr = std::unique_ptr<EVP_PKEY, EvpKeyDeleter>;
using EvpCtxPtr = std::unique_ptr<EVP_PKEY_CTX, EvpCtxDeleter>;

class EcdsaEngine final : public SignatureScheme {
public:
    explicit EcdsaEngine();
    ~EcdsaEngine() override;

    EcdsaEngine(const EcdsaEngine&) = delete;
    EcdsaEngine& operator=(const EcdsaEngine&) = delete;
    EcdsaEngine(EcdsaEngine&&) noexcept = default;
    EcdsaEngine& operator=(EcdsaEngine&&) noexcept = default;

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
    [[nodiscard]] CryptoResult<EvpKeyPtr>
    der_to_evp_key(std::span<const std::byte> der_key, bool is_private) const noexcept;

    [[nodiscard]] CryptoResult<std::vector<std::byte>>
    evp_key_to_der(EVP_PKEY* key, bool is_private) const noexcept;
};

}
