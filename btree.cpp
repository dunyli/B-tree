#define _CRT_SECURE_NO_WARNINGS

#include "btree.h"
#include "btree_node.h"
#include "btree_utils.h"
#include "btree_delete.h"
#include <stdlib.h>
#include <stdio.h>

struct BTree {
    BTreeNode* root;
    int degree;
    int size;
    int height;
    int node_count;
};

BTree* btree_create(int degree)
{
    if (degree < 2) degree = 2;

    BTree* tree = (BTree*)malloc(sizeof(BTree));
    if (!tree) return NULL;

    tree->degree = degree;
    tree->size = 0;
    tree->height = 0;
    tree->node_count = 0;

    tree->root = btree_node_create(degree, true);
    if (!tree->root) {
        free(tree);
        return NULL;
    }

    tree->node_count = 1;
    return tree;
}

void btree_destroy(BTree* tree)
{
    if (!tree) return;

    if (tree->root) {
        btree_node_destroy(tree->root);
        tree->root = NULL;
    }

    free(tree);
}

bool btree_insert(BTree* tree, int key, int value)
{
    if (!tree) return false;

    if (tree->root->key_count == 2 * tree->degree - 1) {
        BTreeNode* old_root = tree->root;
        BTreeNode* new_root = btree_node_create(tree->degree, false);
        if (!new_root) return false;

        new_root->children[0] = old_root;
        tree->root = new_root;
        tree->node_count++;

        btree_split_child(new_root, 0, old_root);
        btree_insert_nonfull(new_root, key, value);
    }
    else {
        btree_insert_nonfull(tree->root, key, value);
    }

    tree->size++;
    tree->height = btree_calculate_height(tree->root);
    tree->node_count = btree_count_nodes(tree->root);

    return true;
}

bool btree_search(BTree* tree, int key, int* out_value)
{
    if (!tree || !tree->root) return false;

    BTreeNode* node = tree->root;

    while (node) {
        int idx = btree_node_find_key(node, key);
        if (idx != -1) {
            if (out_value) *out_value = node->values[idx];
            return true;
        }

        if (node->is_leaf) return false;

        int i = btree_node_find_insert_pos(node, key);
        node = node->children[i];
    }

    return false;
}

bool btree_contains(BTree* tree, int key)
{
    return btree_search(tree, key, NULL);
}

/*
 * УДАЛЕНИЕ КЛЮЧА - использует классическую реализацию
 */
bool btree_delete(BTree* tree, int key)
{
    if (!tree || !tree->root) return false;

    /* Если ключа нет - возвращаем false */
    if (!btree_contains(tree, key)) return false;

    int child_height = 0;
    bool result = btree_delete_from_subtree(tree->root, key, tree->degree, &child_height);

    if (result) {
        tree->size--;
        tree->height = btree_calculate_height(tree->root);
        tree->node_count = btree_count_nodes(tree->root);

        /* Если корень стал пустым и у него есть потомок */
        if (tree->root->key_count == 0 && !tree->root->is_leaf) {
            BTreeNode* old_root = tree->root;
            tree->root = tree->root->children[0];
            tree->node_count--;

            if (old_root) {
                if (old_root->keys) free(old_root->keys);
                if (old_root->values) free(old_root->values);
                if (old_root->children) free(old_root->children);
                free(old_root);
            }
        }
    }

    return result;
}

int btree_size(BTree* tree) { return tree ? tree->size : 0; }
int btree_height(BTree* tree) { return tree ? tree->height : 0; }
int btree_degree(BTree* tree) { return tree ? tree->degree : 0; }
int btree_nodes(BTree* tree) { return tree ? tree->node_count : 0; }

void btree_print_stats(BTree* tree)
{
    if (!tree) {
        printf("Дерево не инициализировано\n");
        return;
    }

    printf("Статистика B-дерева:\n");
    printf("  Степень: %d\n", tree->degree);
    printf("  Количество элементов: %d\n", tree->size);
    printf("  Высота: %d\n", tree->height);
    printf("  Количество узлов: %d\n", tree->node_count);
    printf("  Корень: %s\n", tree->root ? "существует" : "NULL");
}

void btree_print(BTree* tree)
{
    if (!tree || !tree->root) {
        printf("Дерево пустое\n");
        return;
    }

    printf("\nB-ДЕРЕВО:\n");
    printf("Степень: %d, элементов: %d, высота: %d\n",
        tree->degree, tree->size, tree->height);
    btree_print_node(tree->root, 0, "R-");
}

/*
 * ============================================================================
 * ИТЕРАТОР
 * ============================================================================
 */

 /*
  * Рекурсивный сбор всех ключей из дерева (полный обход)
  */
static void btree_collect_keys(BTreeNode* node, int** keys, int** values, int* size, int* capacity)
{
    if (!node) return;

    /* Сначала обходим всех потомков (рекурсивно) */
    for (int i = 0; i <= node->key_count; i++) {
        if (node->children[i]) {
            btree_collect_keys(node->children[i], keys, values, size, capacity);
        }
    }

    /* Добавляем ключи текущего узла */
    for (int i = 0; i < node->key_count; i++) {
        if (*size >= *capacity) {
            *capacity *= 2;
            *keys = (int*)realloc(*keys, *capacity * sizeof(int));
            *values = (int*)realloc(*values, *capacity * sizeof(int));
        }
        (*keys)[*size] = node->keys[i];
        (*values)[*size] = node->values[i];
        (*size)++;
    }
}

BTreeIterator btree_iterator_create(BTree* tree)
{
    BTreeIterator iter;
    iter.is_valid = false;
    iter.size = 0;
    iter.current = 0;
    iter.keys = NULL;
    iter.values = NULL;

    if (!tree || !tree->root) {
        return iter;
    }

    int capacity = 64;
    iter.keys = (int*)malloc(capacity * sizeof(int));
    iter.values = (int*)malloc(capacity * sizeof(int));

    if (!iter.keys || !iter.values) {
        return iter;
    }

    btree_collect_keys(tree->root, &iter.keys, &iter.values, &iter.size, &capacity);
    iter.is_valid = (iter.size > 0);
    iter.current = 0;

    return iter;
}

void btree_iterator_destroy(BTreeIterator* iter)
{
    if (!iter) return;
    if (iter->keys) free(iter->keys);
    if (iter->values) free(iter->values);
    iter->keys = NULL;
    iter->values = NULL;
    iter->size = 0;
    iter->current = 0;
    iter->is_valid = false;
}

bool btree_iterator_next(BTreeIterator* iter)
{
    if (!iter->is_valid) return false;
    iter->current++;
    if (iter->current >= iter->size) {
        iter->is_valid = false;
        return false;
    }
    return true;
}

int btree_iterator_key(BTreeIterator* iter)
{
    if (!iter->is_valid || iter->current >= iter->size) return 0;
    return iter->keys[iter->current];
}

int btree_iterator_value(BTreeIterator* iter)
{
    if (!iter->is_valid || iter->current >= iter->size) return 0;
    return iter->values[iter->current];
}

bool btree_iterator_valid(BTreeIterator* iter)
{
    return iter ? iter->is_valid : false;
}