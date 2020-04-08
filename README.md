# Поисковые таблицы

Реализация следующих видов поисковых таблиц:
- упорядоченная таблица;
- неупорядоченная таблица;
- хеш-таблица с разрешением коллизий методом цепочек;
- хеш-таблица с разрешением коллизий методом открытой адресации (квадратичное пробирование).
    
## Коротко о реализации

1. Сборка с помощью cmake.

2. На все случаи жизни имеются тесты.

3. Используются контейнеры (`std::vector`, `std::list`) и некоторые алгоритмы (бинарный поиск) из STL.

4. Широко используются шаблоны. Таблицы шаблонные, однако ключи только типа uint32.

5. Для каждой таблицы реализован итератор. Для, например, неупорядоченных таблиц можно создать итератор на начало/конец следующими способами.

```
typename UnsortedTable<int>::iterator it1 = table.begin();
auto it2 = table.begin();
typename UnsortedTable<int>::iterator it3 = table.end();
auto it4 = table.end();
```

Итераторы наследуются от std::iterator (т.е. могут использоваться в стандартных алгоритмах) и поддерживают, как минимум, интерфейс InputIterator.
   
## Некоторые интересные моменты

### Паттерн CRTP

В коде (см. include/Table.h) используется паттерн **CRTP (Curiously recurring template pattern, Странно повторяющийся шаблон)** для имитации виртуальных функций (https://ru.wikipedia.org/wiki/Curiously_recurring_template_pattern). Проиллюстрирую на примере.

```
class Base {
public:
    virtual std::string getClassName() = 0;
    void printHello() {
        std::cout << "Hello from " << getClassName() <<std::endl;
    }
};

class Derived : public Base {
public:
    std::string getClassName() override {
        return "Derived";
    }
};

int main() {
    Derived a;
    a.printHello(); // "Hello from Derived"
}
```

Чтобы избежать дополнительных затрат на поддержку динамического полиморфизма, можно сделать так.

```
template <class TDerived>
class Base {
public:
    std::string getClassName() {
        static_cast<TDerived*>(this)->getClassName();
    }
    void printHello() {
        std::cout << "Hello from " << getClassName() <<std::endl;
    }
};

class Derived : public Base<Derived>{
public:
    std::string getClassName() {
        return "Derived";
    }
};
```    

Мы увеличили время компиляции, но избавились от динамического полиморфизма, т.е. уменьшили runtime. Это может играть большую роль, если мы вызываем функцию `getClassName()` в цикле очень много раз.

Для того, чтобы избежать инстанцирования шаблона классом, не являющимся наследником Base, можно использовать `static_assert` https://en.cppreference.com/w/cpp/language/static_assert.

```
static_assert(std::is_base_of<Base, TDerived>::value, "Derived class is not of Base type");
```

Шаблонная структура `std::is_base_of` определена в заголовочном файле `<type_traits>` (https://ru.cppreference.com/w/cpp/header/type_traits) и используется для определения на этапе компиляции, является ли TDerived наследником Base. Если да, то в структуре будет определена статическая константа `value=true`. Также в `<type_traits>` определено много других полезных структур подобного рода, например, `enable_if` https://ru.cppreference.com/w/cpp/types/enable_if. Это все очень крутые вещи, советую почитать.
    
### Семантика перемещения.

Рассмотрим пример:

```
UnsortedTable<std::vector<int>> table;
table.insert(std::vector<int>(10000, 7));
```

Здесь мы кладем в таблицу вектор из 10000 семерок. Что при происходит:
1. создается временный объект `std::vector<int>(10000, 7)`;
2. он передается по ссылке в функцию `insert` с сигнатурой, например, `void UnsortedTable<T>::insert(const T& elem)`;
3. и записывается в функции в конец массива storage - поля класса UnsortedTable<T> `storage\[size\] = elem;`.
    
В пункте 3 по идее должен вызываться `operator=` для `std::vector`, который копирует вектор целиком, после чего при выходе из `insert` временный объект должен уничтожиться. Очевидно, что это лишние затраты, которых можно было бы избежать без каких-либо потерь. Современные компиляторы в таких ситуациях просто перемещают временный объект в контейнер без лишнего копирования. Но что делать, если нужно создать вектор, проинициализировать его некоторым нетривиальным образом, а потом положить его в конейнер? Казалось бы, в следующем коде без копирования не обойтись:

```
UnsortedTable<std::vector<int>> table;
std::vector<int> v(10000);
for (int i = 0; i < 10000; i++)
    v[i] = someFunc(i);
table.insert(v);
```

Однако начиная с С++11 можно написать так:

```
UnsortedTable<std::vector<int>> table;
std::vector<int> v(10000);
for (int i = 0; i < 10000; i++)
    v[i] = someFunc(i);
table.insert(std::move(v));
```

И копирования не произойдет, вектор будет перемещен. После выполнения этого кода в таблице будет лежать проинициализированный вектор, а переменная `v` будет пустым вектором нулевого размера.

Для того, чтобы такая магия была возможной, нужно правильно реализовать как объект, который кладем в контейнер (нужен конструктор перемещения и оператор присваивания перемещением, std::vector реализован правильно, ура), так и функцию вставки (в коде есть по 2 функции `insert` - для копирования и для перемещения). Как именно это сделать, хорошо написано здесь https://habr.com/ru/post/226229/. В статье используются термины rvalue и lvalue, про них подробно можно почитать тут https://habr.com/ru/post/348198/.

### Тестирование

Для всех таблиц тесты выглядят примерно одинаково. Google test позволяет запускать один тест для шаблонной фикстуры, проинстанцированной различными типами (макрос `TYPED_TEST_P`). Как это делается, продемонстрировано в файле test/test_HashTable.cpp.

Также тест может быть параметризован различными значениями. Например, можно написать один тест с использованием некоторого целого параметра *a* и запустить его для *a=1, 10, 145, ...* (макрос `TEST_P`).

Подробно про все это можно почитать в документации к google test (https://github.com/google/googletest/blob/master/googletest/docs/advanced.md и другие файлы в папке doc).

Также можно писать свои макросы для тестирования. Например, есть следующие тесты:

```
TEST(TestUnsortedTable, dont_throw_exception_when_insert) {
    UnsortedTable<std::string> table;

    ASSERT_NO_THROW(table.insert(1, "a"));
}

TEST(TestSortedTable, dont_throw_exception_when_insert) {
    SortedTable<std::string> table;

    ASSERT_NO_THROW(table.insert(1, "a"));
}

TEST(TestHashTable, dont_throw_exception_when_insert) {
    HashTable<std::string> table;

    ASSERT_NO_THROW(table.insert(1, "a"));
}
```

Тесты с почти одинаковым телом, отличие только в типе таблицы. Хотелось бы избежать повторов. Можно было бы использовать `TYPED_TEST_P`, но следующее решение, на мой взгляд, даже красивее (test/test_Table.cpp).

```
#define TEST_FOR_ALL_TABLES(test_case, test_name)                                              \
template <template<class> class TableType> void func##test_case##test_name();                  \
TEST(test_case##UnsortedTable, test_name) {                                                    \
    func##test_case##test_name<UnsortedTable>();                                               \
}                                                                                              \
TEST(test_case##SortedTable, test_name) {                                                      \
    func##test_case##test_name<SortedTable>();                                                 \
}                                                                                              \
TEST(test_case##HashTable, test_name) {                                                        \
    func##test_case##test_name<HashTableOpenAddressing>();                                     \
}                                                                                              \                                                                                            \
template <template<class> class TableType>                                                     \
void func##test_case##test_name()


TEST_FOR_ALL_TABLES(Test, dont_throw_exception_when_insert) {
    TableType<std::string> table;

    ASSERT_NO_THROW(table.insert(1, "a"));
}
```

Макрос многострочный, поэтому в конце каждой строки, кросе последней, при определении макроса стоит слеш. Конструкция `a##b` просто соединяет строки в одну: `ab`, при этом выполняя подстановку аргумента макроса, где это необходимо.
 
Итак, мы определили макрос, который будет для нашего теста развернут препроцессором в следующий код (конечно, макросы `TEST` будут тоже развернуты):

```
template <template<class> class TableType> void funcTestdont_throw_exception_when_insert();
TEST(TestUnsortedTable, dont_throw_exception_when_insert) {
    funcTestdont_throw_exception_when_insert<UnsortedTable>();
}
TEST(TestSortedTable, dont_throw_exception_when_insert) {
    funcTestdont_throw_exception_when_insert<SortedTable>();
}
TEST(TestHashTable, dont_throw_exception_when_insert) {
    funcTestdont_throw_exception_when_insert<HashTableOpenAddressing>();
}
template <template<class> class TableType>
void funcTestdont_throw_exception_when_insert() {
    TableType<std::string> table;

    ASSERT_NO_THROW(table.insert(1, "a"));
}
```

Получается, будто мы объявили шаблонную функцию `funcTestdont_throw_exception_when_insert`, потом вызвали ее из трех разных тестов, а в конце определили. Красиво! Как-то так на самом деле и написана библиотека google test. Можно определить свои ASSERT при желании. Но злоупотреблять всем этим не стоит, потому что макросы - зло :)
  
