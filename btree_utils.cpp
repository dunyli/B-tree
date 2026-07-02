/*
 * btree_utils.c - вспомогательные функции для B-дерева
 * Содержит функции поиска минимума/максимума, подсчёта узлов,
 * вычисления высоты, печати, разделения и вставки
 */

#define _CRT_SECURE_NO_WARNINGS

#include "btree_utils.h"
#include "btree_node.h"
#include <stdio.h>
#include <stdlib.h>

 /*
  * Поиск минимального ключа в поддереве
  * Идём по левым потомкам до самого левого листа
  * Возвращает указатель на узел с минимальным ключом
  */
BTreeNode*
btree_find_minimum(BTreeNode* node)
{
    if (!node) return NULL;          /* Если узел пустой - возвращаем NULL */
    if (node->is_leaf) return node;  /* Если лист - возвращаем его */
    return btree_find_minimum(node->children[0]);  /* Идём к левому потомку */
}

/*
 * Поиск максимального ключа в поддереве
 * Идём по правым потомкам до самого правого листа
 * Возвращает указатель на узел с максимальным ключом
 */
BTreeNode*
btree_find_maximum(BTreeNode* node)
{
    if (!node) return NULL;           /* Если узел пустой - возвращаем NULL */
    if (node->is_leaf) return node;   /* Если лист - возвращаем его */
    return btree_find_maximum(node->children[node->key_count]);  /* Идём к правому потомку */
}

/*
 * Подсчёт количества узлов в дереве
 * Рекурсивно обходит всё дерево и считает узлы
 */
int
btree_count_nodes(BTreeNode* node)
{
    if (!node) return 0;  /* Пустой узел - 0 */

    int count = 1;  /* Считаем текущий узел */

    /* Если не лист, рекурсивно считаем всех потомков */
    if (!node->is_leaf) {
        for (int i = 0; i <= node->key_count; i++) {
            if (node->children[i]) {
                count += btree_count_nodes(node->children[i]);
            }
        }
    }
    return count;
}

/*
 * Вычисление высоты дерева
 * Высота = количество уровней от корня до листа
 * У пустого дерева высота 0, у дерева из одного узла - 1
 */
int
btree_calculate_height(BTreeNode* node)
{
    if (!node) return 0;             /* Пустое дерево - высота 0 */
    if (node->is_leaf) return 1;     /* Один узел - высота 1 */
    return 1 + btree_calculate_height(node->children[0]);  /* 1 + высота потомка */
}

/*
 * Рекурсивная печать узла и всех его потомков
 * Используется для отладки и визуализации структуры дерева
 * level - уровень вложенности (для отступов)
 * side - метка стороны (R-корень, child_0_-левый, child_1_-правый)
 */
void
btree_print_node(BTreeNode* node, int level, const char* side)
{
    if (!node) return;  /* Если узел пустой - выходим */

    /* Выводим отступы для визуализации уровня вложенности */
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
    printf("%s[%d] ", side, level);  /* Выводим метку и уровень */

    /* Выводим все ключи и значения в узле */
    for (int i = 0; i < node->key_count; i++) {
        printf("(%d:%d) ", node->keys[i], node->values[i]);
    }
    printf("(leaf=%d)\n", node->is_leaf);  /* Показываем, лист это или нет */

    /* Если не лист, рекурсивно выводим всех потомков */
    if (!node->is_leaf) {
        for (int i = 0; i <= node->key_count; i++) {
            char child_side[32];
            snprintf(child_side, sizeof(child_side), "child_%d_", i);
            if (node->children[i]) {
                btree_print_node(node->children[i], level + 1, child_side);
            }
        }
    }
}

/*
 * Разделение полного узла (ключевая операция B-дерева)
 *
 * Вызывается когда узел имеет 2t-1 ключей и нужно вставить ещё один
 * Создаётся новый узел, средний ключ поднимается в родителя
 *
 * parent - родительский узел
 * child_idx - индекс потомка в родителе
 * child - полный узел для разделения
 */
void
btree_split_child(BTreeNode* parent, int child_idx, BTreeNode* child)
{
    if (!parent || !child) return;  /* Проверка на NULL */
    if (child_idx < 0 || child_idx > parent->key_count) return;

    int degree = child->degree;  /* Степень дерева */
    int t = degree;

    /* Создаём новый узел (правый брат) */
    BTreeNode* new_node = btree_node_create(degree, child->is_leaf);
    if (!new_node) return;

    /* Копируем последние t-1 ключей из child в new_node */
    for (int i = 0; i < t - 1; i++) {
        new_node->keys[i] = child->keys[i + t];
        new_node->values[i] = child->values[i + t];
    }

    /* Если не лист, копируем соответствующих потомков */
    if (!child->is_leaf) {
        for (int i = 0; i < t; i++) {
            new_node->children[i] = child->children[i + t];
        }
    }

    new_node->key_count = t - 1;
    child->key_count = t - 1;

    /* Сдвигаем потомков родителя, освобождая место для нового узла */
    for (int i = parent->key_count; i > child_idx; i--) {
        parent->children[i + 1] = parent->children[i];
    }
    parent->children[child_idx + 1] = new_node;

    /* Сдвигаем ключи родителя, освобождая место для среднего ключа */
    for (int i = parent->key_count - 1; i >= child_idx; i--) {
        parent->keys[i + 1] = parent->keys[i];
        parent->values[i + 1] = parent->values[i];
    }

    /* Поднимаем средний ключ в родителя */
    parent->keys[child_idx] = child->keys[t - 1];
    parent->values[child_idx] = child->values[t - 1];
    parent->key_count++;
}

/*
 * Вставка ключа в неполный узел
 *
 * Рекурсивно спускается по дереву, находит место для вставки
 * Если потомок полный - разделяет его перед вставкой
 */
void
btree_insert_nonfull(BTreeNode* node, int key, int value)
{
    if (!node) return;

    int i = node->key_count - 1;

    /* Если узел лист - вставляем ключ */
    if (node->is_leaf) {
        /* Находим позицию для вставки */
        i = btree_node_find_insert_pos(node, key);

        /* Сдвигаем ключи и значения вправо, освобождая место */
        for (int j = node->key_count; j > i; j--) {
            node->keys[j] = node->keys[j - 1];
            node->values[j] = node->values[j - 1];
        }

        /* Вставляем новый ключ и значение */
        node->keys[i] = key;
        node->values[i] = value;
        node->key_count++;
    }
    /* Внутренний узел - идём в потомка */
    else {
        /* Находим потомка для вставки */
        i = btree_node_find_insert_pos(node, key);

        /* Если потомок не существует, создаём его */
        if (!node->children[i]) {
            node->children[i] = btree_node_create(node->degree, true);
            if (!node->children[i]) return;
        }

        /* Если потомок полный - разделяем его */
        int max_keys = 2 * node->degree - 1;
        if (node->children[i]->key_count == max_keys) {
            btree_split_child(node, i, node->children[i]);

            /* После разделения выбираем правильного потомка */
            if (key > node->keys[i]) {
                i++;
            }
        }

        /* Рекурсивно вставляем в потомка */
        btree_insert_nonfull(node->children[i], key, value);
    }
}