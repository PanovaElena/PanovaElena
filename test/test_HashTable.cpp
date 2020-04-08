#include "HashTable.h"
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "gtest/gtest-typed-test.h"

template <class HashTableTestType>
class TestHashTable : public HashTableTestType, public testing::Test {

public:

    HashTableTestType* table = this;
    std::vector<KeyType> collisionKeys, notCollisionKeys;

    TestHashTable() : HashTableTestType(3) {  // capacity = 2^3
        this->a = 1;  // in this case there are a lot of collisions
        collisionKeys = { 0, 1, 2, 3, 4, 5 };  // give collisions
        notCollisionKeys = {
            size_t(1) << (W - M),
            size_t(2) << (W - M),
            size_t(3) << (W - M),
            size_t(4) << (W - M),
            size_t(5) << (W - M),
            size_t(6) << (W - M)
        };  // do not give collisions
    }

};

// run typed tests
// this is run all next tests for all hash table types

TYPED_TEST_SUITE_P(TestHashTable);


TYPED_TEST_P(TestHashTable, can_insert_and_find_first_element_if_collision) {
    for (int i = 0; i < 3; i++)
        table->insert(collisionKeys[0], char('a' + i));

    ASSERT_EQ('a', table->find(collisionKeys[0])->second);
}

TYPED_TEST_P(TestHashTable, can_insert_and_find_second_element_if_collision) {
    for (int i = 0; i < 3; i++)
        table->insert(collisionKeys[i], char('a' + i));

    ASSERT_EQ('b', table->find(collisionKeys[1])->second);
}

TYPED_TEST_P(TestHashTable, can_insert_and_find_third_element_if_collision) {
    for (int i = 0; i < 3; i++)
        table->insert(collisionKeys[i], char('a' + i));

    ASSERT_EQ('c', table->find(collisionKeys[2])->second);
}

TYPED_TEST_P(TestHashTable, can_find_third_element_if_collision_and_first_one_is_erased) {
    for (int i = 0; i < 3; i++)
        table->insert(collisionKeys[i], char('a' + i));
    table->erase(collisionKeys[0]);

    ASSERT_EQ('c', table->find(collisionKeys[2])->second);
}

TYPED_TEST_P(TestHashTable, can_find_third_element_if_collision_and_second_one_is_erased) {
    for (int i = 0; i < 3; i++)
        table->insert(collisionKeys[i], char('a' + i));
    table->erase(collisionKeys[1]);

    ASSERT_EQ('c', table->find(collisionKeys[2])->second);
}

TYPED_TEST_P(TestHashTable, cannot_find_element_if_it_is_erased) {
    for (int i = 0; i < 3; i++)
        table->insert(collisionKeys[i], char('a' + i));
    table->erase(collisionKeys[1]);

    ASSERT_EQ(table->end(), table->find(collisionKeys[1]));
}

TYPED_TEST_P(TestHashTable, can_repack_table_if_it_is_almost_filled) {
    // after 6 insertions repack should be called
    size_t size = storage.size();
    for (int i = 0; i < 5; i++)
        table->insert(notCollisionKeys[i], char('a' + i));

    table->insert(notCollisionKeys[5], char('a' + 5));

    ASSERT_GT(storage.size(), size);
}

TYPED_TEST_P(TestHashTable, repack_dont_break_table) {
    for (int i = 0; i < 3; i++)
        table->insert(collisionKeys[i], char('a' + i));

    auto findRes = table->find(collisionKeys[4]);  // repack is called

    for (int i = 0; i < 3; i++)
        ASSERT_EQ(char('a' + i), table->find(collisionKeys[i])->second);
    ASSERT_EQ(table->end(), table->find(collisionKeys[4]));
    ASSERT_EQ(3, table->getSize());
    ASSERT_FALSE(table->isEmpty());
}

TYPED_TEST_P(TestHashTable, hash_table_is_iterable) {
    for (int i = 0; i < 5; i++)
        table->insert(collisionKeys[i], 'a');
    for (int i = 0; i < 5; i++)
        table->insert(notCollisionKeys[i], 'a');

    int ch = 0;
    for (auto it = table->begin(); it != table->end(); ++it, ++ch) {
        ASSERT_EQ(it->first, (*it).first);
        ASSERT_EQ(it->second, 'a');
    }
    ASSERT_EQ(10, ch);
}

TYPED_TEST_P(TestHashTable, hash_table_is_iterable_2) {
    for (int i = 0; i < 5; i++)
        table->insert(notCollisionKeys[i], 'a');

    int ch = 0;
    for (auto it = table->begin(); it != table->end(); ++it, ++ch) {
        ASSERT_EQ(it->first, (*it).first);
        ASSERT_EQ(it->second, 'a');
    }
    ASSERT_EQ(5, ch);
}

REGISTER_TYPED_TEST_SUITE_P(TestHashTable,
    can_insert_and_find_first_element_if_collision,
    can_insert_and_find_second_element_if_collision,
    can_insert_and_find_third_element_if_collision,
    can_find_third_element_if_collision_and_first_one_is_erased,
    can_find_third_element_if_collision_and_second_one_is_erased,
    cannot_find_element_if_it_is_erased,
    can_repack_table_if_it_is_almost_filled,
    repack_dont_break_table,
    hash_table_is_iterable,
    hash_table_is_iterable_2
);

typedef ::testing::Types<HashTableOpenAddressing<char>, HashTableSeparateChaining<char>> TestHashTableTypes;
INSTANTIATE_TYPED_TEST_SUITE_P(Test, TestHashTable, TestHashTableTypes);


typedef TestHashTable<HashTableOpenAddressing<char>> TestHashTableOpenAddressing;

TEST_F(TestHashTableOpenAddressing, can_repack_table_if_insert_is_called_and_empty_cell_didnt_find) {
    // after 4 insertions with collisions repack should be called
    size_t size = storage.size();
    for (int i = 0; i < 3; i++)
        table->insert(collisionKeys[i], char('a' + i));

    table->insert(collisionKeys[3], char('a' + 3));

    ASSERT_GT(storage.size(), size);
}