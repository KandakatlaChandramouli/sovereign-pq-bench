#include "sovereign/metrics/statistical_analysis.hpp"
#include "sovereign/crypto/signature_scheme.hpp"
#include "sovereign/crypto/ecdsa_engine.hpp"
#include "sovereign/crypto/ed25519_engine.hpp"
#include "sovereign/crypto/ml_dsa_engine.hpp"
#include "sovereign/crypto/pq_engine.hpp"
#include "sovereign/crypto/full_kem_engine.hpp"
#include "sovereign/crypto/hash_engine.hpp"
#include "sovereign/metrics/system_info.hpp"
#include <sys/stat.h>
#include <openssl/opensslv.h>
#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <fstream>
#include <chrono>

struct SigResult { double sign_us, verify_us; size_t pub_kb, sig_kb; };
static SigResult quick_sig(auto& engine) {
    auto kp = engine.generate_keypair();
    std::vector<std::byte> msg(256);
    auto t1 = std::chrono::high_resolution_clock::now();
    auto sig = engine.sign(msg, kp.first->private_key);
    auto t2 = std::chrono::high_resolution_clock::now();
    (void)engine.verify(msg, sig.first->data, kp.first->public_key);
    auto t3 = std::chrono::high_resolution_clock::now();
    return {std::chrono::duration<double,std::micro>(t2-t1).count(), std::chrono::duration<double,std::micro>(t3-t2).count(), kp.first->public_key.size(), sig.first->data.size()};
}

struct KemResult { double enc_us, dec_us; size_t pk_b, ct_b; };
static KemResult quick_kem(sovereign::FullKemAlgorithm algo) {
    sovereign::FullKemEngine kem(algo);
    auto kp = kem.keygen();
    if (!kp.first) return {0,0,0,0};
    auto ct = kem.encaps(kp.first->public_key);
    (void)kem.decaps(ct.first->ct, kp.first->private_key);
    return {ct.first->enc_us, ct.first->dec_us, kem.pk_size(), kem.ct_size()};
}

int main() {
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  SOVEREIGN PROTOCOL — Complete Gulf PQC Evidence Matrix     ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    auto sys = sovereign::SystemInfo::collect();
    std::cout << "System: " << sys.cpu_model << " | OpenSSL " << OPENSSL_VERSION_TEXT << "\n\n";
    ::mkdir("results", 0755);
    std::ofstream csv("results/gulf_complete_matrix.csv");
    csv << "category,algorithm,operation,mean_us,key_bytes,sig_or_ct_bytes\n";

    // Signatures
    std::cout << "═══ SIGNATURES ═══\n";
    auto do_sig = [&](auto& e, const char* std, const char* fam) {
        auto r = quick_sig(e);
        csv << "Signature," << e.name() << ",Sign," << r.sign_us << "," << r.pub_kb << "," << r.sig_kb << "\n";
        std::cout << std::left << std::setw(24) << e.name() << std::setw(14) << std << std::right << std::setw(10) << std::fixed << std::setprecision(1) << r.sign_us << "us  " << r.pub_kb << "B  " << r.sig_kb << "B  " << fam << "\n";
    };
    sovereign::EcdsaEngine ecdsa; do_sig(ecdsa,"FIPS 186-4","Classical");
    sovereign::Ed25519Engine ed; do_sig(ed,"RFC 8032","Classical");
    sovereign::MlDsa44Engine ml44; do_sig(ml44,"FIPS 204","Lattice PQ");
    sovereign::MlDsa65Engine ml65; do_sig(ml65,"FIPS 204","Lattice PQ");
    sovereign::MlDsa87Engine ml87; do_sig(ml87,"FIPS 204","Lattice PQ");
    sovereign::Falcon512Engine f512; do_sig(f512,"FIPS 206","Lattice PQ");
    sovereign::Falcon1024Engine f1024; do_sig(f1024,"FIPS 206","Lattice PQ");
    sovereign::Sphincs128sEngine s128s; do_sig(s128s,"FIPS 205","Hash PQ");
    sovereign::Sphincs256sEngine s256s; do_sig(s256s,"FIPS 205","Hash PQ");

    // KEMs  
    std::cout << "\n═══ KEY ENCAPSULATION ═══\n";
    auto do_kem = [&](sovereign::FullKemAlgorithm algo, const char* std, const char* fam) {
        sovereign::FullKemEngine kem(algo);
        auto r = quick_kem(algo);
        csv << "KEM," << kem.name() << ",Encaps," << r.enc_us << "," << r.pk_b << "," << r.ct_b << "\n";
        std::cout << std::left << std::setw(28) << kem.name() << std::setw(14) << std << std::right << std::setw(10) << std::fixed << std::setprecision(1) << r.enc_us << "us  " << r.pk_b << "B  " << r.ct_b << "B  " << fam << "\n";
    };
    do_kem(sovereign::FullKemAlgorithm::ML_KEM_512,"FIPS 203","Lattice");
    do_kem(sovereign::FullKemAlgorithm::ML_KEM_768,"FIPS 203","Lattice");
    do_kem(sovereign::FullKemAlgorithm::ML_KEM_1024,"FIPS 203","Lattice");
    do_kem(sovereign::FullKemAlgorithm::CLASSIC_MCELIECE_348864,"NIST R4","Code");
    do_kem(sovereign::FullKemAlgorithm::HQC_128,"NIST R4","Code");
    do_kem(sovereign::FullKemAlgorithm::BIKE_L1,"NIST R4","Code");
    do_kem(sovereign::FullKemAlgorithm::FRODO_976_AES,"NIST R4","LWE");

    csv.close();
    std::cout << "\n✅ Complete Gulf PQC Matrix exported to results/gulf_complete_matrix.csv\n";
    return 0;
}
