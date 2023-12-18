//
// Created by 19210 on 2023/12/18.
//

#ifndef LIPP_CUSTOMHEAP_H
#define LIPP_CUSTOMHEAP_H

#include <queue>
#include <assert.h>

template <class Type, class TypeEqual, class TypeComparator>
class CustomHeap {
private:
    std::priority_queue<Type, std::vector<Type>, TypeComparator> A, B;
    TypeEqual typeEqual;

public:
    void push(Type i) {
        A.push(i);
    }

    void erase(Type i) {
        B.push(i);
    }

    void pop() {
        while (B.size() && typeEqual(A.top(), B.top())) {
            A.pop();
            B.pop();
        }
        assert(A.size() != 0);
        A.pop();
    }

    Type top() {
        while (B.size() && typeEqual(A.top(), B.top())) {
            A.pop();
            B.pop();
        }
        assert(A.size() != 0);
        return A.top();
    }

    int size() const {
        return A.size() - B.size();
    }
};


#endif //LIPP_CUSTOMHEAP_H
