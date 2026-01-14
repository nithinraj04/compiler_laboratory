%{
#include <stdio.h>
#include <stdlib.h>

int integer = 0, floating = 0;
%}

%option noyywrap
digit [0-9]
letter [a-zA-Z]

%%

^{letter}({digit}|{letter})*  {printf("%s\n", yytext);}
.  {;}


%%

int main(int argc, char **argv) {
    yylex();
    // printf("\nintegers = %d, floating = %d\n", integer, floating);
    return 0;
}

// stmt -> if {count++, ans = max(ans, count)} (cond) stmt {count--} | expr
// expr -> num + num