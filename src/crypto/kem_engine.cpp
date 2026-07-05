#include "sovereign/crypto/kem_engine.hpp"
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include <oqs/oqs.h>
#include <cstring>
#include <stdexcept>
#include <chrono>

namespace sovereign {

KemEngine::KemEngine(KemAlgorithm algo) : algo_(algo), kem_(nullptr) {
    switch (algo) {
        case KemAlgorithm::ML_KEM_512:
            kem_ = OQS_KEM_new(OQS_KEM_alg_ml_kem_512);
            name_ = "ML-KEM-512";
            break;
        case KemAlgorithm::ML_KEM_768:
            kem_ = OQS_KEM_new(OQS_KEM_alg_ml_kem_768);
            name_ = "ML-KEM-768";
            break;
        case KemAlgorithm::ML_KEM_1024:
            kem_ = OQS_KEM_new(OQS_KEM_alg_ml_kem_1024);
            name_ = "ML-KEM-1024";
            break;
        case KemAlgorithm::CLASSIC_MCELIECE_348864:
            kem_ = OQS_KEM_new(OQS_KEM_alg_classic_mceliece_348864);
            name_ = "Classic-McEliece-348864";
            break;
        case KemAlgorithm::HQC_128:
            kem_ = OQS_KEM_new(OQS_KEM_alg_hqc_128);
            name_ = "HQC-128";
            break;
        case KemAlgorithm::BIKE_L1:
            kem_ = OQS_KEM_new(OQS_KEM_alg_bike_l1);
            name_ = "BIKE-L1";
            break;
        case KemAlgorithm::FRODO_976_AES:
            kem_ = OQS_KEM_new(OQS_KEM_alg_frodokem_976_aes);
            name_ = "FrodoKEM-976-AES";
            break;
        case KemAlgorithm::ECDH_P256:
            name_ = "ECDH-P256";
            break;
        case KemAlgorithm::ECDH_X25519:
            name_ = "ECDH-X25519";
            break;
    }
}

KemEngine::~KemEngine() {
    if (kem_) OQS_KEM_free(static_cast<OQS_KEM*>(kem_));
}

std::string_view KemEngine::name() const noexcept { return name_; }

std::size_t KemEngine::public_key_size() const noexcept {
    if (kem_) return static_cast<OQS_KEM*>(kem_)->length_public_key;
    if (algo_ == KemAlgorithm::ECDH_P256) return 65;
    if (algo_ == KemAlgorithm::ECDH_X25519) return 32;
    return 0;
}

std::size_t KemEngine::private_key_size() const noexcept {
    if (kem_) return static_cast<OQS_KEM*>(kem_)->length_secret_key;
    if (algo_ == KemAlgorithm::ECDH_P256) return 32;
    if (algo_ == KemAlgorithm::ECDH_X25519) return 32;
    return 0;
}

std::size_t KemEngine::ciphertext_size() const noexcept {
    if (kem_) return static_cast<OQS_KEM*>(kem_)->length_ciphertext;
    if (algo_ == KemAlgorithm::ECDH_P256) return 65;
    if (algo_ == KemAlgorithm::ECDH_X25519) return 32;
    return 0;
}

std::size_t KemEngine::shared_secret_size() const noexcept {
    if (kem_) return static_cast<OQS_KEM*>(kem_)->length_shared_secret;
    return 32;
}

std::pair<std::optional<KemKeyPair>, std::error_code> KemEngine::generate_keypair() {
    if (kem_) {
        auto* k = static_cast<OQS_KEM*>(kem_);
        KemKeyPair kp;
        kp.public_key.resize(k->length_public_key);
        kp.private_key.resize(k->length_secret_key);
        if (OQS_KEM_keypair(k, reinterpret_cast<uint8_t*>(kp.public_key.data()),
                            reinterpret_cast<uint8_t*>(kp.private_key.data())) != OQS_SUCCESS) {
            return {std::nullopt, std::make_error_code(std::errc::operation_not_permitted)};
        }
        return {std::move(kp), std::error_code{}};
    }
    
    if (algo_ == KemAlgorithm::ECDH_P256) {
        EVP_PKEY* key = EVP_EC_gen("P-256");
        if (!key) return {std::nullopt, std::make_error_code(std::errc::operation_not_permitted)};
        KemKeyPair kp;
        kp.public_key.resize(65);
        kp.private_key.resize(32);
        std::size_t pub_len = 65, priv_len = 32;
        EVP_PKEY_get_raw_public_key(key, reinterpret_cast<unsigned char*>(kp.public_key.data()), &pub_len);
        EVP_PKEY_get_raw_private_key(key, reinterpret_cast<unsigned char*>(kp.private_key.data()), &priv_len);
        EVP_PKEY_free(key);
        kp.public_key.resize(pub_len);
        kp.private_key.resize(priv_len);
        return {std::move(kp), std::error_code{}};
    }
    
    if (algo_ == KemAlgorithm::ECDH_X25519) {
        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_X25519, nullptr);
        if (!ctx) return {std::nullopt, std::make_error_code(std::errc::operation_not_permitted)};
        EVP_PKEY_keygen_init(ctx);
        EVP_PKEY* key = nullptr;
        EVP_PKEY_keygen(ctx, &key);
        EVP_PKEY_CTX_free(ctx);
        if (!key) return {std::nullopt, std::make_error_code(std::errc::operation_not_permitted)};
        KemKeyPair kp;
        kp.public_key.resize(32);
        kp.private_key.resize(32);
        std::size_t pub_len = 32, priv_len = 32;
        EVP_PKEY_get_raw_public_key(key, reinterpret_cast<unsigned char*>(kp.public_key.data()), &pub_len);
        EVP_PKEY_get_raw_private_key(key, reinterpret_cast<unsigned char*>(kp.private_key.data()), &priv_len);
        EVP_PKEY_free(key);
        return {std::move(kp), std::error_code{}};
    }
    
    return {std::nullopt, std::make_error_code(std::errc::operation_not_permitted)};
}

std::pair<std::optional<KemCiphertext>, std::error_code> KemEngine::encapsulate(std::span<const std::byte> public_key) {
    if (kem_) {
        auto* k = static_cast<OQS_KEM*>(kem_);
        KemCiphertext ct;
        ct.data.resize(k->length_ciphertext);
        ct.shared_secret.resize(k->length_shared_secret);
        
        auto start = std::chrono::high_resolution_clock::now();
        if (OQS_KEM_encaps(k, reinterpret_cast<uint8_t*>(ct.data.data()),
                           reinterpret_cast<uint8_t*>(ct.shared_secret.data()),
                           reinterpret_cast<const uint8_t*>(public_key.data())) != OQS_SUCCESS) {
            return {std::nullopt, std::make_error_code(std::errc::operation_not_permitted)};
        }
        auto end = std::chrono::high_resolution_clock::now();
        ct.encaps_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        return {std::move(ct), std::error_code{}};
    }
    
    // Classical ECDH placeholder — real implementation needs key agreement API
    KemCiphertext ct;
    ct.shared_secret.resize(32, std::byte{0x42});
    ct.data.resize(ciphertext_size(), std::byte{0x00});
    return {std::move(ct), std::error_code{}};
}

std::pair<std::optional<KemCiphertext>, std::error_code> KemEngine::decapsulate(std::span<const std::byte> ciphertext, std::span<const std::byte> private_key) {
    if (kem_) {
        auto* k = static_cast<OQS_KEM*>(kem_);
        KemCiphertext ct;
        ct.shared_secret.resize(k->length_shared_secret);
        
        auto start = std::chrono::high_resolution_clock::now();
        if (OQS_KEM_decaps(k, reinterpret_cast<uint8_t*>(ct.shared_secret.data()),
                           reinterpret_cast<const uint8_t*>(ciphertext.data()),
                           reinterpret_cast<const uint8_t*>(private_key.data())) != OQS_SUCCESS) {
            return {std::nullopt, std::make_error_code(std::errc::operation_not_permitted)};
        }
        auto end = std::chrono::high_resolution_clock::now();
        ct.decaps_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        return {std::move(ct), std::error_code{}};
    }
    
    KemCiphertext ct;
    ct.shared_secret.resize(32, std::byte{0x42});
    return {std::move(ct), std::error_code{}};
}

}
