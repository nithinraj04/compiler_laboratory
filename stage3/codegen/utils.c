#include "utils.h"
#include "../tree/tree.h"
#include <stdio.h>
#include <stdlib.h>

extern int codeGen(node* root, FILE* targetFile);

int getReg() {
    if(regCount < 20) {
        return regCount++;
    }
    fprintf(stderr, "Error: Out of registers\n");
    exit(1);
}

void freeReg() {
    if(regCount > 0) {
        regCount--;
    }
}

int getLabel() {
    return label++;
}

int binaryOpHandler(node* root, FILE* targetFile) {
    int leftReg = codeGen(root->left, targetFile);
    int rightReg = codeGen(root->right, targetFile);
    switch(root->nodetype) {
        case NODE_PLUS:
            fprintf(targetFile, "ADD R%d, R%d\n", leftReg, rightReg);
            break;
        case NODE_MINUS:
            fprintf(targetFile, "SUB R%d, R%d\n", leftReg, rightReg);
            break;
        case NODE_MUL:
            fprintf(targetFile, "MUL R%d, R%d\n", leftReg, rightReg);
            break;
        case NODE_DIV:
            fprintf(targetFile, "DIV R%d, R%d\n", leftReg, rightReg);
            break;
        case NODE_GT:
            fprintf(targetFile, "GT R%d, R%d\n", leftReg, rightReg);
            break;
        case NODE_GTE:
            fprintf(targetFile, "GE R%d, R%d\n", leftReg, rightReg);
            break;
        case NODE_LT:
            fprintf(targetFile, "LT R%d, R%d\n", leftReg, rightReg);
            break;
        case NODE_LTE:
            fprintf(targetFile, "LE R%d, R%d\n", leftReg, rightReg);
            break;
        case NODE_EQ:
            fprintf(targetFile, "EQ R%d, R%d\n", leftReg, rightReg);
            break;
        case NODE_NEQ:
            fprintf(targetFile, "NE R%d, R%d\n", leftReg, rightReg);
            break;
        default:
            break;
    }
    freeReg();
    return leftReg;
}

labelStack* createLabelStackNode(int cond, int end) {
    labelStack* temp = (labelStack*) malloc(sizeof(labelStack));
    temp->cond = cond;
    temp->end = end;
    temp->next = NULL;
    temp->prev = NULL;
    return temp;
}

labelStack* pushLabelStack(labelStack* top, int cond, int end) {
    labelStack* newNode = createLabelStackNode(cond, end);
    if(top != NULL) {
        newNode->next = top;
        top->prev = newNode;
    }
    return newNode;
}

labelStack* popLabelStack(labelStack* top) {
    if(top == NULL) {
        return NULL;
    }
    labelStack* temp = top;
    top = top->next;
    if(top != NULL) {
        top->prev = NULL;
    }
    free(temp);
    return top;
}

