typedef enum {
    NODE_EMPTY,
    NODE_CONNECTOR,
    NODE_READ,
    NODE_WRITE,
    NODE_NUM,
    NODE_ID,
    NODE_PLUS,
    NODE_MINUS,
    NODE_MUL,
    NODE_DIV,
    NODE_ASSIGN,
    NODE_GT,
    NODE_GTE,
    NODE_LT,
    NODE_LTE,
    NODE_EQ,
    NODE_NEQ,
    NODE_IFELSE,
    NODE_IF,
    NODE_WHILE
} nodeType;

typedef enum {
    TYPE_INT,
    TYPE_BOOL
} varType;

typedef struct node{
    int val;        // value of a number for NUM nodes.
    varType type;       // type of variable
    char* varname;  // name of a variable for ID nodes
    nodeType nodetype;   // information about non-leaf nodes - read/write/connector/+/* etc.
    struct node *left,*right;  // left and right branches
} node;

node* makeLeafNumNode(int n);
node* makeLeafIdNode(char* varname);
node *makeWriteNode(node* left);
node* makeReadNode(node *left);
node* makeOpNode(char* op, node* left, node* right);
node* makeConnectorNode(node* left, node* right);
node* makeIfNode(node* cond, node* ifStmt);
node* makeIfElseNode(node* cond, node* ifStmt, node* elseStmt);
node* makeWhileNode(node* cond, node* body);