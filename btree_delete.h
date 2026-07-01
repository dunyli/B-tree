/*
 * btree_delete.h (удаление)
 */

#ifndef BTREE_DELETE_H
#define BTREE_DELETE_H

#include "btree_node.h"

 /*
  * Функции для удаления ключей из B-дерева
  */

  /* Удаление ключа из поддерева */
bool btree_delete_from_subtree(BTreeNode* node, int key, int degree, int* child_height);

/* Заимствование ключа от левого брата */
void btree_borrow_from_left(BTreeNode* parent, int child_idx);

/* Заимствование ключа от правого брата */
void btree_borrow_from_right(BTreeNode* parent, int child_idx);

/* Слияние двух соседних узлов */
void btree_merge_nodes(BTreeNode* parent, int child_idx);

#endif