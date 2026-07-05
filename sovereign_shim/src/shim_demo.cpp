#include "sovereign_shim.hpp"
#include <iostream>
#include <thread>
#include <iomanip>

int main() {
    std::cout << "\n";
    
    // Create the proxy shim
    sovereign_shim::SovereignProxyShim shim("127.0.0.1", 8443, "10.0.0.1", 443);
    
    // Configure policy thresholds for Gulf infrastructure
    shim.force_policy(sovereign_shim::PqcPolicy::AUTO);
    
    // Start in background
    std::thread proxy_thread([&shim]() {
        shim.start();
    });
    
    // Simulate 30 seconds of Gulf operational scenarios
    std::cout << "\n=== SIMULATING GULF INFRASTRUCTURE LOAD ===\n\n";
    
    for (int t = 0; t < 30; t++) {
        auto telemetry = shim.get_telemetry();
        auto policy = shim.active_policy();
        
        std::string policy_str;
        switch (policy) {
            case sovereign_shim::PqcPolicy::ML_DSA_44: policy_str = "ML-DSA-44 (Speed)"; break;
            case sovereign_shim::PqcPolicy::FALCON_512: policy_str = "Falcon-512 (Compact)"; break;
            case sovereign_shim::PqcPolicy::SPHINCS_256S: policy_str = "SPHINCS+-256s (Security)"; break;
            case sovereign_shim::PqcPolicy::HYBRID_ED25519: policy_str = "Hybrid Ed25519"; break;
            default: policy_str = "AUTO"; break;
        }
        
        std::cout << "[" << std::setw(2) << t << "s] "
                  << "Policy: " << std::left << std::setw(22) << policy_str
                  << " | Conn: " << std::setw(5) << telemetry.active_connections
                  << " | CacheMiss: " << std::setw(5) << std::fixed << std::setprecision(1) << telemetry.cache_miss_rate << "%"
                  << " | P99: " << std::setw(6) << telemetry.p99_latency_ns/1000 << "µs"
                  << " | Attack: " << (telemetry.under_attack ? "⚠️ YES" : "✅ no")
                  << "\n";
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    shim.stop();
    proxy_thread.join();
    
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  SOVEREIGN-SHIM DEMO COMPLETE                                ║\n";
    std::cout << "║  Drop-in PQC proxy for Gulf legacy infrastructure            ║\n";
    std::cout << "║  Ready for Aramco SCADA, SAMA banking, EDGE defense systems  ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    
    return 0;
}
