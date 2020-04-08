#pragma once
#include "Table.h"
#include <functional>
#include <algorithm>

// class of a sorted table
// iterator for such table is just std::vector::iterator
template <class ElemType>
class SortedTable : public TableByArray<ElemType,
    typename std::vector<std::pair<KeyType, ElemType>>::iterator,
    SortedTable<ElemType>> {

public:

    // binary search O(log(n))
    iterator find(const KeyType& key) {
        std::pair<KeyType, ElemType> tmp(key, ElemType());
        iterator searchRes = std::lower_bound(storage.begin(), storage.end(), tmp,
            [](const std::pair<KeyType, ElemType>& a, const std::pair<KeyType, ElemType>& b) {
            return a.first < b.first;
        });
        if (searchRes == storage.end() || searchRes->first != key)
            return storage.end();
        return searchRes;
    }

    // insertion O(n)
    iterator insertWithoutSearch(KeyType&& key, ElemType&& elem) {
        auto it = storage.begin();
        for (; it != storage.end() && it->first < key; ++it);
        auto insertedIter = storage.insert(it,
            std::make_pair(std::move(key), std::move(elem)));  // here we are moving key and elem
        return insertedIter;
    }

    // erasing O(n)
    void eraseWithoutSearch(const iterator& pos) {
        storage.erase(pos);
    }


    iterator begin() {
        return storage.begin();
    }

    iterator end() {
        return storage.end();
    }

};