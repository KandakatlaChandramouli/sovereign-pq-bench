# ⚜ WRAITH — eBPF Zero-Touch PQC Injection Engine

## "Secure legacy infrastructure without touching a single line of code."

WRAITH is a kernel-level eBPF loader that intercepts TLS/SSL cryptographic
operations in running processes and transparently upgrades them to post-quantum
cryptography — without restarting the process, modifying the binary, or changing
any application configuration.

## How It Works

[ Legacy SCADA Process ]          [ WRAITH eBPF Hook ]
        |                                |
        | SSL_write(data)                |
        |------------------------------->| Intercept syscall
        |                                | Detect ECDSA/RSA usage
        |                                | Swap to ML-DSA-44
        |                                | Inject PQC signature
        |<-------------------------------| Return to kernel
        |                                |
[ Encrypted with PQC — process never knew ]

## Target Use Cases

- Aramco/ADNOC SCADA systems running Modbus/TCP over TLS
- Legacy Siemens/Schneider PLCs that cannot be patched
- Banking cores (SAMA/CBUAE) with 20-year-old binaries
- Any black-box system using OpenSSL/libcrypto

## Architecture

wraith_ebpf/
├── src/
│   ├── wraith_loader.cpp      # User-space eBPF loader
│   ├── wraith_kern.c           # Kernel-space eBPF program
│   └── wraith_detector.cpp    # Algorithm fingerprinting engine
├── include/
│   └── wraith_types.h         # Shared kernel/user types
└── README.md
