#!/usr/bin/env python3
"""
Визуализация процесса имитации отжига (SA) для оптимизации SQL‑запросов.

Скрипт строит два графика на основе CSV-файла `sa_history.csv`:

  - `sa_temperature_score.png` — температура и целевая функция по итерациям
  - `sa_temperature_score_smooth.png` — то же с сильным сглаживанием

CSV-файл должен находиться в каталоге `data/csv/` и содержать колонки:

  iter — номер итерации,
  T    — температура,
  score — значение целевой функции (чем больше, тем лучше),
  performance, index_efficiency, complexity_score — дополнительные метрики,
  accepted_worse — флаг, был ли принят ухудшающий шаг.

Использование:

    python3 .py/plot_sa_process.py
"""

import os
from pathlib import Path

import pandas as pd
import matplotlib.pyplot as plt


ROOT = Path(__file__).resolve().parent.parent
CSV_DIR = ROOT / "data" / "csv"
PNG_DIR = ROOT / "data" / "png"


def make_plot(df: pd.DataFrame, score_col: str, out_path: Path, description: str) -> None:
    """Рисует график температуры и score для SA.

    Параметр `score_col` определяет колонку со значениями score (обычно
    сглаженную).  `description` используется в заголовке.
    """
    if df.empty:
        print("[WARN] Нет данных для построения графика.")
        return
    fig, ax1 = plt.subplots(figsize=(10, 6))
    ax1.plot(df["iter"], df["T"], label="Температура T", color="tab:blue", linewidth=2)
    ax1.set_xlabel("Итерация")
    ax1.set_ylabel("Температура T")
    ax1.grid(True, linestyle="--", alpha=0.5)
    ax2 = ax1.twinx()
    ax2.plot(df["iter"], df[score_col], label="Целевая функция (score)", color="tab:red", linewidth=2)
    ax2.set_ylabel("Значение целевой функции (score)")
    fig.suptitle(f"Имитация отжига: температура и качество ({description})")
    lines, labels = [], []
    for ax in (ax1, ax2):
        line, = ax.get_lines()
        lines.append(line)
        labels.append(line.get_label())
    ax1.legend(lines, labels, loc="upper right")
    PNG_DIR.mkdir(parents=True, exist_ok=True)
    fig.tight_layout()
    fig.savefig(out_path, dpi=200)
    plt.close(fig)
    print(f"[OK] Сохранён график: {out_path}")


def main():
    csv_path = CSV_DIR / "sa_history.csv"
    if not csv_path.exists():
        print(f"[WARN] Файл {csv_path} не найден.")
        return
    df = pd.read_csv(csv_path)
    required = {"iter", "T", "score"}
    if not required.issubset(df.columns):
        print(f"[WARN] В {csv_path} нет требуемых колонок {required}.")
        return
    # Сглаживание: два уровня
    n = len(df)
    window_normal = 20 if n > 20 else max(3, n // 5 or 1)
    window_strong = max(50, n // 10) if n > 100 else max(10, n // 3 or 1)
    df["score_normal"] = df["score"].rolling(window=window_normal).mean()
    df["score_strong"] = df["score"].rolling(window=window_strong).mean()
    df_norm = df.dropna(subset=["score_normal"])
    df_strong = df.dropna(subset=["score_strong"])
    make_plot(df_norm, "score_normal", PNG_DIR / "sa_temperature_score.png", "умеренное сглаживание")
    make_plot(df_strong, "score_strong", PNG_DIR / "sa_temperature_score_smooth.png", "сильное сглаживание")


if __name__ == "__main__":
    main()
