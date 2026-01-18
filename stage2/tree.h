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
    NODE_ASSIGN
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
node* makeOpNode(char op, node* left, node* right);
node* makeConnectorNode(node* left, node* right);