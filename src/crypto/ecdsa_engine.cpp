#include "sovereign/crypto/ecdsa_engine.hpp"
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <cstring>
#include <system_error>

namespace sovereign {

EcdsaEngine::EcdsaEngine() : SignatureScheme(Algorithm::ECDSA_P256) {}

EcdsaEngine::~EcdsaEngine() = default;

std::string_view EcdsaEngine::name() const noexcept {
    return "ECDSA-P256";
}

std::size_t EcdsaEngine::public_key_size() const noexcept {
    return 91;
}

std::size_t EcdsaEngine::private_key_size() const noexcept {
    return 32;
}

std::size_t EcdsaEngine::signature_size() const noexcept {
    return 72;
}

CryptoResult<KeyPair> EcdsaEngine::generate_keypair() {
    EvpCtxPtr ctx(EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr));
    if (!ctx) {
        return {std::nullopt, make_error_code(ErrorCode::KEY_GENERATION_FAILED)};
    }

    if (EVP_PKEY_keygen_init(ctx.get()) <= 0) {
        return {std::nullopt, make_error_code(ErrorCode::KEY_GENERATION_FAILED)};
    }

    if (EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx.get(), NID_X9_62_prime256v1) <= 0) {
        return {std::nullopt, make_error_code(ErrorCode::KEY_GENERATION_FAILED)};
    }

    EVP_PKEY* raw_key = nullptr;
    if (EVP_PKEY_keygen(ctx.get(), &raw_key) <= 0) {
        return {std::nullopt, make_error_code(ErrorCode::KEY_GENERATION_FAILED)};
    }

    EvpKeyPtr key(raw_key);

    auto pub_result = evp_key_to_der(key.get(), false);
    if (!pub_result.first) {
        return {std::nullopt, pub_result.second};
    }

    auto priv_result = evp_key_to_der(key.get(), true);
    if (!priv_result.first) {
        return {std::nullopt, priv_result.second};
    }

    KeyPair kp(std::move(*pub_result.first), std::move(*priv_result.first));
    return {std::move(kp), std::error_code{}};
}

CryptoResult<Signature> EcdsaEngine::sign(
    std::span<const std::byte> message,
    std::span<const std::byte> private_key) {

    auto evp_result = der_to_evp_key(private_key, true);
    if (!evp_result.first) {
        return {std::nullopt, evp_result.second};
    }

    EvpCtxPtr ctx(EVP_PKEY_CTX_new(evp_result.first->get(), nullptr));
    if (!ctx) {
        return {std::nullopt, make_error_code(ErrorCode::SIGNING_FAILED)};
    }

    if (EVP_PKEY_sign_init(ctx.get()) <= 0) {
        return {std::nullopt, make_error_code(ErrorCode::SIGNING_FAILED)};
    }

    std::size_t sig_len = signature_size();
    std::vector<std::byte> sig_data(sig_len);

    if (EVP_PKEY_sign(ctx.get(),
                      reinterpret_cast<unsigned char*>(sig_data.data()),
                      &sig_len,
                      reinterpret_cast<const unsigned char*>(message.data()),
                      message.size()) <= 0) {
        return {std::nullopt, make_error_code(ErrorCode::SIGNING_FAILED)};
    }

    sig_data.resize(sig_len);
    Signature sig(std::move(sig_data), Algorithm::ECDSA_P256);
    return {std::move(sig), std::error_code{}};
}

CryptoResult<bool> EcdsaEngine::verify(
    std::span<const std::byte> message,
    std::span<const std::byte> signature,
    std::span<const std::byte> public_key) {

    auto evp_result = der_to_evp_key(public_key, false);
    if (!evp_result.first) {
        return {std::nullopt, evp_result.second};
    }

    EvpCtxPtr ctx(EVP_PKEY_CTX_new(evp_result.first->get(), nullptr));
    if (!ctx) {
        return {std::nullopt, make_error_code(ErrorCode::VERIFICATION_FAILED)};
    }

    if (EVP_PKEY_verify_init(ctx.get()) <= 0) {
        return {std::nullopt, make_error_code(ErrorCode::VERIFICATION_FAILED)};
    }

    int result = EVP_PKEY_verify(ctx.get(),
                                  reinterpret_cast<const unsigned char*>(signature.data()),
                                  signature.size(),
                                  reinterpret_cast<const unsigned char*>(message.data()),
                                  message.size());

    if (result < 0) {
        return {std::nullopt, make_error_code(ErrorCode::VERIFICATION_FAILED)};
    }

    return {result == 1, std::error_code{}};
}

CryptoResult<std::vector<std::byte>> EcdsaEngine::export_public_key_pem(
    std::span<const std::byte> public_key) {

    auto evp_result = der_to_evp_key(public_key, false);
    if (!evp_result.first) {
        return {std::nullopt, evp_result.second};
    }

    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) {
        return {std::nullopt, make_error_code(ErrorCode::SERIALIZATION_FAILED)};
    }

    if (PEM_write_bio_PUBKEY(bio, evp_result.first->get()) <= 0) {
        BIO_free(bio);
        return {std::nullopt, make_error_code(ErrorCode::SERIALIZATION_FAILED)};
    }

    char* data = nullptr;
    long len = BIO_get_mem_data(bio, &data);
    
    std::vector<std::byte> result(static_cast<std::size_t>(len));
    std::memcpy(result.data(), data, static_cast<std::size_t>(len));
    
    BIO_free(bio);
    return {std::move(result), std::error_code{}};
}

CryptoResult<std::vector<std::byte>> EcdsaEngine::export_private_key_pem(
    std::span<const std::byte> private_key) {

    auto evp_result = der_to_evp_key(private_key, true);
    if (!evp_result.first) {
        return {std::nullopt, evp_result.second};
    }

    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) {
        return {std::nullopt, make_error_code(ErrorCode::SERIALIZATION_FAILED)};
    }

    if (PEM_write_bio_PrivateKey(bio, evp_result.first->get(), nullptr, nullptr, 0, nullptr, nullptr) <= 0) {
        BIO_free(bio);
        return {std::nullopt, make_error_code(ErrorCode::SERIALIZATION_FAILED)};
    }

    char* data = nullptr;
    long len = BIO_get_mem_data(bio, &data);
    
    std::vector<std::byte> result(static_cast<std::size_t>(len));
    std::memcpy(result.data(), data, static_cast<std::size_t>(len));
    
    BIO_free(bio);
    return {std::move(result), std::error_code{}};
}

CryptoResult<std::vector<std::byte>> EcdsaEngine::import_public_key_pem(
    std::span<const std::byte> pem_data) {

    BIO* bio = BIO_new_mem_buf(pem_data.data(), static_cast<int>(pem_data.size()));
    if (!bio) {
        return {std::nullopt, make_error_code(ErrorCode::DESERIALIZATION_FAILED)};
    }

    EVP_PKEY* raw_key = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    if (!raw_key) {
        return {std::nullopt, make_error_code(ErrorCode::DESERIALIZATION_FAILED)};
    }

    EvpKeyPtr key(raw_key);
    return evp_key_to_der(key.get(), false);
}

CryptoResult<std::vector<std::byte>> EcdsaEngine::import_private_key_pem(
    std::span<const std::byte> pem_data) {

    BIO* bio = BIO_new_mem_buf(pem_data.data(), static_cast<int>(pem_data.size()));
    if (!bio) {
        return {std::nullopt, make_error_code(ErrorCode::DESERIALIZATION_FAILED)};
    }

    EVP_PKEY* raw_key = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    if (!raw_key) {
        return {std::nullopt, make_error_code(ErrorCode::DESERIALIZATION_FAILED)};
    }

    EvpKeyPtr key(raw_key);
    return evp_key_to_der(key.get(), true);
}

CryptoResult<EvpKeyPtr> EcdsaEngine::der_to_evp_key(
    std::span<const std::byte> der_key, bool is_private) const noexcept {

    const unsigned char* data = reinterpret_cast<const unsigned char*>(der_key.data());
    
    EVP_PKEY* raw_key = nullptr;
    if (is_private) {
        raw_key = d2i_AutoPrivateKey(nullptr, &data, static_cast<long>(der_key.size()));
    } else {
        raw_key = d2i_PUBKEY(nullptr, &data, static_cast<long>(der_key.size()));
    }

    if (!raw_key) {
        return {std::nullopt, make_error_code(ErrorCode::DESERIALIZATION_FAILED)};
    }

    return {EvpKeyPtr(raw_key), std::error_code{}};
}

CryptoResult<std::vector<std::byte>> EcdsaEngine::evp_key_to_der(
    EVP_PKEY* key, bool is_private) const noexcept {

    unsigned char* der = nullptr;
    int len = 0;

    if (is_private) {
        len = i2d_PrivateKey(key, &der);
    } else {
        len = i2d_PUBKEY(key, &der);
    }

    if (len <= 0 || !der) {
        return {std::nullopt, make_error_code(ErrorCode::SERIALIZATION_FAILED)};
    }

    std::vector<std::byte> result(static_cast<std::size_t>(len));
    std::memcpy(result.data(), der, static_cast<std::size_t>(len));
    OPENSSL_free(der);
    
    return {std::move(result), std::error_code{}};
}

}
