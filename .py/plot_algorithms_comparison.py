#!/usr/bin/env python3
"""
Сравнение итоговых результатов трёх алгоритмов оптимизации SQL‑запросов.

Скрипт считывает файл `summary.csv` из каталога `data/csv/` и строит два
графика:

  - `algorithms_score.png`   — сравнение комбинированного score для HC,
    Beam Search и SA
  - `algorithms_metrics.png` — сравнение отдельных метрик: performance,
    index_efficiency и complexity_score для каждого алгоритма

Файл `summary.csv` должен содержать как минимум следующие колонки:
    algorithm, performance, index_efficiency, complexity_score, score

Использование:

    python3 .py/plot_algorithms_comparison.py
"""

import os
from pathlib import Path

import pandas as pd
import matplotlib.pyplot as plt


ROOT = Path(__file__).resolve().parent.parent
CSV_DIR = ROOT / "data" / "csv"
PNG_DIR = ROOT / "data" / "png"


def plot_score(df: pd.DataFrame, out_path: Path) -> None:
    plt.figure(figsize=(6, 4))
    plt.bar(df["algorithm"], df["score"], color=["tab:blue", "tab:orange", "tab:green"])
    plt.xlabel("Алгоритм")
    plt.ylabel("Значение целевой функции (score)")
    plt.title("Сравнение алгоритмов по комбинированному score")
    plt.tight_layout()
    out_path.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(out_path, dpi=200)
    plt.close()
    print(f"[OK] Сохранён график: {out_path}")


def plot_metrics(df: pd.DataFrame, out_path: Path) -> None:
    plt.figure(figsize=(7, 5))
    x = range(len(df))
    width = 0.25
    plt.bar([i - width for i in x], df["performance"], width, label="performance")
    plt.bar(x, df["index_efficiency"], width, label="index_efficiency")
    plt.bar([i + width for i in x], df["complexity_score"], width, label="complexity_score")
    plt.xticks(list(x), df["algorithm"])
    plt.xlabel("Алгоритм")
    plt.ylabel("Значение метрик")
    plt.title("Сравнение метрик по алгоритмам")
    plt.legend()
    plt.tight_layout()
    out_path.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(out_path, dpi=200)
    plt.close()
    print(f"[OK] Сохранён график: {out_path}")


def main():
    csv_path = CSV_DIR / "summary.csv"
    if not csv_path.exists():
        print(f"[WARN] Файл {csv_path} не найден.")
        return
    df = pd.read_csv(csv_path)
    required = {"algorithm", "performance", "index_efficiency", "complexity_score", "score"}
    if not required.issubset(df.columns):
        print(f"[WARN] В {csv_path} нет требуемых колонок {required}.")
        return
    plot_score(df, PNG_DIR / "algorithms_score.png")
    plot_metrics(df, PNG_DIR / "algorithms_metrics.png")


if __name__ == "__main__":
    main()
