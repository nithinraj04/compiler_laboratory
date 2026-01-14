typedef struct node{
    int val;
    char op; //indicates the name of the operator for a non leaf node
    struct node *left, *right; //left and right branches
} node;

node* makeLeafNode(int n);

node* makeOpNode(char op, node* left, node* right);