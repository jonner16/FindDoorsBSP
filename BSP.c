#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "BSP.h"
#include "Wall.h"
#include "List.h"

BSPTree* BSP_Create(ListPtr data, void* (*GetSubtreeRoot)(ListPtr data), ListPtr (*GetSubtrees)(void* parent, ListPtr remaining), int (*DataCompare)(void* data1, void* data2), void (*DataPrinter)(void* data), void (*FreeData)(void *data)){
    BSPTree* B = (BSPTree*) malloc(sizeof (BSPTree));
    B->GetSubtreeRoot = GetSubtreeRoot;
    B->GetSubtrees = GetSubtrees;
    B->DataCompare = DataCompare;
    B->DataPrinter = DataPrinter;
    B->FreeData = FreeData;
    B->root = NULL;
    B->root = BuildSubtree(B,data,B->root);
    return B;
}

TreeNode* TreeNode_Create(void* data){
    TreeNode* T = (TreeNode*) malloc(sizeof(TreeNode));
    T->parent = NULL;
    T->right = NULL;
    T->left = NULL;
    T->data = data;
    return T;
}

TreeNode* BuildSubtree(BSPTree* tree, ListPtr remaining, TreeNode* parent){
    if(list_length(remaining) != 0) {
        void *data = tree->GetSubtreeRoot(remaining);
        if(data != NULL){
            TreeNode *node = TreeNode_Create(data);
            node->parent = parent;
            ListPtr left = tree->GetSubtrees(node->data, remaining);
            node->left = BuildSubtree(tree, left, node->left);
            node->right = BuildSubtree(tree, remaining, node->right);
            return node;
        }
    }
    return NULL;
}

void BSP_print(BSPTree* tree){
    ListPtr lPrint = BSP_GetTraversal(tree);
    list_print(lPrint);
}	

ListPtr BSP_GetTraversal(BSPTree* tree){
    ListPtr l = list_create(tree->DataCompare,tree->DataPrinter,tree->FreeData);
    PreOrderTraverse(l,tree->root);
    return l;
}

void PreOrderTraverse(ListPtr list, TreeNode* node){
    if(node != NULL){
        list_append(list,node->data);
        PreOrderTraverse(list, node->left);
        PreOrderTraverse(list, node->right);
    }
}
void PostOrderTraverseDel(BSPTree* tree, TreeNode* node){
    if(node != NULL){
        PostOrderTraverseDel(tree,node->left);
        PostOrderTraverseDel(tree,node->right);
        tree->FreeData(node);
    }
}
void DestroyTree(BSPTree* tree, bool freeData){
    if(tree != NULL && freeData){
        PostOrderTraverseDel(tree,tree->root);
    }
    free(tree);
}
