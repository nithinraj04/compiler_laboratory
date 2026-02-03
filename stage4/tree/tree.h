#ifndef TREE_H
#define TREE_H

typedef struct gst gst; // Forward declaration of gst from gst.h

typedef enum {
    NODE_EMPTY,
    NODE_CONNECTOR,
    NODE_READ,
    NODE_WRITE,
    NODE_NUM,
    NODE_STR,
    NODE_ID,
    NODE_PLUS,
    NODE_MINUS,
    NODE_MUL,
    NODE_DIV,
    NODE_MOD,
    NODE_ASSIGN,
    NODE_GT,
    NODE_GTE,
    NODE_LT,
    NODE_LTE,
    NODE_EQ,
    NODE_NEQ,
    NODE_IFELSE,
    NODE_IF,
    NODE_WHILE,
    NODE_BREAK,
    NODE_CONTINUE,
    NODE_BRKP,
    NODE_DOWHILE,
    NODE_TYPE,
    NODE_DECL,
    NODE_ARRAY
} nodeType;

enum varType {
    TYPE_INT,
    TYPE_STR,
    TYPE_BOOL
};

typedef enum varType varType; // Forward declaration of varType for gst.h

#include "../gst/gst.h"

typedef struct node{
    int val;        // value of a number for NUM nodes.
    char* strval;    // value of a string for STR nodes.
    varType type;       // type of variable
    char* varname;  // name of a variable for ID nodes
    nodeType nodetype;   // information about non-leaf nodes - read/write/connector/+/* etc.
    struct gst* gstEntry; // pointer to GST entry for variables
    struct node *left,*right;  // left and right branches
} node;

node* makeLeafNumNode(int n);
node* makeLeafStrNode(char* str);
node* makeLeafIdNode(char* varname);
node *makeWriteNode(node* left);
node* makeReadNode(node *left);
node* makeOpNode(char* op, node* left, node* right);
node* makeConnectorNode(node* left, node* right);
node* makeIfNode(node* cond, node* ifStmt);
node* makeIfElseNode(node* cond, node* ifStmt, node* elseStmt);
node* makeWhileNode(node* cond, node* body);
node* makeBreakNode();
node* makeContinueNode();
node* makeBrkpNode();
node* makeDoWhileNode(node* body, node* cond);
node* makeDeclNode(node* type, node* varlist);
node* makeTypeNode(varType type);
node* makeArrayNode(node* varname, node* size);

#endif