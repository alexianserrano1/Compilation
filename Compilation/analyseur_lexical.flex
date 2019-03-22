/*
 * Analyseur lexical du compilateur L en FLEX
 */
%{
/* code copié AU DÉBUT de l'analyseur */

#include "analyseur_syntaxique.tab.h"
%}
%option yylineno
%option nounput
%option noinput

/* Déclarations à compléter ... */
lettre [A-Za-z_$]
chiffre [0-9]
alphanum {lettre}|{chiffre}


%%

\;                      {return ';' ;}
\+                      {return '+' ;}
\-                      {return '-' ;}
\*                      {return '*' ;}
\/                      {return '/' ;}
\(                      {return '(' ;}
\)                      {return ')' ;}
\[                      {return '[' ;}
\]                      {return ']' ;}
\{                      {return '{' ;}
\}                      {return '}' ;}
\=                      {return '=' ;}
\<                      {return '<' ;}
\&                      {return ET ;}
\|                      {return OU ;}
\!                      {return '!' ;}
"lire"                  {return LIRE;}
"ecrire"                {return ECRIRE;}
"si"                    {return SI;}
"alors"                 {return ALORS;}
"sinon"                 {return SINON;}
"tantque"               {return TANTQUE;}
"faire"                 {return FAIRE;}
"entier"                {return ENTIER;}
"retour"                {return RETOUR;}
{lettre}{alphanum}*     {yylval.sval = strdup(yytext); return IDENTIF;}
[0-9]+|[0-9]*\.[0-9]+ {yylval.ival=atoi(yytext);return NOMBRE;}

\,                      {return ',' ;}
[ \n\t]                 {}
. 						{return yytext[0];}


%%

/* Code copié À LA FIN de l'analyseyur */

int yywrap(){
  return 1;
}

/***********************************************************************
 * Fonction auxiliaire appelée par l'analyseur syntaxique pour
 * afficher des messages d'erreur et l'arbre XML
 **********************************************************************/

char *tableMotsClefs[] = {"si", "alors", "sinon", "tantque", "faire", "entier", "retour", "lire", "ecrire"};
int codeMotClefs[] = {SI, ALORS, SINON, TANTQUE, FAIRE, ENTIER, RETOUR, LIRE, ECRIRE};
int nbMotsClefs = 9;

void nom_token( int token, char *nom, char *valeur ) {
  int i;
  strcpy( nom, "symbole" );
  if(token == ';') strcpy( valeur, "POINT_VIRGULE");
  else if(token == '+') strcpy(valeur, "PLUS");
  else if(token == '-') strcpy(valeur, "MOINS");
  else if(token == '*') strcpy(valeur, "FOIS");
  else if(token == '/') strcpy(valeur, "DIVISE");
  else if(token == '(') strcpy(valeur, "PARENTHESE_OUVRANTE");
  else if(token == ')') strcpy(valeur, "PARENTHESE_FERMANTE");
  else if(token == '[') strcpy(valeur, "CROCHET_OUVRANT");
  else if(token == ']') strcpy(valeur, "CROCHET_FERMANT");
  else if(token == '{') strcpy(valeur, "ACCOLADE_OUVRANTE");
  else if(token == '}') strcpy(valeur, "ACCOLADE_FERMANTE");
  else if(token == '=') strcpy(valeur, "EGAL");
  else if(token == '<') strcpy(valeur, "INFERIEUR");
  else if(token == ET) strcpy(valeur, "ET");
  else if(token == OU) strcpy(valeur, "OU");
  else if(token == '!') strcpy(valeur, "NON");
  else if(token == ',') strcpy(valeur, "VIRGULE");
  else if( token == IDENTIF ) {
    strcpy( nom, "identificateur" );
    strcpy( valeur, yytext );
  }
  else if( token == NOMBRE ) {
    strcpy( nom, "nombre" );
    strcpy( valeur, yytext );
  }
  else {
    strcpy(nom, "mot_clef");
    for(i = 0; i < nbMotsClefs; i++){
      if( token ==  codeMotClefs[i] ){
        strcpy( valeur, tableMotsClefs[i] );
        break;
      }
    }
  }
}
