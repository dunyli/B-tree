/*
 *   B-tree.c (главный файл с main)
 */

#define _CRT_SECURE_NO_WARNINGS

#include "btree.h"        
#include <stdio.h>         
#include <locale.h>          

 /*
  * Объявления тестовых функций из btree_test.c
  * Они определены в другом файле, поэтому нужно объявить их здесь
  */
extern void test_create_destroy(void);
extern void test_insert_search(void);
extern void test_delete(void);
extern void test_iterator(void);
extern void test_large(void);
extern void example_database_index(void);
extern void example_sorting(void);

/*
 * Внешние переменные для статистики тестов (определены в btree_test.c)
 */
extern int passed;
extern int failed;

/*
 * MAIN
 */

int main(void) {
    setlocale(LC_ALL, "Russian");  /* Устанавливаем русскую локаль для Windows */

    printf("  B-ДЕРЕВО\n");
    printf("===============================================================\n");
    printf("Сбалансированное дерево поиска\n");

    /* Запуск всех тестов */
    test_create_destroy();
    test_insert_search();
    test_delete();
    test_iterator();
    test_large();

    /* Запуск примеров применения */
    example_database_index();
    example_sorting();

    /* Вывод итогов всех тестов */
    printf("\n===============================================================\n");
    printf("ИТОГИ ТЕСТОВ:\n");
    printf(" УСПЕШНО: %d\n", passed);
    printf(" ПРОВАЛЕНО: %d\n", failed);
    printf(" ВСЕГО: %d\n", passed + failed);

    if (failed == 0) {
        printf("\n ВСЕ ТЕСТЫ ПРОЙДЕНЫ УСПЕШНО!\n");
    }
    else {
        printf("\n ЕСТЬ ПРОВАЛЕННЫЕ ТЕСТЫ!\n");
    }

    return 0;
}
