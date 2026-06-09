# QERS paper notes (extracted)

Source PDF:
- `Quantum Encryption Resilience Score (QERS) for MQTT, HTTP, and HTTPS under Post-Quantum Cryptography in Computer, IoT, and IIoT Systems.pdf`
  (located at `/home/me/projects/MCP/RAG/data/pdfs/`)

## Metric mapping (symbols)
From the paper’s QERS formulation table:
- `L`: end-to-end latency
- `J`: packet jitter
- `Ploss`: packet loss ratio
- `C`: CPU utilization
- `E`: energy consumption
- `R`: RSSI (signal strength)
- `K`: cryptographic key size (bytes)
- `Co` / `O`: cryptographic overhead (memory/bandwidth/computation)
- `Pr`: proven cryptographic resistance level

## Normalization
The paper uses dataset-derived min–max scaling with a maximum score `MS = 100`:
- $X_{norm} = MS \cdot \dfrac{X - X_{min}}{X_{max} - X_{min}}$

## QERS evaluation layers
The paper defines three layers:

### 1) Basic QERS
- $QERS_{basic} = MS - (\alpha L_{norm} + \beta O_{norm} + \gamma P_{loss,norm})$

### 2) Tuned QERS
Extends Basic with system/environment factors (paper treats RSSI as a benefit term):
- $QERS_{tuned} = MS - (\alpha L_{norm} + \beta O_{norm} + \gamma P_{loss,norm} + \delta C_{norm} + \zeta E_{norm} + \eta K_{norm}) + \epsilon R_{norm}$

### 3) Fusion QERS
Separates performance penalty and security benefit:
- Performance penalty:
  - $P = \sum_i w_i C_{i,norm}$, with $C_i \in \{L, J, P_{loss}, E, C\}$ and $\sum_i w_i = 1$
- Security benefit:
  - $S = \sum_j w_j B_{j,norm}$, with $B_j \in \{K, R, Pr, Co\}$ and $\sum_j w_j = 1$
- Fusion:
  - $QERS_{fusion} = \alpha (MS - P) + \beta S$, with $\alpha + \beta = 1$

## Baseline weights (paper example)
The paper provides an example baseline weight configuration (close-range / near Wi‑Fi AP) with metric types:
- Cost metrics: `L`, `O`, `Ploss`, `C`, `E`, `K`
- Benefit metric: `R`
- Weights: `L=0.25`, `O=0.15`, `Ploss=0.15`, `C=0.15`, `E=0.10`, `K=0.10`, `R=0.10` (sum 1.00)
