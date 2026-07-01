/*  btree_utils.h (вспомогательные функции) */

#ifndef BTREE_UTILS_H           
#define BTREE_UTILS_H           

#include "btree_node.h"          /* Подключаем для работы с узлами */

/*
 * Вспомогательные функции для работы с B-деревом
 */
BTreeNode* btree_find_minimum(BTreeNode* node);      /* Поиск минимального ключа */
BTreeNode* btree_find_maximum(BTreeNode* node);      /* Поиск максимального ключа */
int btree_count_nodes(BTreeNode* node);              /* Подсчёт количества узлов */
int btree_calculate_height(BTreeNode* node);         /* Вычисление высоты дерева */
void btree_print_node(BTreeNode* node, int level, const char* side);  /* Печать узла */

/*
 * Разделение и вставка
 */
void btree_split_child(BTreeNode* parent, int child_idx, BTreeNode* child);      /* Разделение */
void btree_insert_nonfull(BTreeNode* node, int key, int value);                 /* Вставка */

/*
 * Удаление и перебалансировка
 */
bool btree_delete_from_subtree(BTreeNode* node, int key, int degree, int* child_height);  /* Удаление */
void btree_borrow_from_left(BTreeNode* parent, int child_idx);     /* Заимствование слева */
void btree_borrow_from_right(BTreeNode* parent, int child_idx);    /* Заимствование справа */
void btree_merge_nodes(BTreeNode* parent, int child_idx);          /* Слияние узлов */

#endif