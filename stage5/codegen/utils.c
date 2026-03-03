#include "utils.h"
#include "../tree/tree.h"
#include "../symbol_table/gst.h"
#include <stdio.h>
#include <stdlib.h>

extern int codeGen(node* root, FILE* targetFile);
extern struct gst* gstRoot;
extern int fnlabel;

typedef struct regStack {
    int reg;
    struct regStack* next;
    struct regStack* prev;
} regStack;

regStack* regStackTop = NULL;

regStack* createRegStackNode(int reg) {
    regStack* temp = (regStack*) malloc(sizeof(regStack));
    temp->reg = reg;
    temp->next = NULL;
    temp->prev = NULL;
    return temp;
}

void pushRegStack() {
    regStack* newNode = createRegStackNode(regCount);
    if(regStackTop != NULL) {
        regStackTop->next = newNode;
        newNode->prev = regStackTop;
    }
    regStackTop = newNode;
    regCount = 0;
}

void popRegStack() {
    if(regStackTop == NULL) {
        fprintf(stderr, "Error: Trying to pop from empty register stack\n");
        exit(1);
    }
    regStack* temp = regStackTop;
    regStackTop = regStackTop->prev;
    if(regStackTop != NULL) {
        regStackTop->next = NULL;
    }
    regCount = temp->reg;
    free(temp);
}

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

int getRegCount() {
    return regCount;
}

int getLabel() {
    return label++;
}

int getFnLabel() {
    return fnlabel++;
}

static int getDeclaredPtrLevel(node* root) {
    int level = 0;
    node* current = root;
    while(current && current->nodetype == NODE_PTR) {
        level++;
        current = current->right;
    }
    return level;
}

void assignTypesLocal(node* root, varType type, gst** lst, struct paramStruct* paramList) {
    if(root == NULL) {
        return;
    }

    if(root->nodetype == NODE_ID) {
        char* varname = root->varname;
        int ptr_level = 0;
        *lst = lstInstall(*lst, varname, type, ptr_level);
        root->type = type;
        return;
    }

    if(root->nodetype == NODE_PTR) {
        char* varname = root->varname;
        int ptr_level = getDeclaredPtrLevel(root);
        *lst = lstInstall(*lst, varname, type, ptr_level);
        root->type = type;
        return;
    }

    assignTypesLocal(root->left, type, lst, paramList);
    assignTypesLocal(root->right, type, lst, paramList);
}

void buildLST(node* root, gst** lst, struct paramStruct* paramList) {
    if(!root) return;

    if(root->nodetype == NODE_LDECL) {
        varType type = root->left->type;
        assignTypesLocal(root->right, type, lst, paramList);
        return;
    }

    buildLST(root->left, lst, paramList);
    buildLST(root->right, lst, paramList);
}

int binaryOpHandler(node* root, FILE* targetFile) {
    int leftReg = codeGen(root->left, targetFile);
    int rightReg = codeGen(root->right, targetFile);

    if(root->nodetype == NODE_EQ || root->nodetype == NODE_NEQ) {
        if(root->left->type != root->right->type) {
            printf("Error: Type mismatch in equality operation\n");
            exit(1);
        }
        if(root->nodetype == NODE_EQ){
            fprintf(targetFile, "EQ R%d, R%d\n", leftReg, rightReg);
        }
        else if(root->nodetype == NODE_NEQ){
            fprintf(targetFile, "NE R%d, R%d\n", leftReg, rightReg);
        }
        root->type = root->left->type;
        freeReg();
        return leftReg;
    }

    if(root->left->type != TYPE_INT || root->right->type != TYPE_INT) {
        printf("Error: Non-integer operand in binary operation\n");
        exit(1);
    }

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
            case NODE_MOD:
            fprintf(targetFile, "MOD R%d, R%d\n", leftReg, rightReg);
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



