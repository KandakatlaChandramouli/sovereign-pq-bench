#!/usr/bin/env python3
"""
Sovereign Protocol - Evidence Visualization Engine
"Quantifying the Cost of Sovereignty"

Generates publication-ready vector graphs from benchmark CSV data.
"""

import csv
import json
import sys
import os
from pathlib import Path

try:
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    import numpy as np
except ImportError:
    print("ERROR: matplotlib and numpy required. Install: pip install matplotlib numpy")
    sys.exit(1)

plt.rcParams.update({
    'font.family': 'serif',
    'font.size': 11,
    'axes.titlesize': 14,
    'axes.labelsize': 12,
    'figure.dpi': 300,
    'savefig.dpi': 300,
    'savefig.bbox': 'tight',
    'savefig.pad_inches': 0.1,
})

COLORS = {
    'ECDSA-P256': '#2196F3',
    'ML-DSA-44': '#FF5722',
    'ED25519': '#4CAF50',
}

def load_csv(csv_path):
    results = []
    with open(csv_path, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            results.append(row)
    return results

def plot_keygen_comparison(results, output_dir):
    algorithms = [r['algorithm'] for r in results]
    mean_times = [float(r['keygen_mean']) / 1e3 for r in results]
    p99_times = [float(r['keygen_p99']) / 1e3 for r in results]

    x = np.arange(len(algorithms))
    width = 0.35

    fig, ax = plt.subplots(figsize=(8, 5))
    
    bars1 = ax.bar(x - width/2, mean_times, width, label='Mean', 
                   color=[COLORS.get(a, '#999') for a in algorithms], edgecolor='black', linewidth=0.5)
    bars2 = ax.bar(x + width/2, p99_times, width, label='P99',
                   color=[COLORS.get(a, '#999') for a in algorithms], alpha=0.5, edgecolor='black', linewidth=0.5)

    ax.set_ylabel('Latency (microseconds)')
    ax.set_title('Key Generation Performance: Classical vs Post-Quantum')
    ax.set_xticks(x)
    ax.set_xticklabels(algorithms)
    ax.legend()
    ax.grid(axis='y', alpha=0.3)

    for bar in bars1:
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + 5,
                f'{height:.1f}', ha='center', va='bottom', fontsize=8)

    plt.tight_layout()
    path = os.path.join(output_dir, 'fig_keygen_comparison.svg')
    plt.savefig(path, format='svg')
    plt.savefig(path.replace('.svg', '.png'), format='png')
    plt.savefig(path.replace('.svg', '.pdf'), format='pdf')
    plt.close()
    print(f"  -> {path}")

def plot_signing_comparison(results, output_dir):
    algorithms = [r['algorithm'] for r in results]
    sizes = ['1B', '1KB', '1MB']
    size_keys = ['sign_1b_mean', 'sign_1kb_mean', 'sign_1mb_mean']

    fig, ax = plt.subplots(figsize=(10, 6))

    x = np.arange(len(sizes))
    width = 0.35
    multiplier = 0

    for result in results:
        alg = result['algorithm']
        times = [float(result[k]) / 1e3 for k in size_keys]
        offset = width * multiplier
        bars = ax.bar(x + offset, times, width, label=alg,
                      color=COLORS.get(alg, '#999'), edgecolor='black', linewidth=0.5)
        
        for bar in bars:
            height = bar.get_height()
            ax.text(bar.get_x() + bar.get_width()/2., height + 5,
                    f'{height:.1f}', ha='center', va='bottom', fontsize=7)
        
        multiplier += 1

    ax.set_ylabel('Latency (microseconds)')
    ax.set_title('Signing Performance: The Quantum Tax by Message Size')
    ax.set_xticks(x + width * (len(results) - 1) / 2)
    ax.set_xticklabels(sizes)
    ax.legend(loc='upper left')
    ax.grid(axis='y', alpha=0.3)
    ax.set_yscale('log')

    plt.tight_layout()
    path = os.path.join(output_dir, 'fig_signing_comparison.svg')
    plt.savefig(path, format='svg')
    plt.savefig(path.replace('.svg', '.png'), format='png')
    plt.savefig(path.replace('.svg', '.pdf'), format='pdf')
    plt.close()
    print(f"  -> {path}")

def plot_verification_comparison(results, output_dir):
    algorithms = [r['algorithm'] for r in results]
    sizes = ['1B', '1KB', '1MB']
    size_keys = ['verify_1b_mean', 'verify_1kb_mean', 'verify_1mb_mean']

    fig, ax = plt.subplots(figsize=(10, 6))

    x = np.arange(len(sizes))
    width = 0.35
    multiplier = 0

    for result in results:
        alg = result['algorithm']
        times = [float(result[k]) / 1e3 for k in size_keys]
        offset = width * multiplier
        bars = ax.bar(x + offset, times, width, label=alg,
                      color=COLORS.get(alg, '#999'), edgecolor='black', linewidth=0.5)
        
        for bar in bars:
            height = bar.get_height()
            ax.text(bar.get_x() + bar.get_width()/2., height + 5,
                    f'{height:.1f}', ha='center', va='bottom', fontsize=7)
        
        multiplier += 1

    ax.set_ylabel('Latency (microseconds)')
    ax.set_title('Verification Performance: The Verification Tax')
    ax.set_xticks(x + width * (len(results) - 1) / 2)
    ax.set_xticklabels(sizes)
    ax.legend(loc='upper left')
    ax.grid(axis='y', alpha=0.3)

    plt.tight_layout()
    path = os.path.join(output_dir, 'fig_verification_comparison.svg')
    plt.savefig(path, format='svg')
    plt.savefig(path.replace('.svg', '.png'), format='png')
    plt.savefig(path.replace('.svg', '.pdf'), format='pdf')
    plt.close()
    print(f"  -> {path}")

def plot_key_size_comparison(results, output_dir):
    algorithms = [r['algorithm'] for r in results]
    pub_keys = [int(r['pub_key_bytes']) for r in results]
    sig_sizes = [int(r['sig_bytes']) for r in results]

    x = np.arange(len(algorithms))
    width = 0.35

    fig, ax = plt.subplots(figsize=(8, 5))
    
    bars1 = ax.bar(x - width/2, pub_keys, width, label='Public Key',
                   color='#2196F3', edgecolor='black', linewidth=0.5)
    bars2 = ax.bar(x + width/2, sig_sizes, width, label='Signature',
                   color='#FF5722', edgecolor='black', linewidth=0.5)

    ax.set_ylabel('Size (bytes)')
    ax.set_title('Key and Signature Sizes: The Storage Tax')
    ax.set_xticks(x)
    ax.set_xticklabels(algorithms)
    ax.legend()
    ax.grid(axis='y', alpha=0.3)
    ax.set_yscale('log')

    for bar in bars1:
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + 5,
                str(int(height)), ha='center', va='bottom', fontsize=8)

    for bar in bars2:
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + 5,
                str(int(height)), ha='center', va='bottom', fontsize=8)

    plt.tight_layout()
    path = os.path.join(output_dir, 'fig_key_size_comparison.svg')
    plt.savefig(path, format='svg')
    plt.savefig(path.replace('.svg', '.png'), format='png')
    plt.savefig(path.replace('.svg', '.pdf'), format='pdf')
    plt.close()
    print(f"  -> {path}")

def plot_quantum_tax_radar(results, output_dir):
    if len(results) < 2:
        return

    ecdsa = results[0]
    mldsa = results[1]

    categories = ['KeyGen\nSpeed', 'Sign\nSpeed', 'Verify\nSpeed',
                  'Pub Key\nCompactness', 'Sig\nCompactness', 'Throughput']
    
    ecdsa_vals = [
        100.0,
        100.0,
        100.0,
        100.0,
        100.0,
        100.0,
    ]
    
    mldsa_vals = [
        100.0 * float(ecdsa['keygen_mean']) / max(float(mldsa['keygen_mean']), 1.0),
        100.0 * float(ecdsa['sign_1kb_mean']) / max(float(mldsa['sign_1kb_mean']), 1.0),
        100.0 * float(ecdsa['verify_1kb_mean']) / max(float(mldsa['verify_1kb_mean']), 1.0),
        100.0 * float(ecdsa['pub_key_bytes']) / max(float(mldsa['pub_key_bytes']), 1.0),
        100.0 * float(ecdsa['sig_bytes']) / max(float(mldsa['sig_bytes']), 1.0),
        100.0 * float(ecdsa['sign_1kb_mean']) / max(float(mldsa['sign_1kb_mean']), 1.0),
    ]

    num_vars = len(categories)
    angles = np.linspace(0, 2 * np.pi, num_vars, endpoint=False).tolist()
    angles += angles[:1]

    ecdsa_vals += ecdsa_vals[:1]
    mldsa_vals += mldsa_vals[:1]

    fig, ax = plt.subplots(figsize=(8, 8), subplot_kw=dict(polar=True))
    
    ax.fill(angles, ecdsa_vals, color=COLORS['ECDSA-P256'], alpha=0.25, label='ECDSA-P256 (Baseline)')
    ax.plot(angles, ecdsa_vals, color=COLORS['ECDSA-P256'], linewidth=2)
    
    ax.fill(angles, mldsa_vals, color=COLORS['ML-DSA-44'], alpha=0.25, label='ML-DSA-44')
    ax.plot(angles, mldsa_vals, color=COLORS['ML-DSA-44'], linewidth=2)

    ax.set_xticks(angles[:-1])
    ax.set_xticklabels(categories)
    ax.set_ylim(0, 110)
    ax.set_title('The Quantum Tax: Relative Performance Radar\n(ECDSA-P256 = 100%)', pad=20)
    ax.legend(loc='upper right', bbox_to_anchor=(1.3, 1.1))
    ax.grid(True)

    plt.tight_layout()
    path = os.path.join(output_dir, 'fig_quantum_tax_radar.svg')
    plt.savefig(path, format='svg')
    plt.savefig(path.replace('.svg', '.png'), format='png')
    plt.savefig(path.replace('.svg', '.pdf'), format='pdf')
    plt.close()
    print(f"  -> {path}")

def main():
    csv_path = sys.argv[1] if len(sys.argv) > 1 else 'results/benchmark_results.csv'
    output_dir = sys.argv[2] if len(sys.argv) > 2 else 'results/figures'

    if not os.path.exists(csv_path):
        print(f"ERROR: CSV file not found: {csv_path}")
        print("Run the C++ benchmark first: ./build/sovereign_cli")
        sys.exit(1)

    os.makedirs(output_dir, exist_ok=True)

    print(f"\n{'='*60}")
    print(f"  SOVEREIGN PROTOCOL - Evidence Visualization")
    print(f"  Input:  {csv_path}")
    print(f"  Output: {output_dir}/")
    print(f"{'='*60}\n")

    results = load_csv(csv_path)
    print(f"Loaded {len(results)} algorithm results\n")

    print("Generating figures...")
    plot_keygen_comparison(results, output_dir)
    plot_signing_comparison(results, output_dir)
    plot_verification_comparison(results, output_dir)
    plot_key_size_comparison(results, output_dir)
    plot_quantum_tax_radar(results, output_dir)

    print(f"\nDone. {5} figures generated in {output_dir}/")
    print("Files: SVG, PNG, PDF formats for each figure\n")

if __name__ == '__main__':
    main()
