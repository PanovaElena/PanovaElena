#pragma once
#include "Table.h"
#include <functional>

// class of a sorted table
// iterator for such table is just std::vector::iterator
template <class ElemType>
class UnsortedTable : public TableByArray<ElemType,
    typename std::vector<std::pair<KeyType, ElemType>>::iterator,
    UnsortedTable<ElemType>> {

public:

    // search O(n)
    iterator find(const KeyType& key) {
        iterator it = begin();
        for (; it != end() && it->first != key; ++it);
        return it;
    }

    // insertion O(1)
    iterator insertWithoutSearch(const KeyType& key, ElemType&& elem) {
        storage.push_back(std::make_pair(key, std::move(elem)));  // here we are moving key and elem
        return storage.end() - 1;
    }

    // erasing O(1)
    void eraseWithoutSearch(const iterator& pos) {
        std::swap(*pos, *(storage.end() - 1));
        storage.pop_back();
    }


    iterator begin() {
        return storage.begin();
    }

    iterator end() {
        return storage.end();
    }

};