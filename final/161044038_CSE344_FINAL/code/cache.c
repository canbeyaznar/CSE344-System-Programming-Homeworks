#include "cache.h"

struct Cache_tree* new_cachenode(int* path_arr, int size )
{
    int i;
    struct Cache_tree* temp_node = (struct Cache_tree*) malloc(sizeof(struct Cache_tree));
    
    if(size > 0)
    {
        temp_node->path_arr = (int*) malloc(sizeof(int)*size);
        for(i=0; i<size; i++)
            temp_node->path_arr[i] = path_arr[i];
        temp_node->size = size;
        temp_node->left = NULL;
        temp_node->right = NULL;
        temp_node->isHead = 0;
        return temp_node;
    }
    return NULL;
}
struct Cache_tree* insert_path(struct Cache_tree* root, int* path_arr, int size)
{
    if(root == NULL)
        return new_cachenode(path_arr,size);

    int src=path_arr[size-1], dest = path_arr[0];

    // the source of the rank is
    // checked as the 1st source and the 2nd is the destination
    // if source values ​​are equal, destination values ​​are compared
    if(src > root->path_arr[root->size-1]) // src_input > src_cache
        root->right = insert_path(root->right, path_arr, size);

    else if(src == root->path_arr[root->size-1]) // src_input == src_cache
    {
        if(dest > root->path_arr[0])
            root->right = insert_path(root->right, path_arr, size);
        
        else if(dest == root->path_arr[0]) 
        {
            //path is already in cache
            // there will be no insertion
            if(root->isHead == 1)
                root->isHead = 0;
        }
        else
            root->left = insert_path(root->left, path_arr, size);     
    }
    else // src_input < src_cache
        root->left = insert_path(root->left, path_arr, size);

    return root;
}
struct Cache_tree* find_path(struct Cache_tree* root,int src, int dest)
{
    if(root == NULL ||
    ((root->path_arr[root->size-1] == src) && (root->path_arr[0] == dest)))
    {   
        //if(root->isHead == 1)
          //  return NULL;
        return root;
    }
        

    else if(src > root->path_arr[root->size-1])
        return find_path(root->right, src,dest);

    else if(src == root->path_arr[root->size-1])
    {
        if(dest > root->path_arr[0])
            return find_path(root->right, src,dest);
        else 
            return find_path(root->left, src,dest);
    }
    else
        return find_path(root->left, src,dest);
}

void free_cache_tree(struct Cache_tree* root)
{
    if(root != NULL)
    {
        free(root->path_arr);
        free_cache_tree(root->right);
        free_cache_tree(root->left);
        free(root);
    }
}