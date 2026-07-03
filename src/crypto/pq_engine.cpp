#include "sovereign/crypto/pq_engine.hpp"
#include <cstring>
#include <stdexcept>

namespace sovereign {

PqSignatureEngine::PqSignatureEngine(const char* oqs_alg, Algorithm algo, const char* display_name)
    : SignatureScheme(algo), sig_(nullptr), display_name_(display_name) {
    sig_ = OQS_SIG_new(oqs_alg);
    if (!sig_) throw std::runtime_error(std::string("Failed to initialize ") + display_name);
}
PqSignatureEngine::~PqSignatureEngine() { if (sig_) OQS_SIG_free(sig_); }

std::string_view PqSignatureEngine::name() const noexcept { return display_name_; }
std::size_t PqSignatureEngine::public_key_size() const noexcept { return sig_ ? sig_->length_public_key : 0; }
std::size_t PqSignatureEngine::private_key_size() const noexcept { return sig_ ? sig_->length_secret_key : 0; }
std::size_t PqSignatureEngine::signature_size() const noexcept { return sig_ ? sig_->length_signature : 0; }

CryptoResult<KeyPair> PqSignatureEngine::generate_keypair() {
    if (!sig_) return {std::nullopt, make_error_code(ErrorCode::INTERNAL_ERROR)};
    std::vector<std::byte> pub(public_key_size()), priv(private_key_size());
    if (OQS_SIG_keypair(sig_, reinterpret_cast<uint8_t*>(pub.data()), reinterpret_cast<uint8_t*>(priv.data())) != OQS_SUCCESS)
        return {std::nullopt, make_error_code(ErrorCode::KEY_GENERATION_FAILED)};
    return {KeyPair(std::move(pub), std::move(priv)), std::error_code{}};
}

CryptoResult<Signature> PqSignatureEngine::sign(std::span<const std::byte> message, std::span<const std::byte> private_key) {
    if (!sig_) return {std::nullopt, make_error_code(ErrorCode::INTERNAL_ERROR)};
    std::vector<std::byte> sig(signature_size());
    std::size_t sig_len = sig.size();
    if (OQS_SIG_sign(sig_, reinterpret_cast<uint8_t*>(sig.data()), &sig_len,
            reinterpret_cast<const uint8_t*>(message.data()), message.size(),
            reinterpret_cast<const uint8_t*>(private_key.data())) != OQS_SUCCESS)
        return {std::nullopt, make_error_code(ErrorCode::SIGNING_FAILED)};
    sig.resize(sig_len);
    return {Signature(std::move(sig), algo_), std::error_code{}};
}

CryptoResult<bool> PqSignatureEngine::verify(std::span<const std::byte> message, std::span<const std::byte> signature, std::span<const std::byte> public_key) {
    if (!sig_) return {std::nullopt, make_error_code(ErrorCode::INTERNAL_ERROR)};
    OQS_STATUS status = OQS_SIG_verify(sig_,
        reinterpret_cast<const uint8_t*>(message.data()), message.size(),
        reinterpret_cast<const uint8_t*>(signature.data()), signature.size(),
        reinterpret_cast<const uint8_t*>(public_key.data()));
    if (status == OQS_SUCCESS) return {true, std::error_code{}};
    if (status == OQS_ERROR) return {false, std::error_code{}};
    return {std::nullopt, make_error_code(ErrorCode::VERIFICATION_FAILED)};
}

}
