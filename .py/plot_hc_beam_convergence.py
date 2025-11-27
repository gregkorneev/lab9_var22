#!/usr/bin/env python3
"""
Визуализация сходимости алгоритмов Hill Climbing и Beam Search для
оптимизации SQL‑запросов.

Ожидаемые входные файлы (каталог data/csv/):

  - `hc_history.csv`  — история Hill Climbing (iter, score, performance,
    index_efficiency, complexity_score)
  - `beam_history.csv` — история Beam Search (iter, score, performance,
    index_efficiency, complexity_score)

Результатом работы являются два PNG-файла в каталоге `data/png/`:

  - `hc_convergence.png`   — график изменения целевой функции HC
  - `beam_convergence.png` — график изменения целевой функции Beam Search

Если входные файлы отсутствуют, скрипт пропускает соответствующие графики.

Использование:

    python3 .py/plot_hc_beam_convergence.py

Скрипт автоматически создаёт каталоги `data/csv` и `data/png`, если их нет.
"""

import os
from pathlib import Path

import pandas as pd
import matplotlib.pyplot as plt


ROOT = Path(__file__).resolve().parent.parent
CSV_DIR = ROOT / "data" / "csv"
PNG_DIR = ROOT / "data" / "png"


def plot_convergence(csv_path: Path, title: str, out_path: Path) -> None:
    """Строит график сходимости алгоритма по данным csv.

    Ожидает, что файл содержит колонки 'iter' и 'score'.
    """
    if not csv_path.exists():
        print(f"[WARN] Файл {csv_path} не найден. Пропускаю.")
        return
    df = pd.read_csv(csv_path)
    if not {"iter", "score"}.issubset(df.columns):
        print(f"[WARN] В {csv_path} должны быть колонки 'iter' и 'score'. Пропускаю.")
        return
    plt.figure(figsize=(8, 5))
    plt.plot(df["iter"], df["score"], marker="o")
    plt.xlabel("Итерация")
    plt.ylabel("Значение целевой функции (score)")
    plt.title(title)
    plt.grid(True, linestyle="--", alpha=0.5)
    plt.tight_layout()
    out_path.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(out_path, dpi=200)
    plt.close()
    print(f"[OK] Сохранён график: {out_path}")


def main():
    plot_convergence(
        CSV_DIR / "hc_history.csv",
        "Hill Climbing: изменение целевой функции",
        PNG_DIR / "hc_convergence.png",
    )
    plot_convergence(
        CSV_DIR / "beam_history.csv",
        "Beam Search: изменение целевой функции",
        PNG_DIR / "beam_convergence.png",
    )


if __name__ == "__main__":
    main()
