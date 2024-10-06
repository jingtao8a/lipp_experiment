//
// Created by 19210 on 2024/9/27.
//

#include "closure_algorithm.h"
#include "shrinkingcone_algorithm.h"
#include <iostream>
#include <unordered_set>
#include <vector>
#include <fstream>
#include <cstdint>

#define DATA_SIZE 1000000
#define keyType uint64_t

static keyType searchKey = 20000;


template <class T>
bool load_binary_data(T data[], int length, const std::string& file_path) {
    std::ifstream is(file_path.c_str(), std::ios::binary | std::ios::in);
    if (!is.is_open()) {
        return false;
    }
    is.read(reinterpret_cast<char*>(data), std::streamsize(length * sizeof(T)));
    is.close();
    return true;
}
int main() {
    keyType* keys = new keyType[DATA_SIZE];
    load_binary_data(keys, DATA_SIZE, "D:/ALEX/resources/ycsb-200M.bin.data");
    // Generate some random data
    std::vector<keyType> data(keys, keys + DATA_SIZE);

    data.push_back(searchKey);
    std::sort(data.begin(), data.end());
    auto iter = std::lower_bound(data.begin(), data.end(), searchKey);
    std::cout << *data.begin() << std::endl;
    std::cout << data.back() << std::endl;
    std::cout << searchKey << " real_position is " << std::distance(data.begin(), iter) << std::endl;
//    dili::ClosureAlgorithm<keyType> closureAlgorithm(data);
//    auto& segments = closureAlgorithm.getSegments();
    dili::ShrinkingConeAlgorithm<keyType> shrinkingConeAlgorithm(data);
    auto& segments = shrinkingConeAlgorithm.getSegments();
    std::cout << segments.size() << std::endl;
    for (int i = 0; i < segments.size(); ++i) {
        std::cout << "key " << segments[i].key << " ";
        std::cout << "slope " << segments[i].slope << " ";
        std::cout << "intercept " << segments[i].intercept << " ";
        std::cout << std::endl;
    }
    std::cout << searchKey << " predict_position is " << segments[0](searchKey) << std::endl;
}
