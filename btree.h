// btree.h(çàãîëîâî÷íûé ôàéë)

#ifndef BTREE_H
#define BTREE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* ÏÓÁËÈ×ÍÛÅ ÑÒĞÓÊÒÓĞÛ */

typedef struct BTree BTree;
typedef struct BTreeNode BTreeNode;

/*
 * Èòåğàòîğ äëÿ îáõîäà B-äåğåâà
 */
typedef struct {
    BTree* tree;
    BTreeNode** stack;
    int* positions;
    int stack_size;
    int stack_capacity;
    bool is_valid;
} BTreeIterator;

/*
 *   ÔÓÍÊÖÈÈ ÓÏĞÀÂËÅÍÈß ÄÅĞÅÂÎÌ
 */

BTree* btree_create(int degree);
void btree_destroy(BTree* tree);
bool btree_save(BTree* tree, const char* filename);
BTree* btree_load(const char* filename);

/*
 *  ÎÏÅĞÀÖÈÈ Ñ ÄÀÍÍÛÌÈ
 */

bool btree_insert(BTree* tree, int key, int value);
bool btree_search(BTree* tree, int key, int* out_value);
bool btree_delete(BTree* tree, int key);
bool btree_contains(BTree* tree, int key);

/*
 *  ÑÒÀÒÈÑÒÈÊÀ È ÈÍÔÎĞÌÀÖÈß
 */

int btree_size(BTree* tree);
int btree_height(BTree* tree);
int btree_degree(BTree* tree);
int btree_nodes(BTree* tree);
void btree_print_stats(BTree* tree);
void btree_print(BTree* tree);

/*
 *  ÈÒÅĞÀÒÎĞ
 */

BTreeIterator btree_iterator_create(BTree* tree);
void btree_iterator_destroy(BTreeIterator* iter);
bool btree_iterator_next(BTreeIterator* iter);
int btree_iterator_key(BTreeIterator* iter);
int btree_iterator_value(BTreeIterator* iter);
bool btree_iterator_valid(BTreeIterator* iter);

#endif /* BTREE_H */