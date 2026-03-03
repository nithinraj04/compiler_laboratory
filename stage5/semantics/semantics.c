#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../tree/tree.h"
#include "../symbol_table/gst.h"
#include "../codegen/utils.h"

extern struct gst* gstRoot;
extern struct gst* lstRoot;

void matchParamsList(struct paramStruct** paramList, node* argList) {
    if(paramList == NULL || *paramList == NULL) {
        if(argList == NULL) {
            return;
        }
        if(argList->nodetype == NODE_CONNECTOR) {
            matchParamsList(paramList, argList->left);
            matchParamsList(paramList, argList->right);
            return;
        }
        printf("Semantics Error: Too many arguments in function definition/call\n");
        exit(1);
    }

    if(argList == NULL) {
        return;
    }
    
    if(argList->nodetype == NODE_CONNECTOR) {
        matchParamsList(paramList, argList->left);
        matchParamsList(paramList, argList->right);
        return;
    }

    if(*paramList == NULL) {
        printf("Semantics Error: Too many arguments in function definition/call\n");
        exit(1);
    }

    if(argList->nodetype == NODE_PARAM) {
        if((*paramList)->type != argList->type) {
            printf("Semantics Error: Fn def - Parameter typ mismatch for parameter '%s'\n", (*paramList)->name);
            exit(1);
        }
        if(strcmp((*paramList)->name, argList->right->varname) != 0) {
            printf("Semantics Error: Fn def - Parameter name mismatch for parameter '%s'\n", (*paramList)->name);
            exit(1);
        }
        *paramList = (*paramList)->next;
        return;
    }

    if(argList->nodetype == NODE_ARG) {
        if((*paramList)->type != argList->left->type) {
            printf("Semantics Error: Fn call - Parameter type mismatch for parameter '%s'\n", (*paramList)->name);
            exit(1);
        }
        if((*paramList)->ptr_level != getDerefLevel(argList->left)) {
            printf("Semantics Error: Fn call - Dereference level mismatch for parameter '%s'\n", (*paramList)->name);
            exit(1);
        }
        *paramList = (*paramList)->next;
        return;
    }
    *paramList = (*paramList)->next;
    return;
}

int getDerefLevel(node* root) {
    if(root == NULL) {
        return 0;
    }
    if(root->nodetype == NODE_PTR) {
        return getDerefLevel(root->right) - 1;
    }
    if(root->nodetype == NODE_ADDR_OF) {
        return getDerefLevel(root->right) + 1;
    }
    if(root->nodetype == NODE_ID || root->nodetype == NODE_ARRAY) {
        gst*  entry = globalLookup(gstRoot, lstRoot, root->varname);
        return entry ? entry->ptr_level : 0;
    }
    int left = getDerefLevel(root->left);
    int right = getDerefLevel(root->right);
    return left > right ? left : right;
}

void semantics(node* root) {
    if(root == NULL) {
        return;
    }

    switch(root->nodetype) {
        case NODE_DECL: {
            return;
        }
        case NODE_READ:
        case NODE_WRITE: {
            semantics(root->left);

            if(getDerefLevel(root->left) != 0) {
                printf("Semantics Error: Cannot read/write pointer variable '%s' without dereferencing\n", root->left->varname);
                exit(1);
            }
            
            return;
        }
        case NODE_ASSIGN: {
            // Left can be ID, PTR, ARRAY or ADDR_OF
            // Right can be any expr

            node* left = root->left; 
            node* right = root->right;

            semantics(left);
            semantics(right);
            
            if(left->nodetype == NODE_ADDR_OF) {
                printf("Semantics Error: Cannot assign to address of variable '%s'\n", left->right->varname);
                exit(1);
            }

            if(left->nodetype == NODE_ID) {

                // ID nodes that are pointers
                gst* entry = globalLookup(gstRoot, lstRoot, left->varname);
                if(entry && entry->ptr_level > 0) {
                    if(right->nodetype == NODE_NULL) {
                        // Allowed
                        break;
                    }
                }
            }


            //General check for cases where LHS is ID or ARRAY or PTR but RHS is not NULL.
            if(getDerefLevel(left) != getDerefLevel(right)) {
                printf("Semantics Error: Dereference level mismatch in assignment to variable '%s'\n", left->varname);
                exit(1);
            }
            if(left->type != right->type) {
                printf("Semantics Error: type mismatch in assignment to variable '%s'\n", left->varname);
                exit(1);
            }

            return;
        }

        case NODE_ID: {
            gst* entry = globalLookup(gstRoot, lstRoot, root->varname);
            if(entry == NULL) {
                printf("Semantics Error: Undeclared variable '%s'\n", root->varname);
                exit(1);
            }
            if(root->type == -1) {
                root->type = entry->type;
            }
            return;
        }

        case NODE_ARRAY: {
            // Check index expression semantics
            semantics(root->right); 

            if(root->gstEntry == NULL) {
                printf("Semantics Error: Undeclared array '%s'\n", root->varname);
                exit(1);
            }
            if(root->gstEntry->size == 1) {
                printf("Semantics Error: Variable '%s' is not declared as an array \n", root->varname);
                exit(1);
            }
            if(getDerefLevel(root->right) != 0) {
                printf("Semantics Error: Invalid dereference level in index expression for array '%s'\n", root->varname);
                exit(1);
            }
            if(root->right->type != TYPE_INT) {
                printf("Semantics Error: Non-integer index expression for array '%s'\n", root->varname);
                exit(1);
            }
            if(root->right->nodetype == NODE_NUM) {
                int index = root->right->val;
                if (index < 0 || index >= root->gstEntry->size) {
                    printf("Semantics Error: Array index out of bounds for array '%s' (size is %d, index accessed is %d)\n", root->varname, root->gstEntry->size, index);
                    exit(1);
                }
            }
            return;
        }

        case NODE_PTR: {
            gst* entry = globalLookup(gstRoot, lstRoot, root->varname);
            if(!entry) {
                printf("Semantics Error: Undeclared pointer variable '%s'\n", root->varname);
                exit(1);
            }
            if(root->type == -1) {
                root->type = entry->type;
            }
            if(entry->type != TYPE_INT && entry->type != TYPE_STR) {
                printf("Semantics Error: Pointers to type '%d' not supported\n", entry->type);
                exit(1);
            }
            if(globalLookup(gstRoot, lstRoot, root->right->varname) == NULL) {
                printf("Semantics Error: Undeclared pointer variable '%s'\n", root->right->varname);
                exit(1);
            }
            if(getDerefLevel(root) < 0) {
                printf("Semantics Error: Invalid dereference level for pointer variable '%s'\n", root->right->varname);
                exit(1);
            }
            return;
        }

        case NODE_EQ:
        case NODE_NEQ: {
            semantics(root->left);
            semantics(root->right);

            if(getDerefLevel(root->left) != getDerefLevel(root->right)) {
                printf("Semantics Error: Dereference level mismatch in equality comparison\n");
                exit(1);
            }
            if(root->left->type != root->right->type) {
                printf("Semantics Error: Type mismatch in equality comparison\n");
                exit(1);
            }

            return;
        }

        case NODE_PLUS:
        case NODE_MINUS: {
            node* left = root->left;
            node* right = root->right;

            semantics(left);
            semantics(right);

            if(
               left->type == TYPE_INT && getDerefLevel(left) == 0
                &&
               right->type == TYPE_INT && getDerefLevel(right) == 0
            ) {
                // Allowed
                return;
            }

            if (root->nodetype == NODE_MINUS && getDerefLevel(right) > 0) {
                printf("Semantics Error: Cannot subtract a pointer from an integer\n");
                exit(1);
            }

            if(getDerefLevel(left) > 0 && getDerefLevel(right) == 0
                &&
               right->type == TYPE_INT 
            ) {
                // Allowed (pointer arithmetic)
                return;
            }

            if(getDerefLevel(right) > 0 && getDerefLevel(left) == 0
                &&
               left->type == TYPE_INT 
            ) {
                // Allowed (pointer arithmetic)
                return;
            }

            printf("Semantics Error: Invalid operands for operator '%s'\n", root->nodetype == NODE_PLUS ? "+" : "-");
            exit(1);
        }

        case NODE_MUL:
        case NODE_DIV:
        case NODE_MOD: 
        case NODE_GT:
        case NODE_GTE:
        case NODE_LT:
        case NODE_LTE: {
            node* left = root->left;
            node* right = root->right;

            semantics(left);
            semantics(right);

            if(
               left->type == TYPE_INT && getDerefLevel(left) == 0
                &&
               right->type == TYPE_INT && getDerefLevel(right) == 0
            ) {
                // Allowed
                return;
            }

            printf("Semantics Error: Invalid operands for OP node");
            exit(1);
        }

        case NODE_IF: 
        case NODE_IFELSE: {
            semantics(root->left); // condition
            semantics(root->right); // if body

            if(root->left->type != TYPE_BOOL) {
                printf("Semantics Error: Condition expression in if statement must be of type bool\n");
                exit(1);
            }

            return;
        }

        case NODE_WHILE:
        case NODE_DOWHILE: {
            node *left = root->left; // condition
            node *right = root->right; // body

            semantics(left);
            semantics(right); 

            if(left->type != TYPE_BOOL) {
                printf("Semantics Error: Condition expression in while statement must be of type bool\n");
                exit(1);
            }

            return;
        }

        case NODE_FNDEF: {
            gst* gstEntry = root->gstEntry;
            node* paramList = root->left;

            if(!gstLookup(gstRoot, root->varname)) {
                printf("Semantics Error: Function '%s' not declared\n", root->varname);
                exit(1);
            }

            if(root->type != root->gstEntry->type) {
                printf("Semantics Error: Return type mismatch in function definition for function '%s'\n", root->varname);
                exit(1);
            }

            struct paramStruct* params = gstEntry->paramList;
            matchParamsList(&params, paramList);
            if(params != NULL) {
                printf("Semantics Error: Too few arguments in function definition for function '%s'\n", root->varname);
                exit(1);
            }

            // Build LST (Includes semantic checks for redeclaration of local variables and parameters)
            params = gstEntry->paramList; // reset params pointer to head of list
            while(params != NULL) {
                lstRoot = lstInstall(lstRoot, params->name, params->type, params->ptr_level);
                params = params->next;
            }
            bindParams(lstRoot); // assign bindings to parameters

            buildLST(root->right->left, &lstRoot, gstEntry->paramList);

            semantics(root->right->right); // function body

            freeLst(lstRoot);
            lstRoot = NULL;
            return;
        }

        case NODE_FNCALL: {
            gst* gstEntry = root->gstEntry;
            node* argList = root->right;

            if(!gstEntry) {
                printf("Semantics Error: Undeclared function '%s'\n", root->varname);
                exit(1);
            }

            if(root->type != gstEntry->type) {
                printf("Semantics Error: Return type mismatch in function call for function '%s'\n", root->varname);
                exit(1);
            }

            semantics(argList); // resolve/check argument expression types before signature matching

            struct paramStruct* params = gstEntry->paramList;
            matchParamsList(&params, argList);

            if(params != NULL) {
                printf("Semantics Error: Too few arguments in function call for function '%s'\n", root->varname);
                exit(1);
            }

            return;
        }

        case NODE_MAIN: {
            buildLST(root->left, &lstRoot, NULL);
            printGST(lstRoot);
            semantics(root->right);
            freeLst(lstRoot);
            lstRoot = NULL;
            return;
        }
    }

    semantics(root->left);
    semantics(root->right);
}