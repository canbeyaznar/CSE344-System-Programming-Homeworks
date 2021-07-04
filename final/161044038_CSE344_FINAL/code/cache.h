#ifndef _CACHE_H_
#define _CACHE_H_

#include <stdio.h>
#include <stdlib.h>

struct Cache_tree{
    int* path_arr;
    int size;
    int isHead;
    struct Cache_tree* left;
    struct Cache_tree* right;
};

struct Cache_tree* new_cachenode(int* path_arr, int size );
struct Cache_tree* insert_path(struct Cache_tree* root, int* path_arr, int size);
struct Cache_tree* find_path(struct Cache_tree* root,int src, int dest);
void free_cache_tree(struct Cache_tree* root);


#endif