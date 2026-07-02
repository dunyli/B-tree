/*
 * B-tree.c - главный файл программы
 * Запускает все тесты и примеры применения
 */

#define _CRT_SECURE_NO_WARNINGS

#include "btree.h"
#include <stdio.h>
#include <locale.h>

 /* Объявления функций из btree_test.c */
extern void test_create_destroy(void);
extern void test_insert_search(void);
extern void test_delete(void);
extern void test_iterator(void);
extern void test_large(void);
extern void example_database_index(void);
extern void example_sorting(void);

/* Внешние переменные для статистики тестов */
extern int passed;
extern int failed;

int
main(void)
{
#ifdef _WIN32
    setlocale(LC_ALL, "Russian");  /* Устанавливаем русскую локаль для Windows */
#else
    setlocale(LC_ALL, "ru_RU.UTF-8");  /* Устанавливаем русскую локаль для Linux/macOS */
#endif

    printf("\n");
    printf("===============================================================\n");
    printf("                    B-ДЕРЕВО (B-TREE)\n");
    printf("===============================================================\n");
    printf("Сбалансированное дерево поиска\n");
    printf("Все операции: O(log n)\n");
    printf("===============================================================\n");

    printf("\n--- ЗАПУСК ТЕСТОВ ---\n");

    printf("\nТест 1: Создание и удаление\n");
    test_create_destroy();

    printf("\nТест 2: Вставка и поиск\n");
    test_insert_search();

    printf("\nТест 3: Удаление\n");
    test_delete();

    printf("\nТест 4: Итератор\n");
    test_iterator();

    printf("\nТест 5: Большие данные\n");
    test_large();

    printf("\n--- ПРИМЕРЫ ПРИМЕНЕНИЯ ---\n");

    printf("\nПример 1: Индекс базы данных\n");
    example_database_index();

    printf("\nПример 2: Сортировка данных\n");
    example_sorting();

    printf("\n===============================================================\n");
    printf("ИТОГИ ТЕСТОВ:\n");
    printf("  УСПЕШНО: %d\n", passed);
    printf("  ПРОВАЛЕНО: %d\n", failed);
    printf("  ВСЕГО: %d\n", passed + failed);

    if (failed == 0) {
        printf("\nВСЕ ТЕСТЫ ПРОЙДЕНЫ УСПЕШНО!\n");
    }
    else {
        printf("\nЕСТЬ ПРОВАЛЕННЫЕ ТЕСТЫ!\n");
    }

    printf("===============================================================\n");
    return 0;
}