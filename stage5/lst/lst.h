#ifndef LST_H
#define LST_H

typedef varType varType; // Forward declaration of varType from tree.h

typedef struct lst {
    char* name;
    varType type;
    int size;   // size will always be 1 for local vars
    int binding;
    int ptr_level;
    struct lst *next;
} lst;

lst* gstLookup(lst* head, char* name);
lst* gstInstall(lst* head, char* name, varType type, int size, int ptr_level);

#endif