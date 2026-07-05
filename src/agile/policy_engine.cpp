#include "sovereign/agile/policy_engine.hpp"
#include <iostream>
#include <algorithm>
#include <sstream>

namespace sovereign { namespace agile {

CryptoAgileController::CryptoAgileController() {
    std::cout << "[CryptoAgile] Controller initialized.\n";
}
CryptoAgileController::~CryptoAgileController() { stop(); }

void CryptoAgileController::add_rule(const PolicyRule& rule) {
    rules_.push_back(rule);
    std::sort(rules_.begin(), rules_.end(), [](auto& a, auto& b) { return a.priority > b.priority; });
}
void CryptoAgileController::set_update_interval_ms(int ms) { update_interval_ms_ = std::max(10, std::min(ms, 10000)); }

void CryptoAgileController::start() {
    if (running_) return;
    running_ = true;
    evaluation_thread_ = std::make_unique<std::thread>(&CryptoAgileController::evaluation_loop, this);
}
void CryptoAgileController::stop() {
    running_ = false;
    if (evaluation_thread_ && evaluation_thread_->joinable()) evaluation_thread_->join();
}

void CryptoAgileController::update_metrics(const RuntimeMetrics& m) {
    latest_bandwidth_ = m.available_bandwidth_kbps;
    latest_latency_ = m.current_latency_us;
    latest_battery_ = m.battery_level_pct;
    under_attack_ = m.under_attack;
}
void CryptoAgileController::register_scheme(AlgorithmTier tier, std::unique_ptr<SignatureScheme> s) {
    schemes_[tier] = std::move(s);
}
PolicyDecision CryptoAgileController::current_decision() const {
    std::lock_guard<std::mutex> lk(history_mutex_);
    if (decision_history_.empty()) return {current_tier_, tier_to_string(current_tier_), "Init", 1.0, {}, 0};
    return decision_history_.back();
}
const std::vector<PolicyDecision>& CryptoAgileController::decision_history() const { return decision_history_; }
void CryptoAgileController::force_tier(AlgorithmTier t, const std::string& r) {
    execute_switch({t, tier_to_string(t), "MANUAL: "+r, 1.0, std::chrono::system_clock::now(), ++decision_counter_});
}

void CryptoAgileController::evaluation_loop() {
    while (running_) {
        RuntimeMetrics m;
        m.available_bandwidth_kbps = latest_bandwidth_;
        m.current_latency_us = latest_latency_;
        m.battery_level_pct = latest_battery_;
        m.under_attack = under_attack_;
        auto d = evaluate_policy(m);
        if (d.tier != current_tier_) execute_switch(d);
        std::this_thread::sleep_for(std::chrono::milliseconds(update_interval_ms_));
    }
}

PolicyDecision CryptoAgileController::evaluate_policy(const RuntimeMetrics& m) {
    for (auto& r : rules_) {
        if (r.condition(m)) return {r.target_tier, tier_to_string(r.target_tier), r.name, 0.85, std::chrono::system_clock::now(), ++decision_counter_};
    }
    return {current_tier_, tier_to_string(current_tier_), "No rule triggered", 0.5, std::chrono::system_clock::now(), ++decision_counter_};
}

void CryptoAgileController::execute_switch(const PolicyDecision& d) {
    auto old = current_tier_.exchange(d.tier);
    total_switches_++;
    std::lock_guard<std::mutex> lk(history_mutex_);
    decision_history_.push_back(d);
    std::cout << "[SWITCH #" << total_switches_ << "] " << tier_to_string(old) << " -> " << tier_to_string(d.tier) << " | " << d.reason << "\n";
}

std::string CryptoAgileController::tier_to_string(AlgorithmTier t) const {
    switch(t) { case AlgorithmTier::SPEED: return "ML-DSA-44"; case AlgorithmTier::COMPACT: return "Falcon-512"; case AlgorithmTier::SECURITY: return "SPHINCS+-256s"; case AlgorithmTier::CLASSICAL: return "Ed25519"; case AlgorithmTier::HYBRID: return "Hybrid"; default: return "Unknown"; }
}

std::vector<PolicyRule> GulfPolicyBuilder::build_default_gulf_rules() {
    return {
        {"Under Attack", AlgorithmTier::SECURITY, 100, [](auto& m){ return m.under_attack; }},
        {"Low Bandwidth", AlgorithmTier::COMPACT, 90, [](auto& m){ return m.available_bandwidth_kbps < 50.0 && m.available_bandwidth_kbps > 0; }},
        {"Battery Critical", AlgorithmTier::SPEED, 85, [](auto& m){ return m.battery_level_pct < 15.0; }},
        {"High Latency", AlgorithmTier::SPEED, 80, [](auto& m){ return m.current_latency_us > 1000.0; }},
        {"Normal", AlgorithmTier::SPEED, 50, [](auto&){ return true; }}
    };
}

}}
