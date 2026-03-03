#include <stdio.h>
#include <stdlib.h>
#include "../tree/tree.h"
#include "../gst/gst.h"

extern struct gst* gstRoot;

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
        return root->gstEntry ? root->gstEntry->ptr_level : 0;
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
                printf("Error: Cannot read/write pointer variable '%s' without dereferencing\n", root->left->varname);
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
                printf("Error: Cannot assign to address of variable '%s'\n", left->right->varname);
                exit(1);
            }

            if(left->nodetype == NODE_ID) {

                // ID nodes that are pointers
                if(left->gstEntry->ptr_level > 0) {
                    if(right->nodetype == NODE_NULL) {
                        // Allowed
                        break;
                    }
                }
            }


            //General check for cases where LHS is ID or ARRAY or PTR but RHS is not NULL.
            if(getDerefLevel(left) != getDerefLevel(right)) {
                printf("Error: Dereference level mismatch in assignment to variable '%s'\n", left->varname);
                exit(1);
            }
            if(left->type != right->type) {
                printf("Error: type mismatch in assignment to variable '%s'\n", left->varname);
                exit(1);
            }

            return;
        }

        case NODE_ID: {
            if(root->gstEntry == NULL) {
                printf("Error: Undeclared variable '%s'\n", root->varname);
                exit(1);
            }
            return;
        }

        case NODE_ARRAY: {
            // Check index expression semantics
            semantics(root->right); 

            if(root->gstEntry == NULL) {
                printf("Error: Undeclared array '%s'\n", root->varname);
                exit(1);
            }
            if(root->gstEntry->size == 1) {
                printf("Error: Variable '%s' is not declared as an array \n", root->varname);
                exit(1);
            }
            if(getDerefLevel(root->right) != 0) {
                printf("Error: Invalid dereference level in index expression for array '%s'\n", root->varname);
                exit(1);
            }
            if(root->right->type != TYPE_INT) {
                printf("Error: Non-integer index expression for array '%s'\n", root->varname);
                exit(1);
            }
            if(root->right->nodetype == NODE_NUM) {
                int index = root->right->val;
                if (index < 0 || index >= root->gstEntry->size) {
                    printf("Error: Array index out of bounds for array '%s' (size is %d, index accessed is %d)\n", root->varname, root->gstEntry->size, index);
                    exit(1);
                }
            }
            return;
        }

        case NODE_PTR: {
            if(root->type != TYPE_INT && root->type != TYPE_STR) {
                printf("Error: Pointers to type '%d' not supported\n", root->type);
                exit(1);
            }
            if(root->gstEntry == NULL) {
                printf("Error: Undeclared pointer variable '%s'\n", root->right->varname);
                exit(1);
            }
            if(getDerefLevel(root) < 0) {
                printf("Error: Invalid dereference level for pointer variable '%s'\n", root->right->varname);
                exit(1);
            }
            return;
        }

        case NODE_EQ:
        case NODE_NEQ: {
            semantics(root->left);
            semantics(root->right);

            if(getDerefLevel(root->left) != getDerefLevel(root->right)) {
                printf("Error: Dereference level mismatch in equality comparison\n");
                exit(1);
            }
            if(root->left->type != root->right->type) {
                printf("Error: Type mismatch in equality comparison\n");
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
                printf("Error: Cannot subtract a pointer from an integer\n");
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

            printf("Error: Invalid operands for operator '%s'\n", root->nodetype == NODE_PLUS ? "+" : "-");
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

            printf("Error: Invalid operands for OP node");
            exit(1);
        }

        case NODE_IF: 
        case NODE_IFELSE: {
            semantics(root->left); // condition
            semantics(root->right); // if body

            if(root->left->type != TYPE_BOOL) {
                printf("Error: Condition expression in if statement must be of type bool\n");
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
                printf("Error: Condition expression in while statement must be of type bool\n");
                exit(1);
            }

            return;
        }
    }

    // Semantic check for OP already done within tree.c

    semantics(root->left);
    semantics(root->right);
}