/*
 * btree.c - основная реализация B-дерева
 *
 * Содержит:
 * - структуру B-дерева
 * - создание и удаление дерева
 * - вставку элементов
 * - поиск элементов
 * - удаление элементов (классическое)
 * - статистику дерева
 * - красивую печать дерева
 * - итератор для обхода
 */

#define _CRT_SECURE_NO_WARNINGS

#include "btree.h"          
#include "btree_node.h"     /* Работа с узлами */
#include "btree_utils.h"    /* Вспомогательные функции */
#include "btree_delete.h"   /* Функции удаления */
#include <stdlib.h>         /* malloc, free */
#include <stdio.h>          
#include <string.h>         /* snprintf */

 /*
  * ============================================================================
  * СТРУКТУРА B-ДЕРЕВА (скрытая реализация)
  * ============================================================================
  *
  * Поля:
  *   root       - корень дерева
  *   degree     - степень дерева (t)
  *   size       - количество элементов в дереве
  *   height     - высота дерева (количество уровней)
  *   node_count - количество узлов в дереве
  */
struct BTree {
    BTreeNode* root;
    int degree;
    int size;
    int height;
    int node_count;
};

/*
 * ============================================================================
 * СОЗДАНИЕ И УДАЛЕНИЕ
 * ============================================================================
 */

 /*
  * СОЗДАНИЕ B-ДЕРЕВА
  *
  * Параметры: degree - степень дерева (минимальное количество ключей в узле)
  * Возвращает: указатель на созданное дерево или NULL при ошибке
  */
BTree* btree_create(int degree)
{
    if (degree < 2) degree = 2;  /* Минимальная степень 2 */

    /* Выделяем память под структуру дерева */
    BTree* tree = (BTree*)malloc(sizeof(BTree));
    if (!tree) return NULL;

    /* Инициализируем поля */
    tree->degree = degree;
    tree->size = 0;
    tree->height = 0;
    tree->node_count = 0;

    /* Создаём корневой узел (всегда лист) */
    tree->root = btree_node_create(degree, true);
    if (!tree->root) {
        free(tree);
        return NULL;
    }

    tree->node_count = 1;
    return tree;
}

/*
 * ОСВОБОЖДЕНИЕ ПАМЯТИ ДЕРЕВА
 *
 * Рекурсивно удаляет все узлы, затем освобождает структуру дерева
 */
void btree_destroy(BTree* tree)
{
    if (!tree) return;  /* Если дерево NULL - ничего не делаем */

    /* Рекурсивно удаляем все узлы */
    if (tree->root) {
        btree_node_destroy(tree->root);
        tree->root = NULL;
    }

    /* Освобождаем структуру дерева */
    free(tree);
}

/*
 * ============================================================================
 * ВСТАВКА
 * ============================================================================
 */

 /*
  * ВСТАВКА КЛЮЧА И ЗНАЧЕНИЯ
  *
  * Алгоритм:
  * 1. Если корень полный (2t-1 ключей) - создаём новый корень
  * 2. Вставляем ключ в неполный корень (рекурсивно)
  * 3. Обновляем статистику
  *
  * Возвращает: true при успехе, false при ошибке
  */
bool btree_insert(BTree* tree, int key, int value)
{
    if (!tree) return false;  /* Проверка существования дерева */

    /* Если корень полный - разделяем его */
    if (tree->root->key_count == 2 * tree->degree - 1) {
        /* Сохраняем старый корень */
        BTreeNode* old_root = tree->root;

        /* Создаём новый корень (не лист) */
        BTreeNode* new_root = btree_node_create(tree->degree, false);
        if (!new_root) return false;

        /* Старый корень становится первым потомком нового */
        new_root->children[0] = old_root;
        tree->root = new_root;
        tree->node_count++;

        /* Разделяем старый корень */
        btree_split_child(new_root, 0, old_root);

        /* Вставляем в новый корень */
        btree_insert_nonfull(new_root, key, value);
    }
    else {
        /* Корень не полный - вставляем напрямую */
        btree_insert_nonfull(tree->root, key, value);
    }

    /* Обновляем статистику */
    tree->size++;
    tree->height = btree_calculate_height(tree->root);
    tree->node_count = btree_count_nodes(tree->root);

    return true;
}

/*
 * ============================================================================
 * ПОИСК
 * ============================================================================
 */

 /*
  * ПОИСК ЗНАЧЕНИЯ ПО КЛЮЧУ
  *
  * Алгоритм:
  * 1. Начинаем с корня
  * 2. В каждом узле ищем ключ (бинарный поиск)
  * 3. Если нашли - возвращаем значение
  * 4. Если не нашли и узел лист - ключа нет
  * 5. Если не лист - идём в соответствующего потомка
  *
  * Возвращает: true если ключ найден, false если нет
  */
bool btree_search(BTree* tree, int key, int* out_value)
{
    if (!tree || !tree->root) return false;

    BTreeNode* node = tree->root;  /* Начинаем с корня */

    /* Спускаемся по дереву */
    while (node) {
        /* Ищем ключ в текущем узле */
        int idx = btree_node_find_key(node, key);
        if (idx != -1) {
            /* Нашли! Сохраняем значение и возвращаем true */
            if (out_value) *out_value = node->values[idx];
            return true;
        }

        /* Если лист - ключа нет в дереве */
        if (node->is_leaf) return false;

        /* Идём в потомка, который может содержать ключ */
        int i = btree_node_find_insert_pos(node, key);
        node = node->children[i];
    }

    return false;
}

/*
 * ПРОВЕРКА НАЛИЧИЯ КЛЮЧА
 * Просто обёртка над btree_search
 */
bool btree_contains(BTree* tree, int key)
{
    return btree_search(tree, key, NULL);
}

/*
 * ============================================================================
 * УДАЛЕНИЕ
 * ============================================================================
 */

 /*
  * УДАЛЕНИЕ КЛЮЧА
  *
  * Использует классическую реализацию удаления из B-дерева.
  *
  * Алгоритм:
  * 1. Проверяем, что ключ существует
  * 2. Рекурсивно удаляем из поддерева
  * 3. Обновляем статистику
  * 4. Если корень стал пустым - обновляем корень
  *
  * Возвращает: true если ключ удалён, false если не найден
  */
bool btree_delete(BTree* tree, int key)
{
    if (!tree || !tree->root) return false;  /* Проверка существования дерева */

    /* Если ключа нет - возвращаем false */
    if (!btree_contains(tree, key)) return false;

    int child_height = 0;
    bool result = btree_delete_from_subtree(tree->root, key, tree->degree, &child_height);

    if (result) {
        /* Обновляем статистику */
        tree->size--;
        tree->height = btree_calculate_height(tree->root);
        tree->node_count = btree_count_nodes(tree->root);

        /*
         * Если корень стал пустым и у него есть потомок,
         * то потомок становится новым корнем
         */
        if (tree->root->key_count == 0 && !tree->root->is_leaf) {
            BTreeNode* old_root = tree->root;
            tree->root = tree->root->children[0];
            tree->node_count--;

            /* Освобождаем старый корень */
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

/*
 * ============================================================================
 * СТАТИСТИКА
 * ============================================================================
 */

 /*
  * Получение количества элементов в дереве
  */
int btree_size(BTree* tree)
{
    return tree ? tree->size : 0;
}

/*
 * Получение высоты дерева
 */
int btree_height(BTree* tree)
{
    return tree ? tree->height : 0;
}

/*
 * Получение степени дерева
 */
int btree_degree(BTree* tree)
{
    return tree ? tree->degree : 0;
}

/*
 * Получение количества узлов
 */
int btree_nodes(BTree* tree)
{
    return tree ? tree->node_count : 0;
}

/*
 * ПЕЧАТЬ СТАТИСТИКИ ДЕРЕВА
 *
 * Выводит основную информацию о дереве в консоль
 */
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

/*
 * ============================================================================
 * КРАСИВАЯ ПЕЧАТЬ ДЕРЕВА
 * ============================================================================
 */

 /*
  * Рекурсивная печать узла и всех его потомков
  *
  * Использует ASCII-символы для визуализации структуры дерева:
  *   [1 2 3] - узел с ключами
  *   L - листовой узел
  *   |- - не последний потомок
  *   +- - последний потомок
  *   |  - вертикальная линия для отступов
  *
  * Параметры:
  *   node     - текущий узел для печати
  *   level    - уровень вложенности (для отступов)
  *   prefix   - префикс для отступов (пробелы и вертикальные линии)
  *   is_last  - флаг: является ли этот узел последним потомком у родителя
  */
static void btree_print_node(BTreeNode* node, int level, const char* prefix, int is_last)
{
    if (!node) return;  /* Если узел пустой - выходим */

    /* Выводим префикс (отступы) */
    printf("%s", prefix);

    /* Выводим ветку: для корня ничего, для остальных +- или |- */
    if (level > 0) {
        printf("%s ", is_last ? "+-" : "|-");
    }

    /* Выводим ключи в узле в квадратных скобках */
    printf("[");
    for (int i = 0; i < node->key_count; i++) {
        printf("%d", node->keys[i]);
        if (i < node->key_count - 1) printf(" ");
    }
    printf("]");

    /* Если узел лист - помечаем буквой L */
    if (node->is_leaf) {
        printf(" L");
    }
    printf("\n");  /* Переход на новую строку */

    /* Рекурсивно выводим потомков (если узел не лист) */
    if (!node->is_leaf) {
        char child_prefix[256];  /* Буфер для префикса потомков */

        /* Формируем префикс для потомков */
        if (level == 0) {
            /* Для корня префикс пустой */
            snprintf(child_prefix, sizeof(child_prefix), "");
        }
        else {
            /* Добавляем вертикальную линию или пробелы */
            snprintf(child_prefix, sizeof(child_prefix), "%s%s ", prefix, is_last ? "   " : "|  ");
        }

        /* Выводим всех потомков (их количество = key_count + 1) */
        for (int i = 0; i <= node->key_count; i++) {
            int child_is_last = (i == node->key_count);
            btree_print_node(node->children[i], level + 1, child_prefix, child_is_last);
        }
    }
}

/*
 * ПЕЧАТЬ ДЕРЕВА
 *
 * Выводит рамку с информацией о дереве и само дерево.
 * Использует простые символы для визуального выделения элементов.
 *
 * Параметры:
 *   tree - указатель на дерево для печати
 */
void btree_print(BTree* tree)
{
    /* Проверяем, существует ли дерево и есть ли в нём элементы */
    if (!tree || !tree->root) {
        printf("Дерево пустое\n");
        return;
    }

    printf("\n");
    printf("===============================================================\n");
    printf("                        B-ДЕРЕВО\n");
    printf("===============================================================\n");
    printf("  Степень: %d, элементов: %d, высота: %d\n",
        tree->degree, tree->size, tree->height);
    printf("===============================================================\n\n");

    /* Выводим само дерево, начиная с корня (level=0, is_last=1) */
    btree_print_node(tree->root, 0, "", 1);
    printf("\n");
}

/*
 * ============================================================================
 * ИТЕРАТОР
 * ============================================================================
 *
 * Итератор позволяет обойти все элементы дерева в порядке возрастания.
 * Для простоты и надёжности он собирает все ключи в массив при создании.
 *
 * Сложность обхода: O(n)
 * Память: O(n)
 */

 /*
 * Рекурсивный сбор всех ключей из дерева
 *
 * Правильный in-order обход:
 * 1. Левое поддерево
 * 2. Текущий узел
 * 3. Правое поддерево
 */
static void btree_collect_keys(BTreeNode* node, int** keys, int** values, int* size, int* capacity)
{
    if (!node) return;

    /* 1. Сначала обходим левое поддерево (все потомки до последнего) */
    for (int i = 0; i < node->key_count; i++) {
        if (node->children[i]) {
            btree_collect_keys(node->children[i], keys, values, size, capacity);
        }

        /* 2. Добавляем текущий ключ (после левого поддерева) */
        if (*size >= *capacity) {
            *capacity *= 2;
            *keys = (int*)realloc(*keys, *capacity * sizeof(int));
            *values = (int*)realloc(*values, *capacity * sizeof(int));
        }
        (*keys)[*size] = node->keys[i];
        (*values)[*size] = node->values[i];
        (*size)++;
    }

    /* 3. Обходим последнее правое поддерево */
    if (node->children[node->key_count]) {
        btree_collect_keys(node->children[node->key_count], keys, values, size, capacity);
    }
}

/*
 * СОЗДАНИЕ ИТЕРАТОРА
 *
 * Собирает все ключи в массив при создании.
 * Возвращает: итератор для обхода дерева
 */
BTreeIterator btree_iterator_create(BTree* tree)
{
    BTreeIterator iter;

    /* Инициализируем поля итератора */
    iter.is_valid = false;   /* Пока невалиден */
    iter.size = 0;           /* Размер 0 */
    iter.current = 0;        /* Текущая позиция 0 */
    iter.keys = NULL;        /* Массив ключей NULL */
    iter.values = NULL;      /* Массив значений NULL */

    /* Проверяем, что дерево существует и не пустое */
    if (!tree || !tree->root) {
        return iter;
    }

    /* Начальная ёмкость массива */
    int capacity = 64;

    /* Выделяем память под массивы */
    iter.keys = (int*)malloc(capacity * sizeof(int));
    iter.values = (int*)malloc(capacity * sizeof(int));

    if (!iter.keys || !iter.values) {
        /* Если память не выделилась - освобождаем всё */
        if (iter.keys) free(iter.keys);
        if (iter.values) free(iter.values);
        return iter;
    }

    /* Собираем все ключи из дерева */
    btree_collect_keys(tree->root, &iter.keys, &iter.values, &iter.size, &capacity);

    /* Итератор валиден, если есть хотя бы один элемент */
    iter.is_valid = (iter.size > 0);
    iter.current = 0;  /* Начинаем с первого элемента */

    return iter;
}

/*
 * УНИЧТОЖЕНИЕ ИТЕРАТОРА
 *
 * Освобождает выделенную память
 */
void btree_iterator_destroy(BTreeIterator* iter)
{
    if (!iter) return;  /* Если итератор NULL - ничего не делаем */

    /* Освобождаем массивы, если они были выделены */
    if (iter->keys) free(iter->keys);
    if (iter->values) free(iter->values);

    /* Обнуляем все поля */
    iter->keys = NULL;
    iter->values = NULL;
    iter->size = 0;
    iter->current = 0;
    iter->is_valid = false;
}

/*
 * ПЕРЕХОД К СЛЕДУЮЩЕМУ ЭЛЕМЕНТУ
 *
 * Просто увеличивает текущую позицию в массиве.
 * Возвращает: true если есть следующий элемент, false если конец
 */
bool btree_iterator_next(BTreeIterator* iter)
{
    if (!iter->is_valid) return false;  /* Если итератор невалиден - выходим */

    iter->current++;  /* Увеличиваем позицию */

    /* Если дошли до конца - помечаем как невалидный */
    if (iter->current >= iter->size) {
        iter->is_valid = false;
        return false;
    }

    return true;  /* Успешно перешли на следующий элемент */
}

/*
 * ПОЛУЧЕНИЕ ТЕКУЩЕГО КЛЮЧА
 *
 * Возвращает: текущий ключ или 0 если итератор невалиден
 */
int btree_iterator_key(BTreeIterator* iter)
{
    if (!iter->is_valid || iter->current >= iter->size) return 0;
    return iter->keys[iter->current];  /* Возвращаем текущий ключ из массива */
}

/*
 * ПОЛУЧЕНИЕ ТЕКУЩЕГО ЗНАЧЕНИЯ
 *
 * Возвращает: текущее значение или 0 если итератор невалиден
 */
int btree_iterator_value(BTreeIterator* iter)
{
    if (!iter->is_valid || iter->current >= iter->size) return 0;
    return iter->values[iter->current];  /* Возвращаем текущее значение из массива */
}

/*
 * ПРОВЕРКА ВАЛИДНОСТИ ИТЕРАТОРА
 *
 * Возвращает: true если итератор указывает на валидный элемент
 */
bool btree_iterator_valid(BTreeIterator* iter)
{
    return iter ? iter->is_valid : false;  /* Возвращаем флаг валидности */
}