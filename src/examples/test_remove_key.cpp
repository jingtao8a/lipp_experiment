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
#include <bitset>

typedef  int32_t KEY_TYPE;
constexpr KEY_TYPE DATA_MIN = 0, DATA_MAX = 1000000;
constexpr int32_t DATA_SIZE = 5;
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
    LevelTwoNode(double diff_, LevelOneNode* left_child_, LevelOneNode* right_child_,
                 LevelTwoNode* prev_, LevelTwoNode* next_):
                diff(diff_), left_child(left_child_), right_child(right_child_), prev(prev_), next(next_) {}
    LevelTwoNode(const LevelTwoNode& ref) = default;
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
    std::bitset<DATA_SIZE> bitmap;//为1表示删除
    std::vector<LevelOneNode *> levelOneNodeArray;
    std::vector<LevelTwoNode *> levelTwoNodeArray;
    std::vector<LevelTwoNode *> newLevelTwoNodeArray;
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
    for (int i = 1; i < levelOneNodeArray.size(); ++i) {
        auto newTwoNode = new LevelTwoNode {
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
            bitmap[levelTwoNode->right_child->left_pos] = 1;// remove position
            levelTwoNode->right_child->left_pos = levelTwoNode->left_child->left_pos;
            remove_key_count++;

            //remove nextLevelTwoNode
            if (isInMaxHeap(nextLevelTwoNode, avg_key_diff)) {
                max_heap.erase(nextLevelTwoNode);
            } else {
                min_heap.erase(nextLevelTwoNode);
            }
            auto newNextLevelTwoNode = new LevelTwoNode(*nextLevelTwoNode);
            newNextLevelTwoNode->diff = (newNextLevelTwoNode->left_child->diff + newNextLevelTwoNode->right_child->diff) / 2;
            newLevelTwoNodeArray.push_back(newNextLevelTwoNode);
            if (newNextLevelTwoNode->next) {
                newNextLevelTwoNode->next->prev = newNextLevelTwoNode;
            }
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

            //insert newNextLevelTwoNode
            if (newNextLevelTwoNode->diff - avg_key_diff > std::numeric_limits<double>::epsilon()) {// p->diff > avg_key_diff <=> slope < avg_slope
                min_heap.push(newNextLevelTwoNode);
            } else { // p->diff <= avg_key_diff <=> slope >= avg_slope
                max_heap.push(newNextLevelTwoNode);
            }

            //delete levelTwoNode
            newNextLevelTwoNode->prev = NULL;
        } else if (nextLevelTwoNode == NULL) {//最右边的levelTwoNode
            levelTwoNode->left_child->diff = levelTwoNode->left_child->diff + levelTwoNode->right_child->diff;
            bitmap[levelTwoNode->left_child->right_pos] = 1; //remove position
            levelTwoNode->left_child->right_pos = levelTwoNode->right_child->right_pos;
            remove_key_count++;

            //remove prevLevelTwoNode
            if (isInMaxHeap(prevLevelTwoNode, avg_key_diff)) {
                max_heap.erase(prevLevelTwoNode);
            } else {
                min_heap.erase(prevLevelTwoNode);
            }
            auto newPrevLevelTwoNode = new LevelTwoNode(*prevLevelTwoNode);
            newPrevLevelTwoNode->diff = (newPrevLevelTwoNode->left_child->diff + newPrevLevelTwoNode->right_child->diff) / 2;
            newLevelTwoNodeArray.push_back(newPrevLevelTwoNode);
            if (newPrevLevelTwoNode->prev) {
                newPrevLevelTwoNode->prev->next = newPrevLevelTwoNode;
            }

            avg_key_diff = total_diff / (DATA_SIZE - 1 - remove_key_count);//avg_key_diff 增加 slope降低
            //更新max_heap和min_heap
            while (min_heap.size()) {
                auto node = min_heap.top();
                if (node->diff - avg_key_diff > std::numeric_limits<double>::epsilon()) {
                    break;
                }
                min_heap.pop();
                max_heap.push(node);
            }

            //insert newPrevLevelTwoNode
            if (newPrevLevelTwoNode->diff - avg_key_diff > std::numeric_limits<double>::epsilon()) {// p->diff > avg_key_diff <=> slope < avg_slope
                min_heap.push(newPrevLevelTwoNode);
            } else { // p->diff <= avg_key_diff <=> slope >= avg_slope
                max_heap.push(newPrevLevelTwoNode);
            }

            //delete levelTwoNode
            newPrevLevelTwoNode->next = NULL;
        } else {
            levelTwoNode->left_child->diff = levelTwoNode->left_child->diff + levelTwoNode->right_child->diff;
            bitmap[levelTwoNode->left_child->right_pos] = 1;//remove position
            levelTwoNode->left_child->right_pos = levelTwoNode->right_child->right_pos;
            remove_key_count++;

            //remove nextLevelTwoNode
            if (isInMaxHeap(nextLevelTwoNode, avg_key_diff)) {
                max_heap.erase(nextLevelTwoNode);
            } else {
                min_heap.erase(nextLevelTwoNode);
            }
            auto newNextLevelTwoNode = new LevelTwoNode(*nextLevelTwoNode);
            newNextLevelTwoNode->left_child = levelTwoNode->left_child;

            //remove prevLevelTwoNode
            if (isInMaxHeap(prevLevelTwoNode, avg_key_diff)) {
                max_heap.erase(prevLevelTwoNode);
            } else {
                min_heap.erase(prevLevelTwoNode);
            }
            auto newPrevLevelTwoNode = new LevelTwoNode(*prevLevelTwoNode);

            newPrevLevelTwoNode->diff = (newPrevLevelTwoNode->left_child->diff + newPrevLevelTwoNode->right_child->diff) / 2;
            newNextLevelTwoNode->diff = (newNextLevelTwoNode->left_child->diff + newNextLevelTwoNode->right_child->diff) / 2;
            if (newPrevLevelTwoNode->prev) {
                newPrevLevelTwoNode->prev->next = newPrevLevelTwoNode;
            }
            if (newNextLevelTwoNode->next) {
                newNextLevelTwoNode->next->prev = newNextLevelTwoNode;
            }

            avg_key_diff = total_diff / (DATA_SIZE - 1 - remove_key_count);//avg_key_diff 增加 slope降低
            //更新max_heap和min_heap
            while (min_heap.size()) {
                auto node = min_heap.top();
                if (node->diff - avg_key_diff > std::numeric_limits<double>::epsilon()) {
                    break;
                }
                min_heap.pop();
                max_heap.push(node);
            }

            //insert newPrevLevelTwoNode
            if (newPrevLevelTwoNode->diff - avg_key_diff > std::numeric_limits<double>::epsilon()) {// p->diff > avg_key_diff <=> slope < avg_slope
                min_heap.push(newPrevLevelTwoNode);
            } else { // p->diff <= avg_key_diff <=> slope >= avg_slope
                max_heap.push(newPrevLevelTwoNode);
            }

            //insert nextLevelTwoNode
            if (newNextLevelTwoNode->diff - avg_key_diff > std::numeric_limits<double>::epsilon()) {
                min_heap.push(newNextLevelTwoNode);
            } else {
                max_heap.push(newNextLevelTwoNode);
            }

            //delete levelTwoNode
            newPrevLevelTwoNode->next = newNextLevelTwoNode;
            newNextLevelTwoNode->prev = newPrevLevelTwoNode;
        }
    }
    auto sum = 0;
    for (auto i = 0; i < bitmap.size(); ++i) {
        if (bitmap[i] == 1) {
            sum++;
        }
    }
    std::cout << remove_key_count << " " << sum;

    //释放堆内存
    for (auto p : levelOneNodeArray) {
        delete p;
    }

    for (auto p : levelTwoNodeArray) {
        delete p;
    }

    for (auto p : newLevelTwoNodeArray) {
        delete p;
    }
}

int main() {
    std::cout << "test_new_method remove key" << std::endl;
//    testCustomHeap();
    test_new_method();
    return 0;
}
