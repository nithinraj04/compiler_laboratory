#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../tree/tree.h"
#include "lst.h"

int addr_counter = 4096;
lst* lstRoot = NULL;

lst* createGstNode(char* name, varType type, int size) {
    lst* newNode = (lst*) malloc(sizeof(lst));
    newNode->name = strdup(name);
    newNode->type = type;
    newNode->size = size;
    newNode->binding = -1;
    newNode->next = NULL;
    return newNode;
}

lst* gstInstall(lst* head, char* name, varType type, int size, int ptr_level) {
    lst* newNode = createGstNode(name, type, size);
    newNode->ptr_level = ptr_level;
    if (head == NULL) {
        newNode->binding = reserveSpace(size);
        return newNode;
    }
    lst* temp = head;
    lst* prev = NULL;
    while (temp != NULL) {
        prev = temp;
        if(strcmp(temp->name, name) == 0) {
            // Variable already exists
            free(newNode->name);
            free(newNode);
            printf("Error: Redeclaration of variable '%s'.\n", name);
            exit(1);
        }
        temp = temp->next;
    }
    newNode->binding = reserveSpace(size);
    prev->next = newNode;
    return head;
}

lst* gstLookup(lst* head, char* name) {
    lst* temp = head;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

void printGST(lst* head) {
    lst* temp = head;
    printf("Global Symbol Table:\n");
    printf("Name\tType\tSize\tBinding\tPtr_Level\n");
    while (temp != NULL) {
        const char* typeStr;
        switch (temp->type) {
            case TYPE_INT: typeStr = "INT"; break;
            case TYPE_STR: typeStr = "STR"; break;
            case TYPE_BOOL: typeStr = "BOOL"; break;
            default: typeStr = "UNKNOWN"; break;
        }
        printf("%s\t%s\t%d\t%d\t%d\n", temp->name, typeStr, temp->size, temp->binding, temp->ptr_level);
        temp = temp->next;
    }
}

int getSP() {
    return addr_counter-1;
}

int reserveSpace(int size) {
    int temp = addr_counter;
    addr_counter += size;
    return temp;
}