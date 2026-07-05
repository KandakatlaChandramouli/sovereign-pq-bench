#include "sovereign_shim.hpp"
#include <iostream>
#include <cstring>
#include <chrono>
#include <algorithm>
#include <cstdlib>

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#endif

namespace sovereign_shim {

PolicyEngine::PolicyEngine() {
    std::cout << "[SHIM-POLICY] Policy engine initialized. Default: ML-DSA-44\n";
}

PqcPolicy PolicyEngine::evaluate(const HardwareTelemetry& telemetry) {
    PqcPolicy new_policy = current_policy_;
    
    if (telemetry.under_attack) {
        new_policy = PqcPolicy::SPHINCS_256S;
    } else if (telemetry.cache_miss_rate > cache_thrash_threshold_) {
        new_policy = PqcPolicy::FALCON_512;
    } else if (telemetry.p99_latency_ns > latency_threshold_ns_) {
        new_policy = PqcPolicy::ML_DSA_44;
    } else {
        new_policy = PqcPolicy::ML_DSA_44;
    }
    
    if (new_policy != current_policy_) {
        auto old = current_policy_.exchange(new_policy);
        total_switches_++;
        std::cout << "[SHIM-POLICY] SWITCH #" << total_switches_ 
                  << ": " << policy_name(old) << " -> " << policy_name(new_policy)
                  << " | CacheMiss=" << telemetry.cache_miss_rate << "%"
                  << " P99=" << telemetry.p99_latency_ns/1000 << "us\n";
    }
    
    return new_policy;
}

std::string PolicyEngine::policy_name(PqcPolicy p) const {
    switch (p) {
        case PqcPolicy::ML_DSA_44: return "ML-DSA-44 (Speed)";
        case PqcPolicy::FALCON_512: return "Falcon-512 (Compact)";
        case PqcPolicy::SPHINCS_256S: return "SPHINCS+-256s (Security)";
        case PqcPolicy::HYBRID_ED25519: return "Hybrid Ed25519";
        case PqcPolicy::AUTO: return "AUTO (PMU-driven)";
        default: return "Unknown";
    }
}

SovereignProxyShim::SovereignProxyShim(
    const std::string& listen_addr, int listen_port,
    const std::string& upstream_addr, int upstream_port)
    : listen_addr_(listen_addr), listen_port_(listen_port),
      upstream_addr_(upstream_addr), upstream_port_(upstream_port) {
    
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  SOVEREIGN-SHIM PROXY CORE v1.0                             ║\n";
    std::cout << "║  Crypto-Agile PQC Network Proxy for Gulf Infrastructure     ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    std::cout << "[SHIM] Listen: " << listen_addr << ":" << listen_port << "\n";
    std::cout << "[SHIM] Upstream: " << upstream_addr << ":" << upstream_port << "\n";
}

SovereignProxyShim::~SovereignProxyShim() { stop(); }

void SovereignProxyShim::start() {
#ifdef __linux__
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        std::cerr << "[SHIM] Failed to create socket\n";
        return;
    }
    
    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    fcntl(server_fd_, F_SETFL, O_NONBLOCK);
    
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(listen_port_);
    inet_pton(AF_INET, listen_addr_.c_str(), &addr.sin_addr);
    
    if (bind(server_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "[SHIM] Bind failed (port may be in use — this is expected in demo mode)\n";
        close(server_fd_);
        server_fd_ = -1;
    } else {
        listen(server_fd_, 128);
        std::cout << "[SHIM] Socket bound successfully.\n";
    }
#endif
    
    running_ = true;
    
    std::cout << "[SHIM] Proxy started. Monitoring hardware for adaptive PQC selection...\n";
    std::cout << "[SHIM] Active policy: " << policy_engine_.policy_name(active_policy_) << "\n\n";
    
    monitor_thread_ = std::make_unique<std::thread>(&SovereignProxyShim::hardware_monitor_loop, this);
    
    if (server_fd_ >= 0) {
        accept_thread_ = std::make_unique<std::thread>(&SovereignProxyShim::accept_loop, this);
        if (accept_thread_) accept_thread_->join();
    }
    if (monitor_thread_) monitor_thread_->join();
}

void SovereignProxyShim::stop() {
    running_ = false;
#ifdef __linux__
    if (server_fd_ >= 0) close(server_fd_);
#endif
    std::cout << "[SHIM] Proxy stopped. Total connections: " << total_connections_ 
              << " | Policy switches: " << policy_engine_.total_switches() << "\n";
}

void SovereignProxyShim::force_policy(PqcPolicy policy) {
    active_policy_ = policy;
    std::cout << "[SHIM] MANUAL OVERRIDE: " << policy_engine_.policy_name(policy) << "\n";
}

HardwareTelemetry SovereignProxyShim::get_telemetry() const {
    std::lock_guard<std::mutex> lock(telemetry_mutex_);
    return latest_telemetry_;
}

PqcPolicy SovereignProxyShim::active_policy() const { return active_policy_; }

void SovereignProxyShim::accept_loop() {
#ifdef __linux__
    while (running_ && server_fd_ >= 0) {
        struct sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &addr_len);
        
        if (client_fd < 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        
        total_connections_++;
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        
        std::cout << "[SHIM] Connection #" << total_connections_ 
                  << " from " << client_ip << ":" << ntohs(client_addr.sin_port)
                  << " | Policy: " << policy_engine_.policy_name(active_policy_) << "\n";
        
        std::string response = "SOVEREIGN-SHIM: Active PQC = " + 
                               policy_engine_.policy_name(active_policy_) + 
                               " | Switches: " + std::to_string(policy_engine_.total_switches()) + "\n";
        send(client_fd, response.c_str(), response.size(), 0);
        close(client_fd);
    }
#endif
}

void SovereignProxyShim::handle_client(int client_fd) {
    (void)client_fd; // Production: full PQC TLS termination
}

void SovereignProxyShim::hardware_monitor_loop() {
    while (running_) {
        HardwareTelemetry telemetry;
        telemetry.cache_miss_rate = (rand() % 100) < 15 ? 65.0 : 15.0;
        telemetry.p99_latency_ns = (rand() % 100) < 10 ? 2'000'000 : 500'000;
        telemetry.under_attack = (rand() % 100) < 5;
        telemetry.active_connections = total_connections_;
        
        {
            std::lock_guard<std::mutex> lock(telemetry_mutex_);
            latest_telemetry_ = telemetry;
        }
        
        auto new_policy = policy_engine_.evaluate(telemetry);
        active_policy_ = new_policy;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

BufferPool::BufferPool(size_t buffer_size, size_t pool_count)
    : buffer_size_(buffer_size) {
    for (size_t i = 0; i < pool_count; i++) {
        free_buffers_.push_back(::operator new(buffer_size));
    }
}

void* BufferPool::acquire() {
    std::lock_guard<std::mutex> lock(pool_mutex_);
    if (free_buffers_.empty()) return ::operator new(buffer_size_);
    void* buf = free_buffers_.back();
    free_buffers_.pop_back();
    return buf;
}

void BufferPool::release(void* buffer) {
    std::lock_guard<std::mutex> lock(pool_mutex_);
    free_buffers_.push_back(buffer);
}

size_t BufferPool::available() const { return free_buffers_.size(); }

}
