/*
 * btree_delete.h (удаление)
 */

#ifndef BTREE_DELETE_H
#define BTREE_DELETE_H

#include "btree_node.h"
#include <stdbool.h>

 /*
  * Удаление ключа из поддерева (основная функция)
  *
  * Параметры:
  *   node         - текущий узел
  *   key          - ключ для удаления
  *   degree       - степень дерева
  *   child_height - указатель для хранения высоты потомка (не используется)
  *
  * Возвращает: true если ключ найден и удалён
  */
bool btree_delete_from_subtree(BTreeNode* node, int key, int degree, int* child_height);

/*
 * Заимствование ключа от левого брата
 *
 * Параметры:
 *   parent     - родительский узел
 *   child_idx  - индекс потомка в родителе
 */
void btree_borrow_from_left(BTreeNode* parent, int child_idx);

/*
 * Заимствование ключа от правого брата
 *
 * Параметры:
 *   parent     - родительский узел
 *   child_idx  - индекс потомка в родителе
 */
void btree_borrow_from_right(BTreeNode* parent, int child_idx);

/*
 * Слияние двух соседних узлов
 *
 * Параметры:
 *   parent     - родительский узел
 *   child_idx  - индекс левого потомка (правый будет child_idx + 1)
 */
void btree_merge_nodes(BTreeNode* parent, int child_idx);

#endif