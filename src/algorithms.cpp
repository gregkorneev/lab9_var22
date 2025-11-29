// SPDX-License-Identifier: MIT
//
// Реализация алгоритмов оптимизации SQL-запросов (Hill Climbing, Beam Search,
// имитация отжига) для лабораторной работы 22.

#include "query_opt.h"
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace fs = std::filesystem;

// --------------------- Hill Climbing ---------------------- //

QueryPlan hill_climbing(const QueryPlan& start,
                        std::mt19937& rng,
                        int max_iterations,
                        int neighbors_per_step) {
    // Подготовка CSV для истории HC
    fs::path csvDir = fs::path("data") / "csv";
    fs::create_directories(csvDir);
    fs::path hcPath = csvDir / "hc_history.csv";
    std::ofstream hcOut(hcPath);
    if (!hcOut) {
        std::cerr << "[HC] Не удалось открыть " << hcPath << " для записи\n";
    } else {
        hcOut << "iter,score,performance,index_efficiency,complexity_score\n";
    }

    QueryPlan   current = start;
    QueryMetrics curM   = evaluate_query(current);
    double       curScore = score_for_HC(curM);

    // лог итерации 0
    if (hcOut) {
        hcOut << 0 << ","
              << curScore << ","
              << curM.performance << ","
              << curM.index_efficiency << ","
              << curM.complexity_score << "\n";
    }

    for (int iter = 1; iter <= max_iterations; ++iter) {
        QueryPlan   bestNeighbor = current;
        QueryMetrics bestM       = curM;
        double       bestScore   = curScore;

        auto neighbors = generate_neighbors(current, neighbors_per_step, rng);
        for (const auto& n : neighbors) {
            QueryMetrics m = evaluate_query(n);
            double       s = score_for_HC(m);
            if (s > bestScore) {
                bestScore   = s;
                bestNeighbor = n;
                bestM        = m;
            }
        }

        if (bestScore <= curScore) {
            std::cout << "[HC] остановка на итерации " << iter
                      << " — достигнут локальный максимум\n";
            break;
        }

        current  = bestNeighbor;
        curM     = bestM;
        curScore = bestScore;

        if (hcOut) {
            hcOut << iter << ","
                  << curScore << ","
                  << curM.performance << ","
                  << curM.index_efficiency << ","
                  << curM.complexity_score << "\n";
        }
    }

    return current;
}

// --------------------- Beam Search ---------------------- //

QueryPlan beam_search(const QueryPlan& start,
                      std::mt19937& rng,
                      int beam_width,
                      int depth,
                      int neighbors_per_state) {
    // Подготовка CSV для истории Beam Search
    fs::path csvDir = fs::path("data") / "csv";
    fs::create_directories(csvDir);
    fs::path beamPath = csvDir / "beam_history.csv";
    std::ofstream beamOut(beamPath);
    if (!beamOut) {
        std::cerr << "[Beam] Не удалось открыть " << beamPath << " для записи\n";
    } else {
        beamOut << "iter,score,performance,index_efficiency,complexity_score\n";
    }

    std::vector<QueryPlan> beam;
    beam.push_back(start);

    QueryPlan    globalBest    = start;
    QueryMetrics globalBestM   = evaluate_query(start);
    double       globalBestScore = score_for_beam(globalBestM);

    // итерация 0
    if (beamOut) {
        beamOut << 0 << ","
                << globalBestScore << ","
                << globalBestM.performance << ","
                << globalBestM.index_efficiency << ","
                << globalBestM.complexity_score << "\n";
    }

    for (int level = 1; level <= depth; ++level) {
        std::vector<std::pair<double, QueryPlan>> candidates;

        for (const auto& state : beam) {
            auto neigh = generate_neighbors(state, neighbors_per_state, rng);
            for (const auto& n : neigh) {
                QueryMetrics m = evaluate_query(n);
                double       s = score_for_beam(m);
                candidates.push_back({s, n});
            }
        }

        if (candidates.empty()) break;

        std::sort(candidates.begin(), candidates.end(),
                  [](const auto& a, const auto& b) {
                      return a.first > b.first;
                  });

        beam.clear();
        for (int i = 0; i < beam_width && i < (int)candidates.size(); ++i) {
            beam.push_back(candidates[i].second);
            if (candidates[i].first > globalBestScore) {
                globalBestScore = candidates[i].first;
                globalBest      = candidates[i].second;
                globalBestM     = evaluate_query(globalBest);
            }
        }

        if (beamOut) {
            beamOut << level << ","
                    << globalBestScore << ","
                    << globalBestM.performance << ","
                    << globalBestM.index_efficiency << ","
                    << globalBestM.complexity_score << "\n";
        }
    }

    return globalBest;
}

// --------------------- Имитация отжига ---------------------- //

QueryPlan simulated_annealing(const QueryPlan& start,
                              std::mt19937& rng,
                              int max_iterations,
                              double T_start,
                              double T_end,
                              double alpha) {
    // Подготовка CSV для истории SA
    fs::path csvDir = fs::path("data") / "csv";
    fs::create_directories(csvDir);
    fs::path saPath = csvDir / "sa_history.csv";
    std::ofstream saOut(saPath);
    if (!saOut) {
        std::cerr << "[SA] Не удалось открыть " << saPath << " для записи\n";
    } else {
        saOut << "iter,T,score,accepted_worse\n";
    }

    QueryPlan   current = start;
    QueryMetrics curM   = evaluate_query(current);
    double       curScore = score_for_SA(curM);

    QueryPlan best      = current;
    double    bestScore = curScore;

    double T = T_start;

    // итерация 0
    if (saOut) {
        saOut << 0 << ","
              << T << ","
              << curScore << ","
              << 0 << "\n";
    }

    for (int t = 1; t <= max_iterations && T > T_end; ++t) {
        QueryPlan   next    = local_neighbor(current, rng);
        QueryMetrics nextM  = evaluate_query(next);
        double       nextScore = score_for_SA(nextM);

        double dE = curScore - nextScore; // максимизируем score
        bool acceptedWorse = false;

        if (dE < 0) {
            current  = next;
            curM     = nextM;
            curScore = nextScore;
        } else {
            double prob = std::exp(-dE / T);
            std::uniform_real_distribution<double> u(0.0, 1.0);
            if (u(rng) < prob) {
                current  = next;
                curM     = nextM;
                curScore = nextScore;
                acceptedWorse = true;
            }
        }

        if (curScore > bestScore) {
            bestScore = curScore;
            best      = current;
        }

        if (saOut) {
            saOut << t << ","
                  << T << ","
                  << curScore << ","
                  << (acceptedWorse ? 1 : 0) << "\n";
        }

        T *= alpha;
    }

    return best;
}
