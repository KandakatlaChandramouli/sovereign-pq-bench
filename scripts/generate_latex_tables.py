import pandas as pd, os

csv_path = 'results/processed/gulf_complete_matrix.csv'
if not os.path.exists(csv_path):
    print(f"CSV not found: {csv_path}")
    exit(1)

df = pd.read_csv(csv_path)
# Filter for signature algorithms
sig_df = df[df['category'] == 'Signature'].copy()
sig_df = sig_df[['algorithm', 'mean_us', 'key_bytes', 'sig_or_ct_bytes']]
sig_df.columns = ['Algorithm', 'Latency (µs)', 'Key Size (B)', 'Signature Size (B)']

latex_table = sig_df.to_latex(index=False, caption='Performance of Signature Algorithms.', label='tab:sig_perf')
os.makedirs('results/paper_tables', exist_ok=True)
with open('results/paper_tables/signature_performance.tex', 'w') as f:
    f.write(latex_table)
print('LaTeX table generated.')
