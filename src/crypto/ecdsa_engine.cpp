#include "sovereign/crypto/ecdsa_engine.hpp"
#include <openssl/ec.h>
#include <openssl/err.h>
#include <cstring>
#include <system_error>

namespace sovereign {

EcdsaEngine::EcdsaEngine() : SignatureScheme(Algorithm::ECDSA_P256) {}
EcdsaEngine::~EcdsaEngine() = default;

CryptoResult<KeyPair> EcdsaEngine::generate_keypair() {
    EVP_PKEY* key = EVP_EC_gen("P-256");
    if (!key) return {std::nullopt, make_error_code(ErrorCode::KEY_GENERATION_FAILED)};

    std::size_t pub_len = public_key_size();
    std::size_t priv_len = 121;
    std::vector<std::byte> pub(pub_len), priv(priv_len);
    if (!EVP_PKEY_get_raw_public_key(key, reinterpret_cast<unsigned char*>(pub.data()), &pub_len)) {
        EVP_PKEY_free(key);
        return {std::nullopt, make_error_code(ErrorCode::KEY_GENERATION_FAILED)};
    }
    if (!EVP_PKEY_get_raw_private_key(key, reinterpret_cast<unsigned char*>(priv.data()), &priv_len)) {
        EVP_PKEY_free(key);
        return {std::nullopt, make_error_code(ErrorCode::KEY_GENERATION_FAILED)};
    }
    EVP_PKEY_free(key);
    priv.resize(priv_len);
    pub.resize(pub_len);
    return {KeyPair(std::move(pub), std::move(priv)), std::error_code{}};
}

CryptoResult<Signature> EcdsaEngine::sign(std::span<const std::byte> message, std::span<const std::byte> private_key) {
    EVP_PKEY* key = EVP_PKEY_new_raw_private_key(EVP_PKEY_EC, nullptr,
        reinterpret_cast<const unsigned char*>(private_key.data()), private_key.size());
    if (!key) return {std::nullopt, make_error_code(ErrorCode::SIGNING_FAILED)};

    EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
    if (!md_ctx) { EVP_PKEY_free(key); return {std::nullopt, make_error_code(ErrorCode::SIGNING_FAILED)}; }
    
    EVP_DigestSignInit(md_ctx, nullptr, nullptr, nullptr, key);
    std::size_t sig_len = signature_size() + 16;
    std::vector<std::byte> sig(sig_len);
    
    if (!EVP_DigestSign(md_ctx, reinterpret_cast<unsigned char*>(sig.data()), &sig_len,
                   reinterpret_cast<const unsigned char*>(message.data()), message.size())) {
        EVP_MD_CTX_free(md_ctx);
        EVP_PKEY_free(key);
        return {std::nullopt, make_error_code(ErrorCode::SIGNING_FAILED)};
    }
    EVP_MD_CTX_free(md_ctx);
    EVP_PKEY_free(key);
    sig.resize(sig_len);
    return {Signature(std::move(sig), Algorithm::ECDSA_P256), std::error_code{}};
}

CryptoResult<bool> EcdsaEngine::verify(std::span<const std::byte> message, std::span<const std::byte> signature, std::span<const std::byte> public_key) {
    EVP_PKEY* key = EVP_PKEY_new_raw_public_key(EVP_PKEY_EC, nullptr,
        reinterpret_cast<const unsigned char*>(public_key.data()), public_key.size());
    if (!key) return {std::nullopt, make_error_code(ErrorCode::VERIFICATION_FAILED)};

    EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
    if (!md_ctx) { EVP_PKEY_free(key); return {std::nullopt, make_error_code(ErrorCode::VERIFICATION_FAILED)}; }
    
    EVP_DigestVerifyInit(md_ctx, nullptr, nullptr, nullptr, key);
    int result = EVP_DigestVerify(md_ctx, reinterpret_cast<const unsigned char*>(signature.data()), signature.size(),
                                   reinterpret_cast<const unsigned char*>(message.data()), message.size());
    EVP_MD_CTX_free(md_ctx);
    EVP_PKEY_free(key);
    
    if (result < 0) return {std::nullopt, make_error_code(ErrorCode::VERIFICATION_FAILED)};
    return {result == 1, std::error_code{}};
}

}
