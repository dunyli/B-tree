/*
 * btree.c (основная реализация B-дерева)
 *
 * Содержит основную структуру B-дерева и функции.
 */

#define _CRT_SECURE_NO_WARNINGS

#include "btree.h"           
#include "btree_node.h"      /* Подключаем для работы с узлами */
#include "btree_utils.h"     /* Подключаем вспомогательные функции */
#include "btree_delete.h"    /* Подключаем функции удаления */
#include <stdlib.h>          /* malloc, free */
#include <stdio.h>           

 /*
  * Полная структура B-дерева (скрытая от пользователя)
  */
struct BTree {
    BTreeNode* root;     /* Корень дерева */
    int degree;          /* Степень дерева (t) — минимальное количество ключей в узле */
    int size;            /* Общее количество элементов (ключей) в дереве */
    int height;          /* Высота дерева (количество уровней) */
    int node_count;      /* Количество узлов в дереве */
};

/*
 * СОЗДАНИЕ B-ДЕРЕВА
 */
BTree* btree_create(int degree)
{
    if (degree < 2) degree = 2;  /* Минимальная степень — 2 (иначе дерево вырождается) */

    BTree* tree = (BTree*)malloc(sizeof(BTree));  /* Выделяем память под структуру */
    if (!tree) return NULL;                       /* Проверка */

    tree->degree = degree;       /* Сохраняем степень дерева */
    tree->size = 0;              /* Начальный размер — 0 элементов */
    tree->height = 0;            /* Высота пока 0 */
    tree->node_count = 0;        /* Узлов пока нет */

    /* Создаём корневой узел (всегда лист) */
    tree->root = btree_node_create(degree, true);
    if (!tree->root) {           /* Проверка создания */
        free(tree);
        return NULL;
    }

    tree->node_count = 1;        /* Корень считается узлом */
    return tree;
}

/*
 * ОСВОБОЖДЕНИЕ ПАМЯТИ
 */
void btree_destroy(BTree* tree)
{
    if (!tree) return;           /* Если NULL — ничего не делаем */
    btree_node_destroy(tree->root);  /* Рекурсивное удаление всех узлов */
    free(tree);                  /* Освобождаем структуру дерева */
}

/*
 * ВСТАВКА КЛЮЧА И ЗНАЧЕНИЯ
 */
bool btree_insert(BTree* tree, int key, int value)
{
    if (!tree) return false;     /* Проверка существования дерева */

    /* Если корень полный (2t-1 ключей) — разделяем его */
    if (tree->root->key_count == 2 * tree->degree - 1) {
        BTreeNode* old_root = tree->root;          /* Сохраняем старый корень */
        BTreeNode* new_root = btree_node_create(tree->degree, false);  /* Новый корень */
        if (!new_root) return false;

        new_root->children[0] = old_root;          /* Старый корень — потомок нового */
        tree->root = new_root;                     /* Обновляем корень */
        tree->node_count++;                        /* Увеличиваем счётчик узлов */

        btree_split_child(new_root, 0, old_root);  /* Разделяем старый корень */
        btree_insert_nonfull(new_root, key, value); /* Вставляем в новый корень */
    }
    else {
        btree_insert_nonfull(tree->root, key, value);  /* Вставляем в корень */
    }

    /* Обновляем статистику дерева */
    tree->size++;
    tree->height = btree_calculate_height(tree->root);
    tree->node_count = btree_count_nodes(tree->root);

    return true;
}

/*
 * ПОИСК ЗНАЧЕНИЯ ПО КЛЮЧУ
 */
bool btree_search(BTree* tree, int key, int* out_value)
{
    if (!tree || !tree->root) return false;  /* Проверка */

    BTreeNode* node = tree->root;  /* Начинаем с корня */

    /* Цикл спуска по дереву */
    while (node) {
        int idx = btree_node_find_key(node, key);  /* Ищем ключ в текущем узле */
        if (idx != -1) {                           /* Если нашли */
            if (out_value) *out_value = node->values[idx];  /* Сохраняем значение */
            return true;
        }

        if (node->is_leaf) return false;  /* Если лист — ключа нет */

        int i = btree_node_find_insert_pos(node, key);  /* Находим потомка */
        node = node->children[i];  /* Спускаемся на уровень ниже */
    }

    return false;  /* Не нашли */
}

/*
 * УДАЛЕНИЕ КЛЮЧА
 */
bool btree_delete(BTree* tree, int key)
{
    if (!tree || !tree->root) return false;  /* Проверка */
    if (!btree_contains(tree, key)) return false;  /* Проверяем, есть ли ключ */

    int child_height = 0;
    bool result = btree_delete_from_subtree(tree->root, key, tree->degree, &child_height);

    if (result) {
        /* Обновляем статистику */
        tree->size--;
        tree->height = btree_calculate_height(tree->root);
        tree->node_count = btree_count_nodes(tree->root);

        /* Если корень стал пустым и у него есть потомок — корень становится потомком */
        if (tree->root->key_count == 0 && !tree->root->is_leaf) {
            BTreeNode* old_root = tree->root;
            tree->root = tree->root->children[0];
            tree->node_count--;
            free(old_root->keys);
            free(old_root->values);
            free(old_root->children);
            free(old_root);
        }
    }

    return result;
}

/*
 * ПРОВЕРКА НАЛИЧИЯ КЛЮЧА (обёртка над btree_search)
 */
bool btree_contains(BTree* tree, int key)
{
    return btree_search(tree, key, NULL);  /* Вызываем поиск без сохранения значения */
}

/*
 * ПОЛУЧЕНИЕ КОЛИЧЕСТВА ЭЛЕМЕНТОВ
 */
int btree_size(BTree* tree)
{
    return tree ? tree->size : 0;  /* Если дерево NULL — возвращаем 0 */
}

/*
 * ПОЛУЧЕНИЕ ВЫСОТЫ ДЕРЕВА
 */
int btree_height(BTree* tree)
{
    return tree ? tree->height : 0;
}

/*
 * ПОЛУЧЕНИЕ СТЕПЕНИ ДЕРЕВА
 */
int btree_degree(BTree* tree)
{
    return tree ? tree->degree : 0;
}

/*
 * ПОЛУЧЕНИЕ КОЛИЧЕСТВА УЗЛОВ
 */
int btree_nodes(BTree* tree)
{
    return tree ? tree->node_count : 0;
}

/*
 * ПЕЧАТЬ СТАТИСТИКИ ДЕРЕВА
 */
void btree_print_stats(BTree* tree)
{
    if (!tree) {
        printf("Дерево не инициализировано\n");
        return;
    }

    printf(" СТАТИСТИКА B-ДЕРЕВА n");
    printf("  Степень: %d\n", tree->degree);
    printf("  Количество элементов: %d\n", tree->size);
    printf("  Высота: %d\n", tree->height);
    printf("  Количество узлов: %d\n", tree->node_count);
    printf("  Корень: %s\n", tree->root ? "существует" : "NULL");
}

/*
 * ПЕЧАТЬ ДЕРЕВА (для отладки)
 */
void btree_print(BTree* tree)
{
    if (!tree || !tree->root) {
        printf("Дерево пустое\n");
        return;
    }

    printf("\n B-ДЕРЕВО \n");
    printf("Степень: %d, элементов: %d, высота: %d\n",
        tree->degree, tree->size, tree->height);
    btree_print_node(tree->root, 0, "R-");  /* Рекурсивная печать с корня */
}