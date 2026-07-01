/* btree_node.h (узел B-дерева)  */

#ifndef BTREE_NODE_H         
#define BTREE_NODE_H          

#include <stdbool.h>            /* Подключаем для типа bool */

/*
 * Структура узла B-дерева
 */
struct BTreeNode {
    int* keys;                     /* Массив ключей (размер 2t-1) */
    int* values;                   /* Массив значений (размер 2t-1) */
    struct BTreeNode** children;   /* Массив потомков (размер 2t) */
    int key_count;                 /* Текущее количество ключей в узле */
    bool is_leaf;                  /* Флаг: является ли узел листом */
    int degree;                    /* Степень дерева (t) */
};

/*
 * Функции для работы с узлами
 */
BTreeNode* btree_node_create(int degree, bool is_leaf);     /* Создание узла */
void btree_node_destroy(BTreeNode* node);                   /* Удаление узла */
int btree_node_find_key(BTreeNode* node, int key);          /* Поиск ключа в узле */
int btree_node_find_insert_pos(BTreeNode* node, int key);   /* Поиск позиции для вставки */

#endif