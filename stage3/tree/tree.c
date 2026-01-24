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
    temp->type = TYPE_INT; 
    temp->nodetype = NODE_ID;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

node* makeOpNode(char* op, node* left, node* right) {
    // We are still dealing with only integer operands
    if(!left || !right) {
        printf("Error: NULL operand in makeOpNode\n");
        return NULL;
    }

    if(left->type != TYPE_INT || right->type != TYPE_INT) {
        printf("Error: Non-integer operand in makeOpNode\n");
        return NULL;
    }

    node* temp = (node*) malloc(sizeof(node));
    
    if(strcmp(op, "+") == 0) {
        temp->nodetype = NODE_PLUS;
        temp->type = TYPE_INT;
    }
    else if(strcmp(op, "-") == 0) { 
        temp->nodetype = NODE_MINUS;
        temp->type = TYPE_INT;
    }
    else if(strcmp(op, "*") == 0) {
        temp->nodetype = NODE_MUL;
        temp->type = TYPE_INT;
    }
    else if(strcmp(op, "/") == 0) {
        temp->nodetype = NODE_DIV;
        temp->type = TYPE_INT;
    }
    else if(strcmp(op, "=") == 0) {
        temp->nodetype = NODE_ASSIGN;
        temp->type = TYPE_INT;
    }
    else if(strcmp(op, ">") == 0) {
        temp->nodetype = NODE_GT;
        temp->type = TYPE_BOOL;
    }
    else if(strcmp(op, ">=") == 0) {
        temp->nodetype = NODE_GTE;
        temp->type = TYPE_BOOL;
    }
    else if(strcmp(op, "<") == 0) {
        temp->nodetype = NODE_LT;
        temp->type = TYPE_BOOL;
    }
    else if(strcmp(op, "<=") == 0) {
        temp->nodetype = NODE_LTE;
        temp->type = TYPE_BOOL;
    }
    else if(strcmp(op, "==") == 0) {
        temp->nodetype = NODE_EQ;
        temp->type = TYPE_BOOL;
    }
    else if(strcmp(op, "!=") == 0) {
        temp->nodetype = NODE_NEQ;
        temp->type = TYPE_BOOL;
    }
    else {
        printf("Error: Unknown operator %s in makeOpNode\n", op);
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

node* makeIfElseNode(node* cond, node* ifStmt, node* elseStmt) {
    node* temp = (node*) malloc(sizeof(node));
    temp->nodetype = NODE_IFELSE;
    temp->left = cond;
    temp->right = makeConnectorNode(ifStmt, elseStmt);
    return temp;
}

node* makeIfNode(node* cond, node* ifStmt) {
    node* temp = (node*) malloc(sizeof(node));
    temp->nodetype = NODE_IF;
    temp->left = cond;
    temp->right = ifStmt;
    return temp;
}

node* makeWhileNode(node* cond, node* body) {
    node* temp = (node*) malloc(sizeof(node));
    temp->nodetype = NODE_WHILE;
    temp->left = cond;
    temp->right = body;
    return temp;
}