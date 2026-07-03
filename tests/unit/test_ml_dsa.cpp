#include "sovereign/crypto/ml_dsa_engine.hpp"
#include "sovereign/utils/random_generator.hpp"
#include <gtest/gtest.h>
#include <span>

class MlDsaTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine_ = std::make_unique<sovereign::MlDsaEngine>();
        msg_1b_ = rng_.generate_bytes(1);
        msg_1kb_ = rng_.generate_bytes(1024);
    }

    sovereign::RandomGenerator rng_;
    std::unique_ptr<sovereign::MlDsaEngine> engine_;
    std::vector<std::byte> msg_1b_;
    std::vector<std::byte> msg_1kb_;
};

TEST_F(MlDsaTest, KeyGenerationSucceeds) {
    auto result = engine_->generate_keypair();
    ASSERT_TRUE(result.first.has_value());
    EXPECT_FALSE(result.first->public_key.empty());
    EXPECT_FALSE(result.first->private_key.empty());
    EXPECT_EQ(result.first->public_key.size(), engine_->public_key_size());
    EXPECT_EQ(result.first->private_key.size(), engine_->private_key_size());
}

TEST_F(MlDsaTest, SignAndVerify1Byte) {
    auto kp_result = engine_->generate_keypair();
    ASSERT_TRUE(kp_result.first.has_value());
    auto& kp = *kp_result.first;

    auto sig_result = engine_->sign(msg_1b_, kp.private_key);
    ASSERT_TRUE(sig_result.first.has_value());

    auto verify_result = engine_->verify(msg_1b_, sig_result.first->data, kp.public_key);
    ASSERT_TRUE(verify_result.first.has_value());
    EXPECT_TRUE(*verify_result.first);
}

TEST_F(MlDsaTest, SignAndVerify1KB) {
    auto kp_result = engine_->generate_keypair();
    ASSERT_TRUE(kp_result.first.has_value());
    auto& kp = *kp_result.first;

    auto sig_result = engine_->sign(msg_1kb_, kp.private_key);
    ASSERT_TRUE(sig_result.first.has_value());

    auto verify_result = engine_->verify(msg_1kb_, sig_result.first->data, kp.public_key);
    ASSERT_TRUE(verify_result.first.has_value());
    EXPECT_TRUE(*verify_result.first);
}

TEST_F(MlDsaTest, VerificationFailsWithWrongMessage) {
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

TEST_F(MlDsaTest, VerificationFailsWithWrongKey) {
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

TEST_F(MlDsaTest, VerificationFailsWithCorruptedSignature) {
    auto kp_result = engine_->generate_keypair();
    ASSERT_TRUE(kp_result.first.has_value());
    auto& kp = *kp_result.first;

    auto sig_result = engine_->sign(msg_1kb_, kp.private_key);
    ASSERT_TRUE(sig_result.first.has_value());

    if (!sig_result.first->data.empty()) {
        sig_result.first->data[0] = static_cast<std::byte>(
            ~static_cast<unsigned char>(sig_result.first->data[0]));
    }

    auto verify_result = engine_->verify(msg_1kb_, sig_result.first->data, kp.public_key);
    ASSERT_TRUE(verify_result.first.has_value());
    EXPECT_FALSE(*verify_result.first);
}

TEST_F(MlDsaTest, KeyImportExportRoundtrip) {
    auto kp_result = engine_->generate_keypair();
    ASSERT_TRUE(kp_result.first.has_value());
    auto& kp = *kp_result.first;

    auto exported_result = engine_->export_public_key_pem(kp.public_key);
    ASSERT_TRUE(exported_result.first.has_value());

    auto imported_result = engine_->import_public_key_pem(*exported_result.first);
    ASSERT_TRUE(imported_result.first.has_value());

    auto sig_result = engine_->sign(msg_1b_, kp.private_key);
    ASSERT_TRUE(sig_result.first.has_value());

    auto verify_result = engine_->verify(msg_1b_, sig_result.first->data, *imported_result.first);
    ASSERT_TRUE(verify_result.first.has_value());
    EXPECT_TRUE(*verify_result.first);
}

TEST_F(MlDsaTest, MultipleKeyGenerationsAreUnique) {
    auto kp1_result = engine_->generate_keypair();
    auto kp2_result = engine_->generate_keypair();
    ASSERT_TRUE(kp1_result.first.has_value());
    ASSERT_TRUE(kp2_result.first.has_value());

    EXPECT_NE(kp1_result.first->public_key, kp2_result.first->public_key);
    EXPECT_NE(kp1_result.first->private_key, kp2_result.first->private_key);
}

TEST_F(MlDsaTest, LargeMessageSigning) {
    auto large_msg = rng_.generate_bytes(100 * 1024);
    auto kp_result = engine_->generate_keypair();
    ASSERT_TRUE(kp_result.first.has_value());
    auto& kp = *kp_result.first;

    auto sig_result = engine_->sign(large_msg, kp.private_key);
    ASSERT_TRUE(sig_result.first.has_value());

    auto verify_result = engine_->verify(large_msg, sig_result.first->data, kp.public_key);
    ASSERT_TRUE(verify_result.first.has_value());
    EXPECT_TRUE(*verify_result.first);
}
