%{
    #include "parse.tab.h"
    #include "tree.h"
    void yyerror(const char*);
%}

digit [0-9]

%%

{digit}+ { yylval.p = makeLeafNumNode(atoi(yytext)); return NUM; }
"begin"  { return START; }
"end"    { return END; }
"read"   { return READ; }
"write"  { return WRITE; }
[a-z]    { yylval.p = makeLeafIdNode(yytext); return ID; }
"+"      { return '+'; }
"-"      { return '-'; }
"*"      { return '*'; }
"/"      { return '/'; }
"="      { return '='; }
";"      { return ';'; }
"("      { return '('; }
")"      { return ')'; }
[ \t\n]  { /* ignore whitespace */ }
.        { yyerror("Invalid character\n"); }

%%

int yywrap() {
    return 1;
}
