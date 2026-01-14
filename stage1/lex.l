%{
    #include "parse.tab.h"
    #include "tree.h"
    void yyerror(const char*);
%}

digit [0-9]

%%

{digit}+ { yylval.p = makeLeafNode(atoi(yytext)); return NUM; }
"+"      { return '+'; }
"-"      { return '-'; }
"*"      { return '*'; }
"/"      { return '/'; }
"("      { return '('; }
")"      { return ')'; }
[ \t\n]  { /* ignore whitespace */ }
.        { yyerror("Invalid character\n"); }

%%

int yywrap() {
    return 1;
}
