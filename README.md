# Using-machine-learning-to-detect-attacks-in-a-network-of-unmanned-aerial-vehicles-FANET-

## Overview
This project focuses on detecting Denial of Service (DoS) attacks in **FANET (Flying Ad-hoc Network)** using **Machine Learning** techniques.

The research simulates UAV network communication in **NS-3**, generates both normal and malicious traffic, captures packets in `.pcap` format, extracts features into `.csv` datasets, and trains machine learning models to classify attack traffic.

This project is part of a university scientific research project.

---

## Research Objectives

- Simulate UAV/FANET communication environment
- Generate normal network traffic
- Simulate DoS attack traffic
- Capture network packets
- Build dataset from network traffic
- Train machine learning models
- Detect abnormal traffic behavior
- Evaluate detection performance

---

## Technologies Used

- **NS-3**
- **C++**
- **Python**
- **Scikit-learn**
- **Pandas**
- **NumPy**
- **Wireshark **
- **Git/GitHub**

---

# Project Structure

```bash
.
├── ns3/
│   ├── fanet-dos.cc
│   └── scenarios/
│
├── dataset/
│   ├── raw-pcap/
│   ├── csv/
│   └── scripts/
│
├── ml/
│   ├── preprocess.py
│   ├── train_random_forest.py
│   ├── evaluate.py
│   └── models/
│
├── docs/
│   ├── proposal/
│   ├── final-report/
│   └── presentation/
│
├── results/
│   ├── graphs/
│   ├── confusion_matrix/
│   └── logs/
│
└── README.md
```

---


---

# Team Members

### Project Leader
Le Thi Minh Hue

### Simulation & Dataset
Vu Ngoc Thien

### Machine Learning
Tran Dang Cong Tam

### Scientific Advisor
Do Dinh Luc

---

# Future Improvements

- Detect DDoS attacks
- Add more attack types
- Deep learning models
- Real UAV deployment
- Real-time IDS system

---
