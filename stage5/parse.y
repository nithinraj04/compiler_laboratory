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
    void semantics(node* root);
    node* root;
    extern FILE* yyin;
    extern int yylineno;
    extern gst* gstRoot;
%}

%define parse.error verbose

%union {
    struct node* p;
}

%token <p> START END READ WRITE NUM ID GE LE EQ NE IF THEN ELSE ENDIF WHILE DO ENDWHILE BREAK 
%token <p> CONTINUE BRKP DECL ENDDECL INT STR STRVAL NULLVAL RETURN MAIN

%type <p> expr stmt stmtList inputStmt outputStmt assignStmt start ifStmt whileStmt doWhileStmt 
%type <p> gDeclBlock gDeclList gDecl type gVarList identifier gVar index ptr fDefBlock mainBlock
%type <p> paramList param fDef lDeclBlock lDeclList lDecl lVarList lVar fBody retStmt argList

%nonassoc '<' '>' GE LE EQ NE
%left '+' '-'
%left '*' '/' '%'
%right PTR

%%

start : gDeclBlock fDefBlock mainBlock   { root = makeConnectorNode($1, makeConnectorNode($2, $3)); }
      | gDeclBlock mainBlock             { root = makeConnectorNode($1, $2); }
      | mainBlock                        { root = $1; }
      ;

gDeclBlock : DECL gDeclList ENDDECL  { $$ = $2; }
           | DECL ENDDECL        { $$ = NULL; }
           ;

gDeclList : gDeclList gDecl    { $$ = makeConnectorNode($1, $2); }
          | gDecl              { $$ = $1; }
          ;

gDecl : type gVarList ';'   { $$ = makeDeclNode($1, $2); }
      ;

gVarList : gVarList ',' gVar   { $$ = makeConnectorNode($1, $3); }
         | gVar                 { $$ = $1; }
         ;

gVar : ID                   { $$ = $1; }
     | ID '[' NUM ']'       { $$ = makeArrayNode($1, $3); }
     | ID '(' paramList ')' { $$ = makeFnDeclNode($1, $3); }
     | ID '(' ')'           { $$ = makeFnDeclNode($1, NULL); }
     | ptr                  { $$ = $1; }
     ;

paramList : paramList ',' param   { $$ = makeConnectorNode($1, $3); }
          | param                 { $$ = $1; }
          ;

param : type ID   { $$ = makeParamNode($1, $2); }
      | type ptr  { $$ = makeParamNode($1, $2); }   //Hmmmmm
      ;

// ------------------------------------------------------------

fDefBlock : fDefBlock fDef    { $$ = makeConnectorNode($1, $2); }
          | fDef              { $$ = $1; }
          ;

fDef : type ID '(' paramList ')' '{' lDeclBlock fBody '}'   { $$ = makeFnDefNode($1, $2, $4, $7, $8); }
     | type ID '(' ')' '{' lDeclBlock fBody '}'             { $$ = makeFnDefNode($1, $2, NULL, $6, $7); }

lDeclBlock : DECL lDeclList ENDDECL  { $$ = $2; }
           | DECL ENDDECL            { $$ = NULL; }
           | /* empty */             { $$ = NULL; }
           ;

lDeclList : lDeclList lDecl    { $$ = makeConnectorNode($1, $2); }
          | lDecl              { $$ = $1; }
          ;

lDecl : type lVarList ';'   { $$ = makeDeclNode($1, $2); }
      ;

lVarList : lVarList ',' lVar   { $$ = makeConnectorNode($1, $3); }
         | lVar                 { $$ = $1; }
         ;

lVar : ID                   { $$ = $1; }  //No arrays within functions
     | ptr                  { $$ = $1; }
     ;

fBody : START stmtList retStmt END  { $$ = makeConnectorNode($2, $3); }
      ;

retStmt : RETURN expr ';'  { $$ = makeReturnNode($2); }
        ;

stmtList : /* empty */       { $$ = NULL; }
         | stmtList stmt     { $$ = ($1 == NULL) ? $2 : makeConnectorNode($1, $2); }
         ;

stmt : inputStmt             { $$ = $1; }
     | outputStmt            { $$ = $1; }
     | assignStmt            { $$ = $1; }
     | ifStmt                { $$ = $1; }
     | whileStmt             { $$ = $1; }
     | doWhileStmt           { $$ = $1; }
     | BREAK ';'             { $$ = makeBreakNode(); }
     | CONTINUE ';'          { $$ = makeContinueNode(); }
     | BRKP ';'              { $$ = makeBrkpNode(); }
     | ID '(' argList ')' ';' { $$ = makeFnCallNode($1, $3); }
     | ID '(' ')' ';'         { $$ = makeFnCallNode($1, NULL); }
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
     | ID '(' argList ')'    { $$ = makeFnCallNode($1, $3); }
     | ID '(' ')'            { $$ = makeFnCallNode($1, NULL); }
     | NUM                   { $$ = $1; }
     | STRVAL                { $$ = $1; }
     | identifier            { $$ = $1; }
     | NULLVAL               { $$ = makeNullNode(); }
     ;

argList : argList ',' expr   { $$ = makeConnectorNode($1, $3); }
        | expr               { $$ = $1; }
        ;

identifier : ID    { $$ = $1; }
           | ID '[' index ']'   { $$ = makeArrayNode($1, $3); }
           | '&' ID             { $$ = makeAddressNode($2); }
           | ptr  %prec PTR     { $$ = $1; }
           ;

type : INT   { $$ = makeTypeNode(TYPE_INT); }
     | STR   { $$ = makeTypeNode(TYPE_STR); }
     ;

ptr : '*' ptr   { $$ = makePtrNode($2); }
    | '*' ID    { $$ = makePtrNode($2); }
    ;

index : expr    { $$ = $1; }

// -------------------------------------------------------------

mainBlock : INT MAIN '(' ')' '{' lDeclBlock fBody '}'   { $$ = makeMainNode($6, $7); }
          ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s at line %d\n", s, yylineno);
}

int main() {
    yyin = fopen("io/input.txt", "r");

    yyparse();
    semantics(root);

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