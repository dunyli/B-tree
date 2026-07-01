/*  btree_utils.h (вспомогательные функции) */

#ifndef BTREE_UTILS_H
#define BTREE_UTILS_H

#include "btree.h"
#include "btree_node.h"

 /*
  * Вспомогательные функции для работы с деревом
  */
BTreeNode* btree_find_minimum(BTreeNode* node);
BTreeNode* btree_find_maximum(BTreeNode* node);
int btree_count_nodes(BTreeNode* node);
int btree_calculate_height(BTreeNode* node);
void btree_print_node(BTreeNode* node, int level, const char* side);

#endif
