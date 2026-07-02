/*
 * btree_utils.h - заголовочный файл вспомогательных функций B-дерева
 *
 * Содержит объявления всех вспомогательных функций
 */

#ifndef BTREE_UTILS_H
#define BTREE_UTILS_H

#include "btree_node.h"

 /* Поиск минимума/максимума */
BTreeNode* btree_find_minimum(BTreeNode* node);
BTreeNode* btree_find_maximum(BTreeNode* node);

/* Статистика */
int btree_count_nodes(BTreeNode* node);
int btree_calculate_height(BTreeNode* node);

/* Разделение и вставка */
void btree_split_child(BTreeNode* parent, int child_idx, BTreeNode* child);
void btree_insert_nonfull(BTreeNode* node, int key, int value);

#endif