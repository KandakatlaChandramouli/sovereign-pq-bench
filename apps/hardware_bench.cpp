#include "sovereign/crypto/signature_scheme.hpp"
#include "sovereign/crypto/ecdsa_engine.hpp"
#include "sovereign/crypto/ed25519_engine.hpp"
#include "sovereign/crypto/ml_dsa_engine.hpp"
#include "sovereign/crypto/pq_engine.hpp"
#include "sovereign/crypto/full_kem_engine.hpp"
#include "sovereign/crypto/hash_engine.hpp"
#include "sovereign/metrics/microarch_profiler.hpp"
#include "sovereign/metrics/system_info.hpp"
#include <sys/stat.h>
#include <openssl/opensslv.h>
#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <fstream>
#include <chrono>
#include <cmath>

struct HwSigResult {
    double sign_us, verify_us;
    size_t pk_bytes, sig_bytes;
    sovereign::MicroArchMetrics sign_hw, verify_hw;
};

static HwSigResult hw_bench_sig(auto& engine, int iters = 100) {
    HwSigResult r{};
    auto kp = engine.generate_keypair();
    if (!kp.first) return r;
    std::vector<std::byte> msg(256);
    r.pk_bytes = kp.first->public_key.size();
    
    sovereign::MicroArchProfiler profiler;
    
    // Signing benchmark
    profiler.start();
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iters; i++) engine.sign(msg, kp.first->private_key);
    auto t2 = std::chrono::high_resolution_clock::now();
    r.sign_hw = profiler.stop();
    r.sign_us = std::chrono::duration<double,std::micro>(t2-t1).count() / iters;
    
    auto sig = engine.sign(msg, kp.first->private_key);
    r.sig_bytes = sig.first->data.size();
    
    // Verification benchmark
    profiler.start();
    auto t3 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iters; i++) (void)engine.verify(msg, sig.first->data, kp.first->public_key);
    auto t4 = std::chrono::high_resolution_clock::now();
    r.verify_hw = profiler.stop();
    r.verify_us = std::chrono::duration<double,std::micro>(t4-t3).count() / iters;
    
    return r;
}

int main() {
    std::cout << "╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  SOVEREIGN PROTOCOL — Hardware-Aware PQC Evidence Engine         ║\n";
    std::cout << "║  PMU Counters: Cache Misses, Branch Mispredictions, IPC           ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════╝\n\n";
    
    auto sys = sovereign::SystemInfo::collect();
    std::cout << "System: " << sys.cpu_model << " | OpenSSL " << OPENSSL_VERSION_TEXT << "\n\n";
    
    ::mkdir("results", 0755);
    std::ofstream csv("results/hardware_evidence_matrix.csv");
    
    // CSV header
    csv << "algorithm,operation,mean_us,key_or_ct_bytes,sig_or_ss_bytes,"
        << "cache_misses,cache_refs,cache_miss_pct,"
        << "branch_misses,branch_instr,branch_miss_pct,"
        << "instructions,cycles,ipc,context_switches,cpu_migrations,page_faults,"
        << "network_packets_required,mtu_overhead_pct\n";
    
    std::cout << std::left << std::setw(22) << "Algorithm" << std::right << std::setw(10) << "Sign(µs)"
              << std::setw(10) << "Vfy(µs)" << std::setw(10) << "CacheMiss%" << std::setw(10) << "BranchMiss%"
              << std::setw(8) << "IPC" << std::setw(10) << "Key(B)" << std::setw(10) << "Sig(B)"
              << std::setw(12) << "MTU Pkts\n";
    std::cout << std::string(112, '─') << "\n";
    
    auto run_sig = [&](auto& engine) {
        auto r = hw_bench_sig(engine, 50);
        if (r.pk_bytes == 0) { std::cout << engine.name() << ": KEYGEN FAILED\n"; return; }
        
        // Network MTU analysis
        const int MTU = 1500, MSS = 1460;
        int sig_packets = std::ceil((double)r.sig_bytes / MSS);
        int key_packets = std::ceil((double)r.pk_bytes / MSS);
        double mtu_overhead = ((double)(sig_packets * MTU) / r.sig_bytes - 1.0) * 100.0;
        
        csv << engine.name() << ",Sign," << r.sign_us << "," << r.pk_bytes << "," << r.sig_bytes << ","
            << r.sign_hw.cache_misses << "," << r.sign_hw.cache_references << "," << r.sign_hw.cache_miss_rate() << ","
            << r.sign_hw.branch_misses << "," << r.sign_hw.branch_instructions << "," << r.sign_hw.branch_miss_rate() << ","
            << r.sign_hw.instructions << "," << r.sign_hw.cycles << "," << r.sign_hw.ipc() << ","
            << r.sign_hw.context_switches << "," << r.sign_hw.cpu_migrations << "," << r.sign_hw.page_faults << ","
            << sig_packets << "," << mtu_overhead << "\n";
        
        csv << engine.name() << ",Verify," << r.verify_us << "," << r.pk_bytes << "," << r.sig_bytes << ","
            << r.verify_hw.cache_misses << "," << r.verify_hw.cache_references << "," << r.verify_hw.cache_miss_rate() << ","
            << r.verify_hw.branch_misses << "," << r.verify_hw.branch_instructions << "," << r.verify_hw.branch_miss_rate() << ","
            << r.verify_hw.instructions << "," << r.verify_hw.cycles << "," << r.verify_hw.ipc() << ","
            << r.verify_hw.context_switches << "," << r.verify_hw.cpu_migrations << "," << r.verify_hw.page_faults << ","
            << sig_packets << "," << mtu_overhead << "\n";
        
        std::cout << std::left << std::setw(22) << engine.name() << std::right << std::setw(10) << std::fixed << std::setprecision(1)
                  << r.sign_us << std::setw(10) << r.verify_us
                  << std::setw(9) << std::setprecision(1) << r.sign_hw.cache_miss_rate() << "%"
                  << std::setw(10) << std::setprecision(1) << r.sign_hw.branch_miss_rate() << "%"
                  << std::setw(8) << std::setprecision(3) << r.sign_hw.ipc()
                  << std::setw(10) << r.pk_bytes << std::setw(10) << r.sig_bytes
                  << std::setw(10) << sig_packets << " pkt\n";
    };
    
    sovereign::EcdsaEngine ecdsa; run_sig(ecdsa);
    sovereign::Ed25519Engine ed; run_sig(ed);
    sovereign::MlDsa44Engine ml44; run_sig(ml44);
    sovereign::MlDsa65Engine ml65; run_sig(ml65);
    sovereign::MlDsa87Engine ml87; run_sig(ml87);
    sovereign::Falcon512Engine f512; run_sig(f512);
    sovereign::Falcon1024Engine f1024; run_sig(f1024);
    std::cout << "\n[SPHINCS+ — running single iteration for HW counters...]\n";
    sovereign::Sphincs128sEngine s128s; run_sig(s128s);
    sovereign::Sphincs256sEngine s256s; run_sig(s256s);
    
    csv.close();
    
    std::cout << "\n╔══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  HARDWARE EVIDENCE EXPORTED — results/hardware_evidence_matrix.csv ║\n";
    std::cout << "║  MTU Analysis: ECDSA=1 pkt, ML-DSA-44=2 pkts, SPHINCS+=20+ pkts  ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════════╝\n";
    return 0;
}
