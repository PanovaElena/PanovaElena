#pragma once
#include <utility>
#include <vector>
#include <iostream>
#include <type_traits>
#include <cstdint>

// key is a type of uint32_t
typedef uint32_t KeyType;


// a base class for tables
// ElemType is a type of elements
// IteratorType is an interator class for derived table
// DerivedType is a type of derived table
// DerivedType is necessary to imitate virtual behavior without the keyword "virtual"
template <class ElemType, class IteratorType, class DerivedType>
class Table {

public:

    // typedef of IteratorType
    // so we will can to write "typename AnyTable<TypeElem>::iterator it = table.begin()"
    using iterator = IteratorType;

    Table() {
        // static_assert is executed by the compilation stage
        // it just checks if DerivedType is derived by Table
        static_assert(std::is_base_of<Table, DerivedType>::value, "Derived class is not of Table type");
    }

    // insert by copying the element
    std::pair<iterator, bool> insert(const KeyType& key, const ElemType& elem) {
        KeyType keyCopy = key;     // here we are copying key
        ElemType elemCopy = elem;  // here we are copying elem
        // here we are moving copied elem and key
        // std::pair<iterator, bool> insert(KeyType&& key, ElemType&& elem) will be called

        // here we called insert function of derived class
        // so we imitated virtual behavior without keyword "virtual"
        return this->insert(std::move(keyCopy), std::move(elemCopy));
    }

    // more advanced function, move semantics
    // insert by moving the element
    std::pair<iterator, bool> insert(KeyType&& key, ElemType&& elem) {
        DerivedType* der = static_cast<DerivedType*>(this);
        iterator searchRes = der->find(key);
        if (searchRes != end())  // if elem was found
            return std::make_pair(searchRes, false);
        return std::make_pair(der->insertWithoutSearch(std::move(key), std::move(elem)), true);
    }
     
    // insert without search
    // move semantics
    iterator insertWithoutSearch(KeyType&& key, ElemType&& elem) {
        return static_cast<DerivedType*>(this)->insertWithoutSearch(std::move(key), std::move(elem));
    }

    // erase by key
    bool erase(const KeyType& key) {
        DerivedType* der = static_cast<DerivedType*>(this);
        iterator searchRes = der->find(key);
        if (searchRes == end())  // if elem was not found
            return false;
        der->eraseWithoutSearch(searchRes);
        return true;
    }

    // erase by iterator got by "find" without search  
    void eraseWithoutSearch(const iterator& pos) {
        static_cast<DerivedType*>(this)->eraseWithoutSearch(pos);
    }

    // search
    iterator find(const KeyType& key) {
        return static_cast<DerivedType*>(this)->find(key);
    }

    void clear() {
        return static_cast<DerivedType*>(this)->clear();
    }


    bool isEmpty() const {
        return static_cast<DerivedType*>(this)->isEmpty();
    }

    size_t getSize() const {
        return static_cast<DerivedType*>(this)->getSize();
    }


    iterator begin() {
        return static_cast<DerivedType*>(this)->begin();
    }

    iterator end() {
        return static_cast<DerivedType*>(this)->end();
    }

    // operator<< uses iterator so it is the similar for all tables
    friend std::ostream& operator<<(std::ostream& ostr, Table& table) {
        for (iterator it = table.begin(); it != table.end(); ++it)
            ostr << "(" << it->first << ", " << it->second << ")" << std::endl;
        return ostr;
    }

protected:

    using TableType = Table<ElemType, IteratorType, DerivedType>;

};


// it is base class for UnsortedTable, SortedTable and all hash tables
// CellType is a type of one cell of table
// by default it is std::pair<key, value> (for UnsortedTable and SortedTable)
// or it is std::list<std::pair<key, value>> or std::pair<std::pair<key, value>, label> for hash tables 
template <class ElemType, class IteratorType, class DerivedType,
    class CellType = std::pair<KeyType, ElemType>>
class TableByArray : public Table<ElemType, IteratorType, DerivedType> {

public:

    TableByArray(size_t size = 0) : storage(size) {}

    void clear() {
        std::vector<CellType> tmp;
        std::swap(tmp, storage);
    }

    size_t getSize() const {
        return storage.size();
    }

    bool isEmpty() const {
        return storage.size() == 0;
    }

    
protected:

    using TableByArrayType = TableByArray<ElemType, IteratorType, DerivedType, CellType>;

    std::vector<CellType> storage;

};