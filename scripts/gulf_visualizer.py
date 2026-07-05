#!/usr/bin/env python3
"""
Sovereign Protocol — Gulf Infrastructure PQC Visualization Engine
"Quantifying the Cost of Sovereignty"
Produces publication-grade figures for Gulf government stakeholders.
"""

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker
import numpy as np
import os

# ── Gulf Sovereign Style ──
plt.rcParams.update({
    'font.family': 'serif',
    'font.size': 13,
    'axes.titlesize': 16,
    'axes.labelsize': 13,
    'figure.dpi': 300,
    'savefig.dpi': 300,
    'savefig.bbox': 'tight',
    'axes.spines.top': False,
    'axes.spines.right': False,
})

# Gulf-inspired color palette
GOLD = '#C8963E'
DEEP_BLUE = '#1B365D'
TEAL = '#007A87'
SAND = '#D4C5A9'
RED = '#C41E3A'
GREEN = '#2E7D32'
PURPLE = '#6A1B9A'

OUTPUT = '../results/figures'
os.makedirs(OUTPUT, exist_ok=True)

# ── DATA (Real benchmark results) ──
algorithms = ['ECDSA\nP256', 'Ed25519', 'ML-DSA\n44', 'ML-DSA\n65', 'ML-DSA\n87',
              'Falcon\n512', 'Falcon\n1024', 'SPHINCS+\n128s', 'SPHINCS+\n256s']
sign_us = [1313.1, 189.6, 303.6, 139.3, 436.3, 382.1, 1112.8, 211852.9, 380558.2]
verify_us = [487.1, 256.1, 39.3, 111.6, 104.2, 69.4, 202.9, 369.8, 812.2]
key_bytes = [91, 32, 1312, 1952, 2592, 897, 1793, 32, 64]
sig_bytes = [70, 64, 2420, 3309, 4627, 652, 1272, 7856, 29792]
colors = [RED, RED, GREEN, GREEN, GREEN, TEAL, TEAL, PURPLE, PURPLE]
categories = ['Classical', 'Classical', 'PQ Lattice', 'PQ Lattice', 'PQ Lattice',
              'PQ Lattice', 'PQ Lattice', 'PQ Hash', 'PQ Hash']

# ── FIGURE 1: Signing Speed — The Quantum Advantage ──
fig, ax = plt.subplots(figsize=(14, 7))
x = np.arange(len(algorithms))
bars = ax.bar(x, sign_us, color=colors, edgecolor='white', linewidth=1.2, alpha=0.9)
for bar, val in zip(bars, sign_us):
    if val > 10000:
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 5000,
                f'{val/1000:.1f}ms', ha='center', va='bottom', fontsize=8, fontweight='bold')
    else:
        ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 20,
                f'{val:.0f}µs', ha='center', va='bottom', fontsize=9, fontweight='bold')

ax.set_xticks(x)
ax.set_xticklabels(algorithms, fontsize=10)
ax.set_ylabel('Signing Latency (µs)', fontweight='bold')
ax.set_title('Post-Quantum Signature Performance: The Gulf Migration Matrix', fontweight='bold', pad=20)
ax.set_yscale('log')
ax.set_ylim(10, 1_000_000)
ax.axhline(y=1000, color=GOLD, linestyle='--', linewidth=1.5, alpha=0.7)
ax.text(8.2, 1200, 'Real-time threshold (1ms)', color=GOLD, fontsize=9, fontstyle='italic')
ax.grid(axis='y', alpha=0.2)

# Category legend
from matplotlib.patches import Patch
legend_elements = [Patch(facecolor=RED, label='Classical (Vulnerable to Quantum)'),
                   Patch(facecolor=GREEN, label='PQ Lattice (FIPS 204/206)'),
                   Patch(facecolor=TEAL, label='PQ Compact (Falcon)'),
                   Patch(facecolor=PURPLE, label='PQ Hash (FIPS 205)')]
ax.legend(handles=legend_elements, loc='upper left', fontsize=9, framealpha=0.9)
plt.tight_layout()
fig.savefig(f'{OUTPUT}/gulf_signing_speed.svg')
fig.savefig(f'{OUTPUT}/gulf_signing_speed.png')
fig.savefig(f'{OUTPUT}/gulf_signing_speed.pdf')
plt.close()
print("✅ Figure 1: Signing Speed")

# ── FIGURE 2: Key Size Tax — The Storage Burden ──
fig, ax = plt.subplots(figsize=(14, 7))
x = np.arange(len(algorithms))
width = 0.35
bars1 = ax.bar(x - width/2, key_bytes, width, label='Public Key', color=DEEP_BLUE, edgecolor='white')
bars2 = ax.bar(x + width/2, sig_bytes, width, label='Signature', color=GOLD, edgecolor='white')

for bar, val in zip(bars1, key_bytes):
    ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 50,
            f'{val}B', ha='center', va='bottom', fontsize=7, fontweight='bold', rotation=90)
for bar, val in zip(bars2, sig_bytes):
    ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 100,
            f'{val}B', ha='center', va='bottom', fontsize=7, fontweight='bold', rotation=90)

ax.set_xticks(x)
ax.set_xticklabels(algorithms, fontsize=10)
ax.set_ylabel('Size (Bytes)', fontweight='bold')
ax.set_title('The Spatial Quantum Tax: Key & Signature Sizes', fontweight='bold', pad=20)
ax.set_yscale('log')
ax.legend(fontsize=10)
ax.grid(axis='y', alpha=0.2)
plt.tight_layout()
fig.savefig(f'{OUTPUT}/gulf_key_size_tax.svg')
fig.savefig(f'{OUTPUT}/gulf_key_size_tax.png')
fig.savefig(f'{OUTPUT}/gulf_key_size_tax.pdf')
plt.close()
print("✅ Figure 2: Key Size Tax")

# ── FIGURE 3: Gulf Infrastructure Recommendations Radar ──
fig, ax = plt.subplots(figsize=(10, 10), subplot_kw=dict(polar=True))
categories_radar = ['Signing\nSpeed', 'Key\nCompactness', 'Signature\nCompactness',
                    'Security\nMargin', 'Stateless', 'NIST\nMaturity']
N = len(categories_radar)
angles = [n / float(N) * 2 * np.pi for n in range(N)]
angles += angles[:1]

# ML-DSA-44 scores
mldsa44 = [9, 4, 4, 7, 9, 10]
mldsa44 += mldsa44[:1]
# Falcon-512 scores
falcon512 = [8, 7, 8, 7, 9, 9]
falcon512 += falcon512[:1]
# SPHINCS+-256s scores
sphincs256 = [1, 10, 1, 10, 10, 10]
sphincs256 += sphincs256[:1]

ax.fill(angles, mldsa44, alpha=0.25, color=GREEN, label='ML-DSA-44 (General Purpose)')
ax.plot(angles, mldsa44, color=GREEN, linewidth=2)
ax.fill(angles, falcon512, alpha=0.25, color=TEAL, label='Falcon-512 (Bandwidth-Constrained)')
ax.plot(angles, falcon512, color=TEAL, linewidth=2)
ax.fill(angles, sphincs256, alpha=0.25, color=PURPLE, label='SPHINCS+-256s (Maximum Assurance)')
ax.plot(angles, sphincs256, color=PURPLE, linewidth=2)

ax.set_xticks(angles[:-1])
ax.set_xticklabels(categories_radar, fontsize=11)
ax.set_ylim(0, 10)
ax.set_title('Gulf Infrastructure: Algorithm Selection Radar\n(10 = Optimal for Gulf Use Case)', fontweight='bold', pad=25)
ax.legend(loc='upper right', bbox_to_anchor=(1.35, 1.1), fontsize=10)
ax.grid(True, alpha=0.3)
plt.tight_layout()
fig.savefig(f'{OUTPUT}/gulf_algorithm_radar.svg')
fig.savefig(f'{OUTPUT}/gulf_algorithm_radar.png')
fig.savefig(f'{OUTPUT}/gulf_algorithm_radar.pdf')
plt.close()
print("✅ Figure 3: Algorithm Radar")

# ── FIGURE 4: Full PQC Stack — TLS 2.0 Handshake ──
fig, ax = plt.subplots(figsize=(12, 6))
stages = ['Key\nGeneration', 'KEM\nEncaps', 'KEM\nDecaps', 'Sign\nCertificate', 'Verify\nCertificate']
classical_tls = [150, 0.8, 0.6, 1313, 487]
pq_tls = [200, 20, 15.7, 304, 39]

x = np.arange(len(stages))
width = 0.35
ax.bar(x - width/2, classical_tls, width, label='Classical TLS (ECDSA + ECDH)', color=RED, alpha=0.8, edgecolor='white')
ax.bar(x + width/2, pq_tls, width, label='PQ TLS (ML-DSA-44 + ML-KEM-512)', color=GREEN, alpha=0.8, edgecolor='white')

for i, (c, p) in enumerate(zip(classical_tls, pq_tls)):
    ax.text(i - width/2, c + 20, f'{c:.0f}µs', ha='center', fontsize=8, fontweight='bold')
    ax.text(i + width/2, p + 20, f'{p:.0f}µs', ha='center', fontsize=8, fontweight='bold')

ax.set_xticks(x)
ax.set_xticklabels(stages, fontsize=11)
ax.set_ylabel('Latency (µs)', fontweight='bold')
ax.set_title('TLS 2.0 Handshake: Classical vs Post-Quantum', fontweight='bold', pad=20)
ax.legend(fontsize=10)
ax.grid(axis='y', alpha=0.2)
plt.tight_layout()
fig.savefig(f'{OUTPUT}/gulf_tls_handshake.svg')
fig.savefig(f'{OUTPUT}/gulf_tls_handshake.png')
fig.savefig(f'{OUTPUT}/gulf_tls_handshake.pdf')
plt.close()
print("✅ Figure 4: TLS Handshake")

# ── FIGURE 5: Hash Function Throughput ──
fig, ax = plt.subplots(figsize=(12, 6))
hash_algs = ['SHA-256', 'SHA-512', 'SHA3-256', 'SHA3-512']
throughput_1mb = [3191/1000, 2091/1000, 3843/1000, 6946/1000]  # Convert to ms
colors_hash = [DEEP_BLUE, DEEP_BLUE, TEAL, TEAL]
bars = ax.bar(hash_algs, throughput_1mb, color=colors_hash, edgecolor='white', linewidth=1.5)
for bar, val in zip(bars, throughput_1mb):
    ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.1,
            f'{val:.1f}ms', ha='center', fontsize=11, fontweight='bold')
ax.set_ylabel('Latency for 1MB (ms)', fontweight='bold')
ax.set_title('Hash Function Performance: Symmetric PQC Baseline', fontweight='bold', pad=20)
ax.grid(axis='y', alpha=0.2)
plt.tight_layout()
fig.savefig(f'{OUTPUT}/gulf_hash_performance.svg')
fig.savefig(f'{OUTPUT}/gulf_hash_performance.png')
fig.savefig(f'{OUTPUT}/gulf_hash_performance.pdf')
plt.close()
print("✅ Figure 5: Hash Performance")

print(f"\n{'='*60}")
print(f"  5 ELITE GULF FIGURES GENERATED")
print(f"  {OUTPUT}/")
print(f"  Formats: SVG + PNG + PDF")
print(f"{'='*60}")
