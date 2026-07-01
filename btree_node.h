/* btree_node.h (узел B-дерева)  */

#ifndef BTREE_NODE_H
#define BTREE_NODE_H

#include <stdbool.h>

 /*
  * Структура узла B-дерева
  */
struct BTreeNode {
    int* keys;
    int* values;
    struct BTreeNode** children;
    int key_count;
    bool is_leaf;
    int degree;
};

/*
 * Функции для работы с узлами
 */
BTreeNode* btree_node_create(int degree, bool is_leaf);
void btree_node_destroy(BTreeNode* node);
int btree_node_find_key(BTreeNode* node, int key);
int btree_node_find_insert_pos(BTreeNode* node, int key);

#endif
