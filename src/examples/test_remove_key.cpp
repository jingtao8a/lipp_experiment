//
// Created by 19210 on 2023/12/18.
//

#include <iostream>
#include <unordered_set>
#include <algorithm>
#include <queue>
#include <chrono>
#include <random>
#include "linear_regression_model.h"
#include "CustomHeap.h"

typedef  int32_t KEY_TYPE;
constexpr KEY_TYPE DATA_MIN = 0, DATA_MAX = 1000000;
constexpr int32_t DATA_SIZE = 64000;
constexpr int32_t ARRAY_SIZE = 3 * DATA_SIZE;

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

struct LevelOneNode {
    double diff; // diff between two position;
    int left_pos;
    int right_pos;
};

struct LevelTwoNode {
    double diff;//diff = (left_child.diff + right_child.diff) / 2
    LevelOneNode * left_child;
    LevelOneNode * right_child;
    LevelTwoNode * prev;
    LevelTwoNode * next;
};

struct LevelTwoNodePointerEqual {
    bool operator()(LevelTwoNode* lp, LevelTwoNode* rp) {
        if (lp->diff - rp->diff > std::numeric_limits<double>::epsilon() && lp->diff - rp->diff < -std::numeric_limits<double>::epsilon()) {
            return false;
        }
        return (lp->left_child == rp->left_child) && (lp->right_child == rp->right_child);
    }
};

struct LevelTwoNodePointerLess {
    bool operator()(LevelTwoNode* lp, LevelTwoNode* rp) {
        if (lp->diff - rp->diff > std::numeric_limits<double>::epsilon()) {
            return false;
        }
        if (lp->diff - rp->diff < -std::numeric_limits<double>::epsilon()) {
            return true;
        }

        return lp->left_child < rp->right_child;
    }
};

struct LevelTwoNodePointerGreater {
    bool operator()(LevelTwoNode* lp, LevelTwoNode* rp) {
        if (lp->diff - rp->diff > std::numeric_limits<double>::epsilon()) {
            return true;
        }
        if (lp->diff - rp->diff < -std::numeric_limits<double>::epsilon()) {
            return false;
        }

        return lp->left_child > rp->right_child;
    }
};

bool isInMaxHeap(LevelTwoNode * node, double avg_key_diff) {
    if (node->diff - avg_key_diff > std::numeric_limits<double>::epsilon()) {
        return false;
    }
    return true;
}

void test_new_method() {
    std::vector<KEY_TYPE> array = generate_data();
    std::vector<LevelOneNode *> levelOneNodeArray;
    std::vector<LevelTwoNode *> levelTwoNodeArray;
    CustomHeap<LevelTwoNode *, LevelTwoNodePointerEqual, LevelTwoNodePointerGreater> min_heap;
    CustomHeap<LevelTwoNode *, LevelTwoNodePointerEqual, LevelTwoNodePointerLess> max_heap;
    int remove_key_count = 0;
    double total_diff = static_cast<double>(array[DATA_SIZE - 1] - array[0]);
    double avg_key_diff = total_diff / (DATA_SIZE - 1);
    for (int i = 0; i < DATA_SIZE - 1; ++i) {
        levelOneNodeArray.push_back(new LevelOneNode{static_cast<double>(array[i + 1] - array[i]), i, i + 1});
    }
    levelTwoNodeArray.push_back(new LevelTwoNode{
            (levelOneNodeArray[0]->diff + levelOneNodeArray[1]->diff) / 2,
            levelOneNodeArray[0],
            levelOneNodeArray[1],
            NULL,
            NULL,
    });
    int levelTwoNodeArrayIndex = 0;
    for (int i = 1; i < levelOneNodeArray.size(); +i) {
        auto newTwoNode = new LevelTwoNode{
                (levelOneNodeArray[i]->diff + levelOneNodeArray[i + 1]->diff) / 2,
                levelOneNodeArray[i],
                levelOneNodeArray[i + 1],
                levelTwoNodeArray[levelTwoNodeArrayIndex],
                NULL,
        };
        levelTwoNodeArray[levelTwoNodeArrayIndex]->next = newTwoNode;
        levelTwoNodeArray.push_back(newTwoNode);
        levelTwoNodeArrayIndex++;
    }

    //init min_heap and max_heap
    for (auto p : levelTwoNodeArray) {
        if (p->diff - avg_key_diff > std::numeric_limits<double>::epsilon()) {// p->diff > avg_key_diff <=> slope < avg_slope
            min_heap.push(p);
        } else { // p->diff <= avg_key_diff <=> slope >= avg_slope
            max_heap.push(p);
        }
    }

    while (remove_key_count < array.size() / 2) { // plan to remove half of the keys
        auto levelTwoNode = max_heap.top();
        max_heap.pop();
        auto prevLevelTwoNode = levelTwoNode->prev;
        auto nextLevelTwoNode = levelTwoNode->next;
        if (prevLevelTwoNode == NULL) {//最左边的levelTwoNode
            levelTwoNode->right_child->diff = levelTwoNode->right_child->diff + levelTwoNode->left_child->diff;
            levelTwoNode->right_child->left_pos = 0;//remove position 1
            remove_key_count++;

            //remove nextLevelTwoNode
            if (isInMaxHeap(nextLevelTwoNode, avg_key_diff)) {
                max_heap.erase(nextLevelTwoNode);
            } else {
                min_heap.erase(nextLevelTwoNode);
            }
            nextLevelTwoNode->diff = (nextLevelTwoNode->left_child->diff + nextLevelTwoNode->right_child->diff) / 2;

            avg_key_diff = total_diff / (DATA_SIZE - 1 - remove_key_count);//avg_key_diff 增加  slope 降低
            //更新max_heap和min_heap
            while (min_heap.size()) {
                auto node = min_heap.top();
                if (node->diff - avg_key_diff > std::numeric_limits<double>::epsilon()) {
                    break;
                }
                min_heap.pop();
                max_heap.push(node);
            }

            //insert nextLevelTwoNode
            if (nextLevelTwoNode->diff - avg_key_diff > std::numeric_limits<double>::epsilon()) {// p->diff > avg_key_diff <=> slope < avg_slope
                min_heap.push(nextLevelTwoNode);
            } else { // p->diff <= avg_key_diff <=> slope >= avg_slope
                max_heap.push(nextLevelTwoNode);
            }
        } else if (nextLevelTwoNode == NULL) {//最右边的levelTwoNode

        } else {

        }

    }

    //释放堆内存
    for (auto p : levelOneNodeArray) {
        delete p;
    }

    for (auto p : levelTwoNodeArray) {
        delete p;
    }
}

int main() {
    std::cout << "test_new_method remove key" << std::endl;
//    testCustomHeap();
    test_new_method();
    return 0;
}
