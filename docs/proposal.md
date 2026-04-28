# Research Proposal
# DoS Attack Detection in FANET Using Machine Learning

**University Research Project**  
**Team:** [Add team member names]  
**Supervisor:** [Add supervisor name]  
**Date:** [Add date]

---

## 1. Introduction

Flying Ad-hoc Networks (FANETs) are self-organised wireless networks formed by
Unmanned Aerial Vehicles (UAVs). They are increasingly used in disaster relief,
precision agriculture, military surveillance, and smart-city monitoring. Because
UAVs communicate directly with each other (without fixed infrastructure), they
are highly susceptible to Denial-of-Service (DoS) attacks that can degrade or
completely disrupt the network.

Traditional intrusion detection approaches designed for wired or cellular
networks are not directly applicable to FANETs due to the high mobility of
nodes, dynamic topology changes, and constrained on-board computing resources.
Machine learning (ML) offers a data-driven alternative that can adapt to the
complex, time-varying traffic patterns of a FANET.

---

## 2. Problem Statement

A malicious UAV (or a ground-based attacker) can flood the network with high-
rate spoofed packets, exhausting the bandwidth and processing capacity of
legitimate nodes. Detecting such attacks in real time — with low false positives
— is a critical open problem in FANET security.

---

## 3. Research Objectives

1. Simulate a realistic FANET environment using NS-3 (Network Simulator 3).
2. Generate labelled datasets containing both normal and DoS attack traffic.
3. Extract meaningful network-level features from captured packet traces (PCAP).
4. Train and evaluate supervised ML models — with a focus on **Random Forest**.
5. Benchmark detection performance: accuracy, precision, recall, F1-score, and
   false positive rate (FPR).
6. Analyse feature importance to identify the most informative traffic features.

---

## 4. Methodology

### 4.1 Network Simulation (NS-3)
- **Topology:** 10–15 UAV nodes, random-waypoint mobility model, 1 km × 1 km
  operational area, altitude 50–150 m.
- **MAC/PHY:** IEEE 802.11a ad-hoc mode (5.9 GHz band).
- **Routing:** AODV (Ad-hoc On-demand Distance Vector).
- **Normal traffic:** Constant-bit-rate UDP flows between random node pairs.
- **Attack traffic:** UDP flood from one compromised node at ≥ 10× the normal
  data rate.

### 4.2 Dataset Generation
- Export PCAP files per-node interface.
- Extract 14 time-windowed features (see `dataset/scripts/extract_features.py`).
- Label each window: `0 = normal`, `1 = DoS attack`.
- Target: ≥ 5 000 labelled windows per class.

### 4.3 Machine Learning Pipeline
| Step | Tool |
|------|------|
| Preprocessing | `sklearn.preprocessing`, optional SMOTE |
| Model | `sklearn.ensemble.RandomForestClassifier` |
| Cross-validation | 5-fold stratified |
| Evaluation | Accuracy, Precision, Recall, F1, FPR, ROC-AUC |

---

## 5. Expected Contributions

- Open-source NS-3 simulation scripts for reproducible FANET DoS experiments.
- A publicly available labelled dataset for FANET intrusion detection research.
- Quantitative comparison of ML models for lightweight on-board DoS detection.

---

## 6. Timeline

| Week | Activity |
|------|----------|
| 1–2  | Literature review; NS-3 environment setup |
| 3–4  | Implement simulation scripts; run baseline scenarios |
| 5–6  | Feature extraction; dataset cleaning |
| 7–8  | Model training and hyperparameter tuning |
| 9    | Performance evaluation and analysis |
| 10   | Report writing and presentation preparation |

---

## 7. References

1. Bekmezci, I., Sahingoz, O. K., & Temel, Ş. (2013). Flying ad-hoc networks
   (FANETs): A survey. *Ad Hoc Networks*, 11(3), 1254–1270.
2. Basan, E., et al. (2019). Abnormal behavior detection for FANET security.
   *Proc. Int. Conf. on Unmanned Aircraft Systems (ICUAS)*.
3. Breiman, L. (2001). Random forests. *Machine Learning*, 45(1), 5–32.
4. NS-3 Consortium. (2024). *ns-3 Network Simulator*. https://www.nsnam.org/
