import matplotlib.pyplot as plt
import numpy as np
import os

# Hardcoded metrics from the latest live benchmark
algos = ['ECDSA','Ed25519','ML-DSA-44','ML-DSA-65','ML-DSA-87',
         'Falcon-512','Falcon-1024','SPHINCS+-128s','SPHINCS+-256s']
T_sign = np.array([1454,168,160,335,175,251,672,209258,318279])
S_sig  = np.array([71,64,2420,3309,4627,655,1272,7856,29792])

# Pareto frontier: minimize both latency and signature size
sorted_idx = np.argsort(T_sign)
pareto = []
pareto_min_lat = float('inf')
for i in sorted_idx:
    if S_sig[i] < pareto_min_lat:
        pareto.append(i)
        pareto_min_lat = S_sig[i]
    elif T_sign[i] < T_sign[pareto[-1]]:
        pareto.append(i)

fig, ax = plt.subplots(figsize=(10,6))
ax.scatter(T_sign, S_sig, alpha=0.7, label='All algorithms')
ax.scatter(T_sign[pareto], S_sig[pareto], color='red', s=100, edgecolors='black', linewidth=1.5, label='Pareto frontier')
for i in range(len(algos)):
    ax.annotate(algos[i], (T_sign[i], S_sig[i]), fontsize=8)
ax.set_xscale('log'); ax.set_yscale('log')
ax.set_xlabel('Signing latency (µs)'); ax.set_ylabel('Signature size (bytes)')
ax.set_title('Pareto Frontier: Latency vs. Signature Size')
ax.legend(); ax.grid(True, alpha=0.3)
plt.tight_layout()
os.makedirs('results/figures', exist_ok=True)
plt.savefig('results/figures/pareto_latency_vs_sigsize.png', dpi=300)
plt.savefig('results/figures/pareto_latency_vs_sigsize.svg')
print('Pareto chart saved.')
