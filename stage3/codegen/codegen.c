#include "../tree/tree.h"
#include "utils.h"
#include "libfuncs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int regCount = 0;
int label = 0;

labelStack* labelStackTop = NULL;

extern int binaryOpHandler(node* root, FILE* targetFile);

int getAddr(node* varNode);
int codeGen(node* root, FILE* targetFile);
int evaluator(node* root);

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
        case NODE_DIV: 
        case NODE_GT:
        case NODE_GTE:
        case NODE_LT:
        case NODE_LTE:
        case NODE_EQ:
        case NODE_NEQ:
        {
            return binaryOpHandler(root, targetFile);
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
        case NODE_IFELSE: {
            int falseLabel = getLabel();
            int endLabel = getLabel();
            int condReg = codeGen(root->left, targetFile);
            fprintf(targetFile, "JZ R%d, L%d\n", condReg, falseLabel);
            codeGen(root->right->left, targetFile);
            fprintf(targetFile, "JMP L%d\n", endLabel);
            fprintf(targetFile, "L%d:\n", falseLabel);
            codeGen(root->right->right, targetFile);
            fprintf(targetFile, "L%d:\n", endLabel);
            freeReg();
            return -1;
        }
        case NODE_IF: {
            int endLabel = getLabel();
            int condReg = codeGen(root->left, targetFile);
            fprintf(targetFile, "JZ R%d, L%d\n", condReg, endLabel);
            codeGen(root->right, targetFile);
            fprintf(targetFile, "L%d:\n", endLabel);
            freeReg();
            return -1;
        }
        case NODE_WHILE: {
            int condLabel = getLabel();
            int endLabel  = getLabel();
            labelStackTop = pushLabelStack(labelStackTop, condLabel, endLabel);

            fprintf(targetFile, "L%d:\n", condLabel);
            int condReg = codeGen(root->left, targetFile);
            fprintf(targetFile, "JZ R%d, L%d\n", condReg, endLabel);
            codeGen(root->right, targetFile);
            fprintf(targetFile, "JMP L%d\n", condLabel);
            fprintf(targetFile, "L%d:\n", endLabel);
            freeReg();

            labelStackTop = popLabelStack(labelStackTop);
            return -1;
        }
        case NODE_BREAK: {
            labelStack* top = labelStackTop;
            if(top == NULL) break;
            fprintf(targetFile, "JMP L%d\n", top->end);
            return -1;
        }
        case NODE_CONTINUE: {
            labelStack* top = labelStackTop;
            if(top == NULL) break;
            fprintf(targetFile, "JMP L%d\n", top->cond);
            return -1;
        }
        case NODE_BRKP: {
            fprintf(targetFile, "BRKP\n");
            return -1;
        }
        case NODE_DOWHILE: {
            int stmtLabel = getLabel();
            int condLabel = getLabel();
            int endLabel  = getLabel();
            labelStackTop = pushLabelStack(labelStackTop, condLabel, endLabel);

            fprintf(targetFile, "L%d:\n", stmtLabel);
            codeGen(root->right, targetFile);
            fprintf(targetFile, "L%d:\n", condLabel);
            int condReg = codeGen(root->left, targetFile);
            fprintf(targetFile, "JNZ R%d, L%d\n", condReg, stmtLabel);
            fprintf(targetFile, "L%d:\n", endLabel);

            labelStackTop = popLabelStack(labelStackTop);
            freeReg();
            return -1;
        }
        default:
            fprintf(stderr, "Error: Unknown node type %d\n", root->nodetype);
            exit(1);
    }
}


int evaluator(node* root) {
    static int reg[20] = {};
    static int mem[5120] = {};

    if(!root) {
        printf("Error: Null node encountered in evaluator\n");
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
        case NODE_GT: {
            int leftReg = evaluator(root->left);
            int rightReg = evaluator(root->right);
            reg[leftReg] = (reg[leftReg] > reg[rightReg]) ? 1 : 0;
            freeReg();
            return leftReg;
        }
        case NODE_GTE: {
            int leftReg = evaluator(root->left);
            int rightReg = evaluator(root->right);
            reg[leftReg] = (reg[leftReg] >= reg[rightReg]) ? 1 : 0;
            freeReg();
            return leftReg;
        }
        case NODE_LT: {
            int leftReg = evaluator(root->left);
            int rightReg = evaluator(root->right);
            reg[leftReg] = (reg[leftReg] < reg[rightReg]) ? 1 : 0;
            freeReg();
            return leftReg;
        }
        case NODE_LTE: {
            int leftReg = evaluator(root->left);
            int rightReg = evaluator(root->right);
            reg[leftReg] = (reg[leftReg] <= reg[rightReg]) ? 1 : 0;
            freeReg();
            return leftReg;
        }
        case NODE_EQ: {
            int leftReg = evaluator(root->left);
            int rightReg = evaluator(root->right);
            reg[leftReg] = (reg[leftReg] == reg[rightReg]) ? 1 : 0;
            freeReg();
            return leftReg;
        }
        case NODE_NEQ: {
            int leftReg = evaluator(root->left);
            int rightReg = evaluator(root->right);
            reg[leftReg] = (reg[leftReg] != reg[rightReg]) ? 1 : 0;
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
        case NODE_IFELSE: {
            int condReg = evaluator(root->left);
            if(reg[condReg]) {
                evaluator(root->right->left);
            } else {
                evaluator(root->right->right);
            }
            freeReg();
            return -1;
        }
        case NODE_IF: {
            int condReg = evaluator(root->left);
            if(reg[condReg]) {
                evaluator(root->right);
            }
            freeReg();
            return -1;
        }
        case NODE_WHILE: {
            int condReg = evaluator(root->left);
            while(reg[condReg]) {
                evaluator(root->right);
                condReg = evaluator(root->left);
            }
            freeReg();
            return -1;
        }
        default:
            fprintf(stderr, "Error: Unknown node type %d\n", root->nodetype);
            exit(1);
    }
}