//
// Created by 19210 on 2023/12/15.
//

#include "test_insert_blank.h"

#include <iostream>
#include <random>
#include <unordered_set>
#include <algorithm>
#include <queue>
#include "linear_regression_model.h"

typedef  int32_t KEY_TYPE;
constexpr KEY_TYPE DATA_MIN = 0, DATA_MAX = 1000000;
constexpr int32_t DATA_SIZE = 64;
constexpr int32_t ARRAY_SIZE = 2 * DATA_SIZE;

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

struct Node {
    double diff;
    int position;
};

struct NodeLess {
    bool operator() (const Node& lf, const Node& rf) {
        return lf.diff - rf.diff < -std::numeric_limits<double>::epsilon();
    }
};

struct NodeGreater {
    bool operator() (const Node& lf, const Node& rf) {
        return lf.diff - rf.diff > std::numeric_limits<double>::epsilon();
    }
};

void test_new_method() {
    //生成数据
    auto array = generate_data();
    std::vector<int> position_diff(DATA_SIZE - 1, 1), position(DATA_SIZE, 0);
    std::vector<double> key_diff;
    key_diff.reserve(DATA_SIZE - 1);
    for (int i = 0; i < DATA_SIZE - 1; ++i) {
        key_diff.push_back(array[i + 1] - array[i]);
    }

    double total_diff = static_cast<double>(array[DATA_SIZE - 1] - array[0]);
    double avg_key_diff = total_diff / (DATA_SIZE - 1);

    std::priority_queue<Node, std::vector<Node>, NodeGreater> min_heap;
    std::priority_queue<Node, std::vector<Node>, NodeLess> max_heap;

    for (int i = 0; i < DATA_SIZE - 1; ++i) {
        if (key_diff[i] - avg_key_diff > std::numeric_limits<double>::epsilon()) { // key_diff[i] > avg_key_diff <=> slope < avg_slope
            min_heap.push(Node{key_diff[i], i});
        } else { // key_diff[i] <= avg_key_diff <=> slope >= avg_slope
            max_heap.push(Node{key_diff[i], i});
        }
    }
    int32_t CURRENT_ARRAY_SIZE = DATA_SIZE;
    while (CURRENT_ARRAY_SIZE < ARRAY_SIZE) {
        auto tmp_node = min_heap.top();
        min_heap.pop();

        //插空格
        CURRENT_ARRAY_SIZE++;
        position_diff[tmp_node.position]++;
        auto half_diff = tmp_node.diff / 2;
        auto new_node1 = Node{half_diff, tmp_node.position};
        auto new_node2 = Node{half_diff, tmp_node.position};

        //avg_key_diff减少 avg_slope增加
        avg_key_diff = total_diff / (CURRENT_ARRAY_SIZE - 1);

        //更新max_heap和min_heap
        while (!max_heap.empty()) {
            auto tmp_node1 = max_heap.top();
            if (tmp_node1.diff - avg_key_diff > std::numeric_limits<double>::epsilon()) {
                max_heap.pop();
                min_heap.push(tmp_node1);
            } else {
                break;
            }
        }

        if (half_diff - avg_key_diff > std::numeric_limits<double>::epsilon()) {
            min_heap.push(new_node1);
            min_heap.push(new_node2);
        } else {
            max_heap.push(new_node1);
            max_heap.push(new_node2);
        }
    }
    int32_t sum = 0;
    for (int i = 0; i < DATA_SIZE - 1; ++i) {
        sum += position_diff[i];
        position[i + 1] = sum;
    }

    LinearModel<KEY_TYPE> linearModel;
    LinearModelBuilder<KEY_TYPE> linearModelBuilder(&linearModel);

    for (int i = 0; i < DATA_SIZE; ++i) {
        linearModelBuilder.add(array[i], position[i]);
    }
    linearModelBuilder.build();

    std::vector<int32_t> count(ARRAY_SIZE, 0);
    int32_t conflicts = 0;
    for (int i = 0; i < array.size(); ++i) {
        double v = linearModel.predict_double(array[i]);
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
    test_new_method();
    return 0;
}
