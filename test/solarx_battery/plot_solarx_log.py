"""
plot_solarx_log.py
------------------
Author: Antonio A.
Date: 2025-10-08

Purpose:
    This script loads a SolarX battery controller CSV log file and visualizes
    voltage, current, and charge percentage over time.

Usage:
    python plot_solarx_log.py <csv_file> <mode>

Modes:
    separate  -> Displays three individual plots (one per metric)
    combined  -> Displays all metrics on a shared time axis with multiple y-axes
"""

import sys

import matplotlib.pyplot as plt
import pandas as pd


def plot_separate(df):
    # plt.ion()  # modo interactivo: muestra todas sin bloquear

    fig1 = plt.figure(figsize=(8, 4))
    plt.plot(df["timestamp"], df["voltage"], color="tab:blue")
    plt.title("Battery Voltage")
    plt.ylabel("Voltage (V)")
    plt.xlabel("Timestamp")
    plt.tight_layout()

    fig2 = plt.figure(figsize=(8, 4))
    plt.plot(df["timestamp"], df["current"], color="tab:orange")
    plt.title("Battery Current")
    plt.ylabel("Current (A)")
    plt.xlabel("Timestamp")
    plt.tight_layout()

    fig3 = plt.figure(figsize=(8, 4))
    plt.plot(df["timestamp"], df["percentage"], color="tab:green")
    plt.title("Battery Charge Percentage")
    plt.ylabel("Percentage (%)")
    plt.xlabel("Timestamp")
    plt.tight_layout()

    plt.show()


def plot_combined(df):
    fig, ax1 = plt.subplots(figsize=(10, 5))

    # Voltage axis
    ax1.set_xlabel("Timestamp")
    ax1.set_ylabel("Voltage (V)", color="tab:blue")
    ax1.plot(df["timestamp"], df["voltage"], color="tab:blue", label="Voltage (V)")
    ax1.tick_params(axis="y", labelcolor="tab:blue")

    # Percentage axis
    ax2 = ax1.twinx()
    ax2.set_ylabel("Percentage (%)", color="tab:green")
    ax2.plot(df["timestamp"], df["percentage"], color="tab:green", label="Percentage (%)", linestyle="--")
    ax2.tick_params(axis="y", labelcolor="tab:green")

    # Current axis (third)
    ax3 = ax1.twinx()
    ax3.spines["right"].set_position(("outward", 60))
    ax3.set_ylabel("Current (A)", color="tab:orange")
    ax3.plot(df["timestamp"], df["current"], color="tab:orange", label="Current (A)")
    ax3.tick_params(axis="y", labelcolor="tab:orange")

    fig.suptitle("SolarX Battery Controller Metrics")
    fig.tight_layout()
    fig.legend(loc="upper left")
    plt.show()


if len(sys.argv) < 3:
    print("Usage: plot_solarx_log.py <csv_file> <mode: separate|combined>")
    sys.exit(1)

file, mode = sys.argv[1], sys.argv[2]
df = pd.read_csv(file, parse_dates=["timestamp"])

if mode == "separate":
    plot_separate(df)
elif mode == "combined":
    plot_combined(df)
else:
    print("Invalid mode. Use 'separate' or 'combined'.")
