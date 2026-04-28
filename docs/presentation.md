# Presentation Slides Outline
# DoS Attack Detection in FANET Using Machine Learning

> Use this outline as the basis for your slide deck (PowerPoint / Google Slides / Beamer).

---

## Slide 1 – Title

- **Title:** DoS Attack Detection in Flying Ad-hoc Networks (FANET)  
  Using Machine Learning
- Team members, supervisor, institution, date

---

## Slide 2 – Motivation

- UAVs are increasingly used in critical missions
- FANET: self-organised, no fixed infrastructure → easy target
- DoS attacks can ground an entire UAV swarm
- Need: lightweight, accurate on-board detection

---

## Slide 3 – Problem Statement

- Attacker floods network with spoofed packets
- Victim UAVs become unreachable
- Challenge: high mobility + constrained resources + low false alarms

---

## Slide 4 – Research Objectives

1. Simulate realistic FANET with NS-3
2. Generate labelled DoS dataset
3. Train & evaluate Random Forest IDS
4. Achieve high F1 with low FPR

---

## Slide 5 – System Architecture

```
NS-3 Simulation
      ↓
  PCAP files
      ↓
Feature Extraction (Python / Scapy)
      ↓
   CSV Dataset
      ↓
 ML Pipeline (Random Forest)
      ↓
  Evaluation & Results
```

---

## Slide 6 – NS-3 Simulation Setup

| Parameter | Value |
|-----------|-------|
| Nodes | 10–15 UAVs |
| Area | 1 km × 1 km |
| Mobility | Random Waypoint |
| MAC/PHY | IEEE 802.11a ad-hoc |
| Routing | AODV |
| Attack | UDP flood (one node) |

---

## Slide 7 – Feature Extraction

- 14 features extracted per 1-second time window per flow
- Key features: `pkt_rate`, `byte_rate`, `inter_arrival_mean`,
  `unique_src_ports`
- Label: 0 = Normal, 1 = DoS attack

---

## Slide 8 – Machine Learning Pipeline

- **Pre-processing:** label encoding, standard scaling, optional SMOTE
- **Model:** Random Forest (200 trees, balanced class weights)
- **Validation:** 5-fold stratified cross-validation
- **Metrics:** Accuracy, Precision, Recall, F1, FPR, AUC

---

## Slide 9 – Results (placeholder)

| Metric | Random Forest |
|--------|:---:|
| Accuracy | XX.X % |
| Precision | XX.X % |
| Recall | XX.X % |
| F1-Score | XX.X % |
| FPR | X.XX % |
| AUC | X.XX |

*(Replace with actual values after experiments)*

---

## Slide 10 – Confusion Matrix

*(Insert `results/confusion_matrix/confusion_matrix.png`)*

---

## Slide 11 – Feature Importance

*(Insert `results/graphs/feature_importance.png`)*

Most informative: `pkt_rate`, `byte_rate`, `inter_arrival_mean`

---

## Slide 12 – ROC Curve

*(Insert `results/graphs/roc_curve.png`)*

AUC = X.XX

---

## Slide 13 – Discussion

- Random Forest handles high-dimensional, noisy FANET data well
- Limitations: offline model; real-time deployment is future work
- Dataset size depends on simulation length & node count

---

## Slide 14 – Future Work

- Real-time IDS deployment on Raspberry Pi / NVIDIA Jetson
- Distributed federated learning across UAV nodes
- Extend to Sybil, wormhole, and black-hole attacks

---

## Slide 15 – Conclusion

- We built an end-to-end FANET DoS detection pipeline
- Random Forest achieves high accuracy with low FPR
- Open-source code and dataset released for reproducibility

---

## Slide 16 – Q & A

Thank you! Questions?
