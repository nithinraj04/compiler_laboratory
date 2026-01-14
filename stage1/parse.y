%{
    #include "tree.h"
    #include <stdio.h>
    #include <stdlib.h>
    int yylex();  
    void yyerror(const char *s);
    void postorder(node* root);
    int codeGen(node* root, FILE* targetFile);
    void print(int reg, FILE* targetFile);
    node* root;
%}

%union {
    struct node* p;
}

%token <p> NUM
%type <p> expr

%left '+' '-'
%left '*' '/'

%%

start : expr                 { root = $1; }
// expr : expr '+' expr         { $$ = makeOpNode('+', $1, $3); }
//      | expr '-' expr         { $$ = makeOpNode('-', $1, $3); }
//      | expr '*' expr         { $$ = makeOpNode('*', $1, $3); }
//      | expr '/' expr         { $$ = makeOpNode('/', $1, $3); }
//      | '(' expr ')'          { $$ = $2; }
//      | NUM                   { $$ = $1; }
//      ;
expr : '+' expr expr         { $$ = makeOpNode('+', $2, $3); }
     | '-' expr expr         { $$ = makeOpNode('-', $2, $3); }
     | '*' expr expr         { $$ = makeOpNode('*', $2, $3); }
     | '/' expr expr         { $$ = makeOpNode('/', $2, $3); }
    //  | '(' expr ')'          { $$ = $2; }
     | NUM                   { $$ = $1; }
     ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

void postorder(node *root){
    if(!root->left && !root->right){
        printf("%d ", root->val);
        return;
    }

    postorder(root->left);
    postorder(root->right);

    printf("%c ", root->op);
    return;
}

void preorder(node *root){
    if(!root->left && !root->right){
        printf("%d ", root->val);
        return;
    }

    printf("%c ", root->op);

    preorder(root->left);
    preorder(root->right);

    return;
}

int main() {
    yyparse();

    FILE* targetFile = fopen("output.xsm", "w");
    if (targetFile == NULL) {
        fprintf(stderr, "Could not open output.xsm for writing\n");
        return 1;
    }

    // preorder(root);

    fprintf(targetFile, "0\n2056\n0\n0\n0\n0\n0\n0\n");
    int reg = codeGen(root, targetFile);
    print(reg, targetFile);
    fprintf(targetFile, "INT 10\n");

    return 0;
}