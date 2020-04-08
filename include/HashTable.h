#pragma once
#include "Table.h"
#include <functional>
#include <random>
#include <list>


// base class for hash tables
// defines hash function
// CellTypeDerived is the same as CellType in TableByArray
template <class ElemType, class HashTableIteratorType, class DerivedType, class CellTypeDerived>
class HashTable : public TableByArray<ElemType, HashTableIteratorType, DerivedType, CellTypeDerived> {

public:

    // capacity = 2^M
    HashTable(uint32_t M = FIRST_TABLE_SIZE_DEG) :
        M(M), TableByArrayType(getTableSize(M)), size(0) {
        // it is an effective c++ code generating random value "a" of type uint32_t
        // "a" is a parameter of hash function
        std::random_device rd;
        std::default_random_engine randGen(rd());
        std::uniform_int_distribution<uint32_t> dist;
        a = (uint64_t) dist(randGen);
    }

    uint32_t getSize() const {
        return size;
    }

    bool isEmpty() const {
        return size == 0;
    }

    void clear() {
        TableByArrayType::clear();
        M = FIRST_TABLE_SIZE_DEG;
        storage.resize(getTableSize(M));
        size = 0;
    }

protected:

    using CellType = CellTypeDerived;

    uint32_t size = 0;

    // random parameter of hash function
    uint64_t a;

    // capacity = 2^M
    uint32_t M;
    static const uint32_t FIRST_TABLE_SIZE_DEG = 10;

    size_t getTableSize(uint32_t M) {
        return uint32_t(1) << M;
    }

    // length of mashine word (32)
    const uint32_t W = sizeof(uint32_t) * 8;

    const double MAX_FILL_FACTOR = 0.7;       // if size > uint32_t(MAX_FILL_FACTOR*capacity)
                                              // then repack
    const double COEF_INCREASE_SIZE_DEG = 1;  // increases table by 2^COEF_INCREASE_SIZE_DEG
                                              // new size = 2^(M + COEF_INCREASE_SIZE)

    // universal hash function that can be computed quickly
    uint32_t hash(KeyType key) {
        return (uint32_t)(a * (uint64_t)key) >> (W - M);
    }

};


template <class ElemType>
class HashTableSeparateChainingIterator;

// class for a hash table with separate chaining (cell is a list)
// it needs of its own iterator class HashTableSeparateChainingIterator
template <class ElemType>
class HashTableSeparateChaining : public HashTable<ElemType,
    HashTableSeparateChainingIterator<ElemType>,
    HashTableSeparateChaining<ElemType>,
    std::list<std::pair<KeyType, ElemType>>> {

    using HashTableType = HashTable<ElemType,
        HashTableSeparateChainingIterator<ElemType>,
        HashTableSeparateChaining<ElemType>,
        std::list<std::pair<KeyType, ElemType>>>;

public:

    HashTableSeparateChaining(uint32_t M = HashTableType::FIRST_TABLE_SIZE_DEG) :
        HashTableType(M) {}

    // search O(1) on the average
    iterator find(const KeyType& key) {
        uint32_t hashValue = hash(key);
        typename CellType::iterator it = storage[hashValue].begin();
        for (; it != storage[hashValue].end() && it->first != key; ++it);
        if (it == storage[hashValue].end())
            return end();
        return iterator(storage, hashValue, it);
    }

    // insertion O(1) on the average
    iterator insertWithoutSearch(const KeyType& key, ElemType&& elem) {
        // if table is almost full then repack
        if (size + 1 > size_t(MAX_FILL_FACTOR * storage.size()))
            repack();
        uint32_t hashValue = hash(key);
        storage[hashValue].push_front(std::make_pair(key, std::move(elem)));
        size++;
        return iterator(storage, hashValue, storage[hashValue].begin());
    }

    // erasing O(1) on the average
    void eraseWithoutSearch(const iterator& pos) {
        uint32_t hashValue = hash(pos->first);
        storage[hashValue].erase(pos.getListIterator());
        size--;
    }


    iterator begin() {
        return iterator(storage, 0, storage[0].begin());
    }

    iterator end() {
        return iterator(storage, storage.size() - 1, storage[storage.size() - 1].end());
    }

protected:

    // repack if table is almost filled
    void repack() {
        M = uint32_t(M + COEF_INCREASE_SIZE_DEG);
        std::vector<HashTableType::CellType> tmp(getTableSize(M));  // new storage
        std::swap(tmp, storage);  // so tmp is old storage
        // doing a lot of insertions
        size = 0;
        for (size_t i = 0; i < tmp.size(); i++)
            for (auto it = tmp[i].begin(); it != tmp[i].end(); it++)
                insertWithoutSearch(std::move(it->first), std::move(it->second));
    }

};


// iterator for previous hash table
template <class ElemType>
class HashTableSeparateChainingIterator : public std::iterator<std::input_iterator_tag,
    std::pair<KeyType, ElemType>> {

public:

    // prefix
    HashTableSeparateChainingIterator& operator++() {
        ++listIterator;
        moveIteratorToExistingValueOrEnd();
        return *this;
    }

    // postfix
    HashTableSeparateChainingIterator operator++(int) {
        HashTableOpenAddressingIterator copy(*this);
        ++(*this);
        return copy;
    }

    std::pair<KeyType, ElemType>& operator*() const {
        return *listIterator;
    }

    std::pair<KeyType, ElemType>* operator->() const {
        return &(*listIterator);
    }

    friend bool operator==(const HashTableSeparateChainingIterator& it1,
        const HashTableSeparateChainingIterator& it2) {
        return it1.cell == it2.cell && it1.listIterator == it2.listIterator &&
            it1.storage.get().data() == it2.storage.get().data();
    }

    friend bool operator!=(const HashTableSeparateChainingIterator& it1,
        const HashTableSeparateChainingIterator& it2) {
        return !(it1 == it2);
    }

private:

    friend class HashTableSeparateChaining<ElemType>;

    using CellType = std::list<std::pair<KeyType, ElemType>>;

    HashTableSeparateChainingIterator(const std::reference_wrapper<std::vector<CellType>>& storage,
        size_t cell, const typename CellType::iterator& listIterator) :
        storage(storage), cell(cell), listIterator(listIterator) {
        moveIteratorToExistingValueOrEnd();
    }

    typename CellType::iterator getListIterator() const {
        return listIterator;
    }

    // iterator knows about storage
    // iterator = cell of table + list iterator
    std::reference_wrapper<std::vector<CellType>> storage;
    size_t cell;
    typename CellType::iterator listIterator;

    void moveIteratorToExistingValueOrEnd() {
        while (cell < storage.get().size() - 1 && listIterator == storage.get()[cell].end()) {
            cell++;
            listIterator = storage.get()[cell].begin();
        }
    }

};


template <class ElemType>
class HashTableOpenAddressingIterator;


struct HashTableOpenAddressingCellLabel {
    bool is_cell_not_empty = false;
    bool is_element_was_deleted = false;

    HashTableOpenAddressingCellLabel(bool arg1 = false, bool arg2 = false) :
        is_cell_not_empty(arg1), is_element_was_deleted(arg2) {}
};

// class for a hash table with ope addressing (cell is a list)
// it needs of its own iterator class HashTableOpenAddressingIterator
template <class ElemType>
class HashTableOpenAddressing : public HashTable<ElemType,
    HashTableOpenAddressingIterator<ElemType>,
    HashTableOpenAddressing<ElemType>,
    // all cells of hash table contain a label
    // it is necessary to determine if a cell with default key==uint32_t(0) is empty or not
    // or to determine a cell is deleted or not
    std::pair<std::pair<KeyType, ElemType>, HashTableOpenAddressingCellLabel>> {

    using HashTableType = HashTable<ElemType,
        HashTableOpenAddressingIterator<ElemType>,
        HashTableOpenAddressing<ElemType>,
        std::pair<std::pair<KeyType, ElemType>, HashTableOpenAddressingCellLabel>>;

public:

    HashTableOpenAddressing(uint32_t M = HashTableType::FIRST_TABLE_SIZE_DEG) :
        HashTableType(M) {}

    // search O(1) on the average
    // we consider that deleted elements are in the table
    iterator find(const KeyType& key) {
        // looking for empty cell or cell with key
        uint32_t hashValue = hash(key);
        size_t cell = 0;
        size_t i = 0;
        for (; i < storage.size(); ++i) {
            cell = getProbeSequenceElem(hashValue, i);
            if (!storage[cell].second.is_element_was_deleted && 
               (!storage[cell].second.is_cell_not_empty ||   // cell is free and element was not deleted
                storage[cell].first.first == key)) // or value is equal to key and element was not deleted
                break;
        }
        // if all table was looked through and needed cell or empty cell was not found
        // or if cell is free and element was not deleted (that is we didn't find key)
        if (i == storage.size() || (!storage[cell].second.is_cell_not_empty &&
            !storage[cell].second.is_element_was_deleted)) {
            return end();
        }
        // if element was found
        return iterator(storage, cell);
    }

    // insertion O(1) on the average
    // we consider that deleted elements are not in the table
    iterator insertWithoutSearch(const KeyType& key, ElemType&& elem) {
        // if table is almost full then repack
        if (size + 1 > size_t(MAX_FILL_FACTOR * storage.size()))
            repack();
        // looking for cell we can insert to
        uint32_t hashValue = hash(key);
        size_t cell = 0;
        size_t i = 0;
        for (; i < storage.size(); ++i) {
            cell = getProbeSequenceElem(hashValue, i);
            if (!storage[cell].second.is_cell_not_empty)  // if cell is empty, it is not important deleted or not
                break;
        }
        // if all table was looked through and empty cell was not found
        if (i == storage.size()) {
            repack();
            return this->insertWithoutSearch(key, std::move(elem));
        }
        // if empty cell was found
        size++;
        storage[cell] = std::make_pair(std::make_pair(key, std::move(elem)),
            HashTableOpenAddressingCellLabel(true, false));
        return iterator(storage, cell);
    }

    // erasing O(1) on the average
    // just sets a label
    void eraseWithoutSearch(const iterator& pos) {
        size--;
        storage[pos.getCell()].second.is_cell_not_empty = false;
        storage[pos.getCell()].second.is_element_was_deleted = true;
    }

    iterator begin() {
        return iterator(storage, 0);
    }

    iterator end() {
        return iterator(storage, storage.size());
    }

protected:

    size_t getProbeSequenceElem(uint32_t hashValue, size_t i) {
        if (storage.size() == 0)
            throw "Empty table";
        return (hashValue + i * i) & (storage.size() - 1);
    }

    // we add to the new table all elements: existing and deleted
    void repack() {
        M = uint32_t(M + COEF_INCREASE_SIZE_DEG);
        std::vector<HashTableType::CellType> tmp(getTableSize(M));
        std::swap(tmp, storage);
        size = 0;
        for (size_t i = 0; i < tmp.size(); i++)
            if (tmp[i].second.is_cell_not_empty || tmp[i].second.is_element_was_deleted) {
                iterator it = insertWithoutSearch(std::move(tmp[i].first.first), std::move(tmp[i].first.second));
                storage[it.getCell()].second = tmp[i].second;
            }
    }

};


// iterator for previous hash table
template <class ElemType>
class HashTableOpenAddressingIterator : public std::iterator<std::input_iterator_tag, std::pair<KeyType, ElemType>> {

public:

    // prefix
    HashTableOpenAddressingIterator& operator++() {
        cell++;
        moveIteratorToExistingValueOrEnd();
        return *this;
    }

    // postfix
    HashTableOpenAddressingIterator operator++(int) {
        HashTableOpenAddressingIterator copy(*this);
        ++(*this);
        return copy;
    }

    std::pair<KeyType, ElemType>& operator*() const {
        return storage.get()[cell].first;
    }

    std::pair<KeyType, ElemType>* operator->() const {
        return &(storage.get()[cell].first);
    }

    friend bool operator==(const HashTableOpenAddressingIterator& it1,
        const HashTableOpenAddressingIterator& it2) {
        return it1.cell == it2.cell && it1.storage.get().data() == it2.storage.get().data();
    }

    friend bool operator!=(const HashTableOpenAddressingIterator& it1,
        const HashTableOpenAddressingIterator& it2) {
        return !(it1 == it2);
    }

private:

    friend class HashTableOpenAddressing<ElemType>;

    using CellType = std::pair<std::pair<KeyType, ElemType>, HashTableOpenAddressingCellLabel>;

    HashTableOpenAddressingIterator(const std::reference_wrapper<std::vector<CellType>>& storage,
        size_t cell) : storage(storage), cell(cell) {
        moveIteratorToExistingValueOrEnd();
    }

    size_t getCell() const {
        return cell;
    }

    // iterator knows about storage
    // iteartor = cell of table
    std::reference_wrapper<std::vector<CellType>> storage;
    size_t cell;

    void moveIteratorToExistingValueOrEnd() {
        while (cell < storage.get().size() && !storage.get()[cell].second.is_cell_not_empty) {
            cell++;
        }
    }

};