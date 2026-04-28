#!/usr/bin/env python3
"""
evaluate.py
===========
Evaluate a trained classifier against the held-out test set and generate
performance reports.

Metrics produced:
  - Accuracy, Precision, Recall, F1-score (macro & per-class)
  - False Positive Rate (FPR)
  - ROC curve + AUC
  - Confusion matrix (saved as PNG)
  - Full classification report (saved as TXT)

Outputs:
  results/confusion_matrix/confusion_matrix.png
  results/experiment_logs/classification_report.txt
  results/graphs/roc_curve.png

Usage:
  python evaluate.py [--model models/random_forest.pkl]
"""

import argparse
import os
import pickle

import numpy as np
import pandas as pd
from sklearn.metrics import (
    accuracy_score,
    classification_report,
    confusion_matrix,
    f1_score,
    precision_score,
    recall_score,
    roc_auc_score,
    roc_curve,
)

try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import seaborn as sns
    HAS_PLOT = True
except ImportError:
    HAS_PLOT = False


# ---------------------------------------------------------------------------
MODELS_DIR  = os.path.join(os.path.dirname(__file__), "models")
CM_DIR      = os.path.join(os.path.dirname(__file__), "..", "results",
                            "confusion_matrix")
LOG_DIR     = os.path.join(os.path.dirname(__file__), "..", "results",
                            "experiment_logs")
GRAPH_DIR   = os.path.join(os.path.dirname(__file__), "..", "results", "graphs")


def parse_args():
    parser = argparse.ArgumentParser(
        description="Evaluate FANET DoS detection model"
    )
    parser.add_argument(
        "--model",
        default=os.path.join(MODELS_DIR, "random_forest.pkl"),
        help="Path to trained model (.pkl)",
    )
    parser.add_argument(
        "--X_test",
        default=os.path.join(MODELS_DIR, "X_test.npy"),
    )
    parser.add_argument(
        "--y_test",
        default=os.path.join(MODELS_DIR, "y_test.npy"),
    )
    return parser.parse_args()


# ---------------------------------------------------------------------------
def compute_fpr(y_true, y_pred):
    """False Positive Rate = FP / (FP + TN)."""
    tn, fp, fn, tp = confusion_matrix(y_true, y_pred).ravel()
    return fp / (fp + tn) if (fp + tn) > 0 else 0.0


def plot_confusion_matrix(cm, output_path: str):
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    if not HAS_PLOT:
        return

    fig, ax = plt.subplots(figsize=(5, 4))
    sns.heatmap(
        cm, annot=True, fmt="d", cmap="Blues",
        xticklabels=["Normal", "Attack"],
        yticklabels=["Normal", "Attack"],
        ax=ax,
    )
    ax.set_xlabel("Predicted Label")
    ax.set_ylabel("True Label")
    ax.set_title("Confusion Matrix – FANET DoS Detection")
    plt.tight_layout()
    plt.savefig(output_path, dpi=150)
    plt.close()
    print(f"[OK] Confusion matrix saved to {output_path}")


def plot_roc(y_true, y_scores, output_path: str):
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    if not HAS_PLOT:
        return

    fpr, tpr, _ = roc_curve(y_true, y_scores)
    auc          = roc_auc_score(y_true, y_scores)

    fig, ax = plt.subplots(figsize=(6, 5))
    ax.plot(fpr, tpr, label=f"AUC = {auc:.4f}")
    ax.plot([0, 1], [0, 1], "k--", linewidth=0.8)
    ax.set_xlabel("False Positive Rate")
    ax.set_ylabel("True Positive Rate")
    ax.set_title("ROC Curve – FANET DoS Detection")
    ax.legend()
    plt.tight_layout()
    plt.savefig(output_path, dpi=150)
    plt.close()
    print(f"[OK] ROC curve saved to {output_path}")
    return auc


# ---------------------------------------------------------------------------
def main():
    args = parse_args()

    # Load model
    with open(args.model, "rb") as f:
        clf = pickle.load(f)
    print(f"[INFO] Loaded model: {args.model}")

    # Load test data
    X_test = np.load(args.X_test)
    y_test = np.load(args.y_test)
    print(f"[INFO] Test set: {len(y_test)} samples "
          f"(normal={int((y_test==0).sum())} "
          f"attack={int((y_test==1).sum())})")

    # Predictions
    y_pred = clf.predict(X_test)
    if hasattr(clf, "predict_proba"):
        y_scores = clf.predict_proba(X_test)[:, 1]
    else:
        y_scores = y_pred.astype(float)

    # ----------------------------------------------------------------
    # Metrics
    # ----------------------------------------------------------------
    acc       = accuracy_score(y_test, y_pred)
    precision = precision_score(y_test, y_pred, zero_division=0)
    recall    = recall_score(y_test, y_pred, zero_division=0)
    f1        = f1_score(y_test, y_pred, zero_division=0)
    fpr       = compute_fpr(y_test, y_pred)
    auc       = roc_auc_score(y_test, y_scores)

    cm        = confusion_matrix(y_test, y_pred)
    report    = classification_report(
        y_test, y_pred, target_names=["Normal", "Attack"]
    )

    # ----------------------------------------------------------------
    # Print summary
    # ----------------------------------------------------------------
    print("\n" + "="*55)
    print("  FANET DoS Detection – Evaluation Results")
    print("="*55)
    print(f"  Accuracy  : {acc:.4f}")
    print(f"  Precision : {precision:.4f}")
    print(f"  Recall    : {recall:.4f}")
    print(f"  F1-Score  : {f1:.4f}")
    print(f"  FPR       : {fpr:.4f}")
    print(f"  ROC-AUC   : {auc:.4f}")
    print("="*55)
    print("\nClassification Report:\n")
    print(report)
    print("Confusion Matrix:")
    print(cm)

    # ----------------------------------------------------------------
    # Save classification report
    # ----------------------------------------------------------------
    os.makedirs(LOG_DIR, exist_ok=True)
    log_path = os.path.join(LOG_DIR, "classification_report.txt")
    with open(log_path, "w") as f:
        f.write("FANET DoS Detection – Evaluation Results\n")
        f.write("="*55 + "\n")
        f.write(f"Accuracy  : {acc:.4f}\n")
        f.write(f"Precision : {precision:.4f}\n")
        f.write(f"Recall    : {recall:.4f}\n")
        f.write(f"F1-Score  : {f1:.4f}\n")
        f.write(f"FPR       : {fpr:.4f}\n")
        f.write(f"ROC-AUC   : {auc:.4f}\n")
        f.write("="*55 + "\n\n")
        f.write("Classification Report:\n")
        f.write(report)
        f.write("\nConfusion Matrix:\n")
        f.write(str(cm) + "\n")
    print(f"\n[OK] Classification report saved to {log_path}")

    # ----------------------------------------------------------------
    # Plots
    # ----------------------------------------------------------------
    plot_confusion_matrix(
        cm,
        os.path.join(CM_DIR, "confusion_matrix.png")
    )
    plot_roc(
        y_test, y_scores,
        os.path.join(GRAPH_DIR, "roc_curve.png")
    )


if __name__ == "__main__":
    main()
