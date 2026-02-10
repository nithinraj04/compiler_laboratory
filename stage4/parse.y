%{
    #include "tree/tree.h"
    #include "gst/gst.h"
    #include <stdio.h>
    #include <stdlib.h>
    #include "translate/translate.h"
    int yylex();  
    void yyerror(const char *s);
    void postorder(node* root);
    int codeGen(node* root, FILE* targetFile);
    void print(int reg, FILE* targetFile);
    void printAST(node* root, const char* prefix, int isLast);
    node* root;
    extern FILE* yyin;
    extern int yylineno;
    extern gst* gstRoot;
%}

%define parse.error verbose

%union {
    struct node* p;
}

%token <p> START END READ WRITE NUM ID GE LE EQ NE IF THEN ELSE ENDIF WHILE DO ENDWHILE BREAK CONTINUE BRKP DECL ENDDECL INT STR STRVAL
%type <p> expr stmt stmtList inputStmt outputStmt assignStmt start ifStmt whileStmt doWhileStmt declarations declList decl type varList identifier identifier_decl index ptr

%nonassoc '<' '>' GE LE EQ NE
%left '+' '-'
%left '*' '/' '%'
%right PTR

%%

start : declarations stmtList   { root = makeConnectorNode($1, $2); }
      | stmtList            { root = $1; }
      ;
declarations : DECL declList ENDDECL  { $$ = $2; }
             | DECL ENDDECL        { $$ = NULL; }
             ;
declList : declList decl    { $$ = makeConnectorNode($1, $2); }
         | decl              { $$ = $1; }
         ;
decl : type varList ';'   { $$ = makeDeclNode($1, $2); }
     ;
type : INT   { $$ = makeTypeNode(TYPE_INT); }
     | STR   { $$ = makeTypeNode(TYPE_STR); }
     ;
varList : varList ',' identifier_decl   { $$ = makeConnectorNode($1, $3); }
        | identifier_decl               { $$ = $1; }
        ;
identifier_decl : ID    { $$ = $1; }
                | ID '[' NUM ']'   { $$ = makeArrayNode($1, $3); }
                | ptr   { $$ = $1; }
                ;

ptr : '*' ptr   { $$ = makePtrNode($2); }
    | '*' ID    { $$ = makePtrNode($2); }
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
inputStmt : READ '(' identifier ')' ';'    { $$ = makeReadNode($3); }
          ;
outputStmt : WRITE '(' expr ')' ';' { $$ = makeWriteNode($3); }
           ;
assignStmt : identifier '=' expr ';'        { $$ = makeOpNode("=", $1, $3); }
           ;
ifStmt : IF '(' expr ')' THEN stmtList ELSE stmtList ENDIF ';'  { $$ = makeIfElseNode($3, $6, $8); }
       | IF '(' expr ')' THEN stmtList ENDIF ';'           { $$ = makeIfNode($3, $6); }
       ;
whileStmt : WHILE '(' expr ')' DO stmtList ENDWHILE ';'      { $$ = makeWhileNode($3, $6); }
          ;
doWhileStmt : DO stmtList WHILE '(' expr ')' ';'      { $$ = makeDoWhileNode($2, $5); }
           ;
expr : expr '+' expr         { $$ = makeOpNode("+", $1, $3); }
     | expr '-' expr         { $$ = makeOpNode("-", $1, $3); }
     | expr '*' expr         { $$ = makeOpNode("*", $1, $3); }
     | expr '%' expr         { $$ = makeOpNode("%", $1, $3); }
     | expr '/' expr         { $$ = makeOpNode("/", $1, $3); }
     | expr '<' expr         { $$ = makeOpNode("<", $1, $3); }
     | expr '>' expr         { $$ = makeOpNode(">", $1, $3); }
     | expr LE expr      { $$ = makeOpNode("<=", $1, $3); }
     | expr GE expr      { $$ = makeOpNode(">=", $1, $3); }
     | expr EQ expr      { $$ = makeOpNode("==", $1, $3); }
     | expr NE expr      { $$ = makeOpNode("!=", $1, $3); }
     | '(' expr ')'          { $$ = $2; }
     | NUM                   { $$ = $1; }
     | STRVAL                { $$ = $1; }
     | identifier            { $$ = $1; }
     ;

identifier : ID    { $$ = $1; }
           | ID '[' index ']'   { $$ = makeArrayNode($1, $3); }
           | '&' ID             { $$ = makeAddressNode($2); }
           | ptr  %prec PTR     { $$ = $1; }
           ;

index : expr    { $$ = $1; }

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s at line %d\n", s, yylineno);
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
    fprintf(targetFile, "MOV SP, %d\n", getSP());
    codeGen(root, targetFile);
    fprintf(targetFile, "INT 10\n");

    printAST(root, "", 1);
    printGST(gstRoot);

    fclose(targetFile);

    translateLabels(targetFile);

    return 0;
}