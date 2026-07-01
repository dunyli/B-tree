// btree.h(заголовочный файл)

/* B - дерево — сбалансированное дерево поиска.
* Все листья на одном уровне, операции O(log n).
*/

#ifndef BTREE_H              
#define BTREE_H                

#include <stdbool.h>           /* Подключаем для типа bool (true/false) */
#include <stdint.h>            /* Подключаем для целочисленных типов */
#include <stdio.h>             /* Подключаем для FILE* */


typedef struct BTree BTree;                /*  объявление BTree */
typedef struct BTreeNode BTreeNode;        /*  объявление BTreeNode */

/*
 * Итератор для обхода B-дерева в порядке возрастания
 */
typedef struct {
    BTree* tree;             /* Указатель на дерево */
    BTreeNode** stack;       /* Стек узлов для обхода (путь от корня) */
    int* positions;          /* Текущие позиции в узлах */
    int stack_size;          /* Текущий размер стека */
    int stack_capacity;      /* Вместимость стека (для realloc) */
    bool is_valid;           /* Флаг: итератор валиден (не в конце) */
} BTreeIterator;

/*
 * ФУНКЦИИ УПРАВЛЕНИЯ
 */

 /*
  * Создание B-дерева
  * Параметры: degree - степень дерева (минимум 2)
  * Возвращает: указатель на дерево или NULL
  */
BTree* btree_create(int degree);

/*
 * Освобождение памяти дерева
 */
void btree_destroy(BTree* tree);

/*
 *   ОПЕРАЦИИ С ДАННЫМИ
 */

 /*
  * Вставка ключа и значения
  * Возвращает: true при успехе, false при ошибке
  */
bool btree_insert(BTree* tree, int key, int value);

/*
 * Поиск значения по ключу
 * Параметры: out_value - указатель для сохранения значения (может быть NULL)
 * Возвращает: true если ключ найден
 */
bool btree_search(BTree* tree, int key, int* out_value);

/*
 * Удаление ключа
 * Возвращает: true если ключ найден и удалён
 */
bool btree_delete(BTree* tree, int key);

/*
 * Проверка наличия ключа
 */
bool btree_contains(BTree* tree, int key);

/*
 *  СТАТИСТИКА
 */

int btree_size(BTree* tree);          /* Количество элементов в дереве */
int btree_height(BTree* tree);        /* Высота дерева (количество уровней) */
int btree_degree(BTree* tree);        /* Степень дерева (t) */
int btree_nodes(BTree* tree);         /* Количество узлов в дереве */
void btree_print_stats(BTree* tree);  /* Печать статистики */
void btree_print(BTree* tree);        /* Печать дерева для отладки */

/*
 * ИТЕРАТОР
 */

BTreeIterator btree_iterator_create(BTree* tree);      /* Создание итератора */
void btree_iterator_destroy(BTreeIterator* iter);      /* Уничтожение итератора */
bool btree_iterator_next(BTreeIterator* iter);         /* Переход к следующему элементу */
int btree_iterator_key(BTreeIterator* iter);           /* Получение текущего ключа */
int btree_iterator_value(BTreeIterator* iter);         /* Получение текущего значения */
bool btree_iterator_valid(BTreeIterator* iter);        /* Проверка валидности */

#endif