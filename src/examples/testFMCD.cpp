//
// Created by 19210 on 2023/12/15.
//

#include "testFMCD.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <random>
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <iostream>
#include "linear_regression_model.h"

// runtime assert
#define RT_ASSERT(expr) \
{ \
    if (!(expr)) { \
        fprintf(stderr, "RT_ASSERT Error at %s:%d, `%s`\n", __FILE__, __LINE__, #expr); \
        exit(0); \
    } \
}

inline int compute_gap_count(int size) {
    if (size >= 1000000) return 1;
    if (size >= 100000) return 2;
    return 5;
}

typedef  int32_t KEY_TYPE;
constexpr KEY_TYPE DATA_MIN = 0, DATA_MAX = 1000000;
constexpr int32_t DATA_SIZE = 64;


std::vector<KEY_TYPE> generate_data() {
    std::random_device seed;//硬件生成随机数种子
    std::ranlux48 engine(seed());//利用种子生成随机数引擎
    std::uniform_int_distribution<> distrib(DATA_MIN, DATA_MAX);//设置随机数范围，并为均匀分布
    std::vector<KEY_TYPE> array;
    std::unordered_set<KEY_TYPE> hash_set;
    array.reserve(DATA_SIZE);
    while (array.size() < DATA_SIZE) {
        int random = distrib(engine);//随机数
        if (hash_set.count(random) == 0) {
            array.push_back(random);
            hash_set.insert(random);
        }
    }
    std::sort(array.begin(), array.end());
    return array;
}


void testFMCD() {
    auto keys = generate_data();
    // FMCD method
    // Here the implementation is a little different with Algorithm 1 in our paper.
    // In Algorithm 1, U_T should be (keys[size-1-D] - keys[D]) / (L - 2).
    // But according to the derivation described in our paper, M.A should be less than 1 / U_T.
    // So we added a small number (1e-6) to U_T.
    // In fact, it has only a negligible impact of the performance.
    auto BUILD_GAP_CNT = compute_gap_count(DATA_SIZE);
    auto size = DATA_SIZE;
    int32_t ARRAY_SIZE;
    LinearModel<KEY_TYPE> model;
    {
        //D == T  size == N
        const int L = size * static_cast<int>(BUILD_GAP_CNT + 1);
        ARRAY_SIZE = L;
        int i = 0;
        int D = 1;
        RT_ASSERT(D <= size-1-D);
        double Ut = (static_cast<long double>(keys[size - 1 - D]) - static_cast<long double>(keys[D])) /
                    (static_cast<double>(L - 2)) + 1e-6;// a samll number (1e-6)
        while (i < size - 1 - D) {
            while (i + D < size && keys[i + D] - keys[i] >= Ut) {
                i ++;
            }
            if (i + D >= size) {
                break;
            }
            D = D + 1;
            if (D * 3 > size) break;
            RT_ASSERT(D <= size-1-D);
            Ut = (static_cast<long double>(keys[size - 1 - D]) - static_cast<long double>(keys[D])) /
                 (static_cast<double>(L - 2)) + 1e-6;
        }
        if (D * 3 <= size) {//FMCD算法计算的冲突度 <= size / 3,表示成功

            model.a_ = 1.0 / Ut;
            model.b_ = (L - model.a_ * (static_cast<long double>(keys[size - 1 - D]) +
                                                  static_cast<long double>(keys[D]))) / 2;
            RT_ASSERT(std::isfinite(model.a_));
            RT_ASSERT(std::isfinite(model.b_));
        } else {

            int mid1_pos = (size - 1) / 3;
            int mid2_pos = (size - 1) * 2 / 3;

            RT_ASSERT(0 <= mid1_pos);
            RT_ASSERT(mid1_pos < mid2_pos);
            RT_ASSERT(mid2_pos < size - 1);

            const long double mid1_key = (static_cast<long double>(keys[mid1_pos]) +
                                          static_cast<long double>(keys[mid1_pos + 1])) / 2;
            const long double mid2_key = (static_cast<long double>(keys[mid2_pos]) +
                                          static_cast<long double>(keys[mid2_pos + 1])) / 2;

            const double mid1_target = mid1_pos * static_cast<int>(BUILD_GAP_CNT + 1) + static_cast<int>(BUILD_GAP_CNT + 1) / 2;
            const double mid2_target = mid2_pos * static_cast<int>(BUILD_GAP_CNT + 1) + static_cast<int>(BUILD_GAP_CNT + 1) / 2;

            model.a_ = (mid2_target - mid1_target) / (mid2_key - mid1_key);
            model.b_ = mid1_target - model.a_ * mid1_key;
            RT_ASSERT(std::isfinite(model.a_));
            RT_ASSERT(std::isfinite(model.b_));
        }
    }
    RT_ASSERT(model.a_ >= 0);
    std::vector<int32_t> count(ARRAY_SIZE, 0);
    int32_t conflicts = 0;
    for (int i = 0; i < keys.size(); ++i) {
        double v = model.predict_double(keys[i]);
        if (v > std::numeric_limits<int>::max() / 2) {
            count[ARRAY_SIZE - 1]++;
        } else if (v < 0) {
            count[0]++;
        } else {
            count[std::min(ARRAY_SIZE - 1, static_cast<int>(v))]++;
        }
    }
    for (auto i : count) {
        if (i > 1) { // 发生冲突
            conflicts += i - 1;
        }
    }
    std::cout << "conflicts: " << conflicts << std::endl;
}

int main() {
    testFMCD();
    return 0;
}