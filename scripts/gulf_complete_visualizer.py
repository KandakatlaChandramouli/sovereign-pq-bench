#!/usr/bin/env python3
"""
Sovereign Protocol — Complete Gulf Infrastructure PQC Visualization Suite
"Quantifying the Cost of Sovereignty"
Generates 10 publication-grade figures covering all evidence.
"""

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import os

plt.rcParams.update({
    'font.family': 'serif', 'font.size': 12, 'axes.titlesize': 15,
    'axes.labelsize': 12, 'figure.dpi': 300, 'savefig.dpi': 300,
    'savefig.bbox': 'tight', 'axes.spines.top': False, 'axes.spines.right': False,
})

GOLD, DEEP, TEAL, RED, GREEN, PURPLE, ORANGE, SAND = '#C8963E','#1B365D','#007A87','#C41E3A','#2E7D32','#6A1B9A','#E67E22','#D4C5A9'
OUT = '../results/figures'
os.makedirs(OUT, exist_ok=True)

# ═══════════ FIG 1: Full Signing Speed Comparison ═══════════
algs = ['ECDSA\nP256','Ed25519','ML-DSA\n44','ML-DSA\n65','ML-DSA\n87','Falcon\n512','Falcon\n1024','SPHINCS+\n128s','SPHINCS+\n256s']
sign = [2197.5, 241.2, 104.1, 216.4, 708.4, 579.6, 1123.2, 235775.7, 379899.8]
colors = [RED,RED,GREEN,GREEN,GREEN,TEAL,TEAL,PURPLE,PURPLE]
fig, ax = plt.subplots(figsize=(14,7))
bars = ax.bar(range(len(algs)), sign, color=colors, edgecolor='white', linewidth=1.2)
for b,v in zip(bars,sign):
    label = f'{v/1000:.1f}ms' if v>10000 else f'{v:.0f}µs'
    ax.text(b.get_x()+b.get_width()/2, b.get_height()+max(sign)*0.02, label, ha='center', fontsize=8, fontweight='bold')
ax.set_xticks(range(len(algs))); ax.set_xticklabels(algs, fontsize=9)
ax.set_ylabel('Signing Latency (µs)', fontweight='bold')
ax.set_title('Post-Quantum Signature Performance — Gulf Migration Matrix', fontweight='bold', pad=20)
ax.set_yscale('log'); ax.set_ylim(50, 1_000_000)
ax.axhline(y=1000, color=GOLD, linestyle='--', linewidth=1.5, alpha=0.7, label='Real-Time Threshold (1ms)')
from matplotlib.patches import Patch
ax.legend(handles=[Patch(facecolor=RED,label='Classical'),Patch(facecolor=GREEN,label='PQ Lattice'),Patch(facecolor=TEAL,label='PQ Compact'),Patch(facecolor=PURPLE,label='PQ Hash')], loc='upper left')
ax.grid(axis='y', alpha=0.2)
plt.tight_layout(); fig.savefig(f'{OUT}/fig1_signing_speed.svg'); fig.savefig(f'{OUT}/fig1_signing_speed.png'); plt.close()

# ═══════════ FIG 2: Key Size Tax ═══════════
key_b = [91,32,1312,1952,2592,897,1793,32,64]
sig_b = [72,64,2420,3309,4627,655,1272,7856,29792]
fig, ax = plt.subplots(figsize=(14,7))
x = np.arange(len(algs)); w = 0.35
b1 = ax.bar(x-w/2, key_b, w, label='Public Key', color=DEEP, edgecolor='white')
b2 = ax.bar(x+w/2, sig_b, w, label='Signature', color=GOLD, edgecolor='white')
ax.set_xticks(x); ax.set_xticklabels(algs, fontsize=9)
ax.set_ylabel('Size (Bytes)', fontweight='bold')
ax.set_title('The Spatial Quantum Tax — Key & Signature Sizes', fontweight='bold', pad=20)
ax.set_yscale('log'); ax.legend(); ax.grid(axis='y', alpha=0.2)
plt.tight_layout(); fig.savefig(f'{OUT}/fig2_key_size_tax.svg'); fig.savefig(f'{OUT}/fig2_key_size_tax.png'); plt.close()

# ═══════════ FIG 3: KEM Comparison ═══════════
kem_algs = ['ML-KEM\n512','ML-KEM\n768','ML-KEM\n1024','McEliece\n348864','HQC\n128','BIKE\nL1','Frodo\n976']
kem_enc = [19.5, 28.6, 32.2, 78.7, 4964.0, 78.7, 1339.7]
kem_pk = [800, 1184, 1568, 261120, 2249, 1541, 15632]
kem_colors = [GREEN,GREEN,GREEN,ORANGE,ORANGE,ORANGE,TEAL]
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16,7))
bars = ax1.bar(range(len(kem_algs)), kem_enc, color=kem_colors, edgecolor='white')
ax1.set_xticks(range(len(kem_algs))); ax1.set_xticklabels(kem_algs, fontsize=8)
ax1.set_ylabel('Encapsulation (µs)', fontweight='bold'); ax1.set_title('KEM Speed', fontweight='bold')
ax1.set_yscale('log'); ax1.grid(axis='y', alpha=0.2)
bars2 = ax2.bar(range(len(kem_algs)), kem_pk, color=kem_colors, edgecolor='white')
ax2.set_xticks(range(len(kem_algs))); ax2.set_xticklabels(kem_algs, fontsize=8)
ax2.set_ylabel('Public Key (Bytes)', fontweight='bold'); ax2.set_title('KEM Key Size', fontweight='bold')
ax2.set_yscale('log'); ax2.grid(axis='y', alpha=0.2)
fig.suptitle('Key Encapsulation Mechanisms — FIPS 203 + NIST Round 4', fontweight='bold', fontsize=15)
plt.tight_layout(); fig.savefig(f'{OUT}/fig3_kem_comparison.svg'); fig.savefig(f'{OUT}/fig3_kem_comparison.png'); plt.close()

# ═══════════ FIG 4: Case Study — Latency Requirements ═══════
cases = ['Drone\nSwarms','Hajj\nNFC','SCADA','Border\nSurv.','Oil\nSensors','Grid\nMeters','CBDC','Diplo\nComms','Oil\nRig','Sov.\nCloud']
req_us = [1000, 2000, 5000, 10000, 50000, 50000, 100000, 500000, 500000, 10000]
pq_lat = [104, 39, 216, 216, 580, 216, 379900, 379900, 1123, 1123]
fig, ax = plt.subplots(figsize=(14,7))
x = np.arange(len(cases)); w = 0.35
ax.bar(x-w/2, req_us, w, label='Max Allowed Latency', color=RED, alpha=0.6, edgecolor='white')
ax.bar(x+w/2, pq_lat, w, label='Recommended PQ Latency', color=GREEN, alpha=0.8, edgecolor='white')
ax.set_xticks(x); ax.set_xticklabels(cases, fontsize=8)
ax.set_ylabel('Latency (µs)', fontweight='bold'); ax.set_yscale('log')
ax.set_title('Gulf Infrastructure: Latency Requirements vs PQ Performance', fontweight='bold', pad=20)
ax.legend(); ax.grid(axis='y', alpha=0.2)
plt.tight_layout(); fig.savefig(f'{OUT}/fig4_case_study_latency.svg'); fig.savefig(f'{OUT}/fig4_case_study_latency.png'); plt.close()

# ═══════════ FIG 5: Algorithm Selection Radar ═══════
fig, ax = plt.subplots(figsize=(10,10), subplot_kw=dict(polar=True))
cats = ['Speed','Compactness','Security\nMargin','Stateless','NIST\nMaturity','Side-Channel\nResistance']
N = len(cats); angles = [n/float(N)*2*np.pi for n in range(N)]; angles += angles[:1]
mldsa = [9,4,7,9,10,7]; mldsa += mldsa[:1]
falcon = [8,8,7,9,9,6]; falcon += falcon[:1]
sphincs = [1,10,10,10,10,9]; sphincs += sphincs[:1]
ax.fill(angles, mldsa, alpha=0.25, color=GREEN); ax.plot(angles, mldsa, color=GREEN, linewidth=2, label='ML-DSA-44')
ax.fill(angles, falcon, alpha=0.25, color=TEAL); ax.plot(angles, falcon, color=TEAL, linewidth=2, label='Falcon-512')
ax.fill(angles, sphincs, alpha=0.25, color=PURPLE); ax.plot(angles, sphincs, color=PURPLE, linewidth=2, label='SPHINCS+-256s')
ax.set_xticks(angles[:-1]); ax.set_xticklabels(cats, fontsize=10)
ax.set_ylim(0,10); ax.set_title('Gulf Algorithm Selection Radar', fontweight='bold', pad=25)
ax.legend(loc='upper right', bbox_to_anchor=(1.35,1.1)); ax.grid(True, alpha=0.3)
plt.tight_layout(); fig.savefig(f'{OUT}/fig5_algorithm_radar.svg'); fig.savefig(f'{OUT}/fig5_algorithm_radar.png'); plt.close()

# ═══════════ FIG 6: TLS Handshake ═══════
stages = ['KeyGen','KEM\nEncaps','KEM\nDecaps','Sign\nCert','Verify\nCert']
classical = [200, 0.8, 0.6, 2198, 487]
pq = [300, 19.5, 15.7, 104, 39]
fig, ax = plt.subplots(figsize=(12,6))
x = np.arange(len(stages)); w = 0.35
ax.bar(x-w/2, classical, w, label='Classical TLS (ECDSA+ECDH)', color=RED, alpha=0.8)
ax.bar(x+w/2, pq, w, label='PQ TLS (ML-DSA-44+ML-KEM-512)', color=GREEN, alpha=0.8)
for i,(c,p) in enumerate(zip(classical,pq)):
    ax.text(i-w/2, c+5, f'{c:.0f}µs', ha='center', fontsize=8, fontweight='bold')
    ax.text(i+w/2, p+5, f'{p:.0f}µs', ha='center', fontsize=8, fontweight='bold')
ax.set_xticks(x); ax.set_xticklabels(stages, fontsize=10)
ax.set_ylabel('Latency (µs)', fontweight='bold'); ax.legend(); ax.grid(axis='y', alpha=0.2)
ax.set_title('TLS 2.0 Handshake: Classical vs Post-Quantum', fontweight='bold', pad=20)
plt.tight_layout(); fig.savefig(f'{OUT}/fig6_tls_handshake.svg'); fig.savefig(f'{OUT}/fig6_tls_handshake.png'); plt.close()

# ═══════════ FIG 7: Migration Timeline ═══════
fig, ax = plt.subplots(figsize=(14,5))
timeline = ['2026\nQ3','2027\nQ1','2027\nQ3','2028\nQ1','2029\nQ1','2030\nQ1']
milestones = ['Hybrid\nDeploy','Sensor\nMigrate','SCADA\nUpgrade','CBDC\nRoot Keys','Govt TLS\nComplete','Full PQ\nNative']
y_pos = [1]*6
colors_tl = [GOLD, TEAL, GREEN, PURPLE, DEEP, RED]
ax.scatter(range(6), y_pos, s=400, c=colors_tl, edgecolors='white', linewidth=2, zorder=5)
for i,(t,m) in enumerate(zip(timeline, milestones)):
    ax.text(i, 1.15, m, ha='center', fontsize=9, fontweight='bold', color=colors_tl[i])
    ax.text(i, 0.82, t, ha='center', fontsize=8, color='#888')
ax.plot(range(6), y_pos, color=GOLD, linewidth=3, alpha=0.5)
ax.set_ylim(0.5, 1.5); ax.set_xlim(-0.5, 5.5); ax.axis('off')
ax.set_title('Gulf PQC Migration Timeline', fontweight='bold', fontsize=16, pad=20)
plt.tight_layout(); fig.savefig(f'{OUT}/fig7_migration_timeline.svg'); fig.savefig(f'{OUT}/fig7_migration_timeline.png'); plt.close()

# ═══════════ FIG 8: Hash Performance ═══════
hash_a = ['SHA-256','SHA-512','SHA3-256','SHA3-512']
h_1kb = [18.66, 13.09, 16.51, 18.61]
h_1mb = [3191.36, 2091.34, 3842.79, 6945.88]
fig, ax = plt.subplots(figsize=(10,6))
x = np.arange(len(hash_a)); w = 0.35
ax.bar(x-w/2, h_1kb, w, label='1KB Input', color=DEEP, edgecolor='white')
ax.bar(x+w/2, [h/100 for h in h_1mb], w, label='1MB Input (÷100)', color=GOLD, edgecolor='white')
ax.set_xticks(x); ax.set_xticklabels(hash_a, fontsize=11)
ax.set_ylabel('Latency (µs)', fontweight='bold'); ax.legend(); ax.grid(axis='y', alpha=0.2)
ax.set_title('Hash Function Performance — Symmetric PQC Baseline', fontweight='bold', pad=20)
plt.tight_layout(); fig.savefig(f'{OUT}/fig8_hash_performance.svg'); fig.savefig(f'{OUT}/fig8_hash_performance.png'); plt.close()

# ═══════════ FIG 9: Gulf Sector Recommendations ═══════════
sectors = ['Drones','Oil\nSensors','SCADA','Smart\nGrid','Border\nSurv.','CBDC','Diplomatic','Oil\nRig','Hajj','Sov.\nCloud']
recs = ['ML-DSA-44','Falcon-512','Hybrid','ML-DSA-65','ML-KEM+\nML-DSA','SPHINCS+\n256s','SPHINCS+\nAES','Falcon\n1024','ML-DSA-44\nBatch','ML-KEM+\nFalcon']
colors_rec = [GREEN,TEAL,GOLD,GREEN,TEAL,PURPLE,PURPLE,TEAL,GREEN,DEEP]
fig, ax = plt.subplots(figsize=(16,6))
ax.barh(range(len(sectors)), [1]*len(sectors), color=colors_rec, edgecolor='white', height=0.7)
for i,(s,r) in enumerate(zip(sectors,recs)):
    ax.text(0.5, i, f'{s.strip():15s} → {r}', ha='center', va='center', fontsize=9, fontweight='bold', color='white')
ax.axis('off')
ax.set_title('Gulf Infrastructure — Per-Sector PQC Recommendations', fontweight='bold', fontsize=15, pad=20)
plt.tight_layout(); fig.savefig(f'{OUT}/fig9_sector_recommendations.svg'); fig.savefig(f'{OUT}/fig9_sector_recommendations.png'); plt.close()

# ═══════════ FIG 10: Tax Factor Summary ═══════
metrics = ['Signing\nSpeed','Verification\nSpeed','Key\nSize','Signature\nSize','KEM\nSpeed','Hash\nSpeed']
classical_score = [100, 100, 100, 100, 100, 100]
pq_score = [2100, 400, 7, 3, 80, 95]
fig, ax = plt.subplots(figsize=(12,7))
x = np.arange(len(metrics)); w = 0.35
ax.bar(x-w/2, classical_score, w, label='Classical (ECDSA+ECDH)', color=RED, alpha=0.8)
ax.bar(x+w/2, pq_score, w, label='Post-Quantum (ML-DSA+ML-KEM)', color=GREEN, alpha=0.8)
ax.set_xticks(x); ax.set_xticklabels(metrics, fontsize=10)
ax.set_ylabel('Relative Score (higher = better)', fontweight='bold')
ax.set_title('The Quantum Tax — Classical vs PQ Across All Metrics', fontweight='bold', pad=20)
ax.legend(); ax.grid(axis='y', alpha=0.2)
for i,(c,p) in enumerate(zip(classical_score,pq_score)):
    delta = p-c
    ax.annotate(f'{delta:+d}%', (i, max(c,p)+5), ha='center', fontsize=9, fontweight='bold', color=GREEN if delta>0 else RED)
plt.tight_layout(); fig.savefig(f'{OUT}/fig10_quantum_tax_summary.svg'); fig.savefig(f'{OUT}/fig10_quantum_tax_summary.png'); plt.close()

print("✅ ALL 10 FIGURES GENERATED")
print(f"   {OUT}/fig1_signing_speed.*")
print(f"   {OUT}/fig2_key_size_tax.*")
print(f"   {OUT}/fig3_kem_comparison.*")
print(f"   {OUT}/fig4_case_study_latency.*")
print(f"   {OUT}/fig5_algorithm_radar.*")
print(f"   {OUT}/fig6_tls_handshake.*")
print(f"   {OUT}/fig7_migration_timeline.*")
print(f"   {OUT}/fig8_hash_performance.*")
print(f"   {OUT}/fig9_sector_recommendations.*")
print(f"   {OUT}/fig10_quantum_tax_summary.*")
