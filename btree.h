/*
 * btree.h - заголовочный файл B-дерева
 */

#ifndef BTREE_H
#define BTREE_H

#include <stdbool.h>

 /* Опережающие объявления структур */
typedef struct BTree BTree;
typedef struct BTreeNode BTreeNode;

/* Структура итератора */
typedef struct {
    int* keys;        /* Массив ключей */
    int* values;      /* Массив значений */
    int size;         /* Размер массива */
    int current;      /* Текущая позиция */
    bool is_valid;    /* Валидность итератора */
} BTreeIterator;

/* Функции управления деревом */
BTree* btree_create(int degree);
void btree_destroy(BTree* tree);

/* Основные операции */
bool btree_insert(BTree* tree, int key, int value);
bool btree_search(BTree* tree, int key, int* out_value);
bool btree_delete(BTree* tree, int key);
bool btree_contains(BTree* tree, int key);

/* Статистика */
int btree_size(BTree* tree);
int btree_height(BTree* tree);
int btree_degree(BTree* tree);
int btree_nodes(BTree* tree);
void btree_print_stats(BTree* tree);
void btree_print(BTree* tree);

/* Итератор */
BTreeIterator btree_iterator_create(BTree* tree);
void btree_iterator_destroy(BTreeIterator* iter);
bool btree_iterator_next(BTreeIterator* iter);
int btree_iterator_key(BTreeIterator* iter);
int btree_iterator_value(BTreeIterator* iter);
bool btree_iterator_valid(BTreeIterator* iter);

#endif