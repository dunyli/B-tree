/* btree_node.c (реализация узла) */

#define _CRT_SECURE_NO_WARNINGS      /* Отключаем предупреждения безопасности */

#include "btree_node.h"              /* Подключаем заголовочный файл */
#include <stdlib.h>                  /* Подключаем для malloc, free */

/*
 * Создание нового узла B-дерева
 * Выделяет память под узел и все массивы
 */
BTreeNode* btree_node_create(int degree, bool is_leaf)
{
    BTreeNode* node = (BTreeNode*)malloc(sizeof(BTreeNode));  /* Выделяем память под узел */
    if (!node) return NULL;                                   /* Проверка выделения */

    node->degree = degree;           /* Сохраняем степень дерева */
    node->is_leaf = is_leaf;         /* Сохраняем флаг листа (true/false) */
    node->key_count = 0;             /* Изначально ключей нет */

    /* Выделяем память под массив ключей (максимум 2t-1) */
    node->keys = (int*)malloc((2 * degree - 1) * sizeof(int));
    if (!node->keys) { free(node); return NULL; }    /* Проверка и очистка */

    /* Выделяем память под массив значений (максимум 2t-1) */
    node->values = (int*)malloc((2 * degree - 1) * sizeof(int));
    if (!node->values) { free(node->keys); free(node); return NULL; }

    /* Выделяем память под массив потомков (максимум 2t) */
    node->children = (BTreeNode**)malloc((2 * degree) * sizeof(BTreeNode*));
    if (!node->children) {
        free(node->values); free(node->keys); free(node);
        return NULL;
    }

    /* Инициализируем всех потомков как NULL (безопасность) */
    for (int i = 0; i < 2 * degree; i++) {
        node->children[i] = NULL;
    }

    return node;  /* Возвращаем созданный узел */
}

/*
 * Освобождение узла и всех потомков (рекурсивно)
 */
void btree_node_destroy(BTreeNode* node)
{
    if (!node) return;  /* Если узел NULL — ничего не делаем */

    /* Если узел не лист — рекурсивно удаляем всех потомков */
    if (!node->is_leaf) {
        for (int i = 0; i <= node->key_count; i++) {
            btree_node_destroy(node->children[i]);  /* Рекурсивный вызов для каждого потомка */
        }
    }

    /* Освобождаем все динамические массивы */
    free(node->keys);          /* Освобождаем массив ключей */
    free(node->values);        /* Освобождаем массив значений */
    free(node->children);      /* Освобождаем массив потомков */
    free(node);                /* Освобождаем сам узел */
}

/*
 * Поиск ключа в узле (бинарный поиск по отсортированному массиву)
 * Возвращает индекс ключа в массиве или -1 если не найден
 */
int btree_node_find_key(BTreeNode* node, int key)
{
    int left = 0;                      /* Левая граница поиска (начало) */
    int right = node->key_count - 1;   /* Правая граница поиска (конец) */

    /* Стандартный бинарный поиск */
    while (left <= right) {
        int mid = left + (right - left) / 2;  /* Вычисляем середину */
        if (node->keys[mid] == key) return mid;      /* Нашли ключ — возвращаем индекс */
        if (node->keys[mid] < key) left = mid + 1;   /* Ищем в правой половине */
        else right = mid - 1;                        /* Ищем в левой половине */
    }
    return -1;  /* Ключ не найден */
}

/*
 * Поиск позиции для вставки ключа (бинарный поиск)
 * Возвращает индекс, куда нужно вставить новый ключ
 */
int btree_node_find_insert_pos(BTreeNode* node, int key)
{
    int i = 0;
    /* Проходим по ключам, пока не найдём место (первый ключ > key) */
    while (i < node->key_count && node->keys[i] < key) {
        i++;  /* Переходим к следующему ключу */
    }
    return i;  /* Возвращаем позицию для вставки */
}
