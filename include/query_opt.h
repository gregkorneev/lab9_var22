// SPDX-License-Identifier: MIT
//
// Лабораторная работа 22. Оптимизация SQL‑запросов
//
// Этот заголовочный файл содержит определение структуры плана SQL‑запроса,
// метрик эффективности и объявления функций для трёх алгоритмов оптимизации:
// Hill Climbing, Beam Search и имитации отжига. Алгоритмы ищут оптимальную
// комбинацию порядка соединений (JOIN) и использования индексов, оценивая
// эффективность плана с помощью синтетической модели.  Высокие значения
// производительности, эффективности индексов и простоты соединения
// соответствуют лучшим планам.

#pragma once

#include <iostream>
#include <vector>
#include <random>
#include <algorithm>

// Представление плана SQL‑запроса: порядок соединения таблиц и использование индексов.
struct QueryPlan {
    // Порядок соединения таблиц. join_order[i] = индекс таблицы на позиции i.
    std::vector<int> join_order;
    // Использование индекса для каждой таблицы. use_index[i] = true, если для таблицы
    // на позиции i используется индекс при соединении.
    std::vector<bool> use_index;
};

// Метрики оценки плана запроса.
struct QueryMetrics {
    double performance;       // Производительность: чем выше, тем лучше.
    double index_efficiency;  // Эффективность индексов: меньше использованных индексов -> выше значение.
    double complexity_score;  // Простота соединения: меньше «перестановок» -> выше значение.
};

// Перегруженные операторы для удобного вывода на экран.
std::ostream& operator<<(std::ostream& os, const QueryPlan& q);
std::ostream& operator<<(std::ostream& os, const QueryMetrics& m);

// Генерация случайного плана запроса для заданного числа таблиц.
QueryPlan random_queryplan(std::mt19937& rng, int num_tables);

// Генерация локального соседа плана: случайная перестановка порядка соединения
// или переключение использования индекса для одной таблицы.
QueryPlan local_neighbor(const QueryPlan& q, std::mt19937& rng);

// Оценка плана запроса: вычисляет метрики производительности, эффективности
// индексов и сложности соединения. Модель основана на синтетической функции
// со скрытым «идеальным» порядком соединения и использованием индексов.
QueryMetrics evaluate_query(const QueryPlan& q);

// Функции оценки для разных алгоритмов.  HC и SA максимизируют только
// производительность, Beam Search максимизирует взвешенную сумму всех метрик.
double score_for_HC(const QueryMetrics& m);
double score_for_beam(const QueryMetrics& m);
double score_for_SA(const QueryMetrics& m);

// Генерация множества соседей для плана.
std::vector<QueryPlan> generate_neighbors(const QueryPlan& q, int k, std::mt19937& rng);

// Алгоритм Hill Climbing: ищет локальный максимум, улучшая одну метрику (performance).
QueryPlan hill_climbing(const QueryPlan& start,
                        std::mt19937& rng,
                        int max_iterations = 200,
                        int neighbors_per_step = 20);

// Алгоритм Beam Search: рассматривает несколько путей поиска одновременно,
// оптимизируя взвешенную комбинацию метрик.  Параметры beam_width и depth
// задают ширину луча и глубину поиска.
QueryPlan beam_search(const QueryPlan& start,
                      std::mt19937& rng,
                      int beam_width = 5,
                      int depth = 30,
                      int neighbors_per_state = 10);

// Алгоритм имитации отжига: позволяет выходить из локальных максимумов,
// принимая ухудшающие решения с вероятностью, зависящей от температуры.  Вначале
// температура высокая, что стимулирует исследование, затем постепенно
// уменьшается до T_end.
QueryPlan simulated_annealing(const QueryPlan& start,
                              std::mt19937& rng,
                              int max_iterations = 1000,
                              double T_start = 1.0,
                              double T_end   = 1e-3,
                              double alpha   = 0.99);
