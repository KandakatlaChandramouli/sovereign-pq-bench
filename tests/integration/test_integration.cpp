#include "sovereign/crypto/ecdsa_engine.hpp"
#include "sovereign/crypto/ml_dsa_engine.hpp"
#include "sovereign/metrics/system_info.hpp"
#include "sovereign/utils/random_generator.hpp"
#include <gtest/gtest.h>

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        ecdsa_ = std::make_unique<sovereign::EcdsaEngine>();
        ml_dsa_ = std::make_unique<sovereign::MlDsaEngine>();
    }
    std::unique_ptr<sovereign::EcdsaEngine> ecdsa_;
    std::unique_ptr<sovereign::MlDsaEngine> ml_dsa_;
};

TEST_F(IntegrationTest, BothEnginesGenerateValidKeys) {
    auto ecdsa_result = ecdsa_->generate_keypair();
    auto ml_dsa_result = ml_dsa_->generate_keypair();
    ASSERT_TRUE(ecdsa_result.first.has_value());
    ASSERT_TRUE(ml_dsa_result.first.has_value());
    EXPECT_GT(ml_dsa_result.first->public_key.size(), ecdsa_result.first->public_key.size());
}

TEST_F(IntegrationTest, SignatureSizeComparison) {
    EXPECT_GT(ecdsa_->public_key_size(), 0);
    EXPECT_GT(ml_dsa_->public_key_size(), ecdsa_->public_key_size());
}

TEST_F(IntegrationTest, SystemInfoCollection) {
    auto info = sovereign::SystemInfo::collect();
    EXPECT_FALSE(info.cpu_model.empty());
}

TEST_F(IntegrationTest, CrossEngineSignVerify) {
    auto ecdsa_kp = ecdsa_->generate_keypair();
    ASSERT_TRUE(ecdsa_kp.first.has_value());
    sovereign::RandomGenerator rng;
    std::vector<std::byte> msg(64);
    rng.fill_buffer(msg);
    auto sig = ecdsa_->sign(msg, ecdsa_kp.first->private_key);
    ASSERT_TRUE(sig.first.has_value());
    auto verify = ecdsa_->verify(msg, sig.first->data, ecdsa_kp.first->public_key);
    ASSERT_TRUE(verify.first.has_value());
    EXPECT_TRUE(*verify.first);
}
