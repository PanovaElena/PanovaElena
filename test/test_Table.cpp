#include <utility>
#include <string>
#include <vector>

#include "SortedTable.h"
#include "UnsortedTable.h"
#include "HashTable.h"

#include "gtest/gtest.h"

// macro to run a test for all types of search tables
// defines name "TableType" as a type of table inside of test body
// typed tests from google tests can be used instead (look test_HashTable.cpp)
#define TEST_FOR_ALL_TABLES(test_case, test_name)                                              \
template <template<class> class TableType> void func##test_case##test_name();                  \
TEST(test_case##UnsortedTable, test_name) {                                                    \
    func##test_case##test_name<UnsortedTable>();                                               \
}                                                                                              \
TEST(test_case##SortedTable, test_name) {                                                      \
    func##test_case##test_name<SortedTable>();                                                 \
}                                                                                              \
TEST(test_case##HashTableOpenAddressing, test_name) {                                          \
    func##test_case##test_name<HashTableOpenAddressing>();                                     \
}                                                                                              \
TEST(test_case##HashTableSeparateChaining, test_name) {                                        \
    func##test_case##test_name<HashTableSeparateChaining>();                                   \
}                                                                                              \
template <template<class> class TableType>                                                     \
void func##test_case##test_name()



TEST_FOR_ALL_TABLES(TestCommon, dont_throw_exception_when_insert) {
    TableType<std::string> table;

    ASSERT_NO_THROW(table.insert(1, "a"));
}

TEST_FOR_ALL_TABLES(TestCommon, dont_throw_exception_when_find) {
    TableType<std::string> table;
    table.insert(1, "a");

    ASSERT_NO_THROW(table.find(1));
}

TEST_FOR_ALL_TABLES(TestCommon, can_insert_and_find_element) {
    TableType<std::string> table;
    table.insert(1, "a");

    ASSERT_NE(table.find(1), table.end());
}

TEST_FOR_ALL_TABLES(TestCommon, insert_and_find_gives_correct_iterators) {
    TableType<std::string> table;
    table.insert(1, "a");

    auto insRes = table.insert(2, "b");
    auto iterFind = table.find(2);

    ASSERT_EQ(insRes.first, iterFind);
}

TEST_FOR_ALL_TABLES(TestCommon, cant_find_default_key_if_table_is_empty) {
    TableType<std::string> table;

    ASSERT_EQ(table.find(0), table.end());
}

TEST_FOR_ALL_TABLES(TestCommon, insert_existend_key_gives_false) {
    TableType<std::string> table;
    table.insert(1, "a");

    auto insRes = table.insert(1, "b");

    ASSERT_FALSE(insRes.second);
}

TEST_FOR_ALL_TABLES(TestCommon, dont_throw_exception_when_erase_by_key) {
    TableType<std::string> table;
    table.insert(1, "a");

    ASSERT_NO_THROW(table.erase(1));
}

TEST_FOR_ALL_TABLES(TestCommon, can_erase_element) {
    TableType<std::string> table;
    table.insert(1, "a");

    table.erase(1);

    ASSERT_EQ(table.find(1), table.end());
}

TEST_FOR_ALL_TABLES(TestCommon, erase_nonexistend_key_gives_false) {
    TableType<std::string> table;
    table.insert(1, "a");

    ASSERT_FALSE(table.erase(2));
}

TEST_FOR_ALL_TABLES(TestCommon, can_erase_by_iterator) {
    TableType<std::string> table;
    auto resIns = table.insert(1, "a");

    table.eraseWithoutSearch(resIns.first);

    ASSERT_TRUE(table.isEmpty());
}

TEST_FOR_ALL_TABLES(TestCommon, can_insert_by_copying) {
    TableType<std::vector<int>> table;
    std::vector<int> v = { 2, 6, 7 };

    table.insert(1, v);

    ASSERT_EQ(table.find(1)->second[1], 6);
    ASSERT_FALSE(v.empty());
}

TEST_FOR_ALL_TABLES(TestCommon, can_insert_by_moving) {
    TableType<std::vector<int>> table;
    std::vector<int> v = { 2, 6, 7 };

    table.insert(1, std::move(v));

    ASSERT_EQ(table.find(1)->second[1], 6);
    ASSERT_TRUE(v.empty());
}

TEST_FOR_ALL_TABLES(TestCommon, can_print_table) {
    TableType<std::string> table;
    table.insert(1, "a");
    table.insert(4, "b");
    table.insert(2, "c");
    table.insert(15, "d");
    table.erase(4);
    table.insert(0, "e");

    ASSERT_NO_THROW(std::cout << table << std::endl);
}

TEST_FOR_ALL_TABLES(TestCommon, table_is_iterable) {
    TableType<std::string> table;
    table.insert(1, "a");
    table.insert(4, "b");
    table.insert(2, "c");
    table.insert(15, "d");

    for (auto it = table.begin(); it != table.end(); ++it)
        ASSERT_EQ(it->second, (*it).second);
}

TEST_FOR_ALL_TABLES(TestCommon, can_clear_table) {
    TableType<std::string> table;
    table.insert(1, "a");
    table.insert(4, "b");
    table.insert(2, "c");
    table.insert(15, "d");

    table.clear();

    ASSERT_TRUE(table.isEmpty());
}
