#include <stdio.h>
#include <stdlib.h>
#include "../tree/tree.h"
#include "../gst/gst.h"

extern struct gst* gstRoot;

int getPtrLevel(node* root) {
    if(root == NULL) {
        return 0;
    }
    if(root->nodetype == NODE_PTR) {
        return getPtrLevel(root->right) - 1;
    }
    if(root->nodetype == NODE_ADDR_OF) {
        return getPtrLevel(root->right) + 1;
    }
    return root->gstEntry ? root->gstEntry->ptr_level : 0;
}

void semantics(node* root) {
    if(root == NULL) {
        return;
    }

    if(root->nodetype == NODE_READ) {
        if(root->left->nodetype == NODE_ID) {
            if(root->left->gstEntry == NULL) {
                printf("Error: Undeclared variable '%s' in read statement\n", root->left->varname);
                exit(1);
            }
            if(root->left->gstEntry->size > 1) {
                printf("Error: Cannot read into un-indexed vector '%s' in read statement\n", root->left->varname);
                exit(1);
            }
            if(root->left->gstEntry->ptr_level > 0) {
                printf("Error: Cannot read into pointer variable '%s' without dereferencing in read statement\n", root->left->varname);
                exit(1);
            }
        }
        if(root->left->nodetype == NODE_ARRAY) {
            if(root->left->gstEntry == NULL) {
                printf("Error: Undeclared array '%s' in read statement\n", root->left->varname);
                exit(1);
            }
            if (root->right && root->right->nodetype == NODE_NUM) {
                int index = root->right->val;
                if (index < 0 || index >= root->gstEntry->size) {
                    printf("Error: Array index out of bounds for array '%s' (size is %d, index accessed is %d)\n", root->varname, root->gstEntry->size, index);
                    exit(1);
                }
            }
        }
        if(root->left->nodetype == NODE_PTR) {
            if(root->left->gstEntry == NULL) {
                printf("Error: Undeclared pointer variable '%s' in read statement\n", root->left->right->varname);
                exit(1);
            }
            if(getPtrLevel(root->left) != 0) {
                printf("Error: Pointer variable '%s' incorrectly dereferenced in read statement\n", root->left->right->varname);
                exit(1);
            }
        }
    }

    if(root->nodetype == NODE_WRITE) {
        if(root->left->nodetype == NODE_ID) {
            if(root->left->gstEntry == NULL) {
                printf("Error: Undeclared variable '%s' in write statement\n", root->left->varname);
                exit(1);
            }
            if(root->left->gstEntry->size > 1) {
                printf("Error: Cannot write un-indexed vector '%s' in write statement\n", root->left->varname);
                exit(1);
            }
            if(root->left->gstEntry->ptr_level > 0) {
                printf("Error: Cannot write pointer variable '%s' without dereferencing in write statement\n", root->left->varname);
                exit(1);
            }
        }
        if(root->left->nodetype == NODE_ARRAY) {
            if(root->left->gstEntry == NULL) {
                printf("Error: Undeclared array '%s' in write statement\n", root->left->varname);
                exit(1);
            }
            if (root->right && root->right->nodetype == NODE_NUM) {
                int index = root->right->val;
                if (index < 0 || index >= root->gstEntry->size) {
                    printf("Error: Array index out of bounds for array '%s' (size is %d, index accessed is %d)\n", root->varname, root->gstEntry->size, index);
                    exit(1);
                }
            }
        }
        if(root->left->nodetype == NODE_PTR) {
            if(root->left->gstEntry == NULL) {
                printf("Error: Undeclared pointer variable '%s' in write statement\n", root->left->right->varname);
                exit(1);
            }
            if(getPtrLevel(root->left) != 0) {
                printf("Error: Pointer variable '%s' incorrectly dereferenced in write statement\n", root->left->right->varname);
                exit(1);
            }
        }
    }

    if(root->nodetype == NODE_ASSIGN) {
        if(root->left->nodetype == NODE_ID) {
            if(root->left->gstEntry == NULL) {
                printf("Error: Undeclared variable '%s' in assignment statement\n", root->left->varname);
                exit(1);
            }
            if(root->left->gstEntry->size > 1) {
                printf("Error: Cannot assign to un-indexed vector '%s' in assignment statement\n", root->left->varname);
                exit(1);
            }
        }
        if(root->right->nodetype == NODE_ID) {
            if(root->right->gstEntry == NULL) {
                printf("Error: Undeclared variable '%s' in assignment statement\n", root->right->varname);
                exit(1);
            }
            if(root->right->gstEntry->size > 1) {
                printf("Error: Cannot assign un-indexed vector '%s' in assignment statement\n", root->right->varname);
                exit(1);
            }
        }
        if(root->left->type != root->right->type) {
            printf("Error: Type mismatch in assignment statement between '%s' and '%s'\n", root->left->varname, root->right->varname);
            exit(1);
        }
        if(getPtrLevel(root->left) != getPtrLevel(root->right)) {
            printf("Error: Pointer level mismatch in assignment statement between '%s' and '%s'\n", root->left->varname, root->right->varname);
            exit(1);
        }
        if(root->left->nodetype == NODE_ARRAY) {
            if(root->left->gstEntry == NULL) {
                printf("Error: Undeclared array '%s' in assignment statement\n", root->left->varname);
                exit(1);
            }
            if (root->right && root->right->nodetype == NODE_NUM) {
                int index = root->right->val;
                if (index < 0 || index >= root->gstEntry->size) {
                    printf("Error: Array index out of bounds for array '%s' (size is %d, index accessed is %d)\n", root->varname, root->gstEntry->size, index);
                    exit(1);
                }
            }
        }
        if(root->left->nodetype == NODE_PTR) {
            if(root->left->gstEntry == NULL) {
                printf("Error: Undeclared pointer variable '%s' in assignment statement\n", root->left->right->varname);
                exit(1);
            }
        }
    }

    // Semantic check for OP already done within tree.c

    semantics(root->left);
    semantics(root->right);
}