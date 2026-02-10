#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

extern struct gst* gstRoot;

void assignType(node* root, varType type) {
    if(root == NULL) {
        return;
    }
    if(root->nodetype == NODE_ID) {
        int size = 1;
        char* varname = root->varname;

        gstRoot = gstInstall(gstRoot, varname, type, size, 0);
        root->gstEntry = gstLookup(gstRoot, varname);
        root->type = type;
        return;
    }
    if(root->nodetype == NODE_ARRAY) {
        // Right node is guaranteed to be NUM due to grammar
        int size = root->right->val;
        char* varname = root->varname;

        gstRoot = gstInstall(gstRoot, varname, type, size, 0);
        root->gstEntry = gstLookup(gstRoot, varname);
        root->left->gstEntry = root->gstEntry; 
        root->type = type;
        return; // You don't want to assign type to the ID child.
    }
    if(root->nodetype == NODE_PTR) {
        // Right node is the type being pointed to
        assignType(root->right, type);
        root->varname = root->right->varname; 
        root->gstEntry = root->right->gstEntry;
        root->gstEntry->ptr_level++;
        root->type = type;
        return;
    }
    assignType(root->left, type);
    assignType(root->right, type);
}

node* createTreeNode() {
    node* temp = (node*) malloc(sizeof(node));
    temp->varname = NULL;
    temp->left = NULL;
    temp->right = NULL;
    temp->gstEntry = NULL;
    return temp;
}

node* makeLeafNumNode(int n) {
    node* temp = createTreeNode();
    temp->val = n;
    temp->type = TYPE_INT;
    temp->nodetype = NODE_NUM;
    temp->gstEntry = NULL;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

node* makeLeafStrNode(char* str) {
    node* temp = createTreeNode();
    temp->strval = strdup(str);
    temp->type = TYPE_STR;
    temp->nodetype = NODE_STR;
    temp->gstEntry = NULL;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

node* makeLeafIdNode(char* varname) {
    node* temp = createTreeNode();
    temp->varname = strdup(varname);
    temp->nodetype = NODE_ID;
    temp->gstEntry = gstLookup(gstRoot, varname);
    if(temp->gstEntry) {
        temp->type = temp->gstEntry->type;
    } else {
        temp->type = -1;
    }
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

node* makeArrayNode(node* varname, node* sizeNode) {
    if(sizeNode->type != TYPE_INT) {
        printf("Error: Array index must be an integer\n");
        exit(1);
    }

    node* temp = createTreeNode();
    temp->varname = strdup(varname->varname);
    temp->nodetype = NODE_ARRAY;
    // GST entry would have been created when processing DECL node
    temp->gstEntry = gstLookup(gstRoot, varname->varname);
    if(temp->gstEntry) {
        temp->type = temp->gstEntry->type;
    } else {
        temp->type = -1;
    }
    temp->left = varname;
    temp->right = sizeNode;
    return temp;
}

node* makeOpNode(char* op, node* left, node* right) {
    if(!left || !right) {
        printf("Error: NULL operand in makeOpNode\n");
        return NULL;
    }

    if(left->type != right->type) {
        printf("Error: Type mismatch in operands in makeOpNode: %d and %d for variable %s\n", left->type, right->type, left->varname ? left->varname : "unknown");
        exit(1);
    }

    node* temp = (node*) malloc(sizeof(node));

    if(strcmp(op, "=") == 0) {
        temp->nodetype = NODE_ASSIGN;
        temp->type = right->type;
        temp->left = left;
        temp->right = right;
        return temp;
    }
    else if(strcmp(op, "==") == 0) {
        temp->nodetype = NODE_EQ;
        temp->type = TYPE_BOOL;
        temp->left = left;
        temp->right = right;
        return temp;
    }
    else if(strcmp(op, "!=") == 0) {
        temp->nodetype = NODE_NEQ;
        temp->type = TYPE_BOOL;
        temp->left = left;
        temp->right = right;
        return temp;
    }

    if(left->type != TYPE_INT || right->type != TYPE_INT) {
        printf("Error: Non-integer operand in makeOpNode\n");
        exit(1);
    }

    
    if(strcmp(op, "+") == 0) {
        temp->nodetype = NODE_PLUS;
        temp->type = TYPE_INT;
    }
    else if(strcmp(op, "-") == 0) { 
        temp->nodetype = NODE_MINUS;
        temp->type = TYPE_INT;
    }
    else if(strcmp(op, "*") == 0) {
        temp->nodetype = NODE_MUL;
        temp->type = TYPE_INT;
    }
    else if(strcmp(op, "%") == 0) {
        temp->nodetype = NODE_MOD;
        temp->type = TYPE_INT;
    }
    else if(strcmp(op, "/") == 0) {
        temp->nodetype = NODE_DIV;
        temp->type = TYPE_INT;
    }
    else if(strcmp(op, ">") == 0) {
        temp->nodetype = NODE_GT;
        temp->type = TYPE_BOOL;
    }
    else if(strcmp(op, ">=") == 0) {
        temp->nodetype = NODE_GTE;
        temp->type = TYPE_BOOL;
    }
    else if(strcmp(op, "<") == 0) {
        temp->nodetype = NODE_LT;
        temp->type = TYPE_BOOL;
    }
    else if(strcmp(op, "<=") == 0) {
        temp->nodetype = NODE_LTE;
        temp->type = TYPE_BOOL;
    }
    else {
        printf("Error: Unknown operator %s in makeOpNode\n", op);
        free(temp);
        return NULL;
    }

    temp->left = left;
    temp->right = right;
    return temp;
}

node* makeWriteNode(node* left) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_WRITE;
    temp->left = left;
    temp->right = NULL;
    return temp;
}

node* makeReadNode(node *left) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_READ;
    temp->left = left;
    temp->right = NULL;
    return temp;
}

node* makeConnectorNode(node* left, node* right) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_CONNECTOR;
    temp->left = left;
    temp->right = right;
    return temp;
}

node* makeIfElseNode(node* cond, node* ifStmt, node* elseStmt) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_IFELSE;
    temp->left = cond;
    temp->right = makeConnectorNode(ifStmt, elseStmt);
    return temp;
}

node* makeIfNode(node* cond, node* ifStmt) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_IF;
    temp->left = cond;
    temp->right = ifStmt;
    return temp;
}

node* makeWhileNode(node* cond, node* body) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_WHILE;
    temp->left = cond;
    temp->right = body;
    return temp;
}

node* makeBreakNode() {
    node* temp = createTreeNode();
    temp->nodetype = NODE_BREAK;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

node* makeContinueNode() {
    node* temp = createTreeNode();
    temp->nodetype = NODE_CONTINUE;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

node* makeBrkpNode() {
    node* temp = createTreeNode();
    temp->nodetype = NODE_BRKP;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

node* makeDoWhileNode(node* body, node* cond) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_DOWHILE;
    temp->left = cond;
    temp->right = body;
    return temp;
}

node* makeDeclNode(node* type, node* varlist) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_DECL;
    temp->left = type;
    temp->right = varlist;
    assignType(varlist, type->type);
    return temp;
}

node* makeTypeNode(varType type) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_TYPE;
    temp->type = type;
    return temp;
}

node* makePtrNode(node* ptrTo) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_PTR;
    temp->type = ptrTo->type;
    if(ptrTo->gstEntry) {
        temp->gstEntry = ptrTo->gstEntry;
        temp->varname = ptrTo->varname;
        // Level of PTR would have already been updated in decl section
        // Level of all the child PTR nodes are irrelevant here.
    }
    temp->left = NULL;
    temp->right = ptrTo;
    return temp;
}

node* makeAddressNode(node* var) {
    node* temp = createTreeNode();
    temp->nodetype = NODE_ADDR_OF;
    temp->type = var->type;
    if(var->gstEntry) {
        temp->gstEntry = var->gstEntry;
        temp->varname = var->varname;
        temp->val = temp->gstEntry->binding;
    }
    temp->left = NULL;
    temp->right = var;
    return temp;
}
