#include "sovereign/crypto/ecdsa_engine.hpp"
#include "sovereign/utils/random_generator.hpp"
#include <gtest/gtest.h>
#include <span>

class EcdsaTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine_ = std::make_unique<sovereign::EcdsaEngine>();
        msg_1b_ = rng_.generate_bytes(1);
        msg_1kb_ = rng_.generate_bytes(1024);
    }
    sovereign::RandomGenerator rng_;
    std::unique_ptr<sovereign::EcdsaEngine> engine_;
    std::vector<std::byte> msg_1b_;
    std::vector<std::byte> msg_1kb_;
};

TEST_F(EcdsaTest, KeyGenerationSucceeds) {
    auto result = engine_->generate_keypair();
    ASSERT_TRUE(result.first.has_value());
    EXPECT_FALSE(result.first->public_key.empty());
    EXPECT_FALSE(result.first->private_key.empty());
}

TEST_F(EcdsaTest, SignAndVerify1Byte) {
    auto kp_result = engine_->generate_keypair();
    ASSERT_TRUE(kp_result.first.has_value());
    auto& kp = *kp_result.first;
    auto sig_result = engine_->sign(msg_1b_, kp.private_key);
    ASSERT_TRUE(sig_result.first.has_value());
    auto verify_result = engine_->verify(msg_1b_, sig_result.first->data, kp.public_key);
    ASSERT_TRUE(verify_result.first.has_value());
    EXPECT_TRUE(*verify_result.first);
}

TEST_F(EcdsaTest, SignAndVerify1KB) {
    auto kp_result = engine_->generate_keypair();
    ASSERT_TRUE(kp_result.first.has_value());
    auto& kp = *kp_result.first;
    auto sig_result = engine_->sign(msg_1kb_, kp.private_key);
    ASSERT_TRUE(sig_result.first.has_value());
    auto verify_result = engine_->verify(msg_1kb_, sig_result.first->data, kp.public_key);
    ASSERT_TRUE(verify_result.first.has_value());
    EXPECT_TRUE(*verify_result.first);
}

TEST_F(EcdsaTest, VerificationFailsWithWrongMessage) {
    auto kp_result = engine_->generate_keypair();
    ASSERT_TRUE(kp_result.first.has_value());
    auto& kp = *kp_result.first;
    auto sig_result = engine_->sign(msg_1b_, kp.private_key);
    ASSERT_TRUE(sig_result.first.has_value());
    auto wrong_msg = rng_.generate_bytes(1);
    auto verify_result = engine_->verify(wrong_msg, sig_result.first->data, kp.public_key);
    ASSERT_TRUE(verify_result.first.has_value());
    EXPECT_FALSE(*verify_result.first);
}

TEST_F(EcdsaTest, VerificationFailsWithWrongKey) {
    auto kp1_result = engine_->generate_keypair();
    auto kp2_result = engine_->generate_keypair();
    ASSERT_TRUE(kp1_result.first.has_value());
    ASSERT_TRUE(kp2_result.first.has_value());
    auto sig_result = engine_->sign(msg_1b_, kp1_result.first->private_key);
    ASSERT_TRUE(sig_result.first.has_value());
    auto verify_result = engine_->verify(msg_1b_, sig_result.first->data, kp2_result.first->public_key);
    ASSERT_TRUE(verify_result.first.has_value());
    EXPECT_FALSE(*verify_result.first);
}

TEST_F(EcdsaTest, PemExportImportRoundtrip) {
    auto kp_result = engine_->generate_keypair();
    ASSERT_TRUE(kp_result.first.has_value());
    auto& kp = *kp_result.first;
    auto pem_result = engine_->export_public_key_pem(kp.public_key);
    ASSERT_TRUE(pem_result.first.has_value());
    auto imported_result = engine_->import_public_key_pem(*pem_result.first);
    ASSERT_TRUE(imported_result.first.has_value());
}

TEST_F(EcdsaTest, MultipleKeyGenerationsAreUnique) {
    auto kp1_result = engine_->generate_keypair();
    auto kp2_result = engine_->generate_keypair();
    ASSERT_TRUE(kp1_result.first.has_value());
    ASSERT_TRUE(kp2_result.first.has_value());
    EXPECT_NE(kp1_result.first->public_key, kp2_result.first->public_key);
}
