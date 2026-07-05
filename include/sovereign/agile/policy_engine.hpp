#pragma once
#include "sovereign/crypto/signature_scheme.hpp"
#include "sovereign/metrics/microarch_profiler.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>

namespace sovereign { namespace agile {

struct RuntimeMetrics {
    double cpu_utilization_pct{0.0};
    double available_bandwidth_kbps{0.0};
    double battery_level_pct{100.0};
    double current_latency_us{0.0};
    double cache_miss_rate_pct{0.0};
    double branch_miss_rate_pct{0.0};
    double ipc{0.0};
    uint64_t active_connections{0};
    uint64_t signatures_per_second{0};
    bool under_attack{false};
    std::chrono::system_clock::time_point timestamp;
};

enum class AlgorithmTier { SPEED, COMPACT, SECURITY, CLASSICAL, HYBRID };

struct PolicyDecision {
    AlgorithmTier tier;
    std::string algorithm_name;
    std::string reason;
    double confidence;
    std::chrono::system_clock::time_point decided_at;
    uint64_t decision_id;
};

struct PolicyRule {
    std::string name;
    AlgorithmTier target_tier;
    int priority;
    std::function<bool(const RuntimeMetrics&)> condition;
};

class CryptoAgileController {
public:
    CryptoAgileController();
    ~CryptoAgileController();
    void add_rule(const PolicyRule& rule);
    void set_update_interval_ms(int ms);
    void start();
    void stop();
    void update_metrics(const RuntimeMetrics& metrics);
    [[nodiscard]] PolicyDecision current_decision() const;
    void register_scheme(AlgorithmTier tier, std::unique_ptr<SignatureScheme> scheme);
    [[nodiscard]] const std::vector<PolicyDecision>& decision_history() const;
    void force_tier(AlgorithmTier tier, const std::string& reason);
    [[nodiscard]] uint64_t total_switches() const { return total_switches_; }
    [[nodiscard]] AlgorithmTier current_tier() const { return current_tier_; }
private:
    void evaluation_loop();
    PolicyDecision evaluate_policy(const RuntimeMetrics& metrics);
    void execute_switch(const PolicyDecision& decision);
    std::string tier_to_string(AlgorithmTier tier) const;
    std::vector<PolicyRule> rules_;
    std::map<AlgorithmTier, std::unique_ptr<SignatureScheme>> schemes_;
    std::atomic<AlgorithmTier> current_tier_{AlgorithmTier::CLASSICAL};
    std::atomic<bool> running_{false};
    std::atomic<uint64_t> total_switches_{0};
    std::atomic<uint64_t> decision_counter_{0};
    std::atomic<double> latest_bandwidth_{0.0};
    std::atomic<double> latest_latency_{0.0};
    std::atomic<double> latest_battery_{100.0};
    std::atomic<bool> under_attack_{false};
    int update_interval_ms_{100};
    mutable std::mutex history_mutex_;
    std::vector<PolicyDecision> decision_history_;
    std::unique_ptr<std::thread> evaluation_thread_;
};

class GulfPolicyBuilder {
public:
    static std::vector<PolicyRule> build_default_gulf_rules();
};

}}
