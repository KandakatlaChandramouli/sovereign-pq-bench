#pragma once
#include <stdint.h>

// Event types WRAITH can detect
#define WRAITH_EVENT_SSL_WRITE  1
#define WRAITH_EVENT_SSL_READ   2
#define WRAITH_EVENT_KEY_EXCHANGE 3
#define WRAITH_EVENT_CERT_VERIFY 4

// Cryptographic algorithm fingerprints
#define CRYPTO_ECDSA_P256  0x01
#define CRYPTO_RSA_2048    0x02
#define CRYPTO_RSA_4096    0x03
#define CRYPTO_ED25519     0x04
#define CRYPTO_AES_GCM     0x10

// PQC upgrade targets
#define UPGRADE_ML_DSA_44   0x80
#define UPGRADE_FALCON_512  0x81
#define UPGRADE_SPHINCS_256 0x82

// Event structure shared between kernel and user space
struct wraith_event {
    uint32_t pid;           // Process ID
    uint32_t tid;           // Thread ID
    uint32_t uid;           // User ID
    uint64_t timestamp_ns;  // Event timestamp
    uint32_t event_type;    // SSL_write, SSL_read, etc.
    uint32_t crypto_algo;   // Detected algorithm (ECDSA, RSA, etc.)
    uint32_t key_size_bits; // Key size in bits
    uint32_t upgrade_target;// Recommended PQC replacement
    char     comm[16];      // Process name (max 15 chars + null)
    char     remote_ip[16]; // Remote IP if networked
    uint16_t remote_port;   // Remote port
    uint32_t risk_score;    // Quantum risk score (0-100)
};

// Configuration passed from user-space to kernel eBPF
struct wraith_config {
    uint32_t target_pid;        // 0 = all processes
    uint32_t upgrade_mode;       // 0 = monitor only, 1 = auto-upgrade
    uint32_t whitelist_algo;    // Don't touch this algorithm
    uint32_t log_level;         // 0 = silent, 1 = events, 2 = verbose
};
