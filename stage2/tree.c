#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

node* makeLeafNumNode(int n) {
    node* temp = (node*)malloc(sizeof(node));
    temp->val = n;
    temp->type = TYPE_INT;
    temp->nodetype = NODE_NUM;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

node* makeLeafIdNode(char* varname) {
    node* temp = (node*)malloc(sizeof(node));
    temp->varname = strdup(varname);
    temp->nodetype = NODE_ID;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

node* makeOpNode(char op, node* left, node* right) {
    node* temp = (node*) malloc(sizeof(node));
    switch(op) {
        case '+':
            temp->nodetype = NODE_PLUS;
            break;
        case '-':
            temp->nodetype = NODE_MINUS;
            break;
        case '*':
            temp->nodetype = NODE_MUL;
            break;
        case '/':
            temp->nodetype = NODE_DIV;
            break;
        case '=':
            temp->nodetype = NODE_ASSIGN;
            break;
        default:
            printf("Error: Unknown operator %c\n", op);
            free(temp);
            return NULL;
    }
    temp->left = left;
    temp->right = right;
    return temp;
}

node* makeWriteNode(node* left) {
    node* temp = (node*) malloc(sizeof(node));
    temp->nodetype = NODE_WRITE;
    temp->left = left;
    temp->right = NULL;
    return temp;
}

node* makeReadNode(node *left) {
    node* temp = (node*) malloc(sizeof(node));
    temp->nodetype = NODE_READ;
    temp->left = left;
    temp->right = NULL;
    return temp;
}

node* makeConnectorNode(node* left, node* right) {
    node* temp = (node*) malloc(sizeof(node));
    temp->nodetype = NODE_CONNECTOR;
    temp->left = left;
    temp->right = right;
    return temp;
}