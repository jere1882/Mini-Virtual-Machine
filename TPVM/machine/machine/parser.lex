%{
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"
int line=1;
%}
     
DIGIT    [0-9]
ID       [a-z][a-z0-9]*
%%
     
"$-"{DIGIT}+ {
yylval.i=-atoi( yytext+2 );
return TOKIMM;
}


"$"{DIGIT}+ {
yylval.i=atoi( yytext+1 );
return TOKIMM;
}


{DIGIT}+ {
yylval.i=atoi( yytext );
return TOKNUMBER ;
}

"nop" { return TOKNOP;} 
"mov" { return TOKMOV;}
"sw" { return TOKSW;}
"lw" { return TOKLW;}
"push" { return TOKPUSH;}
"pop" { return TOKPOP;}
"print" { return TOKPRINT;}
"read" { return TOKREAD;}
"add" { return TOKADD;}
"sub" { return TOKSUB;}
"div" { return TOKDIV;}
"mul" { return TOKMUL;}
"cmp" { return TOKCMP;}
"jmp" { return TOKJMP;}
"jmpe" { return TOKJMPE;}
"jmpl" { return TOKJMPL;}
"hlt" { return TOKHLT;}
"call" { return TOKCALL;}
"ret" { return TOKRET;}
"loop" { return TOKLOOP;}
"ror" { return TOKROR;}
"rol" { return TOKROL;}
"sal" { return TOKSAL;}
"sar" { return TOKSAR;}
"shl" { return TOKSHL;}
"shr" { return TOKSHR;}
"xor" { return TOKXOR;}
"or" { return TOKOR;}
"not" { return TOKNOT;}
"and" { return TOKAND;}
"inc" { return TOKINC;}
"dec" { return TOKDEC;}




"%"{ID}  {yylval.strval=strdup(yytext); return TOKREG;}

{ID}":"  { yytext[strlen(yytext)-1]=0;yylval.strval=strdup(yytext);return TOKLABEL;}

{ID}  {yylval.strval=strdup(yytext);return TOKID;}
     
"(" {return TOKOPAREN;}
")" {return TOKCPAREN;}
\n {line++;return TOKCR;}
[ \t]+          /* eat up whitespace */
     
","   {return TOKCOMA;}
.           printf( "Unrecognized character: %s\n", yytext );
     
%%
int yyerror(const char *msg) {
  printf("Line %d: %s\n",line,msg);
  exit(-1);

}
int yywrap() {
  return 1;
}
