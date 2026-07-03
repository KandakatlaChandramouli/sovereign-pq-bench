#include "sovereign/crypto/ecdsa_engine.hpp"
#include "sovereign/crypto/ml_dsa_engine.hpp"
#include "sovereign/utils/random_generator.hpp"
#include <benchmark/benchmark.h>
#include <memory>
#include <span>

static std::unique_ptr<sovereign::EcdsaEngine> g_ecdsa;
static std::unique_ptr<sovereign::MlDsaEngine> g_ml_dsa;
static std::vector<std::byte> g_msg_1b;
static std::vector<std::byte> g_msg_1kb;
static std::vector<std::byte> g_msg_1mb;

static void BM_EcdsaKeyGen(benchmark::State& state) {
    for (auto _ : state) {
        auto result = g_ecdsa->generate_keypair();
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_EcdsaKeyGen);

static void BM_MlDsaKeyGen(benchmark::State& state) {
    for (auto _ : state) {
        auto result = g_ml_dsa->generate_keypair();
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_MlDsaKeyGen);

static void BM_EcdsaSign1B(benchmark::State& state) {
    auto kp_result = g_ecdsa->generate_keypair();
    if (!kp_result.first) return;
    auto& kp = *kp_result.first;
    for (auto _ : state) {
        auto sig = g_ecdsa->sign(g_msg_1b, kp.private_key);
        benchmark::DoNotOptimize(sig);
    }
}
BENCHMARK(BM_EcdsaSign1B);

static void BM_EcdsaSign1KB(benchmark::State& state) {
    auto kp_result = g_ecdsa->generate_keypair();
    if (!kp_result.first) return;
    auto& kp = *kp_result.first;
    for (auto _ : state) {
        auto sig = g_ecdsa->sign(g_msg_1kb, kp.private_key);
        benchmark::DoNotOptimize(sig);
    }
}
BENCHMARK(BM_EcdsaSign1KB);

static void BM_MlDsaSign1B(benchmark::State& state) {
    auto kp_result = g_ml_dsa->generate_keypair();
    if (!kp_result.first) return;
    auto& kp = *kp_result.first;
    for (auto _ : state) {
        auto sig = g_ml_dsa->sign(g_msg_1b, kp.private_key);
        benchmark::DoNotOptimize(sig);
    }
}
BENCHMARK(BM_MlDsaSign1B);

static void BM_MlDsaSign1KB(benchmark::State& state) {
    auto kp_result = g_ml_dsa->generate_keypair();
    if (!kp_result.first) return;
    auto& kp = *kp_result.first;
    for (auto _ : state) {
        auto sig = g_ml_dsa->sign(g_msg_1kb, kp.private_key);
        benchmark::DoNotOptimize(sig);
    }
}
BENCHMARK(BM_MlDsaSign1KB);

int main(int argc, char** argv) {
    sovereign::RandomGenerator rng;
    
    g_ecdsa = std::make_unique<sovereign::EcdsaEngine>();
    g_ml_dsa = std::make_unique<sovereign::MlDsaEngine>();
    
    g_msg_1b = rng.generate_bytes(1);
    g_msg_1kb = rng.generate_bytes(1024);

    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();

    return 0;
}
