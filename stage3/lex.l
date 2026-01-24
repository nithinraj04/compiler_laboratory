%{
    #include "parse.tab.h"
    #include "tree/tree.h"
    void yyerror(const char*);
%}

digit [0-9]

%%

{digit}+ { yylval.p = makeLeafNumNode(atoi(yytext)); return NUM; }
"begin"  { return START; }
"end"    { return END; }
"read"   { return READ; }
"write"  { return WRITE; }
"if"     { return IF; }
"then"   { return THEN; }
"else"   { return ELSE; }
"endif"  { return ENDIF; }
"while"  { return WHILE; }
"do"     { return DO; }
"endwhile" { return ENDWHILE; }
"break"    { return BREAK; }
"continue" { return CONTINUE; }
"breakpoint" { return BRKP; }
[a-z]    { yylval.p = makeLeafIdNode(yytext); return ID; }
"+"      { return '+'; }
"-"      { return '-'; }
"*"      { return '*'; }
"/"      { return '/'; }
">"      { return '>'; }
"<"      { return '<'; }
">="     { return GE; }
"<="     { return LE; }
"=="     { return EQ; }
"!="     { return NE; }
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
