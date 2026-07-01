/* btree_node.h (узел B-дерева)  */

#ifndef BTREE_NODE_H
#define BTREE_NODE_H

#include <stdbool.h>     /* Для bool */

/*
 * Структура узла B-дерева
 * Содержит массивы ключей, значений и указателей на потомков
 */
struct BTreeNode {
    int* keys;                     /* Массив ключей (размер 2t-1) */
    int* values;                   /* Массив значений (размер 2t-1) */
    struct BTreeNode** children;   /* Массив потомков (размер 2t) */
    int key_count;                 /* Текущее количество ключей в узле */
    bool is_leaf;                  /* Флаг: является ли узел листом */
    int degree;                    /* Степень дерева (t) */
};

/*
 * Создание нового узла
 * Параметры: degree - степень, is_leaf - флаг листа
 * Возвращает: указатель на узел или NULL
 */
BTreeNode* btree_node_create(int degree, bool is_leaf);

/*
 * Освобождение узла и всех потомков (рекурсивно)
 */
void btree_node_destroy(BTreeNode* node);

/*
 * Поиск ключа в узле (бинарный поиск)
 * Возвращает: индекс ключа или -1
 */
int btree_node_find_key(BTreeNode* node, int key);

/*
 * Поиск позиции для вставки ключа (бинарный поиск)
 * Возвращает: индекс для вставки
 */
int btree_node_find_insert_pos(BTreeNode* node, int key);

#endif