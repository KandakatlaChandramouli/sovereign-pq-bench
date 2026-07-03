#include "sovereign/crypto/ecdsa_engine.hpp"
#include "sovereign/crypto/ed25519_engine.hpp"
#include "sovereign/crypto/ml_dsa_engine.hpp"
#include "sovereign/crypto/pq_engine.hpp"
#include "sovereign/utils/random_generator.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <memory>

class SignatureTest : public ::testing::Test {
protected:
    void SetUp() override { msg_ = rng_.generate_bytes(256); }
    void test_engine(sovereign::SignatureScheme& engine) {
        auto kp = engine.generate_keypair();
        ASSERT_TRUE(kp.first.has_value());
        auto sig = engine.sign(msg_, kp.first->private_key);
        ASSERT_TRUE(sig.first.has_value());
        auto verify = engine.verify(msg_, sig.first->data, kp.first->public_key);
        ASSERT_TRUE(verify.first.has_value());
        EXPECT_TRUE(*verify.first);
    }
    sovereign::RandomGenerator rng_;
    std::vector<std::byte> msg_;
};

TEST_F(SignatureTest, EcdsaP256) { sovereign::EcdsaEngine e; test_engine(e); }
TEST_F(SignatureTest, Ed25519) { sovereign::Ed25519Engine e; test_engine(e); }
TEST_F(SignatureTest, MlDsa44) { sovereign::MlDsa44Engine e; test_engine(e); }
TEST_F(SignatureTest, MlDsa65) { sovereign::MlDsa65Engine e; test_engine(e); }
TEST_F(SignatureTest, MlDsa87) { sovereign::MlDsa87Engine e; test_engine(e); }
TEST_F(SignatureTest, Falcon512) { sovereign::Falcon512Engine e; test_engine(e); }
TEST_F(SignatureTest, Falcon1024) { sovereign::Falcon1024Engine e; test_engine(e); }
TEST_F(SignatureTest, Sphincs128s) { sovereign::Sphincs128sEngine e; test_engine(e); }
TEST_F(SignatureTest, Sphincs128f) { sovereign::Sphincs128fEngine e; test_engine(e); }
TEST_F(SignatureTest, Sphincs192s) { sovereign::Sphincs192sEngine e; test_engine(e); }
TEST_F(SignatureTest, Sphincs256s) { sovereign::Sphincs256sEngine e; test_engine(e); }
