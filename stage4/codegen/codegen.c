#include "../tree/tree.h"
#include "../gst/gst.h"
#include "utils.h"
#include "libfuncs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int regCount = 0;
int label = 0;

labelStack* labelStackTop = NULL;
extern gst* gstRoot;

extern int binaryOpHandler(node* root, FILE* targetFile);

int getAddr(node* varNode);
int codeGen(node* root, FILE* targetFile);

int getAddr(node* varNode) {
    return varNode->gstEntry->binding;
}
int getArrayAddr(FILE* targetFile, node* arrayNode) {
    int reg = getReg();
    int baseAddr = getAddr(arrayNode);
    fprintf(targetFile, "MOV R%d, %d\n", reg, baseAddr);
    int indexReg = codeGen(arrayNode->right, targetFile);
    fprintf(targetFile, "ADD R%d, R%d\n", reg, indexReg);
    freeReg(); // Free indexReg
    return reg;
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
        case NODE_STR: {
            int reg = getReg();
            fprintf(targetFile, "MOV R%d, %s\n", reg, root->strval);
            return reg;
        }
        case NODE_ID: {
            int reg = getReg();
            fprintf(targetFile, "MOV R%d, [%d]\n", reg, getAddr(root));
            return reg;
        }
        case NODE_ARRAY: {
            int reg = getArrayAddr(targetFile, root);
            fprintf(targetFile, "MOV R%d, [R%d]\n", reg, reg);
            return reg;
        }
        case NODE_PLUS:
        case NODE_MINUS:
        case NODE_MUL:
        case NODE_DIV: 
        case NODE_MOD:
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
            if(root->left->nodetype == NODE_ARRAY) {
                int arrayReg = getArrayAddr(targetFile, root->left);
                fprintf(targetFile, "MOV [R%d], R%d\n", arrayReg, rightReg);
                freeReg(); // Free arrayReg
            }
            else if(root->left->nodetype == NODE_PTR) {
                int reg = codeGen(root->left->right, targetFile);
                fprintf(targetFile, "MOV [R%d], R%d\n", reg, rightReg);
                freeReg();
            }
            else {
                fprintf(targetFile, "MOV [%d], R%d\n", leftAddr, rightReg);
            }
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
            if(root->left->nodetype == NODE_ARRAY) {
                int arrayReg = getArrayAddr(targetFile, root->left);
                read(arrayReg, targetFile);
                freeReg(); // Free arrayReg
            }
            else if(root->left->nodetype == NODE_PTR) {
                int reg = codeGen(root->left->right, targetFile);
                read(reg, targetFile);
                freeReg();
            }
            else {
                int leftAddr = getAddr(root->left);
                int reg = getReg();
                fprintf(targetFile, "MOV R%d, %d\n", reg, leftAddr);
                read(reg, targetFile);
                freeReg();
            }
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
        case NODE_DECL: {
            return -1;
        }
        case NODE_TYPE: {
            return -1;
        }
        case NODE_PTR: {
            int reg = codeGen(root->right, targetFile);
            fprintf(targetFile, "MOV R%d, [R%d]\n", reg, reg);
            return reg;
        }
        case NODE_ADDR_OF: {
            int reg = getReg();
            fprintf(targetFile, "MOV R%d, %d\n", reg, root->val);
            return reg;
        }
        default:
            fprintf(stderr, "Error: Unknown node type %d\n", root->nodetype);
            exit(1);
    }
}

void printAST(node* root, const char* prefix, int isLast) {
    if (!root) return;
    
    printf("%s", prefix);
    printf("%s", isLast ? "└── " : "├── ");
    
    switch(root->nodetype) {
        case NODE_NUM:
            printf("NUM(%d)\n", root->val);
            break;
        case NODE_ID:
            printf("ID(%s)\n", root->varname);
            break;
        case NODE_PLUS:
            printf("PLUS\n");
            break;
        case NODE_MINUS:
            printf("MINUS\n");
            break;
        case NODE_MUL:
            printf("MUL\n");
            break;
        case NODE_DIV:
            printf("DIV\n");
            break;
        case NODE_GT:
            printf("GT\n");
            break;
        case NODE_GTE:
            printf("GTE\n");
            break;
        case NODE_LT:
            printf("LT\n");
            break;
        case NODE_LTE:
            printf("LTE\n");
            break;
        case NODE_EQ:
            printf("EQ\n");
            break;
        case NODE_NEQ:
            printf("NEQ\n");
            break;
        case NODE_ASSIGN:
            printf("ASSIGN\n");
            break;
        case NODE_CONNECTOR:
            printf("CONNECTOR\n");
            break;
        case NODE_IF:
            printf("IF\n");
            break;
        case NODE_IFELSE:
            printf("IFELSE\n");
            break;
        case NODE_WHILE:
            printf("WHILE\n");
            break;
        case NODE_DOWHILE:
            printf("DOWHILE\n");
            break;
        case NODE_BREAK:
            printf("BREAK\n");
            break;
        case NODE_CONTINUE:
            printf("CONTINUE\n");
            break;
        case NODE_READ:
            printf("READ\n");
            break;
        case NODE_WRITE:
            printf("WRITE\n");
            break;
        case NODE_BRKP:
            printf("BRKP\n");
            break;
        case NODE_DECL:
            printf("DECL\n");
            break;
        case NODE_TYPE:
            printf("TYPE(%d)\n", root->type);
            break;
        case NODE_PTR:
            printf("PTR(%s)\n", root->varname ? root->varname : "unknown");
            break;
        case NODE_ADDR_OF:
            printf("ADDR_OF(%s)\n", root->varname ? root->varname : "unknown");
            break;
        default:
            printf("UNKNOWN(%d)\n", root->nodetype);
    }
    
    char newPrefix[256];
    snprintf(newPrefix, sizeof(newPrefix), "%s%s", prefix, isLast ? "    " : "│   ");
    
    if (root->left) printAST(root->left, newPrefix, !root->right);
    if (root->right) printAST(root->right, newPrefix, 1);
}