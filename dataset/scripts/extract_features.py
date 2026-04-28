#!/usr/bin/env python3
"""
extract_features.py
===================
Extract network-level features from PCAP files produced by NS-3 and write
a labelled CSV dataset suitable for machine-learning experiments.

Features extracted per time-window (default 1 second):
  - src_ip, dst_ip, protocol
  - pkt_count        : number of packets in window
  - byte_count       : total bytes in window
  - avg_pkt_size     : average packet size (bytes)
  - pkt_rate         : packets per second
  - byte_rate        : bytes per second
  - inter_arrival_mean : mean inter-arrival time (seconds)
  - inter_arrival_std  : std of inter-arrival time
  - unique_src_ports : number of unique source ports
  - unique_dst_ports : number of unique destination ports
  - label            : 0 = normal, 1 = DoS attack

Usage:
  python extract_features.py --pcap_dir ../raw-pcap \
                              --output   ../csv/features.csv \
                              --window   1.0 \
                              --attack_start 30.0 \
                              --attack_stop  60.0

Dependencies:
  pip install scapy pandas numpy tqdm
"""

import argparse
import csv
import glob
import os
import sys
from collections import defaultdict

import numpy as np
import pandas as pd
from scapy.all import PcapReader, IP, TCP, UDP, ICMP
from tqdm import tqdm


# ---------------------------------------------------------------------------
# Argument parser
# ---------------------------------------------------------------------------
def parse_args():
    parser = argparse.ArgumentParser(
        description="Extract ML features from FANET PCAP files"
    )
    parser.add_argument(
        "--pcap_dir",
        default=os.path.join(os.path.dirname(__file__), "..", "raw-pcap"),
        help="Directory containing .pcap files",
    )
    parser.add_argument(
        "--output",
        default=os.path.join(os.path.dirname(__file__), "..", "csv", "features.csv"),
        help="Output CSV file path",
    )
    parser.add_argument(
        "--window",
        type=float,
        default=1.0,
        help="Time-window size in seconds (default: 1.0)",
    )
    parser.add_argument(
        "--attack_start",
        type=float,
        default=30.0,
        help="Simulation time (s) when attack starts",
    )
    parser.add_argument(
        "--attack_stop",
        type=float,
        default=60.0,
        help="Simulation time (s) when attack stops",
    )
    return parser.parse_args()


# ---------------------------------------------------------------------------
# Feature extraction
# ---------------------------------------------------------------------------
def extract_features_from_pcap(pcap_path: str, window: float,
                                attack_start: float, attack_stop: float):
    """
    Returns a list of dicts, one per (flow, window) combination.
    A flow is identified by (src_ip, dst_ip, protocol).
    """
    # Bin packets into windows
    # Structure: bins[bin_idx][(src_ip, dst_ip, proto)] = list of (time, size)
    bins = defaultdict(lambda: defaultdict(list))

    with PcapReader(pcap_path) as reader:
        for pkt in reader:
            if IP not in pkt:
                continue

            ts   = float(pkt.time)
            size = len(pkt)
            src  = pkt[IP].src
            dst  = pkt[IP].dst

            if TCP in pkt:
                proto      = "TCP"
                src_port   = pkt[TCP].sport
                dst_port   = pkt[TCP].dport
            elif UDP in pkt:
                proto      = "UDP"
                src_port   = pkt[UDP].sport
                dst_port   = pkt[UDP].dport
            elif ICMP in pkt:
                proto      = "ICMP"
                src_port   = 0
                dst_port   = 0
            else:
                proto      = "OTHER"
                src_port   = 0
                dst_port   = 0

            bin_idx = int(ts / window)
            bins[bin_idx][(src, dst, proto)].append(
                (ts, size, src_port, dst_port)
            )

    rows = []
    for bin_idx in sorted(bins.keys()):
        t_start = bin_idx * window
        t_stop  = t_start + window
        label   = 1 if (t_start >= attack_start and t_start < attack_stop) else 0

        for (src, dst, proto), packets in bins[bin_idx].items():
            times      = np.array([p[0] for p in packets])
            sizes      = np.array([p[1] for p in packets])
            src_ports  = set(p[2] for p in packets)
            dst_ports  = set(p[3] for p in packets)

            pkt_count  = len(packets)
            byte_count = int(sizes.sum())

            if pkt_count > 1:
                iats = np.diff(np.sort(times))
                ia_mean = float(iats.mean())
                ia_std  = float(iats.std())
            else:
                ia_mean = 0.0
                ia_std  = 0.0

            rows.append({
                "window_start":        round(t_start, 3),
                "window_stop":         round(t_stop,  3),
                "src_ip":              src,
                "dst_ip":              dst,
                "protocol":            proto,
                "pkt_count":           pkt_count,
                "byte_count":          byte_count,
                "avg_pkt_size":        round(float(sizes.mean()), 3),
                "pkt_rate":            round(pkt_count / window, 3),
                "byte_rate":           round(byte_count / window, 3),
                "inter_arrival_mean":  round(ia_mean, 6),
                "inter_arrival_std":   round(ia_std,  6),
                "unique_src_ports":    len(src_ports),
                "unique_dst_ports":    len(dst_ports),
                "label":               label,
            })

    return rows


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
def main():
    args = parse_args()

    pcap_files = sorted(glob.glob(os.path.join(args.pcap_dir, "*.pcap")))
    if not pcap_files:
        print(f"[ERROR] No .pcap files found in {args.pcap_dir}", file=sys.stderr)
        sys.exit(1)

    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)

    all_rows = []
    for pcap_path in tqdm(pcap_files, desc="Processing PCAP files"):
        rows = extract_features_from_pcap(
            pcap_path, args.window, args.attack_start, args.attack_stop
        )
        all_rows.extend(rows)

    if not all_rows:
        print("[WARNING] No IP packets found in any PCAP file.", file=sys.stderr)
        sys.exit(0)

    df = pd.DataFrame(all_rows)
    df.to_csv(args.output, index=False)

    normal_count = int((df["label"] == 0).sum())
    attack_count = int((df["label"] == 1).sum())
    print(f"\n[OK] Wrote {len(df)} rows → {args.output}")
    print(f"     Normal: {normal_count}  |  Attack: {attack_count}")


if __name__ == "__main__":
    main()
