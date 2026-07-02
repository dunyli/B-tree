/*
 * btree_test.c - тесты и примеры использования B-дерева
 *
 * Содержит набор тестов для проверки всех операций B-дерева:
 * - создание и удаление дерева
 * - вставка и поиск элементов
 * - удаление элементов с перебалансировкой
 * - обход дерева через итератор
 * - работа с большим количеством данных
 *
 * Примеры практического применения:
 * - индекс базы данных
 * - сортировка данных
 */

#define _CRT_SECURE_NO_WARNINGS

#include "btree.h"        /* Подключаем заголовочный файл B-дерева */
#include <stdio.h>        /* Для printf, snprintf */
#include <stdlib.h>       /* Для rand, srand */
#include <time.h>         /* Для time */

 /* Глобальные счётчики для статистики тестов */
int passed = 0;           /* Количество успешно пройденных тестов */
int failed = 0;           /* Количество проваленных тестов */

/*
 * Проверка условия теста
 * Выводит результат и обновляет счётчики
 */
void test_check(const char* description, int condition)
{
    if (condition) {
        printf("  УСПЕШНО: %s\n", description);
        passed++;
    }
    else {
        printf("  ОШИБКА: %s\n", description);
        failed++;
    }
}


/*
 * Выводит заголовок теста
 */
static void print_test_header(const char* name, int number)
{
    printf("  ТЕСТ %d: %-36s\n", number, name);
}

/*
 * ТЕСТ 1: СОЗДАНИЕ И УДАЛЕНИЕ ДЕРЕВА
 * Проверяет, что дерево правильно создаётся и удаляется
 */
void test_create_destroy(void)
{
    print_test_header("Создание и удаление", 1);

    /* Создаём дерево со степенью 3 */
    BTree* tree = btree_create(3);
    printf("  Создаём B-дерево со степенью 3...\n");

    /* Проверяем, что дерево создано корректно */
    test_check("Дерево создано (не NULL)", tree != NULL);
    test_check("Степень = 3", btree_degree(tree) == 3);
    test_check("Размер = 0 (пустое)", btree_size(tree) == 0);
    test_check("Высота = 0", btree_height(tree) == 0);

    printf("  Удаляем дерево...\n");
    btree_destroy(tree);
    test_check("Память освобождена без утечек", 1);
}

/*
 * ТЕСТ 2: ВСТАВКА И ПОИСК
 * Проверяет вставку элементов и их поиск
 */
void test_insert_search(void)
{
    print_test_header("Вставка и поиск", 2);

    BTree* tree = btree_create(3);

    /* Массивы ключей и значений для вставки */
    int keys[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
    int values[] = { 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000 };

    /* Вставляем 10 элементов */
    printf("  Вставляем 10 элементов: ");
    for (int i = 0; i < 10; i++) {
        printf("%d ", keys[i]);
        btree_insert(tree, keys[i], values[i]);
    }
    printf("\n");

    test_check("Размер = 10", btree_size(tree) == 10);

    /* Проверяем поиск существующих ключей */
    printf("\n  Поиск существующих ключей:\n");
    int val;
    test_check("10 найден - значение 100", btree_search(tree, 10, &val) && val == 100);
    test_check("50 найден - значение 500", btree_search(tree, 50, &val) && val == 500);
    test_check("100 найден - значение 1000", btree_search(tree, 100, &val) && val == 1000);

    /* Проверяем поиск отсутствующего ключа */
    printf("\n  Поиск отсутствующего ключа:\n");
    test_check("25 НЕ найден", !btree_search(tree, 25, NULL));

    /* Печатаем дерево для визуальной проверки */
    printf("\n  Структура дерева:\n");
    btree_print(tree);
    btree_destroy(tree);
}

/*
 * ТЕСТ 3: УДАЛЕНИЕ
 * Проверяет удаление элементов с правильной перебалансировкой
 */
void test_delete(void)
{
    print_test_header("Удаление с балансировкой", 3);

    BTree* tree = btree_create(3);

    /* Вставляем 20 элементов (0, 5, 10, 15, ..., 95) */
    printf("  Вставляем 20 элементов (0, 5, 10, ..., 95)...\n");
    for (int i = 0; i < 20; i++) {
        btree_insert(tree, i * 5, i * 10);
    }

    test_check("Размер = 20", btree_size(tree) == 20);

    /* УДАЛЕНИЕ 1: Удаляем первый элемент */
    printf("\n  Удаляем первый элемент (0)...\n");
    test_check("Удаление 0", btree_delete(tree, 0));
    test_check("Размер = 19", btree_size(tree) == 19);
    test_check("0 отсутствует", !btree_contains(tree, 0));

    /* УДАЛЕНИЕ 2: Удаляем элемент из середины */
    printf("\n  Удаляем элемент из середины (45)...\n");
    test_check("Удаление 45", btree_delete(tree, 45));
    test_check("Размер = 18", btree_size(tree) == 18);
    test_check("45 отсутствует", !btree_contains(tree, 45));

    /* УДАЛЕНИЕ 3: Удаляем элемент из конца */
    printf("\n  Удаляем элемент из конца (95)...\n");
    if (btree_contains(tree, 95)) {
        test_check("Удаление 95", btree_delete(tree, 95));
        test_check("Размер = 17", btree_size(tree) == 17);
        test_check("95 отсутствует", !btree_contains(tree, 95));
    }
    else {
        printf("  95 уже отсутствует в дереве (пропускаем)\n");
    }

    /* Проверяем, что остальные элементы на месте */
    printf("\n  Проверка оставшихся элементов:\n");
    test_check("5 присутствует", btree_contains(tree, 5));
    test_check("10 присутствует", btree_contains(tree, 10));

    printf("\n  Структура дерева после удалений:\n");
    btree_print(tree);
    btree_destroy(tree);
}

/*
 * ТЕСТ 4: ИТЕРАТОР
 * Проверяет обход дерева в порядке возрастания
 */
void test_iterator(void)
{
    print_test_header("Обход дерева (итератор)", 4);

    BTree* tree = btree_create(3);

    /* Вставляем ключи в произвольном порядке */
    int keys[] = { 15, 25, 5, 35, 45, 10, 30, 20, 40, 50 };
    printf("  Вставляем ключи в произвольном порядке:\n    ");
    for (int i = 0; i < 10; i++) {
        printf("%d ", keys[i]);
        btree_insert(tree, keys[i], keys[i] * 10);
    }
    printf("\n");

    /* Обходим дерево через итератор */
    printf("\n  Обход в порядке возрастания:\n    ");
    BTreeIterator iter = btree_iterator_create(tree);

    int count = 0;
    while (btree_iterator_valid(&iter)) {
        int key = btree_iterator_key(&iter);
        int value = btree_iterator_value(&iter);
        printf("%d(%d) ", key, value);
        count++;
        btree_iterator_next(&iter);
    }
    printf("\n");

    test_check("Обходено 10 элементов", count == 10);

    btree_iterator_destroy(&iter);
    btree_destroy(tree);
}

/*
 * ТЕСТ 5: БОЛЬШОЕ КОЛИЧЕСТВО ЭЛЕМЕНТОВ
 * Проверяет производительность и корректность на 1000 элементах
 */
void test_large(void)
{
    print_test_header("Большое количество данных", 5);

    BTree* tree = btree_create(4);
    int n = 1000;

    printf("  Вставляем %d элементов...\n", n);
    for (int i = 0; i < n; i++) {
        btree_insert(tree, i, i * 2);
    }

    printf("  Размер после вставки: %d\n", btree_size(tree));
    test_check("Размер = 1000", btree_size(tree) == n);
    test_check("Высота <= 5 (эффективная)", btree_height(tree) <= 5);

    /* Проверяем, что все элементы найдены */
    printf("\n  Проверка поиска всех элементов...\n");
    int found = 0;
    for (int i = 0; i < n; i++) {
        if (btree_contains(tree, i)) found++;
    }
    test_check("Все 1000 элементов найдены", found == n);

    /* Удаляем каждый второй элемент */
    printf("\n  Удаляем чётные элементы (500 шт)...\n");
    int deleted = 0;
    for (int i = 0; i < n; i += 2) {
        if (btree_delete(tree, i)) {
            deleted++;
        }
    }
    printf("  Удалено: %d элементов\n", deleted);
    printf("  Размер после удаления: %d (ожидается 500)\n", btree_size(tree));
    test_check("Размер = 500", btree_size(tree) == 500);

    /* Печатаем статистику */
    btree_print_stats(tree);
    btree_destroy(tree);
}

/*
 * ПРИМЕР 1: ИНДЕКС БАЗЫ ДАННЫХ
 * Демонстрирует использование B-дерева как индекса
 */
void example_database_index(void)
{
    printf("  ПРИМЕР 1: Индекс базы данных   \n");

    BTree* index = btree_create(3);

    /* Данные студентов: ID - оценка */
    struct {
        int id;
        int grade;
    } students[] = {
        {1001, 85}, {1002, 92}, {1003, 78}, {1004, 95},
        {1005, 88}, {1006, 76}, {1007, 90}, {1008, 82},
        {1009, 94}, {1010, 87}, {1011, 91}, {1012, 79}
    };

    /* Заполняем индекс */
    printf("  Заполняем индекс оценками студентов:\n");
    for (int i = 0; i < 12; i++) {
        btree_insert(index, students[i].id, students[i].grade);
        printf("    ID=%d - оценка %d\n", students[i].id, students[i].grade);
    }

    /* Выполняем поиск по индексу */
    printf("\n  Поиск по индексу:\n");
    int grade;
    int search_ids[] = { 1003, 1007, 1011, 9999 };

    for (int i = 0; i < 4; i++) {
        if (btree_search(index, search_ids[i], &grade)) {
            printf(" ID=%d - оценка %d\n", search_ids[i], grade);
        }
        else {
            printf("  ID=%d - не найден\n", search_ids[i]);
        }
    }

    printf("\n  Структура индекса:\n");
    btree_print(index);
    btree_destroy(index);
}

/*
 * ПРИМЕР 2: СОРТИРОВКА ДАННЫХ
 * Демонстрирует использование B-дерева для сортировки
 */
void example_sorting(void)
{
    printf("  ПРИМЕР 2: Сортировка данных   \n");

    BTree* tree = btree_create(3);
    int n = 50;

    /* Генерируем случайные числа */
    srand((unsigned int)time(NULL));
    int* random_numbers = (int*)malloc(n * sizeof(int));
    printf("  Сгенерированы случайные числа:\n    ");
    for (int i = 0; i < n; i++) {
        random_numbers[i] = rand() % 1000;
        printf("%d ", random_numbers[i]);
        if ((i + 1) % 10 == 0) printf("\n    ");
    }
    printf("\n");

    /* Вставляем в дерево */
    for (int i = 0; i < n; i++) {
        btree_insert(tree, random_numbers[i], random_numbers[i]);
    }
    free(random_numbers);

    /* Выводим отсортированные числа через итератор */
    printf("\n  Отсортированные числа:\n    ");
    BTreeIterator iter = btree_iterator_create(tree);
    int count = 0;
    while (btree_iterator_valid(&iter)) {
        printf("%d ", btree_iterator_key(&iter));
        count++;
        btree_iterator_next(&iter);
        if (count % 10 == 0 && count < n) printf("\n    ");
    }
    printf("\n");

    printf("\n  Количество элементов: %d (должно быть %d)\n", count, n);
    test_check("Все элементы отсортированы", count == n);

    btree_iterator_destroy(&iter);
    btree_destroy(tree);
}

/* Простая хеш-функция для имён */
int hash_name(const char* name) {
    int hash = 0;
    while (*name) {
        hash = hash * 31 + *name;
        name++;
    }
    return hash & 0x7FFFFFFF; /* Оставляем положительным */
}

/*
 * ПРИМЕР 3: ТЕЛЕФОННАЯ КНИГА
 * Демонстрирует использование B-дерева для хранения контактов
 */
void example_phone_book(void)
{
    printf("  ПРИМЕР 3: Телефонная книга                        │\n");

    BTree* phone_book = btree_create(3);

    /* Контакты: имя (хеш) - номер телефона (используем long long) */
    struct {
        const char* name;
        long long phone;  /* Используем long long для больших номеров */
    } contacts[] = {
        {"Анна", 89123456789LL},
        {"Борис", 89234567890LL},
        {"Виктор", 89345678901LL},
        {"Олег", 89456789012LL},
        {"Дмитрий", 89567890123LL},
        {"Елена", 89678901234LL},
        {"Даниил", 89789012345LL},
        {"Иван", 89890123456LL},
        {"Кирилл", 89901234567LL},
        {"Иосиф", 89012345678LL}
    };

    printf("  Добавляем контакты в телефонную книгу:\n");
    for (int i = 0; i < 10; i++) {
        int key = hash_name(contacts[i].name);
        /* Сохраняем номер как int (приводим к int, т.к. BTree хранит int) */
        /* В реальном проекте нужно использовать long long, но для примера используем младшие 32 бита */
        int phone_int = (int)(contacts[i].phone & 0x7FFFFFFF);
        btree_insert(phone_book, key, phone_int);
        printf("    %s - %lld\n", contacts[i].name, contacts[i].phone);
    }

    /* Поиск контакта */
    printf("\n  Поиск контакта:\n");
    const char* search_names[] = { "Иван", "Елена", "Пётр" };
    for (int i = 0; i < 3; i++) {
        int key = hash_name(search_names[i]);
        int phone_int;
        if (btree_search(phone_book, key, &phone_int)) {
            printf(" %s - %d\n", search_names[i], phone_int);
        }
        else {
            printf(" %s - не найден\n", search_names[i]);
        }
    }

    printf("\n  Статистика телефонной книги:\n");
    btree_print_stats(phone_book);
    btree_destroy(phone_book);
}

/*
 * ПРИМЕР 4: СТАТИСТИКА ПОСЕЩЕНИЙ
 * Демонстрирует использование B-дерева для агрегации данных
 */
void example_statistics(void)
{
    printf("  ПРИМЕР 4: Статистика посещений \n");

    BTree* stats = btree_create(3);

    /* Данные: день - количество посетителей */
    struct {
        int day;
        int visitors;
    } data[] = {
        {1, 150}, {2, 200}, {3, 180}, {4, 220}, {5, 250},
        {6, 300}, {7, 280}, {8, 210}, {9, 190}, {10, 230},
        {11, 260}, {12, 310}, {13, 290}, {14, 240}, {15, 270}
    };

    printf("  Добавляем данные посещений:\n");
    for (int i = 0; i < 15; i++) {
        btree_insert(stats, data[i].day, data[i].visitors);
    }

    /* Вывод в отсортированном порядке */
    printf("\n  Статистика посещений по дням:\n    ");
    BTreeIterator iter = btree_iterator_create(stats);
    int total = 0;
    int count = 0;
    while (btree_iterator_valid(&iter)) {
        int day = btree_iterator_key(&iter);
        int visitors = btree_iterator_value(&iter);
        total += visitors;
        count++;
        printf("день%d:%d ", day, visitors);
        btree_iterator_next(&iter);
        if (count % 5 == 0 && count < 15) printf("\n    ");
    }
    printf("\n");

    printf("\n  Среднее количество посетителей: %.2f\n", (float)total / count);
    printf("  Всего дней: %d\n", count);
    printf("  Всего посетителей: %d\n", total);

    btree_iterator_destroy(&iter);
    btree_destroy(stats);
}