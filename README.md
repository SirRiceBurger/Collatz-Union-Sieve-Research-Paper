# Union Sieve for Accelerated Collatz Verification

This repository contains the research paper and reference implementation for an optimized residue sieve designed to accelerate the computational verification of the Collatz conjecture. 

By implementing a **Union Sieve** approach, this method strengthens the state-of-the-art one-bit pruning rule proposed by **David Barina (2025)**. Our approach captures "early descents" across multiple bit-widths while maintaining a constant-time $O(1)$ lookup.



## 🚀 Key Contributions
* **The Union Sieve:** A method to combine multiple residue certificates ($k_1, k_2, \dots, k_{max}$) into a single $2^{k_{max}}$ bitset.
* **65% Reduction in Fallback:** Effectively prunes significantly more candidates before they hit the expensive big-integer simulation phase.
* **1.3x Throughput Gain:** Demonstrated performance improvements across diverse sweep regimes (Anchor and Dense).
* **Zero Overhead:** The optimization is precomputed; the runtime cost remains identical to a standard single-width sieve.

## 📊 Performance Summary
Benchmarks conducted on an Intel Core i5-13500 ($k_{max} = 20$).

| Metric | Barina (2025) Baseline | Union Sieve ($K=\{8..20\}$) | Improvement |
| :--- | :--- | :--- | :--- |
| **Prune Rate** | 86.88% | **95.38%** | +8.50% |
| **Fallback Survivors** | 13.12% | **4.62%** | **~64.7% reduction** |
| **Relative Throughput**| 1.00x | **1.28x - 1.32x** | **~30% faster** |
