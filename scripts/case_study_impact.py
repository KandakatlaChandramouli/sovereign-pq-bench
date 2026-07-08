import matplotlib.pyplot as plt
import numpy as np

labels = ['TLS\nHandshake','VPN\nSetup','IoT\nFlash','Blockchain\nBlock','PKI\nCert','Edge\nMemory','Cloud\nPod','Satellite\nMTU','Smart\nBattery','CBDC\nLatency']
overhead = [18,12,45,8,400,95,12,100,-15,340]  # % overhead (negative = reduction, but here is penalty)
fig, ax = plt.subplots(figsize=(14,6))
colors = ['red' if x>50 else 'orange' if x>0 else 'green' for x in overhead]
ax.bar(range(len(labels)), [abs(x) for x in overhead], color=colors)
ax.set_xticks(range(len(labels)))
ax.set_xticklabels(labels, fontsize=8)
ax.set_ylabel('Overhead (%)')
ax.set_title('Quantified PQC Migration Overhead per Scenario')
for i, v in enumerate(overhead):
    ax.text(i, abs(v)+1, f'{v:+d}%', ha='center', fontweight='bold')
plt.tight_layout()
plt.savefig('results/figures/case_study_overhead.png', dpi=300)
print('Case study figure saved.')
