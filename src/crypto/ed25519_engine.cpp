#include "sovereign/crypto/ed25519_engine.hpp"
#include <openssl/err.h>
#include <cstring>

namespace sovereign {

Ed25519Engine::Ed25519Engine() : SignatureScheme(Algorithm::ED25519) {}
Ed25519Engine::~Ed25519Engine() = default;

CryptoResult<KeyPair> Ed25519Engine::generate_keypair() {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, nullptr);
    if (!ctx) return {std::nullopt, make_error_code(ErrorCode::KEY_GENERATION_FAILED)};
    EVP_PKEY_keygen_init(ctx);
    EVP_PKEY* key = nullptr;
    EVP_PKEY_keygen(ctx, &key);
    EVP_PKEY_CTX_free(ctx);
    if (!key) return {std::nullopt, make_error_code(ErrorCode::KEY_GENERATION_FAILED)};

    std::size_t pub_len = public_key_size();
    std::size_t priv_len = private_key_size();
    std::vector<std::byte> pub(pub_len), priv(priv_len);
    EVP_PKEY_get_raw_public_key(key, reinterpret_cast<unsigned char*>(pub.data()), &pub_len);
    EVP_PKEY_get_raw_private_key(key, reinterpret_cast<unsigned char*>(priv.data()), &priv_len);
    EVP_PKEY_free(key);

    KeyPair kp(std::move(pub), std::move(priv));
    return {std::move(kp), std::error_code{}};
}

CryptoResult<Signature> Ed25519Engine::sign(std::span<const std::byte> message, std::span<const std::byte> private_key) {
    EVP_PKEY* key = EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, nullptr,
        reinterpret_cast<const unsigned char*>(private_key.data()), private_key.size());
    if (!key) return {std::nullopt, make_error_code(ErrorCode::SIGNING_FAILED)};

    EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
    EVP_DigestSignInit(md_ctx, nullptr, nullptr, nullptr, key);
    std::size_t sig_len = signature_size();
    std::vector<std::byte> sig(sig_len);
    EVP_DigestSign(md_ctx, reinterpret_cast<unsigned char*>(sig.data()), &sig_len,
                   reinterpret_cast<const unsigned char*>(message.data()), message.size());
    EVP_MD_CTX_free(md_ctx);
    EVP_PKEY_free(key);
    sig.resize(sig_len);
    Signature s(std::move(sig), Algorithm::ED25519);
    return {std::move(s), std::error_code{}};
}

CryptoResult<bool> Ed25519Engine::verify(std::span<const std::byte> message, std::span<const std::byte> signature, std::span<const std::byte> public_key) {
    EVP_PKEY* key = EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, nullptr,
        reinterpret_cast<const unsigned char*>(public_key.data()), public_key.size());
    if (!key) return {std::nullopt, make_error_code(ErrorCode::VERIFICATION_FAILED)};

    EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
    EVP_DigestVerifyInit(md_ctx, nullptr, nullptr, nullptr, key);
    int result = EVP_DigestVerify(md_ctx, reinterpret_cast<const unsigned char*>(signature.data()), signature.size(),
                                   reinterpret_cast<const unsigned char*>(message.data()), message.size());
    EVP_MD_CTX_free(md_ctx);
    EVP_PKEY_free(key);
    return {result == 1, std::error_code{}};
}

}
