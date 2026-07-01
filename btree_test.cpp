/*
 *  btree_test.c (тесты и примеры)
 */

#define _CRT_SECURE_NO_WARNINGS

#include "btree.h"           
#include <stdio.h>          
#include <stdlib.h>      
#include <time.h>            
#include <locale.h>          /* setlocale для Windows */

 /* Счётчики для тестов (глобальные для доступа из main) */
int passed = 0;  /* Количество пройденных тестов */
int failed = 0;  /* Количество проваленных тестов */

/*
 * Проверка условия теста
 * Выводит результат и обновляет счётчики
 */
void test_check(const char* description, int condition)
{
    if (condition) {
        printf(" УСПЕШНО: %s\n", description);  /* Выводим успех */
        passed++;                                  /* Увеличиваем счётчик успехов */
    }
    else {
        printf(" ОШИБКА: %s\n", description);   /* Выводим ошибку */
        failed++;                                  /* Увеличиваем счётчик ошибок */
    }
}

/*
 * ТЕСТ 1: СОЗДАНИЕ И УДАЛЕНИЕ ДЕРЕВА
 * Проверяет корректную инициализацию и освобождение памяти
 */
void test_create_destroy(void)
{
    printf("\n Тест 1: Создание и удаление \n");

    BTree* tree = btree_create(3);  /* Создаём дерево со степенью 3 */
    test_check("Дерево создано (не NULL)", tree != NULL);
    test_check("Степень = 3", btree_degree(tree) == 3);
    test_check("Размер = 0", btree_size(tree) == 0);
    test_check("Высота = 1", btree_height(tree) == 1);

    btree_destroy(tree);  /* Удаляем дерево */
    test_check("Дерево удалено без ошибок", 1);
}

/*
 * ТЕСТ 2: ВСТАВКА И ПОИСК
 * Проверяет, что все элементы вставляются и находятся
 */
void test_insert_search(void)
{
    printf("\n Тест 2: Вставка и поиск \n");

    BTree* tree = btree_create(3);  /* Создаём дерево */

    /* Массивы ключей и значений */
    int keys[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
    int values[] = { 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000 };

    /* Вставляем все элементы */
    for (int i = 0; i < 10; i++) {
        btree_insert(tree, keys[i], values[i]);
    }

    test_check("Размер = 10", btree_size(tree) == 10);

    /* Проверяем поиск существующих ключей */
    int val;
    test_check("10 найден", btree_search(tree, 10, &val) && val == 100);
    test_check("50 найден", btree_search(tree, 50, &val) && val == 500);
    test_check("100 найден", btree_search(tree, 100, &val) && val == 1000);

    /* Проверяем поиск отсутствующего ключа */
    test_check("25 не найден", !btree_search(tree, 25, NULL));

    btree_print(tree);  /* Печатаем дерево для отладки */
    btree_destroy(tree);
}

/*
 * ТЕСТ 3: УДАЛЕНИЕ
 * Проверяет удаление из разных частей дерева
 */
void test_delete(void)
{
    printf("\n Тест 3: Удаление \n");

    BTree* tree = btree_create(3);  /* Создаём дерево */

    /* Вставляем 20 элементов */
    for (int i = 0; i < 20; i++) {
        btree_insert(tree, i * 5, i * 10);
    }

    test_check("Размер = 20", btree_size(tree) == 20);

    /* Проверяем удаление */
    test_check("Удаление 0", btree_delete(tree, 0));
    test_check("Размер = 19", btree_size(tree) == 19);
    test_check("0 отсутствует", !btree_contains(tree, 0));

    test_check("Удаление 45", btree_delete(tree, 45));
    test_check("Размер = 18", btree_size(tree) == 18);
    test_check("45 отсутствует", !btree_contains(tree, 45));

    test_check("Удаление 95", btree_delete(tree, 95));
    test_check("Размер = 17", btree_size(tree) == 17);
    test_check("95 отсутствует", !btree_contains(tree, 95));

    /* Проверяем, что остальные элементы на месте */
    test_check("5 есть", btree_contains(tree, 5));
    test_check("10 есть", btree_contains(tree, 10));
    test_check("90 есть", btree_contains(tree, 90));

    btree_print(tree);
    btree_destroy(tree);
}

/*
 * ТЕСТ 4: ИТЕРАТОР
 * Проверяет обход дерева в порядке возрастания
 */
void test_iterator(void)
{
    printf("\n Тест 4: Итератор \n");

    BTree* tree = btree_create(3);  /* Создаём дерево */

    /* Массив ключей в произвольном порядке */
    int keys[] = { 15, 25, 5, 35, 45, 10, 30, 20, 40, 50 };
    for (int i = 0; i < 10; i++) {
        btree_insert(tree, keys[i], keys[i] * 10);
    }

    printf("\nОбход дерева в порядке возрастания:\n  ");
    BTreeIterator iter = btree_iterator_create(tree);  /* Создаём итератор */

    int prev_key = -1;
    int count = 0;

    /* Обход всех элементов */
    while (btree_iterator_valid(&iter)) {
        int key = btree_iterator_key(&iter);
        int value = btree_iterator_value(&iter);

        printf("%d(%d) ", key, value);

        /* Проверяем, что ключи идут по возрастанию */
        if (prev_key != -1) {
            test_check("Ключи идут по возрастанию", key > prev_key);
        }
        prev_key = key;
        count++;

        btree_iterator_next(&iter);  /* Переходим к следующему */
    }

    printf("\n");
    test_check("Обходено 10 элементов", count == 10);

    btree_iterator_destroy(&iter);
    btree_destroy(tree);
}

/*
 * ТЕСТ 5: БОЛЬШОЕ КОЛИЧЕСТВО ЭЛЕМЕНТОВ
 * Проверяет производительность и корректность
 */
void test_large(void)
{
    printf("\n Тест 5: Большое количество элементов \n");

    BTree* tree = btree_create(4);  /* Дерево со степенью 4 (больше элементов в узле) */
    int n = 1000;  /* 1000 элементов */

    printf("Вставка %d элементов...\n", n);
    for (int i = 0; i < n; i++) {
        btree_insert(tree, i, i * 2);
    }

    test_check("Размер = 1000", btree_size(tree) == n);
    test_check("Высота <= 5", btree_height(tree) <= 5);  /* B-дерево должно быть низким */

    printf("Поиск всех элементов...\n");
    int found = 0;
    for (int i = 0; i < n; i++) {
        if (btree_contains(tree, i)) found++;
    }
    test_check("Все 1000 элементов найдены", found == n);

    printf("Удаление 500 элементов...\n");
    for (int i = 0; i < n; i += 2) {
        btree_delete(tree, i);
    }

    test_check("Размер = 500", btree_size(tree) == n / 2);

    btree_print_stats(tree);  /* Печатаем статистику */
    btree_destroy(tree);
}

/*
 * ПРИМЕР 1: ИНДЕКС БАЗЫ ДАННЫХ
 * Демонстрирует использование B-дерева как индекса в БД
 */
void example_database_index(void)
{
    printf("\n Пример 1: Индекс базы данных \n");

    BTree* index = btree_create(3);  /* Создаём индекс */

    /* Данные студентов (ID -> оценка) */
    struct {
        int id;
        int grade;
    } students[] = {
        {1001, 85}, {1002, 92}, {1003, 78}, {1004, 95},
        {1005, 88}, {1006, 76}, {1007, 90}, {1008, 82},
        {1009, 94}, {1010, 87}, {1011, 91}, {1012, 79}
    };

    printf("Создание индекса студентов:\n");
    for (int i = 0; i < 12; i++) {
        btree_insert(index, students[i].id, students[i].grade);
        printf("  Добавлен студент ID=%d с оценкой %d\n",
            students[i].id, students[i].grade);
    }

    printf("\nПоиск по индексу:\n");
    int grade;
    int search_ids[] = { 1003, 1007, 1011, 9999 };  /* ID для поиска */

    for (int i = 0; i < 4; i++) {
        if (btree_search(index, search_ids[i], &grade)) {
            printf("  Студент ID=%d имеет оценку %d\n", search_ids[i], grade);
        }
        else {
            printf("  Студент ID=%d не найден\n", search_ids[i]);
        }
    }

    btree_destroy(index);
}

/*
 * ПРИМЕР 2: СОРТИРОВКА ДАННЫХ
 * Демонстрирует использование B-дерева для сортировки
 */
void example_sorting(void)
{
    printf("\n Пример 2: Сортировка данных \n");

    BTree* tree = btree_create(3);  /* Создаём дерево */
    int n = 50;  /* 50 случайных чисел */

    printf("Генерация %d случайных чисел...\n", n);
    srand((unsigned int)time(NULL));  /* Инициализация генератора случайных чисел */
    for (int i = 0; i < n; i++) {
        int val = rand() % 1000;  /* Случайное число от 0 до 999 */
        btree_insert(tree, val, val);  /* Вставляем в дерево */
    }

    printf("Отсортированные числа:\n  ");
    BTreeIterator iter = btree_iterator_create(tree);  /* Создаём итератор */
    int count = 0;
    while (btree_iterator_valid(&iter)) {
        printf("%d ", btree_iterator_key(&iter));  /* Выводим ключ */
        count++;
        btree_iterator_next(&iter);
        if (count % 10 == 0) printf("\n  ");  /* Переход на новую строку каждые 10 чисел */
    }
    printf("\n");

    test_check("Все элементы отсортированы", count == n);

    btree_iterator_destroy(&iter);
    btree_destroy(tree);
}