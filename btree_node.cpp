/* btree_node.c (реализация узла) */

#define _CRT_SECURE_NO_WARNINGS

#include "btree_node.h"    /* Заголовочный файл */
#include <stdlib.h>        /* Для malloc, free */

/*
 * Создание нового узла B-дерева
 * Выделяет память под все массивы
 */
BTreeNode*
btree_node_create(int degree, bool is_leaf)
{
    BTreeNode* node = (BTreeNode*)malloc(sizeof(BTreeNode));  /* Память под узел */
    if (!node) return NULL;                                   /* Проверка */

    node->degree = degree;           /* Сохраняем степень */
    node->is_leaf = is_leaf;         /* Сохраняем флаг листа */
    node->key_count = 0;             /* Ключей пока нет */

    /* Выделяем память под ключи (максимум 2t-1) */
    node->keys = (int*)malloc((2 * degree - 1) * sizeof(int));
    if (!node->keys) { free(node); return NULL; }

    /* Выделяем память под значения (максимум 2t-1) */
    node->values = (int*)malloc((2 * degree - 1) * sizeof(int));
    if (!node->values) { free(node->keys); free(node); return NULL; }

    /* Выделяем память под потомков (максимум 2t) */
    node->children = (BTreeNode**)malloc((2 * degree) * sizeof(BTreeNode*));
    if (!node->children) {
        free(node->values); free(node->keys); free(node);
        return NULL;
    }

    /* Инициализируем всех потомков как NULL */
    for (int i = 0; i < 2 * degree; i++) {
        node->children[i] = NULL;
    }

    return node;  /* Возвращаем созданный узел */
}

/*
 * Освобождение узла и всех потомков
 * Рекурсивно обходит и удаляет всё поддерево
 */
void
btree_node_destroy(BTreeNode* node)
{
    if (!node) return;  /* Если узел NULL — ничего не делаем */

    /* Если не лист, рекурсивно удаляем всех потомков */
    if (!node->is_leaf) {
        for (int i = 0; i <= node->key_count; i++) {
            btree_node_destroy(node->children[i]);  /* Рекурсивный вызов */
        }
    }

    /* Освобождаем все массивы */
    free(node->keys);          /* Освобождаем ключи */
    free(node->values);        /* Освобождаем значения */
    free(node->children);      /* Освобождаем потомков */
    free(node);                /* Освобождаем сам узел */
}

/*
 * Поиск ключа в узле (бинарный поиск)
 * Возвращает индекс ключа в массиве или -1
 */
int
btree_node_find_key(BTreeNode* node, int key)
{
    int left = 0;                      /* Левая граница */
    int right = node->key_count - 1;   /* Правая граница */

    /* Бинарный поиск по отсортированному массиву ключей */
    while (left <= right) {
        int mid = left + (right - left) / 2;  /* Середина */
        if (node->keys[mid] == key) return mid;      /* Нашли */
        if (node->keys[mid] < key) left = mid + 1;   /* Ищем в правой части */
        else right = mid - 1;                        /* Ищем в левой части */
    }
    return -1;  /* Не найден */
}

/*
 * Поиск позиции для вставки ключа
 * Возвращает индекс, куда нужно вставить новый ключ
 */
int
btree_node_find_insert_pos(BTreeNode* node, int key)
{
    int i = 0;
    /* Проходим по ключам, пока не найдём место */
    while (i < node->key_count && node->keys[i] < key) {
        i++;
    }
    return i;  /* Возвращаем позицию для вставки */
}