// SPDX-License-Identifier: MIT
//
// Точка входа для лабораторной работы 22.
// Программа демонстрирует работу трёх алгоритмов оптимизации SQL-запросов:
// Hill Climbing, Beam Search и имитации отжига.

#include "query_opt.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace fs = std::filesystem;

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    const int NUM_TABLES = 4;

    std::mt19937 rng(
        static_cast<std::uint64_t>(
            std::chrono::high_resolution_clock::now()
                .time_since_epoch()
                .count()));

    // случайный стартовый план
    QueryPlan start = random_queryplan(rng, NUM_TABLES);
    QueryMetrics startM = evaluate_query(start);
    std::cout << "Стартовый план:  " << start
              << " -> метрики " << startM << "\n\n";

    // -------- 1) Hill Climbing --------
    std::cout << "==== Hill Climbing: поиск очевидных улучшений ====\n";
    QueryPlan bestHC = hill_climbing(start, rng);
    QueryMetrics mHC = evaluate_query(bestHC);
    std::cout << "Лучший план (Hill Climbing): " << bestHC << "\n";
    std::cout << "Метрики:                    " << mHC
              << "  (score=" << score_for_HC(mHC) << ")\n\n";

    // -------- 2) Beam Search --------
    std::cout << "==== Beam Search: перебор JOIN и индексов ====\n";
    QueryPlan bestBeam = beam_search(start, rng, 5);
    QueryMetrics mBeam = evaluate_query(bestBeam);
    std::cout << "Лучший план (Beam Search):   " << bestBeam << "\n";
    std::cout << "Метрики:                     " << mBeam
              << "  (combined score=" << score_for_beam(mBeam) << ")\n\n";

    // -------- 3) Имитация отжига --------
    std::cout << "==== Имитация отжига: поиск неочевидных перестановок ====\n";
    QueryPlan middle;
    middle.join_order.resize(NUM_TABLES);
    middle.use_index.resize(NUM_TABLES);
    for (int i = 0; i < NUM_TABLES; ++i) {
        middle.join_order[i] = i;
        middle.use_index[i]  = (i % 2 == 0);
    }

    QueryPlan bestSA = simulated_annealing(
        middle, rng,
        /*max_iterations=*/2000,
        /*T_start=*/1.5,
        /*T_end=*/1e-4,
        /*alpha=*/0.995);

    QueryMetrics mSA = evaluate_query(bestSA);
    std::cout << "Лучший план (SA):            " << bestSA << "\n";
    std::cout << "Метрики:                     " << mSA
              << "  (score=" << score_for_SA(mSA) << ")\n";

    // -------- summary.csv для Python --------
    fs::path csvDir = fs::path("data") / "csv";
    fs::create_directories(csvDir);
    fs::path summaryPath = csvDir / "summary.csv";

    std::ofstream out(summaryPath);
    if (!out) {
        std::cerr << "[WARN] Не удалось открыть " << summaryPath << " для записи\n";
        return 0;
    }

    out << "algorithm,performance,index_efficiency,complexity_score,score\n";
    out << std::fixed << std::setprecision(6);

    out << "HC,"
        << mHC.performance << ","
        << mHC.index_efficiency << ","
        << mHC.complexity_score << ","
        << score_for_HC(mHC) << "\n";

    out << "Beam,"
        << mBeam.performance << ","
        << mBeam.index_efficiency << ","
        << mBeam.complexity_score << ","
        << score_for_beam(mBeam) << "\n";

    out << "SA,"
        << mSA.performance << ","
        << mSA.index_efficiency << ","
        << mSA.complexity_score << ","
        << score_for_SA(mSA) << "\n";

    std::cout << "[INFO] Итоговые результаты сохранены в \""
              << summaryPath.string() << "\"\n";

    return 0;
}
