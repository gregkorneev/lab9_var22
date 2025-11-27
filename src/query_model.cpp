// SPDX-License-Identifier: MIT
//
// Реализация модели оценки SQL‑запросов для лабораторной работы 22.
// Модуль содержит определения операторов вывода, функции генерации
// случайных планов и их соседей, а также вычисление метрик для плана.

#include "query_opt.h"
#include <cmath>

// Вывод QueryPlan
std::ostream& operator<<(std::ostream& os, const QueryPlan& q) {
    os << "{order=[";
    for (size_t i = 0; i < q.join_order.size(); ++i) {
        os << q.join_order[i];
        if (i + 1 < q.join_order.size()) os << ", ";
    }
    os << "]";
    os << ", idx=[";
    for (size_t i = 0; i < q.use_index.size(); ++i) {
        os << (q.use_index[i] ? '1' : '0');
        if (i + 1 < q.use_index.size()) os << ", ";
    }
    os << "]}";
    return os;
}

// Вывод QueryMetrics
std::ostream& operator<<(std::ostream& os, const QueryMetrics& m) {
    os << "{perf=" << m.performance
       << ", idx_eff=" << m.index_efficiency
       << ", complexity=" << m.complexity_score << "}";
    return os;
}

// Генерация случайного плана: случайная перестановка [0..n-1] и случайные
// значения use_index.
QueryPlan random_queryplan(std::mt19937& rng, int num_tables) {
    QueryPlan q;
    q.join_order.resize(num_tables);
    q.use_index.resize(num_tables);
    // Заполняем порядок соединения 0..n-1 и перемешиваем
    for (int i = 0; i < num_tables; ++i) {
        q.join_order[i] = i;
    }
    std::shuffle(q.join_order.begin(), q.join_order.end(), rng);
    // Индексы задаём случайно с равной вероятностью
    std::bernoulli_distribution bern(0.5);
    for (int i = 0; i < num_tables; ++i) {
        q.use_index[i] = bern(rng);
    }
    return q;
}

// Создание локального соседа: с вероятностью 0.5 меняем местами две
// случайные позиции в порядке соединения; иначе переключаем использование
// индекса для одной случайной таблицы.
QueryPlan local_neighbor(const QueryPlan& q, std::mt19937& rng) {
    QueryPlan n = q;
    std::uniform_real_distribution<double> uni(0.0, 1.0);
    if (uni(rng) < 0.5 && n.join_order.size() >= 2) {
        // Меняем местами две различные позиции
        std::uniform_int_distribution<int> dist(0, static_cast<int>(n.join_order.size()) - 1);
        int i = dist(rng);
        int j = dist(rng);
        while (j == i) {
            j = dist(rng);
        }
        std::swap(n.join_order[i], n.join_order[j]);
    } else {
        // Переключаем индекс для случайной таблицы
        if (!n.use_index.empty()) {
            std::uniform_int_distribution<int> dist(0, static_cast<int>(n.use_index.size()) - 1);
            int i = dist(rng);
            n.use_index[i] = !n.use_index[i];
        }
    }
    return n;
}

// Оценка плана запроса.
// Модель основана на скрытом «идеальном» порядке соединения (от 0 до n-1)
// и использовании индексов для первой половины таблиц. Чем ближе план к идеалу,
// тем ниже стоимость. Метрики нормируются так, что более низкая стоимость
// даёт более высокие значения performance.
QueryMetrics evaluate_query(const QueryPlan& q) {
    int n = static_cast<int>(q.join_order.size());
    // Вычисляем базовую стоимость
    double cost = 10.0;
    // Разница порядка от идеального [0,1,2,...,n-1]
    double order_diff = 0.0;
    for (int i = 0; i < n; ++i) {
        order_diff += std::abs(q.join_order[i] - i);
    }
    cost += 2.0 * order_diff;
    // Идеальное использование индексов: для первых n/2 таблиц индекс=true,
    // для остальных=false
    for (int i = 0; i < n; ++i) {
        bool ideal_idx = (i < n / 2);
        if (q.use_index[i] != ideal_idx) {
            cost += 5.0;
        }
    }
    // Считаем количество инверсий в join_order как меру сложности соединения
    int inversions = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (q.join_order[i] > q.join_order[j]) {
                inversions++;
            }
        }
    }
    // Вносим небольшой шум, чтобы получить локальные оптимумы
    static std::mt19937 noise_rng{1234567};
    std::uniform_real_distribution<double> noise_dist(-0.5, 0.5);
    cost += noise_dist(noise_rng);
    // Нормируем метрики
    double performance = 1.0 / (1.0 + cost);
    // Эффективность индексов: чем меньше true в use_index, тем лучше.  Мы
    // не знаем реального числа таблиц, поэтому считаем 1/(1+count).
    int index_count = 0;
    for (bool use : q.use_index) {
        if (use) index_count++;
    }
    double index_efficiency = 1.0 / (1.0 + index_count);
    // Простота соединения: меньше инверсий -> выше значение
    double complexity_score = 1.0 / (1.0 + inversions);
    return {performance, index_efficiency, complexity_score};
}

double score_for_HC(const QueryMetrics& m) {
    return m.performance;
}

double score_for_beam(const QueryMetrics& m) {
    // Взвешенная сумма: приоритет производительности, но учитываются индекс и сложность
    return 0.6 * m.performance + 0.2 * m.index_efficiency + 0.2 * m.complexity_score;
}

double score_for_SA(const QueryMetrics& m) {
    return m.performance;
}

// Генерация множества соседей
std::vector<QueryPlan> generate_neighbors(const QueryPlan& q, int k, std::mt19937& rng) {
    std::vector<QueryPlan> res;
    res.reserve(k);
    for (int i = 0; i < k; ++i) {
        res.push_back(local_neighbor(q, rng));
    }
    return res;
}
