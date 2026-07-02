/*
 * btree_delete.c - реализация удаления из B-дерева
 * Полностью соответствует классическому алгоритму из C++ кода
 */

#define _CRT_SECURE_NO_WARNINGS

#include "btree_delete.h"
#include "btree_utils.h"
#include <stdlib.h>
#include <stdio.h>

 /*
  * Поиск первого ключа >= k
  */
static int find_key(BTreeNode* node, int k)
{
    int idx = 0;
    while (idx < node->key_count && node->keys[idx] < k) {
        idx++;
    }
    return idx;
}

/*
 * ЗАИМСТВОВАНИЕ ОТ ЛЕВОГО БРАТА
 */
static void borrow_from_left(BTreeNode* parent, int idx)
{
    if (!parent || idx <= 0 || idx > parent->key_count) return;

    BTreeNode* child = parent->children[idx];
    BTreeNode* sibling = parent->children[idx - 1];

    if (!child || !sibling) return;
    if (sibling->key_count <= 0) return;

    int max_keys = 2 * child->degree - 1;
    if (child->key_count >= max_keys) return;

    /* Сдвигаем ключи child вправо */
    for (int i = child->key_count - 1; i >= 0; i--) {
        child->keys[i + 1] = child->keys[i];
        child->values[i + 1] = child->values[i];
    }

    if (!child->is_leaf) {
        for (int i = child->key_count; i >= 0; i--) {
            child->children[i + 1] = child->children[i];
        }
    }

    child->keys[0] = parent->keys[idx - 1];
    child->values[0] = parent->values[idx - 1];

    parent->keys[idx - 1] = sibling->keys[sibling->key_count - 1];
    parent->values[idx - 1] = sibling->values[sibling->key_count - 1];

    if (!child->is_leaf) {
        child->children[0] = sibling->children[sibling->key_count];
    }

    child->key_count++;
    sibling->key_count--;
}

/*
 * ЗАИМСТВОВАНИЕ ОТ ПРАВОГО БРАТА
 */
static void borrow_from_right(BTreeNode* parent, int idx)
{
    if (!parent || idx >= parent->key_count) return;

    BTreeNode* child = parent->children[idx];
    BTreeNode* sibling = parent->children[idx + 1];

    if (!child || !sibling) return;
    if (sibling->key_count <= 0) return;

    int max_keys = 2 * child->degree - 1;
    if (child->key_count >= max_keys) return;

    child->keys[child->key_count] = parent->keys[idx];
    child->values[child->key_count] = parent->values[idx];

    if (!child->is_leaf) {
        child->children[child->key_count + 1] = sibling->children[0];
    }

    parent->keys[idx] = sibling->keys[0];
    parent->values[idx] = sibling->values[0];

    for (int i = 1; i < sibling->key_count; i++) {
        sibling->keys[i - 1] = sibling->keys[i];
        sibling->values[i - 1] = sibling->values[i];
    }

    if (!sibling->is_leaf) {
        for (int i = 1; i <= sibling->key_count; i++) {
            sibling->children[i - 1] = sibling->children[i];
        }
    }

    child->key_count++;
    sibling->key_count--;
}

/*
 * СЛИЯНИЕ ДВУХ УЗЛОВ
 */
static void merge_nodes(BTreeNode* parent, int idx)
{
    if (!parent || idx >= parent->key_count) return;

    BTreeNode* left_child = parent->children[idx];
    BTreeNode* right_child = parent->children[idx + 1];

    if (!left_child || !right_child) return;

    int t = left_child->degree;

    /* Переносим ключ из родителя в левый узел */
    left_child->keys[t - 1] = parent->keys[idx];
    left_child->key_count++;

    /* Переносим все ключи из правого в левый */
    for (int i = 0; i < right_child->key_count; i++) {
        left_child->keys[i + t] = right_child->keys[i];
        left_child->key_count++;
    }

    if (!left_child->is_leaf) {
        for (int i = 0; i <= right_child->key_count; i++) {
            left_child->children[i + t] = right_child->children[i];
        }
    }

    /* Удаляем ключ из родителя */
    for (int i = idx + 1; i < parent->key_count; i++) {
        parent->keys[i - 1] = parent->keys[i];
        parent->values[i - 1] = parent->values[i];
    }

    for (int i = idx + 2; i <= parent->key_count; i++) {
        parent->children[i - 1] = parent->children[i];
    }

    parent->key_count--;

    /* Освобождаем правый узел */
    if (right_child) {
        if (right_child->keys) free(right_child->keys);
        if (right_child->values) free(right_child->values);
        if (right_child->children) free(right_child->children);
        free(right_child);
    }
}

/*
 * ПЕРЕБАЛАНСИРОВКА
 */
static void fill_node(BTreeNode* parent, int idx)
{
    if (!parent) return;

    /* Случай 1: Заимствуем у левого брата */
    if (idx != 0 && parent->children[idx - 1]->key_count >= parent->degree) {
        borrow_from_left(parent, idx);
        return;
    }

    /* Случай 2: Заимствуем у правого брата */
    if (idx != parent->key_count && parent->children[idx + 1]->key_count >= parent->degree) {
        borrow_from_right(parent, idx);
        return;
    }

    /* Случай 3: Сливаем с братом */
    if (idx != parent->key_count) {
        merge_nodes(parent, idx);
    }
    else {
        merge_nodes(parent, idx - 1);
    }
}

/*
 * УДАЛЕНИЕ КЛЮЧА
 */
bool btree_delete_from_subtree(BTreeNode* node, int key, int degree, int* child_height)
{
    if (!node) return false;

    int idx = find_key(node, key);

    /* Ключ найден в текущем узле */
    if (idx < node->key_count && node->keys[idx] == key) {

        /* Если лист - удаляем */
        if (node->is_leaf) {
            for (int i = idx + 1; i < node->key_count; i++) {
                node->keys[i - 1] = node->keys[i];
                node->values[i - 1] = node->values[i];
            }
            node->key_count--;
            return true;
        }

        /* Внутренний узел */
        else {
            /* Используем предшественника */
            if (node->children[idx]->key_count >= degree) {
                BTreeNode* cur = node->children[idx];
                while (!cur->is_leaf) {
                    cur = cur->children[cur->key_count];
                }
                int pred = cur->keys[cur->key_count - 1];
                int pred_val = cur->values[cur->key_count - 1];

                node->keys[idx] = pred;
                node->values[idx] = pred_val;

                return btree_delete_from_subtree(node->children[idx], pred, degree, child_height);
            }

            /* Используем преемника */
            else if (node->children[idx + 1]->key_count >= degree) {
                BTreeNode* cur = node->children[idx + 1];
                while (!cur->is_leaf) {
                    cur = cur->children[0];
                }
                int succ = cur->keys[0];
                int succ_val = cur->values[0];

                node->keys[idx] = succ;
                node->values[idx] = succ_val;

                return btree_delete_from_subtree(node->children[idx + 1], succ, degree, child_height);
            }

            /* Сливаем и удаляем */
            else {
                fill_node(node, idx);
                return btree_delete_from_subtree(node->children[idx], key, degree, child_height);
            }
        }
    }

    /* Ключ не найден - идём в потомка */
    else {
        if (node->is_leaf) return false;

        bool flag = (idx == node->key_count);

        if (node->children[idx]->key_count < degree) {
            fill_node(node, idx);
        }

        if (flag && idx > node->key_count) {
            return btree_delete_from_subtree(node->children[idx - 1], key, degree, child_height);
        }

        return btree_delete_from_subtree(node->children[idx], key, degree, child_height);
    }
}

/*
 * ОБЁРТКИ ДЛЯ СОВМЕСТИМОСТИ
 */
void btree_borrow_from_left(BTreeNode* parent, int child_idx)
{
    borrow_from_left(parent, child_idx);
}

void btree_borrow_from_right(BTreeNode* parent, int child_idx)
{
    borrow_from_right(parent, child_idx);
}

void btree_merge_nodes(BTreeNode* parent, int child_idx)
{
    merge_nodes(parent, child_idx);
}