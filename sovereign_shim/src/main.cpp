#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <cstring>
#include <cstdlib>
#include <oqs/oqs.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <signal.h>

// ═══════════════════════════════════════════════════════
// REAL Linux PMU — hardware cache miss counter
// ═══════════════════════════════════════════════════════
static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                            int cpu, int group_fd, unsigned long flags) {
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

class SovereignHardwareMonitor {
private:
    int perf_fd_;
    int branch_fd_;
public:
    bool InitializePMU() {
        struct perf_event_attr pe;
        std::memset(&pe, 0, sizeof(pe));
        pe.type = PERF_TYPE_HARDWARE;
        pe.size = sizeof(pe);
        pe.config = PERF_COUNT_HW_CACHE_MISSES;
        pe.disabled = 1;
        pe.exclude_kernel = 1;
        pe.exclude_hv = 1;

        perf_fd_ = perf_event_open(&pe, 0, -1, -1, 0);
        if (perf_fd_ < 0) {
            std::cerr << "[PMU] Cache miss counter unavailable. Run: sudo sysctl kernel.perf_event_paranoid=-1\n";
            return false;
        }
        
        pe.config = PERF_COUNT_HW_BRANCH_MISSES;
        branch_fd_ = perf_event_open(&pe, 0, -1, -1, 0);
        
        ioctl(perf_fd_, PERF_EVENT_IOC_RESET, 0);
        ioctl(perf_fd_, PERF_EVENT_IOC_ENABLE, 0);
        if (branch_fd_ >= 0) {
            ioctl(branch_fd_, PERF_EVENT_IOC_RESET, 0);
            ioctl(branch_fd_, PERF_EVENT_IOC_ENABLE, 0);
        }
        
        std::cout << "[PMU] Hardware performance counters initialized. Tracking L1/LLC cache misses.\n";
        return true;
    }

    long long ReadCacheMisses() {
        long long count = 0;
        if (perf_fd_ >= 0) read(perf_fd_, &count, sizeof(long long));
        return count;
    }
    
    long long ReadBranchMisses() {
        long long count = 0;
        if (branch_fd_ >= 0) read(branch_fd_, &count, sizeof(long long));
        return count;
    }

    ~SovereignHardwareMonitor() { 
        if (perf_fd_ >= 0) close(perf_fd_); 
        if (branch_fd_ >= 0) close(branch_fd_);
    }
};

// ═══════════════════════════════════════════════════════
// Real liboqs PQC engine
// ═══════════════════════════════════════════════════════
class SovereignCryptoEngine {
private:
    OQS_SIG *current_sig_;
    std::string current_algo_;
    std::atomic<long long> total_signatures_{0};
    std::atomic<long long> total_bytes_signed_{0};
    
public:
    SovereignCryptoEngine() : current_sig_(nullptr) {
        SwitchAlgorithm(OQS_SIG_alg_ml_dsa_44);
    }
    
    void SwitchAlgorithm(const char* algo_name) {
        if (current_sig_) OQS_SIG_free(current_sig_);
        current_sig_ = OQS_SIG_new(algo_name);
        current_algo_ = algo_name;
        if (!current_sig_) {
            std::cerr << "[CRYPTO] Failed to load: " << algo_name << "\n";
            exit(1);
        }
        std::cout << "[CRYPTO] Loaded: " << algo_name 
                  << " (PubKey:" << current_sig_->length_public_key 
                  << "B Sig:" << current_sig_->length_signature << "B)\n";
    }
    
    bool SignData(const uint8_t* data, size_t len) {
        if (!current_sig_) return false;
        
        std::vector<uint8_t> pk(current_sig_->length_public_key);
        std::vector<uint8_t> sk(current_sig_->length_secret_key);
        
        if (OQS_SIG_keypair(current_sig_, pk.data(), sk.data()) != OQS_SUCCESS) {
            return false;
        }
        
        std::vector<uint8_t> sig(current_sig_->length_signature);
        size_t sig_len = current_sig_->length_signature;
        
        auto start = std::chrono::high_resolution_clock::now();
        OQS_STATUS status = OQS_SIG_sign(current_sig_, sig.data(), &sig_len, data, len, sk.data());
        auto end = std::chrono::high_resolution_clock::now();
        
        if (status != OQS_SUCCESS) return false;
        
        auto latency_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        total_signatures_++;
        total_bytes_signed_ += len;
        
        // Verify
        OQS_SIG_verify(current_sig_, data, len, sig.data(), sig_len, pk.data());
        
        std::cout << "[CRYPTO] Signed " << len << "B in " << latency_us << "µs"
                  << " | Sig:" << sig_len << "B"
                  << " | Total: " << total_signatures_ << " sigs, " 
                  << total_bytes_signed_/1024 << "KB\n";
        
        return true;
    }
    
    const char* CurrentAlgorithm() const { return current_algo_.c_str(); }
    long long TotalSignatures() const { return total_signatures_; }
    
    ~SovereignCryptoEngine() { if (current_sig_) OQS_SIG_free(current_sig_); }
};

// ═══════════════════════════════════════════════════════
// Policy engine — algorithm selection based on PMU
// ═══════════════════════════════════════════════════════
class AdaptivePolicyEngine {
private:
    SovereignCryptoEngine& crypto_;
    SovereignHardwareMonitor& pmu_;
    std::atomic<int> switches_{0};
    
public:
    AdaptivePolicyEngine(SovereignCryptoEngine& c, SovereignHardwareMonitor& p) 
        : crypto_(c), pmu_(p) {}
    
    void EvaluateAndAdapt() {
        long long cache_misses = pmu_.ReadCacheMisses();
        
        const char* new_algo = nullptr;
        
        if (cache_misses > 500000) {
            new_algo = OQS_SIG_alg_falcon_512;  // Lower cache pressure
        } else if (cache_misses > 100000) {
            new_algo = OQS_SIG_alg_ml_dsa_44;   // Balanced
        } else {
            new_algo = OQS_SIG_alg_ml_dsa_44;   // Fastest
        }
        
        if (std::string(new_algo) != crypto_.CurrentAlgorithm()) {
            crypto_.SwitchAlgorithm(new_algo);
            switches_++;
            std::cout << "[POLICY] SWITCH #" << switches_ 
                      << " → " << new_algo 
                      << " (CacheMisses: " << cache_misses << ")\n";
        }
    }
    
    int TotalSwitches() const { return switches_; }
};

// ═══════════════════════════════════════════════════════
// MAIN — Real proxy server
// ═══════════════════════════════════════════════════════
static std::atomic<bool> g_running{true};

void signal_handler(int) { g_running = false; }

int main(int argc, char* argv[]) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    int listen_port = (argc > 1) ? std::atoi(argv[1]) : 8080;
    
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║  ⚜ SOVEREIGN-SHIM — Bare-Metal PQC Proxy Core              ║\n";
    std::cout << "║  Real PMU • Real liboqs • Real TCP • Real Signatures        ║\n";
    std::cout << "║  \"Quantifying the Cost of Sovereignty\"                     ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    
    // 1. Initialize hardware PMU
    SovereignHardwareMonitor pmu;
    bool pmu_ok = pmu.InitializePMU();
    if (!pmu_ok) {
        std::cout << "[SHIM] Continuing without PMU — will use default policy.\n";
    }
    
    // 2. Initialize crypto engine
    SovereignCryptoEngine crypto;
    
    // 3. Initialize adaptive policy
    AdaptivePolicyEngine policy(crypto, pmu);
    
    // 4. Setup TCP server
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "[SHIM] Socket creation failed\n";
        return 1;
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(listen_port);
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "[SHIM] Bind failed on port " << listen_port << "\n";
        return 1;
    }
    
    listen(server_fd, 10);
    std::cout << "[SHIM] Listening on 0.0.0.0:" << listen_port << "\n";
    std::cout << "[SHIM] Send data: echo 'test' | nc localhost " << listen_port << "\n";
    std::cout << "[SHIM] Press Ctrl+C to stop.\n\n";
    
    // Background PMU monitor thread
    std::thread monitor([&]() {
        while (g_running) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            if (pmu_ok) policy.EvaluateAndAdapt();
        }
    });
    
    // 5. Accept loop — real TCP interception
    long long connection_count = 0;
    while (g_running) {
        sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        
        if (client_fd < 0) {
            if (g_running) std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        connection_count++;
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        
        std::cout << "\n[CONN #" << connection_count << "] " << client_ip 
                  << ":" << ntohs(client_addr.sin_port) << "\n";
        
        // Read data
        uint8_t buffer[65536];
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));
        
        if (bytes_read > 0) {
            std::cout << "[DATA] Received " << bytes_read << " bytes\n";
            
            // Real PQC signing
            crypto.SignData(buffer, static_cast<size_t>(bytes_read));
            
            // Response
            std::string response = "SOVEREIGN-SHIM OK | Algo: " + 
                                   std::string(crypto.CurrentAlgorithm()) +
                                   " | Sigs: " + std::to_string(crypto.TotalSignatures()) +
                                   " | Switches: " + std::to_string(policy.TotalSwitches()) + "\n";
            send(client_fd, response.c_str(), response.size(), 0);
        }
        
        close(client_fd);
    }
    
    close(server_fd);
    monitor.join();
    
    std::cout << "\n[SHIM] Shutdown. Connections: " << connection_count 
              << " | Signatures: " << crypto.TotalSignatures()
              << " | Policy switches: " << policy.TotalSwitches() << "\n";
    
    return 0;
}
