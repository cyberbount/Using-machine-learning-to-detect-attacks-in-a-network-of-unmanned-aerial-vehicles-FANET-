# Final Report
# DoS Attack Detection in FANET Using Machine Learning

**University Research Project**  
**Team:** [Add team member names]  
**Supervisor:** [Add supervisor name]  
**Submission Date:** [Add date]

---

## Abstract

> *(Fill in after experiments are complete)*
>
> This paper presents a machine-learning-based intrusion detection system (IDS)
> for detecting Denial-of-Service (DoS) attacks in Flying Ad-hoc Networks
> (FANETs). We simulate a 10-node UAV network using NS-3 and generate a
> labelled dataset of 14 per-flow time-windowed features. A Random Forest
> classifier achieves **XX.X %** detection accuracy with a false positive rate
> of **X.XX %**, outperforming baseline Naïve Bayes and Decision Tree models.

---

## 1. Introduction

*(Expand from proposal introduction)*

---

## 2. Related Work

*(Summarise 5–10 related papers on FANET security and ML-based IDS)*

---

## 3. System Design

### 3.1 FANET Simulation

Describe NS-3 configuration, node count, mobility model, attack parameters.

### 3.2 Feature Extraction

Describe the 14 features (refer to `dataset/scripts/extract_features.py`):

| # | Feature | Description |
|---|---------|-------------|
| 1 | `pkt_count` | Packets per time window |
| 2 | `byte_count` | Total bytes per window |
| 3 | `avg_pkt_size` | Average packet size |
| 4 | `pkt_rate` | Packets / second |
| 5 | `byte_rate` | Bytes / second |
| 6 | `inter_arrival_mean` | Mean inter-arrival time |
| 7 | `inter_arrival_std` | Std of inter-arrival time |
| 8 | `unique_src_ports` | Distinct source ports |
| 9 | `unique_dst_ports` | Distinct destination ports |
| 10 | `src_ip` | Encoded source IP |
| 11 | `dst_ip` | Encoded destination IP |
| 12 | `protocol` | Encoded protocol (TCP/UDP/ICMP/OTHER) |
| 13 | `window_start` | Start time of the feature window (s) |
| 14 | `window_stop` | End time of the feature window (s) |

### 3.3 ML Pipeline

Describe training procedure, cross-validation, hyperparameters.

---

## 4. Experimental Results

### 4.1 Dataset Statistics

| Metric | Value |
|--------|-------|
| Total windows | |
| Normal | |
| Attack | |
| Train split | 80 % |
| Test split | 20 % |

### 4.2 Classification Performance

| Model | Accuracy | Precision | Recall | F1 | FPR |
|-------|----------|-----------|--------|-----|-----|
| Random Forest | | | | | |
| Decision Tree | | | | | |
| Naïve Bayes | | | | | |

### 4.3 Feature Importance

*(Insert feature importance bar chart from `results/graphs/feature_importance.png`)*

### 4.4 Confusion Matrix

*(Insert confusion matrix from `results/confusion_matrix/confusion_matrix.png`)*

### 4.5 ROC Curve

*(Insert ROC curve from `results/graphs/roc_curve.png`)*

---

## 5. Discussion

*(Interpret results, limitations, future work)*

---

## 6. Conclusion

*(Summary of findings)*

---

## References

*(Full reference list)*
