#include "sovereign/crypto/ml_dsa_engine.hpp"
#include <cstring>
#include <stdexcept>
#include <system_error>

namespace sovereign {

MlDsaEngine::MlDsaEngine() 
    : SignatureScheme(Algorithm::ML_DSA_44)
    , sig_(nullptr)
    , algorithm_name_("ML-DSA-44") {
    
    sig_ = OQS_SIG_new(OQS_SIG_alg_ml_dsa_44);
    if (!sig_) {
        throw std::runtime_error("Failed to initialize ML-DSA-44 via liboqs");
    }
}

MlDsaEngine::~MlDsaEngine() {
    if (sig_) {
        OQS_SIG_free(sig_);
    }
}

std::string_view MlDsaEngine::name() const noexcept {
    return algorithm_name_;
}

std::size_t MlDsaEngine::public_key_size() const noexcept {
    if (!sig_) return 0;
    return sig_->length_public_key;
}

std::size_t MlDsaEngine::private_key_size() const noexcept {
    if (!sig_) return 0;
    return sig_->length_secret_key;
}

std::size_t MlDsaEngine::signature_size() const noexcept {
    if (!sig_) return 0;
    return sig_->length_signature;
}

CryptoResult<KeyPair> MlDsaEngine::generate_keypair() {
    if (!sig_) {
        return {std::nullopt, make_error_code(ErrorCode::INTERNAL_ERROR)};
    }

    std::vector<std::byte> public_key(public_key_size());
    std::vector<std::byte> private_key(private_key_size());

    OQS_STATUS status = OQS_SIG_keypair(
        sig_,
        reinterpret_cast<uint8_t*>(public_key.data()),
        reinterpret_cast<uint8_t*>(private_key.data()));

    if (status != OQS_SUCCESS) {
        return {std::nullopt, make_error_code(ErrorCode::KEY_GENERATION_FAILED)};
    }

    KeyPair kp(std::move(public_key), std::move(private_key));
    return {std::move(kp), std::error_code{}};
}

CryptoResult<Signature> MlDsaEngine::sign(
    std::span<const std::byte> message,
    std::span<const std::byte> private_key) {

    if (!sig_) {
        return {std::nullopt, make_error_code(ErrorCode::INTERNAL_ERROR)};
    }

    if (private_key.size() != private_key_size()) {
        return {std::nullopt, make_error_code(ErrorCode::INVALID_KEY)};
    }

    std::vector<std::byte> signature(signature_size());
    std::size_t actual_sig_len = signature_size();

    OQS_STATUS status = OQS_SIG_sign(
        sig_,
        reinterpret_cast<uint8_t*>(signature.data()),
        &actual_sig_len,
        reinterpret_cast<const uint8_t*>(message.data()),
        message.size(),
        reinterpret_cast<const uint8_t*>(private_key.data()));

    if (status != OQS_SUCCESS) {
        return {std::nullopt, make_error_code(ErrorCode::SIGNING_FAILED)};
    }

    signature.resize(actual_sig_len);
    Signature sig(std::move(signature), Algorithm::ML_DSA_44);
    return {std::move(sig), std::error_code{}};
}

CryptoResult<bool> MlDsaEngine::verify(
    std::span<const std::byte> message,
    std::span<const std::byte> signature,
    std::span<const std::byte> public_key) {

    if (!sig_) {
        return {std::nullopt, make_error_code(ErrorCode::INTERNAL_ERROR)};
    }

    if (public_key.size() != public_key_size()) {
        return {std::nullopt, make_error_code(ErrorCode::INVALID_KEY)};
    }

    if (signature.size() != signature_size()) {
        return {std::nullopt, make_error_code(ErrorCode::INVALID_SIGNATURE)};
    }

    OQS_STATUS status = OQS_SIG_verify(
        sig_,
        reinterpret_cast<const uint8_t*>(message.data()),
        message.size(),
        reinterpret_cast<const uint8_t*>(signature.data()),
        signature.size(),
        reinterpret_cast<const uint8_t*>(public_key.data()));

    if (status == OQS_SUCCESS) {
        return {true, std::error_code{}};
    } else if (status == OQS_ERROR) {
        return {false, std::error_code{}};
    }

    return {std::nullopt, make_error_code(ErrorCode::VERIFICATION_FAILED)};
}

CryptoResult<std::vector<std::byte>> MlDsaEngine::export_public_key_pem(
    std::span<const std::byte> public_key) {
    return {std::vector<std::byte>(public_key.begin(), public_key.end()), std::error_code{}};
}

CryptoResult<std::vector<std::byte>> MlDsaEngine::export_private_key_pem(
    std::span<const std::byte> private_key) {
    return {std::vector<std::byte>(private_key.begin(), private_key.end()), std::error_code{}};
}

CryptoResult<std::vector<std::byte>> MlDsaEngine::import_public_key_pem(
    std::span<const std::byte> pem_data) {
    if (pem_data.size() != public_key_size()) {
        return {std::nullopt, make_error_code(ErrorCode::INVALID_KEY)};
    }
    return {std::vector<std::byte>(pem_data.begin(), pem_data.end()), std::error_code{}};
}

CryptoResult<std::vector<std::byte>> MlDsaEngine::import_private_key_pem(
    std::span<const std::byte> pem_data) {
    if (pem_data.size() != private_key_size()) {
        return {std::nullopt, make_error_code(ErrorCode::INVALID_KEY)};
    }
    return {std::vector<std::byte>(pem_data.begin(), pem_data.end()), std::error_code{}};
}

}
