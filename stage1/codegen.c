#include "tree.h"
#include <stdio.h>
#include <stdlib.h>

int regCount = 0;

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

int codeGen(node* root, FILE* targetFile) {
    if(!root) {
        return -1;
    }

    if(!root->left && !root->right) {
        int reg = getReg();
        fprintf(targetFile, "MOV R%d, %d\n", reg, root->val);
        return reg;
    }

    int left = codeGen(root->left, targetFile);
    int right = codeGen(root->right, targetFile);

    if(root->op == '+') {
        fprintf(targetFile, "ADD R%d, R%d\n", left, right);
    }
    else if(root->op == '-') {
        fprintf(targetFile, "SUB R%d, R%d\n", left, right);
    }
    else if(root->op == '*') {
        fprintf(targetFile, "MUL R%d, R%d\n", left, right);
    }
    else if(root->op == '/') {
        fprintf(targetFile, "DIV R%d, R%d\n", left, right);
    }

    freeReg(); // Free the right register
    return left;
}

void print(int reg, FILE* targetFile){
    int tmp = getReg();
    fprintf(targetFile, "MOV R%d, %s\n", tmp, "\"Write\"");
    fprintf(targetFile, "PUSH R%d\n", tmp);
    fprintf(targetFile, "MOV R%d, %d\n", tmp, -2);
    fprintf(targetFile, "PUSH R%d\n", tmp);
    fprintf(targetFile, "PUSH R%d\n", reg);
    fprintf(targetFile, "PUSH R%d\n", tmp);
    fprintf(targetFile, "PUSH R%d\n", tmp);
    freeReg();
    fprintf(targetFile, "CALL 0\n");
    tmp = getReg();
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    fprintf(targetFile, "POP R%d\n", tmp);
    freeReg();
}