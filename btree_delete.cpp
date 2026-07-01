/*
 *  btree_delete.c (удаление)
 */

#define _CRT_SECURE_NO_WARNINGS

#include "btree_delete.h"   /* Заголовочный файл */
#include "btree_node.h"     /* Для работы с узлами */
#include "btree_utils.h"    /* Для вспомогательных функций */
#include <stdlib.h>         /* Для free */

 /*
  * ЗАИМСТВОВАНИЕ КЛЮЧА ОТ ЛЕВОГО БРАТА
  *
  * Используется, когда у узла мало ключей (< t-1),
  * а у левого брата есть лишние ключи (>= t)
  *
  * Переносим один ключ от левого брата через родителя
  */
void
btree_borrow_from_left(BTreeNode* parent, int child_idx)
{
    BTreeNode* child = parent->children[child_idx];
    BTreeNode* left_sibling = parent->children[child_idx - 1];

    /* 1. Сдвигаем ключи child вправо, освобождая место в начале */
    for (int i = child->key_count - 1; i >= 0; i--) {
        child->keys[i + 1] = child->keys[i];
        child->values[i + 1] = child->values[i];
    }

    /* 2. Сдвигаем потомков child вправо */
    if (!child->is_leaf) {
        for (int i = child->key_count; i >= 0; i--) {
            child->children[i + 1] = child->children[i];
        }
    }

    /* 3. Переносим ключ из родителя в child (на позицию 0) */
    child->keys[0] = parent->keys[child_idx - 1];
    child->values[0] = parent->values[child_idx - 1];

    /* 4. Переносим последний ключ из левого брата в родителя */
    parent->keys[child_idx - 1] = left_sibling->keys[left_sibling->key_count - 1];
    parent->values[child_idx - 1] = left_sibling->values[left_sibling->key_count - 1];

    /* 5. Переносим последнего потомка из левого брата в child */
    if (!child->is_leaf) {
        child->children[0] = left_sibling->children[left_sibling->key_count];
    }

    /* 6. Обновляем счётчики ключей */
    child->key_count++;                /* У child стало на 1 больше */
    left_sibling->key_count--;         /* У левого брата на 1 меньше */
}

/*
 * ЗАИМСТВОВАНИЕ КЛЮЧА ОТ ПРАВОГО БРАТА
 *
 * Симметрично заимствованию от левого брата
 */
void
btree_borrow_from_right(BTreeNode* parent, int child_idx)
{
    BTreeNode* child = parent->children[child_idx];
    BTreeNode* right_sibling = parent->children[child_idx + 1];

    /* 1. Переносим ключ из родителя в child (в конец) */
    child->keys[child->key_count] = parent->keys[child_idx];
    child->values[child->key_count] = parent->values[child_idx];

    /* 2. Переносим первого потомка из правого брата в child */
    if (!child->is_leaf) {
        child->children[child->key_count + 1] = right_sibling->children[0];
    }

    /* 3. Переносим первый ключ из правого брата в родителя */
    parent->keys[child_idx] = right_sibling->keys[0];
    parent->values[child_idx] = right_sibling->values[0];

    /* 4. Сдвигаем ключи правого брата влево */
    for (int i = 0; i < right_sibling->key_count - 1; i++) {
        right_sibling->keys[i] = right_sibling->keys[i + 1];
        right_sibling->values[i] = right_sibling->values[i + 1];
    }

    /* 5. Сдвигаем потомков правого брата влево */
    if (!right_sibling->is_leaf) {
        for (int i = 0; i < right_sibling->key_count; i++) {
            right_sibling->children[i] = right_sibling->children[i + 1];
        }
    }

    /* 6. Обновляем счётчики ключей */
    child->key_count++;                 /* У child стало на 1 больше */
    right_sibling->key_count--;         /* У правого брата на 1 меньше */
}

/*
 * СЛИЯНИЕ ДВУХ СОСЕДНИХ УЗЛОВ
 *
 * Используется, когда оба брата не могут отдать ключи (у них < t ключей)
 * Два узла сливаются в один, ключ из родителя опускается вниз
 */
void
btree_merge_nodes(BTreeNode* parent, int child_idx)
{
    BTreeNode* left_child = parent->children[child_idx];
    BTreeNode* right_child = parent->children[child_idx + 1];
    int degree = left_child->degree;

    /* 1. Переносим ключ из родителя в левый узел (в конец) */
    left_child->keys[left_child->key_count] = parent->keys[child_idx];
    left_child->values[left_child->key_count] = parent->values[child_idx];
    left_child->key_count++;

    /* 2. Переносим все ключи из правого узла в левый */
    for (int i = 0; i < right_child->key_count; i++) {
        left_child->keys[left_child->key_count] = right_child->keys[i];
        left_child->values[left_child->key_count] = right_child->values[i];
        left_child->key_count++;
    }

    /* 3. Переносим всех потомков из правого узла в левый */
    if (!left_child->is_leaf) {
        for (int i = 0; i <= right_child->key_count; i++) {
            left_child->children[left_child->key_count] = right_child->children[i];
        }
    }

    /* 4. Удаляем ключ из родителя (сдвигаем влево) */
    for (int i = child_idx; i < parent->key_count - 1; i++) {
        parent->keys[i] = parent->keys[i + 1];
        parent->values[i] = parent->values[i + 1];
    }

    /* 5. Удаляем правого потомка из родителя (сдвигаем влево) */
    for (int i = child_idx + 1; i < parent->key_count; i++) {
        parent->children[i] = parent->children[i + 1];
    }

    parent->key_count--;  /* У родителя стало на 1 меньше ключей */

    /* 6. Освобождаем правый узел */
    free(right_child->keys);
    free(right_child->values);
    free(right_child->children);
    free(right_child);
}

/*
 * УДАЛЕНИЕ КЛЮЧА ИЗ ПОДДЕРЕВА (основная логика)
 *
 * Это самая сложная операция B-дерева.
 *
 * 3 основных случая:
 *
 * СЛУЧАЙ 1: Ключ в текущем узле
 *   1А: Узел — лист → просто удаляем ключ
 *   1Б: Узел — внутренний → заменяем предшественником или преемником
 *
 * СЛУЧАЙ 2: Ключ не в текущем узле
 *   Идём в потомка, при необходимости перебалансируем (заимствование или слияние)
 */
bool
btree_delete_from_subtree(BTreeNode* node, int key, int degree, int* child_height)
{
    int i = btree_node_find_insert_pos(node, key);  /* Позиция для вставки/поиска */

    /* ======================================================
     * СЛУЧАЙ 1: КЛЮЧ В ТЕКУЩЕМ УЗЛЕ
     * ====================================================== */
    if (i < node->key_count && node->keys[i] == key) {

        /* СЛУЧАЙ 1А: Ключ в листе — просто удаляем */
        if (node->is_leaf) {
            /* Сдвигаем все ключи и значения влево, перезаписывая удаляемый */
            for (int j = i; j < node->key_count - 1; j++) {
                node->keys[j] = node->keys[j + 1];
                node->values[j] = node->values[j + 1];
            }
            node->key_count--;  /* Уменьшаем количество ключей */
            return true;
        }
        /* СЛУЧАЙ 1Б: Ключ во внутреннем узле */
        else {
            BTreeNode* left_child = node->children[i];
            BTreeNode* right_child = node->children[i + 1];

            /* Если левый потомок имеет достаточно ключей (>= t) */
            if (left_child->key_count >= degree) {
                /* Находим ПРЕДШЕСТВЕННИКА (максимальный ключ в левом поддереве) */
                BTreeNode* pred_node = btree_find_maximum(left_child);
                int pred_key = pred_node->keys[pred_node->key_count - 1];
                int pred_value = pred_node->values[pred_node->key_count - 1];

                /* Заменяем удаляемый ключ на ключ-предшественник */
                node->keys[i] = pred_key;
                node->values[i] = pred_value;

                /* Рекурсивно удаляем предшественника из левого поддерева */
                return btree_delete_from_subtree(left_child, pred_key, degree, child_height);
            }
            /* Если правый потомок имеет достаточно ключей (>= t) */
            else if (right_child->key_count >= degree) {
                /* Находим ПРЕЕМНИКА (минимальный ключ в правом поддереве) */
                BTreeNode* succ_node = btree_find_minimum(right_child);
                int succ_key = succ_node->keys[0];
                int succ_value = succ_node->values[0];

                /* Заменяем удаляемый ключ на ключ-преемник */
                node->keys[i] = succ_key;
                node->values[i] = succ_value;

                /* Рекурсивно удаляем преемника из правого поддерева */
                return btree_delete_from_subtree(right_child, succ_key, degree, child_height);
            }
            /* Оба ребёнка имеют недостаточно ключей — сливаем их */
            else {
                btree_merge_nodes(node, i);
                /* Рекурсивно удаляем из левого ребёнка (после слияния) */
                return btree_delete_from_subtree(left_child, key, degree, child_height);
            }
        }
    }

    /* ======================================================
     * СЛУЧАЙ 2: КЛЮЧ НЕ В ТЕКУЩЕМ УЗЛЕ — идём в потомка
     * ====================================================== */
    else {
        /* Если узел лист — ключ не найден */
        if (node->is_leaf) {
            return false;
        }

        /*
         * Если потомок имеет недостаточно ключей (< t),
         * нужно перебалансировать поддерево
         */
        if (node->children[i]->key_count < degree) {
            /* Пытаемся заимствовать у левого брата */
            if (i > 0 && node->children[i - 1]->key_count >= degree) {
                btree_borrow_from_left(node, i);
            }
            /* Пытаемся заимствовать у правого брата */
            else if (i < node->key_count && node->children[i + 1]->key_count >= degree) {
                btree_borrow_from_right(node, i);
            }
            /* Если не у кого заимствовать — сливаем с братом */
            else {
                if (i < node->key_count) {
                    btree_merge_nodes(node, i);
                }
                else {
                    btree_merge_nodes(node, i - 1);
                    i--;  /* После слияния индексы сдвигаются */
                }
            }
        }

        /* Рекурсивно удаляем из потомка */
        bool result = btree_delete_from_subtree(node->children[i], key, degree, child_height);

        /* Если потомок стал пустым (key_count == 0) — удаляем его */
        if (node->children[i]->key_count == 0 && node != NULL) {
            /* Освобождаем память потомка */
            free(node->children[i]->keys);
            free(node->children[i]->values);
            free(node->children[i]->children);
            free(node->children[i]);
            node->children[i] = NULL;

            /* Сдвигаем оставшихся потомков влево */
            for (int j = i; j < node->key_count; j++) {
                node->children[j] = node->children[j + 1];
            }
        }

        return result;
    }
}