#include "sovereign/crypto/ecdsa_engine.hpp"
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/obj_mac.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <cstring>
#include <system_error>

namespace sovereign {

EcdsaEngine::EcdsaEngine() : SignatureScheme(Algorithm::ECDSA_P256) {}
EcdsaEngine::~EcdsaEngine() = default;

CryptoResult<KeyPair> EcdsaEngine::generate_keypair() {
    // Generate EC key
    EVP_PKEY* key = EVP_EC_gen("P-256");
    if (!key) {
        EVP_PKEY* key2 = nullptr;
        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
        if (ctx) {
            EVP_PKEY_keygen_init(ctx);
            EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, NID_X9_62_prime256v1);
            EVP_PKEY_keygen(ctx, &key2);
            EVP_PKEY_CTX_free(ctx);
        }
        key = key2;
    }
    if (!key) return {std::nullopt, make_error_code(ErrorCode::KEY_GENERATION_FAILED)};

    // Export private key to DER
    unsigned char* priv_der = nullptr;
    int priv_len = i2d_PrivateKey(key, &priv_der);
    
    // Export public key to DER (SubjectPublicKeyInfo)
    unsigned char* pub_der = nullptr;
    int pub_len = i2d_PUBKEY(key, &pub_der);
    
    EVP_PKEY_free(key);
    
    if (priv_len <= 0 || !priv_der || pub_len <= 0 || !pub_der) {
        if (priv_der) OPENSSL_free(priv_der);
        if (pub_der) OPENSSL_free(pub_der);
        return {std::nullopt, make_error_code(ErrorCode::KEY_GENERATION_FAILED)};
    }
    
    std::vector<std::byte> priv(static_cast<std::size_t>(priv_len));
    std::vector<std::byte> pub(static_cast<std::size_t>(pub_len));
    std::memcpy(priv.data(), priv_der, static_cast<std::size_t>(priv_len));
    std::memcpy(pub.data(), pub_der, static_cast<std::size_t>(pub_len));
    
    OPENSSL_free(priv_der);
    OPENSSL_free(pub_der);
    
    return {KeyPair(std::move(pub), std::move(priv)), std::error_code{}};
}

CryptoResult<Signature> EcdsaEngine::sign(std::span<const std::byte> message, std::span<const std::byte> private_key) {
    const unsigned char* p = reinterpret_cast<const unsigned char*>(private_key.data());
    EVP_PKEY* key = d2i_AutoPrivateKey(nullptr, &p, static_cast<long>(private_key.size()));
    if (!key) return {std::nullopt, make_error_code(ErrorCode::SIGNING_FAILED)};

    EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
    if (!md_ctx || EVP_DigestSignInit(md_ctx, nullptr, nullptr, nullptr, key) != 1) {
        if (md_ctx) EVP_MD_CTX_free(md_ctx);
        EVP_PKEY_free(key);
        return {std::nullopt, make_error_code(ErrorCode::SIGNING_FAILED)};
    }
    
    std::size_t sig_len = 256;
    std::vector<std::byte> sig(sig_len);
    
    if (EVP_DigestSign(md_ctx, reinterpret_cast<unsigned char*>(sig.data()), &sig_len,
                   reinterpret_cast<const unsigned char*>(message.data()), message.size()) != 1) {
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
    const unsigned char* p = reinterpret_cast<const unsigned char*>(public_key.data());
    EVP_PKEY* key = d2i_PUBKEY(nullptr, &p, static_cast<long>(public_key.size()));
    if (!key) return {std::nullopt, make_error_code(ErrorCode::VERIFICATION_FAILED)};

    EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
    if (!md_ctx || EVP_DigestVerifyInit(md_ctx, nullptr, nullptr, nullptr, key) != 1) {
        if (md_ctx) EVP_MD_CTX_free(md_ctx);
        EVP_PKEY_free(key);
        return {std::nullopt, make_error_code(ErrorCode::VERIFICATION_FAILED)};
    }
    
    int result = EVP_DigestVerify(md_ctx, reinterpret_cast<const unsigned char*>(signature.data()), signature.size(),
                                   reinterpret_cast<const unsigned char*>(message.data()), message.size());
    EVP_MD_CTX_free(md_ctx);
    EVP_PKEY_free(key);
    
    if (result < 0) return {std::nullopt, make_error_code(ErrorCode::VERIFICATION_FAILED)};
    return {result == 1, std::error_code{}};
}

}
