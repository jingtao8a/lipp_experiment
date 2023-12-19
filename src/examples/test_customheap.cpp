//
// Created by 19210 on 2023/12/18.
//
#include "CustomHeap.h"
#include <iostream>

struct Node{
    int key_;
};

struct NodePointerEqual{
    bool operator()(Node* lp, Node* rp) {
        return lp == rp;
    }
};

struct NodePointerLess {
    bool operator()(Node* lp, Node* rp) {
        return lp->key_ < rp->key_;
    }
};

void testCustomHeap() {
    CustomHeap<Node*, NodePointerEqual, NodePointerLess> heap;
    std::vector<int> array{12, 32, 32, 4324, 3249};
    std::vector<Node *> node_pointer_array;
    node_pointer_array.reserve(array.size());
    for (auto i : array) {
        node_pointer_array.push_back(new Node{i});
    }

    for (auto i : node_pointer_array) {
        heap.push(i);
    }
    heap.erase(node_pointer_array[0]);
    heap.erase(node_pointer_array[1]);
    heap.erase(node_pointer_array[4]);
    node_pointer_array.push_back(new Node{123242});
    heap.push(node_pointer_array.back());
    while (heap.size()) {
        auto i = heap.top();
        heap.pop();
        std::cout << i->key_ << std::endl;
    }
    for (auto i : node_pointer_array) {
        delete i;
    }
}

int main() {
    testCustomHeap();
    return 0;
}