#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getReg();
void freeReg();
int getAddr(node* varNode);
int codeGen(node* root, FILE* targetFile);
void write(int reg, FILE* targetFile);
void read(int addr, FILE* targetFile);
int evaluator(node* root);

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

int getAddr(node* varNode) {
    return 4096 + (varNode->varname[0] - 'a');
}

int codeGen(node* root, FILE* targetFile) {
    if(!root) {
        return -1;
    }

    switch(root->nodetype) {
        case NODE_NUM: {
            int reg = getReg();
            fprintf(targetFile, "MOV R%d, %d\n", reg, root->val);
            return reg;
        }
        case NODE_ID: {
            int reg = getReg();
            fprintf(targetFile, "MOV R%d, [%d]\n", reg, getAddr(root));
            return reg;
        }
        case NODE_PLUS:
        case NODE_MINUS:
        case NODE_MUL:
        case NODE_DIV: {
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
                default:
                    break;
            }
            freeReg();
            return leftReg;
        }
        case NODE_ASSIGN: {
            int leftAddr = getAddr(root->left);
            int rightReg = codeGen(root->right, targetFile);
            fprintf(targetFile, "MOV [%d], R%d\n", leftAddr, rightReg);
            freeReg();
            return -1;
        }
        case NODE_WRITE: {
            int leftReg = codeGen(root->left, targetFile);
            write(leftReg, targetFile);
            freeReg();
            return -1;
        }
        case NODE_READ: {
            int leftAddr = getAddr(root->left);
            read(leftAddr, targetFile);
            return -1;
        }
        case NODE_CONNECTOR: {
            codeGen(root->left, targetFile);
            codeGen(root->right, targetFile);
            return -1;
        }
        default:
            fprintf(stderr, "Error: Unknown node type %d\n", root->nodetype);
            exit(1);
    }
}

void write(int reg, FILE* targetFile){
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

void read(int addr, FILE* targetFile){
    int tmp = getReg();
    fprintf(targetFile, "MOV R%d, %s\n", tmp, "\"Read\"");
    fprintf(targetFile, "PUSH R%d\n", tmp);
    fprintf(targetFile, "MOV R%d, %d\n", tmp, -1);
    fprintf(targetFile, "PUSH R%d\n", tmp);
    fprintf(targetFile, "MOV R%d, %d\n", tmp, addr);
    fprintf(targetFile, "PUSH R%d\n", tmp);
    fprintf(targetFile, "PUSH R%d\n", tmp); // 3rd arg
    fprintf(targetFile, "PUSH R%d\n", tmp); // ret addr
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

int evaluator(node* root) {
    static int reg[20] = {};
    static int mem[5120] = {};

    if(!root) {
        return -1;
    }

    switch (root->nodetype) {
        case NODE_NUM: {
            int regNum = getReg();
            reg[regNum] = root->val;
            return regNum;
        }
        case NODE_ID: {
            int regNum = getReg();
            int addr = getAddr(root);
            reg[regNum] = mem[addr];
            return regNum;
        }
        case NODE_PLUS: {
            int leftReg = evaluator(root->left);
            int rightReg = evaluator(root->right);
            reg[leftReg] = reg[leftReg] +  reg[rightReg];
            freeReg();
            return leftReg;
        }
        case NODE_MINUS: {
            int leftReg = evaluator(root->left);
            int rightReg = evaluator(root->right);
            reg[leftReg] = reg[leftReg] -  reg[rightReg];
            freeReg();
            return leftReg;
        }
        case NODE_MUL: {
            int leftReg = evaluator(root->left);
            int rightReg = evaluator(root->right);
            reg[leftReg] = reg[leftReg] *  reg[rightReg];
            freeReg();
            return leftReg;
        }
        case NODE_DIV: {
            int leftReg = evaluator(root->left);
            int rightReg = evaluator(root->right);
            reg[leftReg] = reg[leftReg] /  reg[rightReg];
            freeReg();
            return leftReg;
        }
        case NODE_ASSIGN: {
            int leftAddr = getAddr(root->left);
            int rightReg = evaluator(root->right);
            mem[leftAddr] = reg[rightReg];
            freeReg();
            return -1;
        }
        case NODE_WRITE: {
            int leftReg = evaluator(root->left);
            printf("%d\n", reg[leftReg]);
            freeReg();
            return -1;
        }
        case NODE_READ: {
            int leftAddr = getAddr(root->left);
            printf("Enter value for %s: ", root->left->varname);
            scanf("%d", &mem[leftAddr]);
            // mem[leftAddr] = 10;
            return -1;
        }
        case NODE_CONNECTOR: {
            evaluator(root->left);
            evaluator(root->right);
            return -1;
        }
        default:
            fprintf(stderr, "Error: Unknown node type %d\n", root->nodetype);
            exit(1);
    }
}