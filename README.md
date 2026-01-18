# Union Sieve for Accelerated Collatz Verification

This repository contains the research paper and reference implementation for an optimised residue sieve designed to accelerate the computational verification of the Collatz conjecture. 

**Abstract**

[cite_start]We present a constant-time residue sieve that strengthens Barina's one-bit $2^{k_{max}}$ pruning rule for computational verification of Collatz convergence[cite: 340]. [cite_start]The baseline sieve marks residues $n_L \pmod{2^{k_{max}}}$ that guarantee a strict descent after $k_{max}$ iterations of the accelerated Collatz map, allowing the corresponding starting values to be pruned without explicit fallback simulation[cite: 341]. [cite_start]Our method replaces this baseline table with a strictly stronger precomputed bitset of the same width, preserving identical runtime cost (one modular index and one memory access) while increasing the density of certified residues[cite: 342]. [cite_start]In CPU experiments with $k_{max} = 20$, the strengthened sieve increases the prune rate from 86.88% to 95.38%, reduces fallback survivors by approximately 65%, and improves throughput by $1.28\times$–$1.32\times$ across dense and anchor sweep benchmarks[cite: 343].


## Key Contributions
* **The Union Sieve:** A method to combine multiple residue certificates ($k_1, k_2, \dots, k_{max}$) into a single $2^{k_{max}}$ bitset.
* **65% Reduction in Fallback:** Effectively prunes significantly more candidates before they hit the expensive big-integer simulation phase.
* **1.3x Throughput Gain:** Demonstrated performance improvements across diverse sweep regimes (Anchor and Dense).
* **Zero Overhead:** The optimization is precomputed; the runtime cost remains identical to a standard single-width sieve.

## Performance Summary
Benchmarks conducted on an Intel Core i5-13500 ($k_{max} = 20$).

| Metric | Barina (2025) Baseline | Union Sieve ($K=\{8..20\}$) | Improvement |
| :--- | :--- | :--- | :--- |
| **Prune Rate** | 86.88% | **95.38%** | +8.50% |
| **Fallback Survivors** | 13.12% | **4.62%** | **~64.7% reduction** |
| **Relative Throughput**| 1.00x | **1.28x - 1.32x** | **~30% faster** |
