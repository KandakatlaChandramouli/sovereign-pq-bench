#include "sovereign/crypto/signature_scheme.hpp"

namespace sovereign {
namespace {
    struct ErrorCategory : std::error_category {
        [[nodiscard]] const char* name() const noexcept override { return "sovereign::SignatureScheme"; }
        [[nodiscard]] std::string message(int ev) const override {
            switch (static_cast<ErrorCode>(ev)) {
                case ErrorCode::SUCCESS: return "Success";
                case ErrorCode::KEY_GENERATION_FAILED: return "Key generation failed";
                case ErrorCode::SIGNING_FAILED: return "Signing operation failed";
                case ErrorCode::VERIFICATION_FAILED: return "Verification failed";
                case ErrorCode::INVALID_KEY: return "Invalid key provided";
                case ErrorCode::INVALID_SIGNATURE: return "Invalid signature provided";
                case ErrorCode::INVALID_MESSAGE: return "Invalid message provided";
                case ErrorCode::SERIALIZATION_FAILED: return "Key serialization failed";
                case ErrorCode::DESERIALIZATION_FAILED: return "Key deserialization failed";
                case ErrorCode::UNSUPPORTED_ALGORITHM: return "Unsupported algorithm";
                case ErrorCode::INTERNAL_ERROR: return "Internal error";
                default: return "Unknown error";
            }
        }
    };
    const ErrorCategory error_category_instance{};
}
std::error_code make_error_code(ErrorCode ec) noexcept { return {static_cast<int>(ec), error_category_instance}; }
SignatureScheme::SignatureScheme(Algorithm algo) : algo_(algo) {}
SignatureScheme::~SignatureScheme() = default;
SignatureScheme::SignatureScheme(SignatureScheme&&) noexcept = default;
SignatureScheme& SignatureScheme::operator=(SignatureScheme&&) noexcept = default;
}
