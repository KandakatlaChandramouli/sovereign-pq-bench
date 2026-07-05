#pragma once

#include <cstdint>
#include <atomic>
#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#endif

namespace sovereign_shim {

// ── Active PQC Policy (atomically switchable) ──
enum class PqcPolicy : uint8_t {
    ML_DSA_44 = 0,      // Fastest signing, general purpose
    FALCON_512 = 1,      // Compact, low variance, bandwidth-constrained
    SPHINCS_256S = 2,    // Maximum security, under-attack mode
    HYBRID_ED25519 = 3,  // Legacy compatibility
    AUTO = 255           // PMU-driven auto-selection
};

// ── Hardware telemetry snapshot ──
struct HardwareTelemetry {
    uint64_t cache_misses{0};
    uint64_t cache_references{0};
    uint64_t branch_misses{0};
    uint64_t instructions{0};
    uint64_t cycles{0};
    double cache_miss_rate{0.0};
    double ipc{0.0};
    uint64_t p99_latency_ns{0};
    uint64_t active_connections{0};
    uint64_t bytes_processed{0};
    bool under_attack{false};
};

// ── Proxy session state ──
struct ProxySession {
    int client_fd{-1};
    int upstream_fd{-1};
    PqcPolicy active_policy{PqcPolicy::AUTO};
    uint64_t session_id{0};
    uint64_t bytes_in{0};
    uint64_t bytes_out{0};
    std::chrono::steady_clock::time_point established_at;
};

// ── Policy decision engine ──
class PolicyEngine {
public:
    PolicyEngine();
    
    // Feed hardware telemetry and get policy recommendation
    [[nodiscard]] PqcPolicy evaluate(const HardwareTelemetry& telemetry);
    
    // Override thresholds
    void set_cache_thrash_threshold(double pct) { cache_thrash_threshold_ = pct; }
    void set_latency_threshold_us(uint64_t us) { latency_threshold_ns_ = us * 1000; }
    void set_bandwidth_threshold_kbps(double kbps) { bandwidth_threshold_kbps_ = kbps; }
    
    [[nodiscard]] std::string policy_name(PqcPolicy p) const;
    [[nodiscard]] uint64_t total_switches() const { return total_switches_; }

private:
    std::atomic<PqcPolicy> current_policy_{PqcPolicy::ML_DSA_44};
    std::atomic<uint64_t> total_switches_{0};
    double cache_thrash_threshold_{40.0};    // 40% cache miss = thrashing
    uint64_t latency_threshold_ns_{1'200'000}; // 1.2ms = excessive
    double bandwidth_threshold_kbps_{50.0};    // 50kbps = constrained
};

// ── The Sovereign-Shim Proxy Core ──
class SovereignProxyShim {
public:
    SovereignProxyShim(const std::string& listen_addr, int listen_port,
                       const std::string& upstream_addr, int upstream_port);
    ~SovereignProxyShim();

    // Start the proxy (blocking)
    void start();
    
    // Stop the proxy gracefully
    void stop();
    
    // Manual policy override
    void force_policy(PqcPolicy policy);
    
    // Get current operational status
    [[nodiscard]] HardwareTelemetry get_telemetry() const;
    [[nodiscard]] PqcPolicy active_policy() const;
    [[nodiscard]] uint64_t total_connections() const { return total_connections_; }
    [[nodiscard]] uint64_t total_bytes_proxied() const { return total_bytes_proxied_; }

private:
    void accept_loop();
    void handle_client(int client_fd);
    void hardware_monitor_loop();
    ssize_t secure_read(int fd, void* buf, size_t len);
    ssize_t secure_write(int fd, const void* buf, size_t len);
    
    std::string listen_addr_, upstream_addr_;
    int listen_port_, upstream_port_;
    int server_fd_{-1};
    
    PolicyEngine policy_engine_;
    std::atomic<PqcPolicy> active_policy_{PqcPolicy::AUTO};
    std::atomic<bool> running_{false};
    std::atomic<uint64_t> total_connections_{0};
    std::atomic<uint64_t> total_bytes_proxied_{0};
    
    std::vector<std::unique_ptr<ProxySession>> sessions_;
    std::mutex sessions_mutex_;
    
    std::unique_ptr<std::thread> accept_thread_;
    std::unique_ptr<std::thread> monitor_thread_;
    
    HardwareTelemetry latest_telemetry_;
    std::mutex telemetry_mutex_;
};

// ── Zero-copy buffer pool for io_uring-style operations ──
class BufferPool {
public:
    explicit BufferPool(size_t buffer_size = 65536, size_t pool_count = 64);
    
    [[nodiscard]] void* acquire();
    void release(void* buffer);
    [[nodiscard]] size_t buffer_size() const { return buffer_size_; }
    [[nodiscard]] size_t available() const;

private:
    size_t buffer_size_;
    std::vector<void*> free_buffers_;
    std::mutex pool_mutex_;
};

}
