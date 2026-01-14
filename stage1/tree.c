#include <stdio.h>
#include <stdlib.h>
#include "tree.h"

node* makeLeafNode(int n) {
    node* temp = (node*)malloc(sizeof(node));
    temp->val = n;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

node* makeOpNode(char op, node* left, node* right) {
    node* temp = (node*) malloc(sizeof(node));
    temp->op = op;
    temp->left = left;
    temp->right = right;
    return temp;
}