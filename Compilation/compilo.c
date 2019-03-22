#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analyseur_lexical_flex.h"
#include "analyseur_syntaxique.tab.h"
#include "affiche_arbre_abstrait.h"
#include "syntabs.h"
#include "tabsymboles.h"
#include "parcours_arbre_abstrait.h"

FILE *yyin;
extern char *yytext;   // déclaré dans analyseur_lexical
extern n_prog *n;

/***********************************************************************
 * Fonction auxiliaire appelée par le compilo en mode -l pour tester
 * l'analyseur lexical et, étant donné un programme en entrée, afficher
 * la liste des tokens.
 ******************************************************************************/

void test_yylex(FILE *yyin) {
  int uniteCourante;
  char nom[100], valeur[100];
  while( ( uniteCourante = yylex() ) != 0 ) {
    nom_token( uniteCourante, nom, valeur );
    printf("%s\t%s\t%s\n", yytext, nom, valeur);
  }
}

/**********************************************************************/

void affiche_message_aide(char *nom_prog) {
  fprintf(stderr, "usage: %s OPT fichier_source\n", nom_prog);
  fprintf(stderr, "\t-l affiche les tokens de l'analyse lexicale\n");
  fprintf(stderr, "\t-s affiche l'arbre de derivation\n");
  fprintf(stderr, "\t-a affiche l'arbre abstrait\n");
  fprintf(stderr, "\t-t affiche la table des symboles\n");
  fprintf(stderr, "\t-3 affiche le code trois adresses\n");
  fprintf(stderr, "\t-n affiche le code nasm (actif par defaut)\n");
  fprintf(stderr, "\t-h affiche ce message\n");
  exit(1);
}

/**********************************************************************/

int main(int argc, char **argv) {
  int i;
  int affiche_lex = 0;
  int affiche_syntaxe_abstraite = 0;
  int affiche_code3a = 0;
  int affiche_mips = 0;
  int affiche_nasm = 0;
  int affiche_tabsymb = 0;
  int affiche_syntaxe = 0;
	char *fichier_source = NULL;

  if(argc == 1){
    affiche_message_aide(argv[0]);
  }

  for(i=1; i<argc; i++){
    if(!strcmp(argv[i], "-l")) {
       affiche_lex = 1;
    }
    else if(!strcmp(argv[i], "-s")) {
       affiche_syntaxe = 1;
    }
    else if(!strcmp(argv[i], "-a")) {
       affiche_syntaxe_abstraite = 1;
    }
    else if(!strcmp(argv[i], "-3")) {
       affiche_code3a = 1;
    }
    else if(!strcmp(argv[i], "-n")) {
       affiche_nasm = 1;
    }
    else if(!strcmp(argv[i], "-t")) {
       affiche_tabsymb = 1;
    }
    else if(!strcmp(argv[i], "-h")){
       affiche_message_aide(argv[0]);
    }
    else {
	fichier_source = strdup(argv[i]);
	}
  }

  if( !( affiche_lex || affiche_syntaxe_abstraite || affiche_code3a ||
      affiche_tabsymb || affiche_mips ) ) {
    affiche_nasm = 1; /* Par défaut, affiche code cible NASM */
  }

  if(affiche_lex == 1) {
      yyin = fopen(fichier_source, "r");
      if(yyin == NULL){
        fprintf(stderr, "impossible d'ouvrir le fichier %s\n", argv[i]);
        exit(1);
      }

    test_yylex( yyin );

  }
  if(affiche_syntaxe == 1) {
    yyin = fopen(fichier_source, "r");
    if(yyin == NULL){
      fprintf(stderr, "impossible d'ouvrir le fichier %s\n", argv[i]);
      exit(1);
    }
	  yyparse();
	  printf("Analyse réussie\n");
  }
  if( affiche_syntaxe_abstraite ) {
      yyin = fopen(fichier_source, "r");
      if(yyin == NULL){
        fprintf(stderr, "impossible d'ouvrir le fichier %s\n", argv[i]);
        exit(1);
      }
    yyparse();
	  affiche_n_prog(n);
  }
  if(affiche_code3a){
  	yyin = fopen(fichier_source, "r");
      if(yyin == NULL){
        fprintf(stderr, "impossible d'ouvrir le fichier %s\n", argv[i]);
        exit(1);
      }
    yyparse();
    if(!affiche_tabsymb)
      fprintf(stderr, "le code 3a necessite -t\n");
    parcours_n_prog(n);
    code3a_affiche_code();
  }
  if(affiche_tabsymb){
    yyin = fopen(fichier_source, "r");
    if(yyin == NULL){
      fprintf(stderr, "impossible d'ouvrir le fichier %s\n", argv[i]);
      exit(1);
    }
    yyparse();
	  parcours_n_prog(n);
  }
  if(affiche_nasm){
    //Affiche code cible NASM
  }
  return 0;
}
