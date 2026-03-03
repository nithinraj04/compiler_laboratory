#ifndef GST_H
#define GST_H

typedef varType varType; // Forward declaration of varType from tree.h

typedef struct paramStruct {
    char* name;
    varType type;
    int ptr_level;
    struct paramStruct* next;
} paramStruct;

typedef struct gst {
    char* name;       // name of the variable
    varType type;     // type of the variable
    int size;         // size of the type of the variable
    int binding;      // stores the static memory address allocated to the variable
    int ptr_level;    // 0 for normal variables, 1 for pointers, 2 for double pointers, etc.
    paramStruct* paramList; // List of parameters if the variable is a function
    int fLabel;       // Label for the function entry point in code generation
    struct gst *next;
} gst;

gst* gstLookup(gst* head, char* name);
gst* gstInstall(gst* head, char* name, varType type, int size, int ptr_level);
void printGST(gst* head);
int getSP();
int reserveSpace(int size);

#endif