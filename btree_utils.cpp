/* btree_utils.c (вспомогательные функции) */

#define _CRT_SECURE_NO_WARNINGS

#include "btree_utils.h"   /* Заголовочный файл */
#include "btree_node.h"    /* Для работы с узлами */
#include <stdio.h>         /* Для printf */
#include <stdlib.h>        /* Для malloc, free */

 /*
  * Поиск минимального ключа в поддереве
  * Идём по левым потомкам до самого левого листа
  */
BTreeNode*
btree_find_minimum(BTreeNode* node)
{
    /* Если узел лист — возвращаем его */
    if (node->is_leaf)
        return node;
    /* Иначе идём к крайнему левому потомку (индекс 0) */
    return btree_find_minimum(node->children[0]);
}

/*
 * Поиск максимального ключа в поддереве
 * Идём по правым потомкам до самого правого листа
 */
BTreeNode*
btree_find_maximum(BTreeNode* node)
{
    /* Если узел лист — возвращаем его */
    if (node->is_leaf)
        return node;
    /* Иначе идём к крайнему правому потомку (последний) */
    return btree_find_maximum(node->children[node->key_count]);
}

/*
 * Подсчёт количества узлов в дереве
 * Рекурсивно обходит всё дерево
 */
int
btree_count_nodes(BTreeNode* node)
{
    if (!node) return 0;  /* Если нет узла — 0 */

    int count = 1;  /* Считаем текущий узел */

    /* Если не лист, считаем всех потомков */
    if (!node->is_leaf) {
        for (int i = 0; i <= node->key_count; i++) {
            count += btree_count_nodes(node->children[i]);
        }
    }
    return count;  /* Возвращаем общее количество */
}

/*
 * Вычисление высоты дерева
 * Высота = количество уровней (все листья на одном уровне)
 * Рекурсивно спускаемся к самому левому листу
 */
int
btree_calculate_height(BTreeNode* node)
{
    /* Базовый случай: лист или NULL */
    if (!node || node->is_leaf)
        return 1;
    /* Иначе: 1 + высота потомка (все потомки имеют одинаковую высоту) */
    return 1 + btree_calculate_height(node->children[0]);
}

/*
 * Рекурсивная печать узла и всех его потомков
 * Используется для отладки и визуализации структуры
 */
void
btree_print_node(BTreeNode* node, int level, const char* side)
{
    if (!node) return;  /* Если узел NULL — выходим */

    /* Выводим отступы для визуализации уровня вложенности */
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
    printf("%s[%d] ", side, level);  /* Выводим сторону (L/R) и уровень */

    /* Выводим все ключи и значения в узле */
    for (int i = 0; i < node->key_count; i++) {
        printf("(%d:%d) ", node->keys[i], node->values[i]);
    }
    printf("(leaf=%d)\n", node->is_leaf);  /* Выводим, лист или нет */

    /* Рекурсивно выводим всех потомков */
    if (!node->is_leaf) {
        for (int i = 0; i <= node->key_count; i++) {
            char child_side[16];
            snprintf(child_side, sizeof(child_side), "  └─%d─", i);
            btree_print_node(node->children[i], level + 1, child_side);
        }
    }
}

/*
 * РАЗДЕЛЕНИЕ ПОЛНОГО УЗЛА
 *
 * Это одна из ключевых операций B-дерева.
 * Вызывается, когда узел содержит 2t-1 ключей (полный).
 *
 * Алгоритм:
 * 1. Создаётся новый узел (правый брат)
 * 2. Средний ключ (индекс t-1) поднимается в родителя
 * 3. Ключи после среднего перемещаются в новый узел
 * 4. Связи между узлами обновляются
 */
void
btree_split_child(BTreeNode* parent, int child_idx, BTreeNode* child)
{
    int degree = child->degree;   /* Степень дерева */
    int t = degree;               /* t — минимальная степень */

    /*
     * ШАГ 1: СОЗДАЁМ НОВЫЙ УЗЕЛ (правый брат)
     * У нового узла та же степень и тот же признак листа
     */
    BTreeNode* new_node = btree_node_create(degree, child->is_leaf);
    if (!new_node) return;  /* Проверка */

    /*
     * ШАГ 2: КОПИРУЕМ ПОСЛЕДНИЕ t-1 КЛЮЧЕЙ из child в new_node
     * Ключи с индекса t до 2t-2 (всего t-1 штук)
     */
    for (int i = 0; i < t - 1; i++) {
        new_node->keys[i] = child->keys[i + t];
        new_node->values[i] = child->values[i + t];
    }

    /*
     * ШАГ 3: ЕСЛИ НЕ ЛИСТ, КОПИРУЕМ ПОТОМКОВ
     * Потомки с индекса t до 2t-1
     */
    if (!child->is_leaf) {
        for (int i = 0; i < t; i++) {
            new_node->children[i] = child->children[i + t];
        }
    }

    /* Устанавливаем количество ключей в обоих узлах */
    new_node->key_count = t - 1;   /* В новом узле t-1 ключей */
    child->key_count = t - 1;      /* В старом узле тоже t-1 ключей (средний уходит) */

    /*
     * ШАГ 4: СДВИГАЕМ ПОТОМКОВ РОДИТЕЛЯ
     * Освобождаем место для нового узла
     */
    for (int i = parent->key_count; i > child_idx + 1; i--) {
        parent->children[i + 1] = parent->children[i];
    }
    parent->children[child_idx + 1] = new_node;  /* Вставляем новый узел */

    /*
     * ШАГ 5: СДВИГАЕМ КЛЮЧИ РОДИТЕЛЯ
     * Освобождаем место для среднего ключа
     */
    for (int i = parent->key_count - 1; i >= child_idx; i--) {
        parent->keys[i + 1] = parent->keys[i];
        parent->values[i + 1] = parent->values[i];
    }

    /*
     * ШАГ 6: ПОДНИМАЕМ СРЕДНИЙ КЛЮЧ в родителя
     * Это ключ с индексом t-1 из child
     */
    parent->keys[child_idx] = child->keys[t - 1];
    parent->values[child_idx] = child->values[t - 1];
    parent->key_count++;  /* Увеличиваем количество ключей в родителе */
}

/*
 * ВСТАВКА КЛЮЧА В НЕПОЛНЫЙ УЗЕЛ
 *
 * Рекурсивная функция, которая вставляет ключ в поддерево.
 * Предполагается, что узел не полный (key_count < 2t-1)
 */
void
btree_insert_nonfull(BTreeNode* node, int key, int value)
{
    int i = node->key_count - 1;  /* Индекс последнего ключа */

    /* СЛУЧАЙ 1: УЗЕЛ ЛИСТ — вставляем ключ */
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
    /* СЛУЧАЙ 2: ВНУТРЕННИЙ УЗЕЛ — идём в потомка */
    else {
        /* Находим потомка, в который нужно вставить */
        i = btree_node_find_insert_pos(node, key);

        /* Если потомок полный — разделяем его */
        if (node->children[i]->key_count == 2 * node->degree - 1) {
            btree_split_child(node, i, node->children[i]);

            /* После разделения выбираем правильного потомка */
            if (key > node->keys[i]) {
                i++;  /* Переключаемся на нового правого потомка */
            }
        }

        /* Рекурсивно вставляем в потомка */
        btree_insert_nonfull(node->children[i], key, value);
    }
}
