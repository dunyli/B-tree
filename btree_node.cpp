/* btree_node.c (реализация узла) */

#define _CRT_SECURE_NO_WARNINGS

#include "btree_node.h"
#include <stdlib.h>
#include <string.h>

 /*
  * Создание нового узла B-дерева
  */
BTreeNode*
btree_node_create(int degree, bool is_leaf)
{
    BTreeNode* node = (BTreeNode*)malloc(sizeof(BTreeNode));
    if (!node)
        return NULL;

    node->degree = degree;
    node->is_leaf = is_leaf;
    node->key_count = 0;

    node->keys = (int*)malloc((2 * degree - 1) * sizeof(int));
    if (!node->keys) {
        free(node);
        return NULL;
    }

    node->values = (int*)malloc((2 * degree - 1) * sizeof(int));
    if (!node->values) {
        free(node->keys);
        free(node);
        return NULL;
    }

    node->children = (BTreeNode**)malloc((2 * degree) * sizeof(BTreeNode*));
    if (!node->children) {
        free(node->values);
        free(node->keys);
        free(node);
        return NULL;
    }

    for (int i = 0; i < 2 * degree; i++) {
        node->children[i] = NULL;
    }

    return node;
}

/*
 * Освобождение узла и всех его потомков
 */
void
btree_node_destroy(BTreeNode* node)
{
    if (!node)
        return;

    if (!node->is_leaf) {
        for (int i = 0; i <= node->key_count; i++) {
            btree_node_destroy(node->children[i]);
        }
    }

    free(node->keys);
    free(node->values);
    free(node->children);
    free(node);
}

/*
 * Поиск ключа в узле (бинарный поиск)
 */
int
btree_node_find_key(BTreeNode* node, int key)
{
    int left = 0;
    int right = node->key_count - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (node->keys[mid] == key)
            return mid;
        if (node->keys[mid] < key)
            left = mid + 1;
        else
            right = mid - 1;
    }

    return -1;
}

/*
 * Поиск позиции для вставки ключа
 */
int
btree_node_find_insert_pos(BTreeNode* node, int key)
{
    int i = 0;
    while (i < node->key_count && node->keys[i] < key) {
        i++;
    }
    return i;
}