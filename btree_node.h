/*
 * ============================================================================
 *                          btree_node.h (заголовочный файл узла)
 * ============================================================================
 */

#ifndef BTREE_NODE_H
#define BTREE_NODE_H

#include <stdbool.h>

 /*
  * Структура узла B-дерева
  */
typedef struct BTreeNode {
    int* keys;                     /* Массив ключей (размер 2t-1) */
    int* values;                   /* Массив значений (размер 2t-1) */
    struct BTreeNode** children;   /* Массив потомков (размер 2t) */
    int key_count;                 /* Текущее количество ключей в узле */
    bool is_leaf;                  /* Флаг: является ли узел листом */
    int degree;                    /* Степень дерева (t) */
} BTreeNode;

/*
 * Функции для работы с узлами
 */
BTreeNode* btree_node_create(int degree, bool is_leaf);
void btree_node_destroy(BTreeNode* node);
int btree_node_find_key(BTreeNode* node, int key);
int btree_node_find_insert_pos(BTreeNode* node, int key);

#endif /* BTREE_NODE_H */