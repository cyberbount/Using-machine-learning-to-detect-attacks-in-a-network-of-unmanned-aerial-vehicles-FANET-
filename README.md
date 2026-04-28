# DoS Attack Detection in FANET Using Machine Learning

A university student research project that combines **NS-3 network simulation**
and **machine learning** to detect Denial-of-Service (DoS) attacks in Flying
Ad-hoc Networks (FANETs).

---

## Project Structure

```
.
├── ns3/                         # NS-3 C++ simulation scripts
│   ├── fanet-dos.cc             # Main FANET + DoS simulation
│   ├── topology.cc              # Static topology explorer
│   └── attack_scenario.cc      # Three-phase attack scenario
│
├── dataset/
│   ├── raw-pcap/                # PCAP files (output of NS-3, not committed)
│   ├── csv/                     # Extracted feature CSVs
│   └── scripts/
│       └── extract_features.py # PCAP → CSV feature extraction (Scapy)
│
├── ml/
│   ├── preprocessing.py         # Data cleaning, encoding, train/test split
│   ├── train_random_forest.py   # Random Forest training + cross-validation
│   ├── evaluate.py              # Metrics, confusion matrix, ROC curve
│   └── models/                  # Saved model artifacts (*.pkl, *.npy)
│
├── docs/
│   ├── proposal.md              # Research proposal
│   ├── final_report.md          # Final report template
│   └── presentation.md          # Slide-deck outline
│
├── results/
│   ├── graphs/                  # Feature importance, ROC curve PNGs
│   ├── confusion_matrix/        # Confusion matrix PNGs
│   └── experiment_logs/         # Classification report TXTs
│
├── .github/
│   └── ISSUE_TEMPLATE/          # Bug, feature, experiment, docs templates
│
├── requirements.txt             # Python dependencies
└── README.md
```

---

## Prerequisites

| Tool | Version | Notes |
|------|---------|-------|
| NS-3 | ≥ 3.38 | https://www.nsnam.org/wiki/Installation |
| Python | ≥ 3.9 | |
| pip | ≥ 23 | |
| CMake | ≥ 3.16 | Required by NS-3 build |
| gcc / g++ | ≥ 9 | C++17 support required |

---

## Quick Start

### 1. Clone the repository

```bash
git clone <repo-url>
cd Using-machine-learning-to-detect-attacks-in-a-network-of-unmanned-aerial-vehicles-FANET-
```

### 2. Install Python dependencies

```bash
pip install -r requirements.txt
```

### 3. Install and build NS-3

```bash
# Download NS-3 (example: version 3.42)
wget https://www.nsnam.org/releases/ns-allinone-3.42.tar.bz2
tar xjf ns-allinone-3.42.tar.bz2
cd ns-allinone-3.42/ns-3.42

# Build
./ns3 configure --enable-examples --enable-tests
./ns3 build
```

### 4. Run the FANET DoS simulation

```bash
# Copy simulation script to scratch directory
cp <repo>/ns3/fanet-dos.cc scratch/

# Run with default parameters (10 nodes, 60 s, attack enabled)
./ns3 run "scratch/fanet-dos"

# Custom parameters
./ns3 run "scratch/fanet-dos --nNodes=15 --simTime=120 --attack=true"
```

The simulation writes `fanet-dos-*.pcap` files in the NS-3 run directory.
Move them to `dataset/raw-pcap/` before the next step.

```bash
mv fanet-dos-*.pcap <repo>/dataset/raw-pcap/
```

### 5. Extract features from PCAP files

```bash
python dataset/scripts/extract_features.py \
    --pcap_dir  dataset/raw-pcap \
    --output    dataset/csv/features.csv \
    --window    1.0 \
    --attack_start 30.0 \
    --attack_stop  60.0
```

### 6. Preprocess the dataset

```bash
python ml/preprocessing.py \
    --csv       dataset/csv/features.csv \
    --test_size 0.2 \
    --seed      42
```

Add `--smote` to apply SMOTE oversampling if the dataset is imbalanced.

### 7. Train the Random Forest model

```bash
python ml/train_random_forest.py \
    --n_estimators 200 \
    --cv_folds     5
```

### 8. Evaluate model performance

```bash
python ml/evaluate.py
```

Results are written to:
- `results/experiment_logs/classification_report.txt`
- `results/confusion_matrix/confusion_matrix.png`
- `results/graphs/roc_curve.png`
- `results/graphs/feature_importance.png`

---

## Running the Alternative Scenario Scripts

```bash
# Static topology explorer
cp ns3/topology.cc scratch/
./ns3 run "scratch/topology --nNodes=6 --simTime=20"

# Three-phase attack scenario
cp ns3/attack_scenario.cc scratch/
./ns3 run "scratch/attack_scenario --nNodes=12 --simTime=60"
```

---

## Evaluation Metrics

| Metric | Definition |
|--------|-----------|
| Accuracy | (TP+TN) / total |
| Precision | TP / (TP+FP) |
| Recall (TPR) | TP / (TP+FN) |
| F1-Score | 2 × Precision × Recall / (Precision + Recall) |
| FPR | FP / (FP+TN) |
| AUC | Area under ROC curve |

---

## Team Collaboration

Use the GitHub Issue Templates in `.github/ISSUE_TEMPLATE/` to track:
- 🐛 Bug reports
- ✨ Feature requests
- 🔬 Experiment tasks
- 📚 Documentation updates

---

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/my-feature`)
3. Commit your changes (`git commit -m "Add my feature"`)
4. Push to the branch (`git push origin feature/my-feature`)
5. Open a Pull Request

---

## References

- Bekmezci, I. et al. (2013). *Flying ad-hoc networks (FANETs): A survey.*
  Ad Hoc Networks, 11(3), 1254–1270.
- Breiman, L. (2001). *Random Forests.* Machine Learning, 45(1), 5–32.
- NS-3 Consortium. *ns-3 Network Simulator.* https://www.nsnam.org/

---

## License

This project is released for academic use. See individual files for details.