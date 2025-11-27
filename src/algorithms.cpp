// SPDX-License-Identifier: MIT
//
// Реализация алгоритмов оптимизации SQL‑запросов (Hill Climbing, Beam Search,
// имитация отжига) для лабораторной работы 22.

#include "query_opt.h"
#include <algorithm>
#include <cmath>

// --------------------- Hill Climbing ---------------------- //

QueryPlan hill_climbing(const QueryPlan& start,
                        std::mt19937& rng,
                        int max_iterations,
                        int neighbors_per_step) {
    QueryPlan current = start;
    QueryMetrics curM = evaluate_query(current);
    double curScore = score_for_HC(curM);
    for (int iter = 0; iter < max_iterations; ++iter) {
        QueryPlan bestNeighbor = current;
        double bestScore = curScore;
        auto neighbors = generate_neighbors(current, neighbors_per_step, rng);
        for (const auto& n : neighbors) {
            QueryMetrics m = evaluate_query(n);
            double s = score_for_HC(m);
            if (s > bestScore) {
                bestScore = s;
                bestNeighbor = n;
            }
        }
        if (bestScore <= curScore) {
            std::cout << "[HC] остановка на итерации " << iter
                      << " — достигнут локальный максимум\n";
            break;
        }
        current = bestNeighbor;
        curScore = bestScore;
    }
    return current;
}

// --------------------- Beam Search ---------------------- //

QueryPlan beam_search(const QueryPlan& start,
                      std::mt19937& rng,
                      int beam_width,
                      int depth,
                      int neighbors_per_state) {
    std::vector<QueryPlan> beam;
    beam.push_back(start);
    QueryPlan globalBest = start;
    double globalBestScore = score_for_beam(evaluate_query(start));
    for (int level = 0; level < depth; ++level) {
        std::vector<std::pair<double, QueryPlan>> candidates;
        for (const auto& state : beam) {
            auto neigh = generate_neighbors(state, neighbors_per_state, rng);
            for (const auto& n : neigh) {
                double score = score_for_beam(evaluate_query(n));
                candidates.push_back({score, n});
            }
        }
        if (candidates.empty()) break;
        std::sort(candidates.begin(), candidates.end(),
                  [](const auto& a, const auto& b) { return a.first > b.first; });
        beam.clear();
        for (int i = 0; i < beam_width && i < (int)candidates.size(); ++i) {
            beam.push_back(candidates[i].second);
            if (candidates[i].first > globalBestScore) {
                globalBestScore = candidates[i].first;
                globalBest = candidates[i].second;
            }
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
    QueryPlan current = start;
    QueryMetrics curM = evaluate_query(current);
    double curScore = score_for_SA(curM);
    QueryPlan best = current;
    double bestScore = curScore;
    double T = T_start;
    for (int t = 0; t < max_iterations && T > T_end; ++t) {
        QueryPlan next = local_neighbor(current, rng);
        QueryMetrics nextM = evaluate_query(next);
        double nextScore = score_for_SA(nextM);
        double dE = curScore - nextScore; // максимизируем score
        if (dE < 0) {
            current = next;
            curScore = nextScore;
        } else {
            double prob = std::exp(-dE / T);
            std::uniform_real_distribution<double> u(0.0, 1.0);
            if (u(rng) < prob) {
                current = next;
                curScore = nextScore;
            }
        }
        if (curScore > bestScore) {
            bestScore = curScore;
            best = current;
        }
        T *= alpha;
    }
    return best;
}
