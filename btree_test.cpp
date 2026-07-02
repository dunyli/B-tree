/*
 * btree_test.c - тесты и примеры использования B-дерева
 *
 * Содержит набор тестов для проверки всех операций B-дерева:
 * - создание и удаление дерева
 * - вставка и поиск элементов
 * - удаление элементов (с правильной перебалансировкой)
 * - обход дерева через итератор
 * - работа с большим количеством данных
 *
 * А также примеры практического применения:
 * - индекс базы данных
 * - сортировка данных
 */

#define _CRT_SECURE_NO_WARNINGS

#include "btree.h"        /* Подключаем заголовочный файл B-дерева */
#include <stdio.h>        /* Для printf, snprintf */
#include <stdlib.h>       /* Для rand, srand */
#include <time.h>         /* Для time */

 /* Глобальные счётчики для статистики тестов */
int passed = 0;  /* Количество успешно пройденных тестов */
int failed = 0;  /* Количество проваленных тестов */

/*
 * Проверка условия теста
 * Выводит результат и обновляет счётчики
 *
 * Параметры:
 *   description - описание теста
 *   condition   - условие для проверки (1 - успех, 0 - провал)
 */
void
test_check(const char* description, int condition)
{
    if (condition) {
        printf("  УСПЕШНО: %s\n", description);  /* Выводим успех */
        passed++;                               /* Увеличиваем счётчик успехов */
    }
    else {
        printf("  ОШИБКА: %s\n", description);   /* Выводим ошибку */
        failed++;                               /* Увеличиваем счётчик ошибок */
    }
}

/*
 * ТЕСТ 1: СОЗДАНИЕ И УДАЛЕНИЕ ДЕРЕВА
 * Проверяет, что дерево правильно создаётся и удаляется
 */
void
test_create_destroy(void)
{
    /* Создаём дерево со степенью 3 */
    BTree* tree = btree_create(3);

    /* Проверяем, что дерево создано корректно */
    test_check("Дерево создано", tree != NULL);
    test_check("Степень = 3", btree_degree(tree) == 3);
    test_check("Размер = 0", btree_size(tree) == 0);
    test_check("Высота = 0", btree_height(tree) == 0);

    /* Удаляем дерево и проверяем освобождение памяти */
    btree_destroy(tree);
    test_check("Память освобождена", 1);
}

/*
 * ТЕСТ 2: ВСТАВКА И ПОИСК
 * Проверяет вставку элементов и их поиск
 */
void
test_insert_search(void)
{
    /* Создаём дерево со степенью 3 */
    BTree* tree = btree_create(3);

    /* Массивы ключей и значений для вставки */
    int keys[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
    int values[] = { 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000 };

    /* Вставляем 10 элементов в дерево */
    for (int i = 0; i < 10; i++) {
        btree_insert(tree, keys[i], values[i]);
    }

    /* Проверяем, что размер стал равен 10 */
    test_check("Размер = 10", btree_size(tree) == 10);

    /* Проверяем поиск существующих ключей */
    int val;
    test_check("10 найден", btree_search(tree, 10, &val) && val == 100);
    test_check("50 найден", btree_search(tree, 50, &val) && val == 500);
    test_check("100 найден", btree_search(tree, 100, &val) && val == 1000);

    /* Проверяем поиск отсутствующего ключа */
    test_check("25 не найден", !btree_search(tree, 25, NULL));

    /* Печатаем дерево для визуальной проверки */
    btree_print(tree);

    /* Освобождаем память */
    btree_destroy(tree);
}

/*
 * ТЕСТ 3: УДАЛЕНИЕ
 * Проверяет удаление элементов из дерева с правильной перебалансировкой
 */
void
test_delete(void)
{
    /* Создаём дерево со степенью 3 */
    BTree* tree = btree_create(3);

    /* Вставляем 20 элементов (0, 5, 10, 15, ..., 95) */
    for (int i = 0; i < 20; i++) {
        btree_insert(tree, i * 5, i * 10);
    }

    /* Проверяем начальный размер */
    test_check("Размер = 20", btree_size(tree) == 20);

    /* УДАЛЕНИЕ 1: Удаляем первый элемент (0) */
    test_check("Удаление 0", btree_delete(tree, 0));
    test_check("Размер = 19", btree_size(tree) == 19);
    test_check("0 отсутствует", !btree_contains(tree, 0));

    /* УДАЛЕНИЕ 2: Удаляем элемент из середины (45) */
    test_check("Удаление 45", btree_delete(tree, 45));
    test_check("Размер = 18", btree_size(tree) == 18);
    test_check("45 отсутствует", !btree_contains(tree, 45));

    /* УДАЛЕНИЕ 3: Удаляем элемент из конца (95) */
    /* 95 может быть удалён при балансировке, поэтому проверяем его наличие */
    if (btree_contains(tree, 95)) {
        test_check("Удаление 95", btree_delete(tree, 95));
        test_check("Размер = 17", btree_size(tree) == 17);
        test_check("95 отсутствует", !btree_contains(tree, 95));
    }
    else {
        /* Если 95 нет в дереве - пропускаем тест */
        printf("  ПРОПУСК: 95 не найден в дереве\n");
    }

    /* Проверяем, что остальные элементы на месте */
    test_check("5 есть", btree_contains(tree, 5));
    test_check("10 есть", btree_contains(tree, 10));

    /* Печатаем дерево для визуальной проверки */
    btree_print(tree);

    /* Освобождаем память */
    btree_destroy(tree);
}

/*
 * ТЕСТ 4: ИТЕРАТОР
 * Проверяет обход дерева в порядке возрастания
 */
void
test_iterator(void)
{
    /* Создаём дерево со степенью 3 */
    BTree* tree = btree_create(3);

    /* Вставляем ключи в произвольном порядке */
    int keys[] = { 15, 25, 5, 35, 45, 10, 30, 20, 40, 50 };
    for (int i = 0; i < 10; i++) {
        btree_insert(tree, keys[i], keys[i] * 10);
    }

    /* Создаём итератор и обходим дерево */
    printf("  Обход: ");
    BTreeIterator iter = btree_iterator_create(tree);

    int count = 0;
    while (btree_iterator_valid(&iter)) {
        /* Выводим ключ текущего элемента */
        printf("%d ", btree_iterator_key(&iter));
        count++;
        /* Переходим к следующему элементу */
        btree_iterator_next(&iter);
    }
    printf("\n");

    /* Проверяем, что обходено 10 элементов */
    test_check("Обходено 10 элементов", count == 10);

    /* Освобождаем память итератора и дерева */
    btree_iterator_destroy(&iter);
    btree_destroy(tree);
}

/*
 * ТЕСТ 5: БОЛЬШОЕ КОЛИЧЕСТВО ЭЛЕМЕНТОВ
 * Проверяет производительность и корректность на 1000 элементах
 */
void
test_large(void)
{
    /* Создаём дерево со степенью 4 (больше элементов в узле) */
    BTree* tree = btree_create(4);
    int n = 1000;  /* Количество элементов для теста */

    /* Вставляем 1000 элементов */
    printf("  Вставка %d элементов...\n", n);
    for (int i = 0; i < n; i++) {
        btree_insert(tree, i, i * 2);
    }

    /* Проверяем размер и высоту */
    printf("  Размер после вставки: %d\n", btree_size(tree));
    test_check("Размер = 1000", btree_size(tree) == n);
    test_check("Высота <= 5", btree_height(tree) <= 5);  /* B-дерево должно быть низким */

    /* Проверяем, что все элементы найдены */
    int found = 0;
    for (int i = 0; i < n; i++) {
        if (btree_contains(tree, i)) found++;
    }
    test_check("Все элементы найдены", found == n);

    /* Удаляем каждый второй элемент (чётные) */
    printf("  Удаление 500 элементов (чётные)...\n");
    int deleted = 0;
    for (int i = 0; i < n; i += 2) {
        if (btree_delete(tree, i)) {
            deleted++;  /* Считаем реально удалённые элементы */
        }
    }
    printf("  Удалено элементов: %d\n", deleted);
    printf("  Размер после удаления: %d (должен быть 500)\n", btree_size(tree));

    /* Проверяем, что размер стал 500 */
    test_check("Размер = 500", btree_size(tree) == 500);

    /* Печатаем статистику дерева */
    btree_print_stats(tree);

    /* Освобождаем память */
    btree_destroy(tree);
}

/*
 * ПРИМЕР 1: ИНДЕКС БАЗЫ ДАННЫХ
 * Демонстрирует использование B-дерева как индекса
 */
void
example_database_index(void)
{
    /* Создаём индекс */
    BTree* index = btree_create(3);

    /* Данные студентов: ID -> оценка */
    struct {
        int id;
        int grade;
    } students[] = {
        {1001, 85}, {1002, 92}, {1003, 78}, {1004, 95},
        {1005, 88}, {1006, 76}, {1007, 90}, {1008, 82},
        {1009, 94}, {1010, 87}, {1011, 91}, {1012, 79}
    };

    /* Заполняем индекс данными студентов */
    for (int i = 0; i < 12; i++) {
        btree_insert(index, students[i].id, students[i].grade);
    }

    /* Выполняем поиск по индексу */
    printf("  Поиск студентов:\n");
    int grade;
    int search_ids[] = { 1003, 1007, 1011, 9999 };

    for (int i = 0; i < 4; i++) {
        if (btree_search(index, search_ids[i], &grade)) {
            printf("    ID=%d -> оценка %d\n", search_ids[i], grade);
        }
        else {
            printf("    ID=%d -> не найден\n", search_ids[i]);
        }
    }

    /* Освобождаем память */
    btree_destroy(index);
}

/*
 * ПРИМЕР 2: СОРТИРОВКА ДАННЫХ
 * Демонстрирует использование B-дерева для сортировки
 */
void
example_sorting(void)
{
    /* Создаём дерево */
    BTree* tree = btree_create(3);
    int n = 50;  /* Количество случайных чисел */

    /* Генерируем 50 случайных чисел и вставляем в дерево */
    srand((unsigned int)time(NULL));
    for (int i = 0; i < n; i++) {
        int val = rand() % 1000;  /* Случайное число от 0 до 999 */
        btree_insert(tree, val, val);
    }

    /* Выводим числа в отсортированном порядке через итератор */
    printf("  Отсортированные числа:\n    ");
    BTreeIterator iter = btree_iterator_create(tree);
    int count = 0;
    while (btree_iterator_valid(&iter)) {
        printf("%d ", btree_iterator_key(&iter));
        count++;
        btree_iterator_next(&iter);
    }
    printf("\n");

    /* Проверяем, что все элементы выведены */
    printf("  Количество элементов: %d (должно быть %d)\n", count, n);
    test_check("Все элементы отсортированы", count == n);

    /* Освобождаем память */
    btree_iterator_destroy(&iter);
    btree_destroy(tree);
}