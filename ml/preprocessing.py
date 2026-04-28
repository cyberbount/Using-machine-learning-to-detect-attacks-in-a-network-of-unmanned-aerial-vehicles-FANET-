#!/usr/bin/env python3
"""
preprocessing.py
================
Load the raw feature CSV, clean it, engineer additional features,
handle class imbalance (SMOTE), and split into train/test sets.

Outputs:
  ml/models/X_train.npy
  ml/models/X_test.npy
  ml/models/y_train.npy
  ml/models/y_test.npy
  ml/models/feature_names.txt
  ml/models/preprocessing_artifacts.pkl  (contains label encoders and scaler)

Usage:
  python preprocessing.py --csv ../../dataset/csv/features.csv
"""

import argparse
import os
import pickle

import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder, StandardScaler

# Optional: SMOTE for class balancing (requires imbalanced-learn)
try:
    from imblearn.over_sampling import SMOTE
    HAS_SMOTE = True
except ImportError:
    HAS_SMOTE = False


# ---------------------------------------------------------------------------
MODELS_DIR = os.path.join(os.path.dirname(__file__), "models")
DEFAULT_CSV = os.path.join(
    os.path.dirname(__file__), "..", "dataset", "csv", "features.csv"
)


def parse_args():
    parser = argparse.ArgumentParser(description="Preprocess FANET feature CSV")
    parser.add_argument("--csv",       default=DEFAULT_CSV,
                        help="Path to feature CSV")
    parser.add_argument("--test_size", type=float, default=0.2,
                        help="Fraction of data for test set (default: 0.2)")
    parser.add_argument("--smote",     action="store_true",
                        help="Apply SMOTE oversampling on training set")
    parser.add_argument("--seed",      type=int, default=42,
                        help="Random seed")
    return parser.parse_args()


# ---------------------------------------------------------------------------
def load_and_clean(csv_path: str) -> pd.DataFrame:
    df = pd.read_csv(csv_path)
    print(f"[INFO] Loaded {len(df)} rows, {df.shape[1]} columns")

    # Drop rows with any NaN
    before = len(df)
    df = df.dropna()
    if len(df) < before:
        print(f"[INFO] Dropped {before - len(df)} rows with NaN values")

    # Drop non-feature columns
    drop_cols = ["window_start", "window_stop"]
    df = df.drop(columns=[c for c in drop_cols if c in df.columns], errors="ignore")

    return df


def encode_categoricals(df: pd.DataFrame):
    """Label-encode string columns; return (df, encoders_dict)."""
    encoders = {}
    for col in ["src_ip", "dst_ip", "protocol"]:
        if col in df.columns:
            le = LabelEncoder()
            df[col] = le.fit_transform(df[col].astype(str))
            encoders[col] = le
    return df, encoders


def split_features_labels(df: pd.DataFrame):
    y = df["label"].values
    X = df.drop(columns=["label"]).values
    feature_names = list(df.drop(columns=["label"]).columns)
    return X, y, feature_names


# ---------------------------------------------------------------------------
def main():
    args = parse_args()
    os.makedirs(MODELS_DIR, exist_ok=True)

    df = load_and_clean(args.csv)
    df, encoders = encode_categoricals(df)
    X, y, feature_names = split_features_labels(df)

    # Scale features
    scaler = StandardScaler()
    X = scaler.fit_transform(X)

    # Train / test split (stratified)
    X_train, X_test, y_train, y_test = train_test_split(
        X, y,
        test_size=args.test_size,
        random_state=args.seed,
        stratify=y,
    )
    print(f"[INFO] Train: {len(X_train)} samples | Test: {len(X_test)} samples")
    print(f"[INFO] Train label distribution: "
          f"normal={int((y_train==0).sum())} attack={int((y_train==1).sum())}")

    # Optional SMOTE
    if args.smote:
        if HAS_SMOTE:
            sm = SMOTE(random_state=args.seed)
            X_train, y_train = sm.fit_resample(X_train, y_train)
            print(f"[INFO] After SMOTE: {len(X_train)} train samples "
                  f"(normal={int((y_train==0).sum())} "
                  f"attack={int((y_train==1).sum())})")
        else:
            print("[WARNING] imbalanced-learn not installed; skipping SMOTE")

    # Save arrays
    np.save(os.path.join(MODELS_DIR, "X_train.npy"), X_train)
    np.save(os.path.join(MODELS_DIR, "X_test.npy"),  X_test)
    np.save(os.path.join(MODELS_DIR, "y_train.npy"), y_train)
    np.save(os.path.join(MODELS_DIR, "y_test.npy"),  y_test)

    # Save feature names
    with open(os.path.join(MODELS_DIR, "feature_names.txt"), "w") as f:
        f.write("\n".join(feature_names))

    # Save encoders and scaler
    artifacts = {"label_encoders": encoders, "scaler": scaler}
    with open(os.path.join(MODELS_DIR, "preprocessing_artifacts.pkl"), "wb") as f:
        pickle.dump(artifacts, f)

    print(f"[OK] Preprocessing complete. Artifacts saved to {MODELS_DIR}/")


if __name__ == "__main__":
    main()
