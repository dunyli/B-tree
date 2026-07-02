/*
 * btree_utils.c - вспомогательные функции для B-дерева
 *
 * Содержит реализации вспомогательных функций:
 * - поиск минимума/максимума в поддереве
 * - подсчёт количества узлов
 * - вычисление высоты дерева
 * - разделение полных узлов
 * - вставка в неполный узел
 *
 * Примечание: Функция печати дерева (btree_print_node и btree_print)
 * находится в файле btree.c, так как ей требуется доступ к полной
 * структуре BTree (которая определена только в btree.c)
 */

#define _CRT_SECURE_NO_WARNINGS

#include "btree_utils.h"   /* Подключаем заголовочный файл */
#include "btree_node.h"    /* Для работы с узлами B-дерева */
#include <stdio.h>         /* Для printf (используется в отладке) */
#include <stdlib.h>        /* Для malloc, free */

 /*
  * ПОИСК МИНИМАЛЬНОГО КЛЮЧА В ПОДДЕРЕВЕ
  *
  * Алгоритм: рекурсивно спускаемся по левым потомкам до листа
  * Возвращает: указатель на узел с минимальным ключом
  */
BTreeNode* btree_find_minimum(BTreeNode* node)
{
    if (!node) return NULL;           /* Если узел пустой - возвращаем NULL */
    if (node->is_leaf) return node;   /* Если лист - возвращаем его */
    return btree_find_minimum(node->children[0]);  /* Идём к левому потомку */
}

/*
 * ПОИСК МАКСИМАЛЬНОГО КЛЮЧА В ПОДДЕРЕВЕ
 *
 * Алгоритм: рекурсивно спускаемся по правым потомкам до листа
 * Возвращает: указатель на узел с максимальным ключом
 */
BTreeNode* btree_find_maximum(BTreeNode* node)
{
    if (!node) return NULL;                /* Если узел пустой - возвращаем NULL */
    if (node->is_leaf) return node;        /* Если лист - возвращаем его */
    return btree_find_maximum(node->children[node->key_count]);  /* Идём к правому потомку */
}

/*
 * ПОДСЧЁТ КОЛИЧЕСТВА УЗЛОВ В ДЕРЕВЕ
 *
 * Рекурсивно обходит всё дерево и считает каждый узел
 * Возвращает: общее количество узлов в поддереве
 */
int btree_count_nodes(BTreeNode* node)
{
    if (!node) return 0;           /* Пустой узел - возвращаем 0 */

    int count = 1;                 /* Считаем текущий узел */

    /* Если не лист - рекурсивно считаем всех потомков */
    if (!node->is_leaf) {
        for (int i = 0; i <= node->key_count; i++) {
            if (node->children[i]) {
                count += btree_count_nodes(node->children[i]);
            }
        }
    }
    return count;                  /* Возвращаем общее количество */
}

/*
 * ВЫЧИСЛЕНИЕ ВЫСОТЫ ДЕРЕВА
 *
 * Высота = количество уровней от корня до самого дальнего листа
 * У пустого дерева высота 0, у дерева из одного узла - 1
 * Возвращает: высоту поддерева
 */
int btree_calculate_height(BTreeNode* node)
{
    if (!node) return 0;             /* Пустое дерево - высота 0 */
    if (node->is_leaf) return 1;     /* Один узел - высота 1 */
    return 1 + btree_calculate_height(node->children[0]);  /* 1 + высота потомка */
}

/*
 * РАЗДЕЛЕНИЕ ПОЛНОГО УЗЛА (КЛЮЧЕВАЯ ОПЕРАЦИЯ B-ДЕРЕВА)
 *
 * Вызывается когда узел имеет 2t-1 ключей (полный) и нужно вставить ещё один.
 *
 * Алгоритм:
 * 1. Создаётся новый узел (правый брат)
 * 2. Средний ключ (индекс t-1) поднимается в родителя
 * 3. Ключи после среднего (индексы t..2t-2) копируются в новый узел
 * 4. Если не лист - копируются соответствующие потомки
 * 5. Обновляются связи в родителе
 *
 * Параметры:
 *   parent     - родительский узел (будет содержать поднятый ключ)
 *   child_idx  - индекс полного потомка в родителе
 *   child      - полный узел для разделения (имеет 2t-1 ключей)
 */
void btree_split_child(BTreeNode* parent, int child_idx, BTreeNode* child)
{
    /* Проверка корректности входных данных */
    if (!parent || !child) return;
    if (child_idx < 0 || child_idx > parent->key_count) return;

    int degree = child->degree;    /* Получаем степень дерева */
    int t = degree;                /* t - минимальная степень */

    /* ШАГ 1: СОЗДАЁМ НОВЫЙ УЗЕЛ (правый брат) */
    BTreeNode* new_node = btree_node_create(degree, child->is_leaf);
    if (!new_node) return;

    /* ШАГ 2: КОПИРУЕМ ПОСЛЕДНИЕ t-1 КЛЮЧЕЙ из child в new_node */
    for (int i = 0; i < t - 1; i++) {
        new_node->keys[i] = child->keys[i + t];
        new_node->values[i] = child->values[i + t];
    }

    /* ШАГ 3: ЕСЛИ НЕ ЛИСТ - КОПИРУЕМ ПОТОМКОВ */
    if (!child->is_leaf) {
        for (int i = 0; i < t; i++) {
            new_node->children[i] = child->children[i + t];
        }
    }

    /* Устанавливаем количество ключей в обоих узлах */
    new_node->key_count = t - 1;   /* В новом узле t-1 ключей */
    child->key_count = t - 1;      /* В старом тоже t-1 (средний уходит вверх) */

    /* ШАГ 4: СДВИГАЕМ ПОТОМКОВ РОДИТЕЛЯ */
    for (int i = parent->key_count; i > child_idx; i--) {
        parent->children[i + 1] = parent->children[i];
    }
    parent->children[child_idx + 1] = new_node;  /* Вставляем новый узел */

    /* ШАГ 5: СДВИГАЕМ КЛЮЧИ РОДИТЕЛЯ */
    for (int i = parent->key_count - 1; i >= child_idx; i--) {
        parent->keys[i + 1] = parent->keys[i];
        parent->values[i + 1] = parent->values[i];
    }

    /* ШАГ 6: ПОДНИМАЕМ СРЕДНИЙ КЛЮЧ в родителя */
    parent->keys[child_idx] = child->keys[t - 1];
    parent->values[child_idx] = child->values[t - 1];
    parent->key_count++;  /* Увеличиваем количество ключей в родителе */
}

/*
 * ВСТАВКА КЛЮЧА В НЕПОЛНЫЙ УЗЕЛ
 *
 * Рекурсивная функция, которая вставляет ключ в поддерево.
 * Предполагается, что узел НЕ полный (key_count < 2t-1)
 *
 * Алгоритм:
 * 1. Если узел лист - вставляем ключ на правильную позицию
 * 2. Если внутренний узел:
 *    a) Находим потомка для вставки
 *    b) Если потомок полный - разделяем его
 *    c) Рекурсивно вставляем в потомка
 *
 * Параметры:
 *   node   - текущий узел (не полный)
 *   key    - ключ для вставки
 *   value  - значение для вставки
 */
void btree_insert_nonfull(BTreeNode* node, int key, int value)
{
    if (!node) return;   /* Если узел пустой - выходим */

    int i = node->key_count - 1;  /* Индекс последнего ключа */

    /* ============================================================
     * СЛУЧАЙ 1: УЗЕЛ ЛИСТ - вставляем ключ напрямую
     * ============================================================ */
    if (node->is_leaf) {
        /* Находим позицию для вставки */
        i = btree_node_find_insert_pos(node, key);

        /* Сдвигаем все ключи и значения вправо, освобождая место */
        for (int j = node->key_count; j > i; j--) {
            node->keys[j] = node->keys[j - 1];
            node->values[j] = node->values[j - 1];
        }

        /* Вставляем новый ключ и значение */
        node->keys[i] = key;
        node->values[i] = value;
        node->key_count++;  /* Увеличиваем количество ключей */
    }
    /* ============================================================
     * СЛУЧАЙ 2: ВНУТРЕННИЙ УЗЕЛ - идём в потомка
     * ============================================================ */
    else {
        /* Находим потомка, в который нужно вставить */
        i = btree_node_find_insert_pos(node, key);

        /* Если потомок не существует - создаём его */
        if (!node->children[i]) {
            node->children[i] = btree_node_create(node->degree, true);
            if (!node->children[i]) return;
        }

        /* Если потомок полный (2t-1 ключей) - разделяем его */
        int max_keys = 2 * node->degree - 1;
        if (node->children[i]->key_count == max_keys) {
            btree_split_child(node, i, node->children[i]);

            /* После разделения выбираем правильного потомка */
            if (key > node->keys[i]) {
                i++;  /* Переключаемся на правого потомка (новый узел) */
            }
        }

        /* Рекурсивно вставляем в потомка */
        btree_insert_nonfull(node->children[i], key, value);
    }
}