#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>

#include "../tree/tree.h"

typedef struct labelStack {
    int cond;
    int end;
    struct labelStack* next;
    struct labelStack* prev;
} labelStack;

labelStack* createLabelStackNode(int cond, int end);
labelStack* pushLabelStack(labelStack* top, int cond, int end);
labelStack* popLabelStack(labelStack* top);

int getReg();
void freeReg();
int getLabel();

extern int regCount;
extern int label;

#endif