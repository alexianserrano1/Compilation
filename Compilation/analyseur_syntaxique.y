%{
#include<stdlib.h>
#include<stdio.h>
#define YYDEBUG 1
#include"syntabs.h" // pour syntaxe abstraite
#include "util.h"
#include "affiche_arbre_abstrait.h"

n_prog *n;   // pour syntaxe abstraite

extern FILE *yyin;    // declare dans compilo
extern int yylineno;  // declare dans analyseur lexical
int yylex();          // declare dans analyseur lexical
int yyerror(char *s); // declare ci-dessous
%}
%union {int ival; int op; char* sval; struct n_prog_ *n_prog; struct n_l_dec_ *n_l_dec; struct n_dec_ *n_dec; struct n_exp_ *n_exp; struct n_instr_ *n_instr; struct n_l_instr_ *n_l_instr; struct n_var_ *n_var; struct n_appel_ *n_appel; struct n_l_exp_ *n_l_exp; }

%type <n_prog> programme
%type <n_l_dec> LDecVarOpt
%type <n_l_dec> LDecVar

%type <n_l_dec> LDecVarBis
%type <n_dec> DecVar

%type <sval> Type
%type <n_l_dec> LDecFct
%type <n_dec> DecFct
%type <n_l_dec> LDecArgOpt
%type <n_instr> Ibloc
%type <n_l_instr> LInstr
%type <n_instr> Instr
%type <n_instr> IAffect
%type <n_instr> Isi
%type <n_instr> Itantque
%type <n_instr> IAppel
%type <n_instr> IRetour
%type <n_instr> SinonOpt
%type <n_appel> AppelFct
%type <n_l_exp> LExp
%type <n_l_exp> LExpBis
%type <n_exp> Exp
%type <n_var> Var

%token <ival> NOMBRE
%token <sval> IDENTIF
%token <sval> ENTIER
%token <sval> ALORS
%token <sval> SI
%token <sval> SINON
%token <sval> FAIRE
%token <sval> TANTQUE
%token <sval> RETOUR
%token <op> OU
%token <op> ET
%token <sval> LIRE
%token <sval> ECRIRE
%token <op> '!'
%token <sval> ';'
%token <op> '+'
%token <op> '-'
%token <op> '*'
%token <op> '/'
%token <sval> '('
%token <sval> ')'
%token <sval> '['
%token <sval> ']'
%token <sval> '{'
%token <sval> '}'
%token <op> '='
%token <op> '<'
%token <sval> ','

%left OU
%left ET
%left '='
%left '<'
%left '+' '-'
%left '*' '/'
%left '!'
%nonassoc MOINSU

%start programme
%%

programme : LDecVarOpt LDecFct {$$ = cree_n_prog($1, $2); n=$$;};
LDecVarOpt : LDecVar ';'
	|	 {$$ = NULL;}
	;
LDecVar : DecVar LDecVarBis {$$ = cree_n_l_dec($1, $2);};
LDecVarBis : ',' DecVar LDecVarBis {$$ = cree_n_l_dec($2, $3);}
	|	 {$$ = NULL;}
	;
DecVar : Type IDENTIF {$$ = cree_n_dec_var($2);}
	| 	 Type IDENTIF '[' NOMBRE ']' {$$ = cree_n_dec_tab($2, $4);}
	;

Type : ENTIER;

LDecFct : DecFct {$$ = cree_n_l_dec($1, NULL);}
	|  	  DecFct LDecFct {$$ = cree_n_l_dec($1, $2);}
	;
DecFct : IDENTIF'(' LDecArgOpt ')' LDecVarOpt Ibloc {$$ = cree_n_dec_fonc($1, $3, $5, $6);};
LDecArgOpt : LDecVar
	| 	{$$ = NULL;}
	;
Ibloc : '{' LInstr '}' {$$ = cree_n_instr_bloc($2);};
LInstr : Instr LInstr {$$ = cree_n_l_instr($1, $2);}
	| 	{$$ = NULL;}
	;
Instr : IAffect
	|   Isi
	|   Itantque
	|   IAppel
	|   IRetour
	|		ECRIRE '(' Exp ')'';' {$$ = cree_n_instr_ecrire($3);}
	|	';' {$$ = cree_n_instr_vide();}
	;
IAffect : Var '=' Exp ';' {$$ = cree_n_instr_affect($1, $3);};
Isi : SI Exp ALORS Ibloc SinonOpt {$$ = cree_n_instr_si($2, $4, $5);};
SinonOpt : SINON Ibloc {$$ = $2;}
	| {$$ = NULL;}
	;
Itantque : TANTQUE Exp FAIRE Ibloc {$$ = cree_n_instr_tantque($2, $4);};
IAppel : AppelFct ';' {$$ = cree_n_instr_appel($1);} ;
IRetour : RETOUR Exp ';' {$$ = cree_n_instr_retour($2);} ;
AppelFct : IDENTIF '(' LExp ')' {$$ = cree_n_appel($1, $3);};
LExp : Exp LExpBis {$$ = cree_n_l_exp($1, $2);}
	| {$$ = NULL;}
	;
LExpBis : ',' Exp LExpBis {$$ = cree_n_l_exp($2, $3);}
	| {$$ = NULL;}
	;

Exp : Exp OU Exp {$$ = cree_n_exp_op(ou, $1, $3);}
	| Exp ET Exp {$$ = cree_n_exp_op(et, $1, $3);}
	| Exp '=' Exp {$$ = cree_n_exp_op(egal, $1, $3);}
	| Exp '<' Exp {$$ = cree_n_exp_op(inferieur, $1, $3);}
	| Exp '+' Exp {$$ = cree_n_exp_op(plus, $1, $3);}
	| Exp '-' Exp {$$ = cree_n_exp_op(moins, $1, $3);}
	| Exp '*' Exp {$$ = cree_n_exp_op(fois, $1, $3);}
	| Exp '/' Exp {$$ = cree_n_exp_op(divise, $1, $3);}
	| '!' Exp {$$ = cree_n_exp_op(non, $2, NULL);}
	| '(' Exp ')' {$$ = $2;}
	| NOMBRE {$$ = cree_n_exp_entier($1);}
	| '-' Exp %prec MOINSU {$$ = cree_n_exp_op(moins,$2, NULL);}
	| AppelFct {$$ = cree_n_exp_appel($1);}
	| LIRE '(' ')' {$$ = cree_n_exp_lire();}
	| Var {$$ = cree_n_exp_var($1);}
	;

Var : IDENTIF {$$ = cree_n_var_simple($1);}
	| IDENTIF '[' Exp ']' {$$ = cree_n_var_indicee($1, $3);}
	;

%%

int yyerror(char *s) {
  fprintf(stderr, "erreur de syntaxe ligne %d\n", yylineno);
  fprintf(stderr, "%s\n", s);
  fclose(yyin);
  exit(1);
}
