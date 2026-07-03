#pragma once

#include "sovereign/crypto/signature_scheme.hpp"
#include <oqs/oqs.h>
#include <string>

namespace sovereign {

class PqSignatureEngine : public SignatureScheme {
public:
    PqSignatureEngine(const char* oqs_alg, Algorithm algo, const char* display_name);
    ~PqSignatureEngine() override;

    PqSignatureEngine(const PqSignatureEngine&) = delete;
    PqSignatureEngine& operator=(const PqSignatureEngine&) = delete;
    PqSignatureEngine(PqSignatureEngine&&) noexcept = default;
    PqSignatureEngine& operator=(PqSignatureEngine&&) noexcept = default;

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

// Falcon
class Falcon512Engine final : public PqSignatureEngine {
public:
    Falcon512Engine() : PqSignatureEngine(OQS_SIG_alg_falcon_512, Algorithm::FALCON_512, "Falcon-512") {}
};

class Falcon1024Engine final : public PqSignatureEngine {
public:
    Falcon1024Engine() : PqSignatureEngine(OQS_SIG_alg_falcon_1024, Algorithm::FALCON_1024, "Falcon-1024") {}
};

// SPHINCS+
class Sphincs128sEngine final : public PqSignatureEngine {
public:
    Sphincs128sEngine() : PqSignatureEngine(OQS_SIG_alg_sphincs_sha2_128s_simple, Algorithm::SPHINCS_128S, "SPHINCS+-128s") {}
};

class Sphincs128fEngine final : public PqSignatureEngine {
public:
    Sphincs128fEngine() : PqSignatureEngine(OQS_SIG_alg_sphincs_sha2_128f_simple, Algorithm::SPHINCS_128F, "SPHINCS+-128f") {}
};

class Sphincs192sEngine final : public PqSignatureEngine {
public:
    Sphincs192sEngine() : PqSignatureEngine(OQS_SIG_alg_sphincs_sha2_192s_simple, Algorithm::SPHINCS_192S, "SPHINCS+-192s") {}
};

class Sphincs256sEngine final : public PqSignatureEngine {
public:
    Sphincs256sEngine() : PqSignatureEngine(OQS_SIG_alg_sphincs_sha2_256s_simple, Algorithm::SPHINCS_256S, "SPHINCS+-256s") {}
};

}
