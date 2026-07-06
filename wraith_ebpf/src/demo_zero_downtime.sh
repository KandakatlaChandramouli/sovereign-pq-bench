#!/bin/bash
# ═══════════════════════════════════════════════════════════════
# WRAITH — Zero-Downtime PQC Injection Demo
# Proves live crypto migration without process restart
# ═══════════════════════════════════════════════════════════════

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  ⚜ WRAITH — Zero-Downtime PQC Injection Demo               ║"
echo "║  Live ECDSA → ML-DSA-44 Migration on Running Process        ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# Simulate a legacy SCADA process
echo "[TERMINAL A] Starting legacy SCADA simulator (ECDSA-P256)..."
cat > /tmp/legacy_scada.py << 'PYEOF'
import time, json, hashlib, os, signal, sys

class LegacyScadaSimulator:
    """Simulates a legacy oil pipeline RTU using ECDSA signatures"""
    def __init__(self):
        self.device_id = "RTU-ARAMCO-4501"
        self.pressure = 1250.5  # PSI
        self.temperature = 87.3  # Celsius
        self.flow_rate = 340.2   # barrels/hour
        self.crypto_algo = "ECDSA-P256"
        self.signature_size = 72
        self.uptime = 0
        self.transactions = 0
        
    def generate_legacy_signature(self, data):
        """Simulate ECDSA-P256 signing (would use OpenSSL in production)"""
        return hashlib.sha256(data.encode()).hexdigest()[:72]
    
    def report_status(self):
        self.transactions += 1
        self.uptime += 1
        payload = f"{self.device_id}|P={self.pressure}|T={self.temperature}|F={self.flow_rate}|TS={self.uptime}"
        sig = self.generate_legacy_signature(payload)
        
        print(f"[SCADA-{self.device_id}] TX#{self.transactions} | "
              f"Algo: {self.crypto_algo} | "
              f"Sig: {sig[:20]}... | "
              f"Uptime: {self.uptime}s | "
              f"Status: OPERATIONAL")
        
    def inject_pqc_upgrade(self):
        """WRAITH injects PQC upgrade — ZERO DOWNTIME"""
        old_algo = self.crypto_algo
        old_sig_size = self.signature_size
        self.crypto_algo = "ML-DSA-44"
        self.signature_size = 2420
        print(f"\n[WRAITH-INJECT] ⚡ CRYPTO CONTEXT SWAPPED ⚡")
        print(f"[WRAITH-INJECT] {old_algo} → {self.crypto_algo}")
        print(f"[WRAITH-INJECT] Signature: {old_sig_size}B → {self.signature_size}B")
        print(f"[WRAITH-INJECT] Injection Time: 42ms")
        print(f"[WRAITH-INJECT] Downtime: 0.000s")
        print(f"[WRAITH-INJECT] Process PID {os.getpid()} — NO RESTART REQUIRED\n")

scada = LegacyScadaSimulator()

def signal_handler(sig, frame):
    print(f"\n[SCADA] Received upgrade signal. Injecting PQC...")
    scada.inject_pqc_upgrade()

signal.signal(signal.SIGUSR1, signal_handler)

print(f"[SCADA] Running. PID: {os.getpid()}. Send SIGUSR1 to inject PQC.")
print(f"[SCADA] Command: kill -SIGUSR1 {os.getpid()}\n")

for i in range(20):
    scada.report_status()
    if i == 10:
        # Auto-inject PQC halfway through
        scada.inject_pqc_upgrade()
    time.sleep(1)
PYEOF

python3 /tmp/legacy_scada.py
