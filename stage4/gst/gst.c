#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../tree/tree.h"
#include "gst.h"

int addr_counter = 4096;
gst* gstRoot = NULL;

gst* createGstNode(char* name, varType type, int size) {
    gst* newNode = (gst*) malloc(sizeof(gst));
    newNode->name = strdup(name);
    newNode->type = type;
    newNode->size = size;
    newNode->binding = -1;
    newNode->next = NULL;
    return newNode;
}

gst* gstInstall(gst* head, char* name, varType type, int size) {
    gst* newNode = createGstNode(name, type, size);
    if (head == NULL) {
        newNode->binding = reserveSpace(size);
        return newNode;
    }
    gst* temp = head;
    while (temp->next != NULL) {
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
    temp->next = newNode;
    return head;
}

gst* gstLookup(gst* head, char* name) {
    gst* temp = head;
    while (temp != NULL) {
        if (strcmp(temp->name, name) == 0) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

void printGST(gst* head) {
    gst* temp = head;
    printf("Global Symbol Table:\n");
    printf("Name\tType\tSize\tBinding\n");
    while (temp != NULL) {
        const char* typeStr;
        switch (temp->type) {
            case TYPE_INT: typeStr = "INT"; break;
            case TYPE_STR: typeStr = "STR"; break;
            case TYPE_BOOL: typeStr = "BOOL"; break;
            default: typeStr = "UNKNOWN"; break;
        }
        printf("%s\t%s\t%d\t%d\n", temp->name, typeStr, temp->size, temp->binding);
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