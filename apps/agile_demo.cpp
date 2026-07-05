#include "sovereign/agile/policy_engine.hpp"
#include "sovereign/crypto/ecdsa_engine.hpp"
#include "sovereign/crypto/ed25519_engine.hpp"
#include "sovereign/crypto/ml_dsa_engine.hpp"
#include "sovereign/crypto/pq_engine.hpp"
#include <iostream>
#include <thread>
#include <iomanip>

using namespace sovereign::agile;

int main() {
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  SOVEREIGN PROTOCOL — Crypto-Agile Policy Engine Demo        ║\n";
    std::cout << "║  Runtime-Adaptive Algorithm Selection for Gulf Infrastructure ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";

    // 1. Create controller
    CryptoAgileController controller;
    controller.set_update_interval_ms(200);

    // 2. Register available schemes
    controller.register_scheme(AlgorithmTier::SPEED, std::make_unique<sovereign::MlDsa44Engine>());
    controller.register_scheme(AlgorithmTier::COMPACT, std::make_unique<sovereign::Falcon512Engine>());
    controller.register_scheme(AlgorithmTier::SECURITY, std::make_unique<sovereign::Sphincs256sEngine>());
    controller.register_scheme(AlgorithmTier::CLASSICAL, std::make_unique<sovereign::Ed25519Engine>());

    // 3. Load Gulf policy rules
    auto rules = GulfPolicyBuilder::build_default_gulf_rules();
    for (auto& r : rules) {
        controller.add_rule(r);
    }
    std::cout << "Loaded " << rules.size() << " policy rules.\n\n";

    // 4. Start the controller
    controller.start();

    // 5. Simulate changing conditions
    std::cout << "=== SIMULATING GULF INFRASTRUCTURE SCENARIOS ===\n\n";
    
    struct Scenario {
        const char* name;
        RuntimeMetrics metrics;
        int duration_ms;
    };

    Scenario scenarios[] = {
        {"Normal Operation (Oil Field)", 
         {.available_bandwidth_kbps=100.0, .battery_level_pct=85.0, .current_latency_us=500.0}, 800},
        
        {"LoRa Bandwidth Drop (Pipeline Sensor)",
         {.available_bandwidth_kbps=25.0, .battery_level_pct=60.0, .current_latency_us=800.0}, 800},
        
        {"Battery Critical (Remote Drone)",
         {.available_bandwidth_kbps=80.0, .battery_level_pct=10.0, .current_latency_us=300.0}, 800},
        
        {"CYBER ATTACK DETECTED (Cache Miss Spike)",
         {.available_bandwidth_kbps=100.0, .battery_level_pct=90.0, .current_latency_us=200.0, 
          .cache_miss_rate_pct=65.0, .under_attack=true}, 1000},
        
        {"Recovery — Return to Normal",
         {.available_bandwidth_kbps=100.0, .battery_level_pct=90.0, .current_latency_us=400.0}, 800},
    };

    for (auto& scenario : scenarios) {
        std::cout << "\n▶ " << scenario.name << "\n";
        std::cout << "  BW=" << scenario.metrics.available_bandwidth_kbps 
                  << "kbps Bat=" << scenario.metrics.battery_level_pct 
                  << "% Lat=" << scenario.metrics.current_latency_us 
                  << "us Attack=" << (scenario.metrics.under_attack ? "YES" : "no") << "\n";
        
        controller.update_metrics(scenario.metrics);
        std::this_thread::sleep_for(std::chrono::milliseconds(scenario.duration_ms));
        
        auto decision = controller.current_decision();
        std::cout << "  → ACTIVE: " << decision.algorithm_name 
                  << " (" << decision.reason << ")\n";
    }

    // 6. Show decision history
    std::cout << "\n\n=== AUDIT TRAIL ===\n";
    std::cout << std::left << std::setw(6) << "ID" << std::setw(30) << "Algorithm" 
              << std::setw(40) << "Reason" << "Confidence\n";
    std::cout << std::string(90, '─') << "\n";
    
    for (auto& d : controller.decision_history()) {
        std::cout << std::left << std::setw(6) << d.decision_id 
                  << std::setw(30) << d.algorithm_name
                  << std::setw(40) << d.reason
                  << std::right << std::setw(8) << std::fixed << std::setprecision(0) 
                  << d.confidence*100 << "%\n";
    }

    std::cout << "\nTotal algorithm switches: " << controller.total_switches() << "\n";
    
    controller.stop();
    
    std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  DEMO COMPLETE — Crypto-Agile Controller Operational         ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    
    return 0;
}
