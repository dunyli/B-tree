/*
 *  btree_delete.c (удаление)
 */

#define _CRT_SECURE_NO_WARNINGS

#include "btree_delete.h"
#include "btree_utils.h"
#include <stdlib.h>

 /*
  * ЗАИМСТВОВАНИЕ КЛЮЧА ОТ ЛЕВОГО БРАТА
  *
  * Используется, когда у узла мало ключей, а у левого брата есть лишние
  */
void
btree_borrow_from_left(BTreeNode* parent, int child_idx)
{
    BTreeNode* child = parent->children[child_idx];           /* Текущий узел */
    BTreeNode* left_sibling = parent->children[child_idx - 1]; /* Левый брат */

    /* Сдвигаем ключи child вправо, освобождая место в начале */
    for (int i = child->key_count - 1; i >= 0; i--) {
        child->keys[i + 1] = child->keys[i];
        child->values[i + 1] = child->values[i];
    }

    /* Сдвигаем потомков child вправо */
    if (!child->is_leaf) {
        for (int i = child->key_count; i >= 0; i--) {
            child->children[i + 1] = child->children[i];
        }
    }

    /* Переносим ключ из родителя в child */
    child->keys[0] = parent->keys[child_idx - 1];
    child->values[0] = parent->values[child_idx - 1];

    /* Переносим последний ключ из левого брата в родителя */
    parent->keys[child_idx - 1] = left_sibling->keys[left_sibling->key_count - 1];
    parent->values[child_idx - 1] = left_sibling->values[left_sibling->key_count - 1];

    /* Переносим последнего потомка из левого брата в child */
    if (!child->is_leaf) {
        child->children[0] = left_sibling->children[left_sibling->key_count];
    }

    child->key_count++;         /* У child стало на 1 больше */
    left_sibling->key_count--;  /* У левого брата на 1 меньше */
}

/*
 * ЗАИМСТВОВАНИЕ КЛЮЧА ОТ ПРАВОГО БРАТА (симметрично)
 */
void
btree_borrow_from_right(BTreeNode* parent, int child_idx)
{
    BTreeNode* child = parent->children[child_idx];            /* Текущий узел */
    BTreeNode* right_sibling = parent->children[child_idx + 1]; /* Правый брат */

    /* Переносим ключ из родителя в child (в конец) */
    child->keys[child->key_count] = parent->keys[child_idx];
    child->values[child->key_count] = parent->values[child_idx];

    /* Переносим первого потомка из правого брата в child */
    if (!child->is_leaf) {
        child->children[child->key_count + 1] = right_sibling->children[0];
    }

    /* Переносим первый ключ из правого брата в родителя */
    parent->keys[child_idx] = right_sibling->keys[0];
    parent->values[child_idx] = right_sibling->values[0];

    /* Сдвигаем ключи правого брата влево (удаляем первый) */
    for (int i = 0; i < right_sibling->key_count - 1; i++) {
        right_sibling->keys[i] = right_sibling->keys[i + 1];
        right_sibling->values[i] = right_sibling->values[i + 1];
    }

    /* Сдвигаем потомков правого брата влево */
    if (!right_sibling->is_leaf) {
        for (int i = 0; i < right_sibling->key_count; i++) {
            right_sibling->children[i] = right_sibling->children[i + 1];
        }
    }

    child->key_count++;          /* У child стало на 1 больше */
    right_sibling->key_count--;  /* У правого брата на 1 меньше */
}

/*
 * СЛИЯНИЕ ДВУХ СОСЕДНИХ УЗЛОВ
 *
 * Используется, когда оба брата не могут отдать ключи
 */
void
btree_merge_nodes(BTreeNode* parent, int child_idx)
{
    BTreeNode* left_child = parent->children[child_idx];      /* Левый потомок */
    BTreeNode* right_child = parent->children[child_idx + 1]; /* Правый потомок */
    int degree = left_child->degree;

    /* Переносим ключ из родителя в левый узел (в конец) */
    left_child->keys[left_child->key_count] = parent->keys[child_idx];
    left_child->values[left_child->key_count] = parent->values[child_idx];
    left_child->key_count++;

    /* Переносим все ключи из правого узла в левый */
    for (int i = 0; i < right_child->key_count; i++) {
        left_child->keys[left_child->key_count] = right_child->keys[i];
        left_child->values[left_child->key_count] = right_child->values[i];
        left_child->key_count++;
    }

    /* Переносим всех потомков из правого узла в левый */
    if (!left_child->is_leaf) {
        for (int i = 0; i <= right_child->key_count; i++) {
            left_child->children[left_child->key_count] = right_child->children[i];
        }
    }

    /* Удаляем ключ из родителя (сдвигаем влево) */
    for (int i = child_idx; i < parent->key_count - 1; i++) {
        parent->keys[i] = parent->keys[i + 1];
        parent->values[i] = parent->values[i + 1];
    }

    /* Удаляем правого потомка из родителя (сдвигаем влево) */
    for (int i = child_idx + 1; i < parent->key_count; i++) {
        parent->children[i] = parent->children[i + 1];
    }

    parent->key_count--;  /* У родителя стало на 1 меньше ключей */

    /* Освобождаем правый узел (он больше не нужен) */
    free(right_child->keys);
    free(right_child->values);
    free(right_child->children);
    free(right_child);
}

/*
 * УДАЛЕНИЕ КЛЮЧА ИЗ ПОДДЕРЕВА (основная логика)
 *
 * 3 случая удаления:
 * 1. Ключ в листе — просто удаляем
 * 2. Ключ во внутреннем узле — заменяем предшественником или преемником
 * 3. Ключ не в текущем узле — идём в потомка, при необходимости перебалансируем
 */
bool
btree_delete_from_subtree(BTreeNode* node, int key, int degree, int* child_height)
{
    int i = btree_node_find_insert_pos(node, key);  /* Находим позицию ключа */

    /* ============================================================
     * СЛУЧАЙ 1: КЛЮЧ В ТЕКУЩЕМ УЗЛЕ
     * ============================================================ */
    if (i < node->key_count && node->keys[i] == key) {

        /* 1А: Ключ в листе — просто удаляем (сдвигаем влево) */
        if (node->is_leaf) {
            for (int j = i; j < node->key_count - 1; j++) {
                node->keys[j] = node->keys[j + 1];
                node->values[j] = node->values[j + 1];
            }
            node->key_count--;  /* Уменьшаем количество ключей */
            return true;
        }
        /* 1Б: Ключ во внутреннем узле */
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
                btree_merge_nodes(node, i);  /* Сливаем левого и правого */
                /* Рекурсивно удаляем из левого ребёнка (после слияния) */
                return btree_delete_from_subtree(left_child, key, degree, child_height);
            }
        }
    }

    /* ============================================================
     * СЛУЧАЙ 2: КЛЮЧ НЕ В ТЕКУЩЕМ УЗЛЕ — идём в потомка
     * ============================================================ */
    else {
        if (node->is_leaf) return false;  /* Если лист — ключа нет */

        /* Если потомок имеет недостаточно ключей (< t) — перебалансируем */
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

        return result;  /* Возвращаем результат удаления */
    }
}