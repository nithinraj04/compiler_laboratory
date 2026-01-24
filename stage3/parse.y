%{
    #include "tree/tree.h"
    #include <stdio.h>
    #include <stdlib.h>
    #include "translate/translate.h"
    int yylex();  
    void yyerror(const char *s);
    void postorder(node* root);
    int codeGen(node* root, FILE* targetFile);
    void print(int reg, FILE* targetFile);
    int evaluator(node* root);
    node* root;
    extern FILE* yyin;
%}

%define parse.error verbose

%union {
    struct node* p;
}

%token <p> START END READ WRITE NUM ID GE LE EQ NE IF THEN ELSE ENDIF WHILE DO ENDWHILE BREAK CONTINUE BRKP
%type <p> expr stmt stmtList inputStmt outputStmt assignStmt start ifStmt whileStmt doWhileStmt

%left '+' '-'
%left '*' '/'
%nonassoc '<' '>' GE LE EQ NE

%%

start : START stmtList END   { root = $2; }
      | START END            { root = NULL; }
      ;
stmtList : stmtList stmt     { $$ = makeConnectorNode($1, $2); }
         | stmt              { $$ = $1; }
         ;
stmt : inputStmt             { $$ = $1; }
     | outputStmt            { $$ = $1; }
     | assignStmt            { $$ = $1; }
     | ifStmt                { $$ = $1; }
     | whileStmt             { $$ = $1; }
     | doWhileStmt           { $$ = $1; }
     | BREAK ';'            { $$ = makeBreakNode(); }
     | CONTINUE ';'         { $$ = makeContinueNode(); }
     | BRKP ';'             { $$ = makeBrkpNode(); }
     ;
inputStmt : READ '(' ID ')' ';'    { $$ = makeReadNode($3); }
          ;
outputStmt : WRITE '(' expr ')' ';' { $$ = makeWriteNode($3); }
           ;
assignStmt : ID '=' expr ';'        { $$ = makeOpNode("=", $1, $3); }
           ;
ifStmt : IF '(' expr ')' THEN stmtList ELSE stmtList ENDIF  { $$ = makeIfElseNode($3, $6, $8); }
       | IF '(' expr ')' THEN stmtList ENDIF                { $$ = makeIfNode($3, $6); }
       ;
whileStmt : WHILE '(' expr ')' DO stmtList ENDWHILE      { $$ = makeWhileNode($3, $6); }
          ;
doWhileStmt : DO stmtList WHILE '(' expr ')' ';'      { $$ = makeDoWhileNode($2, $5); }
           ;
expr : expr '+' expr         { $$ = makeOpNode("+", $1, $3); }
     | expr '-' expr         { $$ = makeOpNode("-", $1, $3); }
     | expr '*' expr         { $$ = makeOpNode("*", $1, $3); }
     | expr '/' expr         { $$ = makeOpNode("/", $1, $3); }
     | expr '<' expr         { $$ = makeOpNode("<", $1, $3); }
     | expr '>' expr         { $$ = makeOpNode(">", $1, $3); }
     | expr LE expr      { $$ = makeOpNode("<=", $1, $3); }
     | expr GE expr      { $$ = makeOpNode(">=", $1, $3); }
     | expr EQ expr      { $$ = makeOpNode("==", $1, $3); }
     | expr NE expr      { $$ = makeOpNode("!=", $1, $3); }
     | '(' expr ')'          { $$ = $2; }
     | NUM                   { $$ = $1; }
     | ID                    { $$ = $1; }
     ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

int main() {
    yyin = fopen("io/input.txt", "r");

    yyparse();

    FILE* targetFile = fopen("output.xsm", "w");
    if (targetFile == NULL) {
        fprintf(stderr, "Could not open output.xsm for writing\n");
        return 1;
    }

    fprintf(targetFile, "0\n2056\n0\n0\n0\n0\n0\n0\n");
    fprintf(targetFile, "BRKP\n");
    fprintf(targetFile, "MOV SP, 4121\n");
    int reg = codeGen(root, targetFile);
    fprintf(targetFile, "INT 10\n");

    // evaluator(root);

    fclose(targetFile);

    translateLabels(targetFile);

    return 0;
}