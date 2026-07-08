// ═══════════════════════════════════════════════════════════
// WRAITH KERNEL — eBPF Program
// Hooks: SSL_read, SSL_write, connect, accept
// Compile: clang -O2 -target bpf -c wraith_kern.c -o wraith_kern.o
// ═══════════════════════════════════════════════════════════

#include <linux/bpf.h>
#include <linux/ptrace.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include "../include/wraith_types.h"

// BPF maps shared with user-space
struct {
    __uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
    __uint(key_size, sizeof(__u32));
    __uint(value_size, sizeof(__u32));
} wraith_events SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_ARRAY);
    __uint(key_size, sizeof(__u32));
    __uint(value_size, sizeof(struct wraith_config));
    __uint(max_entries, 1);
} wraith_config SEC(".maps");

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(key_size, sizeof(__u32));   // PID
    __uint(value_size, sizeof(__u32)); // crypto_algo
    __uint(max_entries, 10240);
} process_crypto_map SEC(".maps");

// License required for eBPF
char LICENSE[] SEC("license") = "GPL";

// ═══════════════════════════════════════════════
// Hook 1: Intercept SSL_write syscalls
// ═══════════════════════════════════════════════
SEC("kprobe/SSL_write")
int wraith_hook_ssl_write(struct pt_regs *ctx)
{
    struct wraith_event event = {};
    struct wraith_config *config;
    __u32 config_key = 0;
    
    // Read configuration
    config = bpf_map_lookup_elem(&wraith_config, &config_key);
    if (!config) return 0;
    
    // Get current process info
    __u64 pid_tgid = bpf_get_current_pid_tgid();
    event.pid = pid_tgid >> 32;
    event.tid = pid_tgid & 0xFFFFFFFF;
    event.uid = bpf_get_current_uid_gid() & 0xFFFFFFFF;
    event.timestamp_ns = bpf_ktime_get_ns();
    event.event_type = WRAITH_EVENT_SSL_WRITE;
    
    // Get process name
    bpf_get_current_comm(&event.comm, sizeof(event.comm));
    
    // Filter by target PID if configured
    if (config->target_pid != 0 && event.pid != config->target_pid) {
        return 0;
    }
    
    // Check if this process has been fingerprinted
    __u32 *algo = bpf_map_lookup_elem(&process_crypto_map, &event.pid);
    if (algo) {
        event.crypto_algo = *algo;
        event.risk_score = (*algo == CRYPTO_RSA_2048 || *algo == CRYPTO_ECDSA_P256) ? 95 : 10;
        event.upgrade_target = UPGRADE_ML_DSA_44;
    } else {
        // First time seeing this process — mark for fingerprinting
        event.crypto_algo = 0;
        event.risk_score = 50;
        event.upgrade_target = UPGRADE_ML_DSA_44;
    }
    
    // Send event to user-space
    bpf_perf_event_output(ctx, &wraith_events, BPF_F_CURRENT_CPU,
                          &event, sizeof(event));
    
    return 0;
}

// ═══════════════════════════════════════════════
// Hook 2: Intercept SSL_read syscalls
// ═══════════════════════════════════════════════
SEC("kprobe/SSL_read")
int wraith_hook_ssl_read(struct pt_regs *ctx)
{
    struct wraith_event event = {};
    __u32 config_key = 0;
    struct wraith_config *config;
    
    config = bpf_map_lookup_elem(&wraith_config, &config_key);
    if (!config) return 0;
    
    __u64 pid_tgid = bpf_get_current_pid_tgid();
    event.pid = pid_tgid >> 32;
    event.tid = pid_tgid & 0xFFFFFFFF;
    event.uid = bpf_get_current_uid_gid() & 0xFFFFFFFF;
    event.timestamp_ns = bpf_ktime_get_ns();
    event.event_type = WRAITH_EVENT_SSL_READ;
    bpf_get_current_comm(&event.comm, sizeof(event.comm));
    
    if (config->target_pid != 0 && event.pid != config->target_pid) return 0;
    
    __u32 *algo = bpf_map_lookup_elem(&process_crypto_map, &event.pid);
    if (algo) {
        event.crypto_algo = *algo;
        event.risk_score = (*algo == CRYPTO_RSA_2048) ? 95 : 10;
    }
    
    bpf_perf_event_output(ctx, &wraith_events, BPF_F_CURRENT_CPU, &event, sizeof(event));
    return 0;
}

// ═══════════════════════════════════════════════
// Hook 3: Intercept TCP connect (detect external comms)
// ═══════════════════════════════════════════════
SEC("kprobe/tcp_v4_connect")
int wraith_hook_tcp_connect(struct pt_regs *ctx)
{
    struct wraith_event event = {};
    __u32 config_key = 0;
    struct wraith_config *config;
    
    config = bpf_map_lookup_elem(&wraith_config, &config_key);
    if (!config) return 0;
    
    __u64 pid_tgid = bpf_get_current_pid_tgid();
    event.pid = pid_tgid >> 32;
    event.timestamp_ns = bpf_ktime_get_ns();
    event.event_type = WRAITH_EVENT_KEY_EXCHANGE;
    bpf_get_current_comm(&event.comm, sizeof(event.comm));
    
    // Mark as potential key exchange — needs PQC upgrade
    event.crypto_algo = CRYPTO_ECDSA_P256;  // Assume ECDSA for key exchange
    event.risk_score = 90;
    event.upgrade_target = UPGRADE_ML_DSA_44;
    
    bpf_perf_event_output(ctx, &wraith_events, BPF_F_CURRENT_CPU, &event, sizeof(event));
    return 0;
}
