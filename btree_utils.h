/*  btree_utils.h (вспомогательные функции) */

#ifndef BTREE_UTILS_H
#define BTREE_UTILS_H

#include "btree_node.h"

/*
 * Вспомогательные функции для работы с B-деревом
 */

 /* Поиск минимального ключа в поддереве (крайний левый) */
BTreeNode* btree_find_minimum(BTreeNode* node);

/* Поиск максимального ключа в поддереве (крайний правый) */
BTreeNode* btree_find_maximum(BTreeNode* node);

/* Подсчёт количества узлов в дереве */
int btree_count_nodes(BTreeNode* node);

/* Вычисление высоты дерева */
int btree_calculate_height(BTreeNode* node);

/* Рекурсивная печать узла и всех потомков */
void btree_print_node(BTreeNode* node, int level, const char* side);

/* Разделение полного узла */
void btree_split_child(BTreeNode* parent, int child_idx, BTreeNode* child);

/* Вставка в неполный узел */
void btree_insert_nonfull(BTreeNode* node, int key, int value);

#endif
