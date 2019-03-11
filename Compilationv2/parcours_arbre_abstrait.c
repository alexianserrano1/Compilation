#include <stdio.h>
#include "syntabs.h"
#include "util.h"
#include "tabsymboles.h"

void parcours_n_prog(n_prog *n);
void parcours_l_instr(n_l_instr *n);
void parcours_instr(n_instr *n);
void parcours_instr_si(n_instr *n);
void parcours_instr_tantque(n_instr *n);
void parcours_instr_affect(n_instr *n);
void parcours_instr_appel(n_instr *n);
void parcours_instr_retour(n_instr *n);
void parcours_instr_ecrire(n_instr *n);
void parcours_l_exp(n_l_exp *n);
void parcours_exp(n_exp *n);
void parcours_varExp(n_exp *n);
void parcours_opExp(n_exp *n);
void parcours_intExp(n_exp *n);
void parcours_lireExp(n_exp *n);
void parcours_appelExp(n_exp *n);
void parcours_l_dec(n_l_dec *n);
void parcours_dec(n_dec *n);
void parcours_foncDec(n_dec *n);
void parcours_varDec(n_dec *n);
void parcours_tabDec(n_dec *n);
void parcours_var(n_var *n);
void parcours_var_simple(n_var *n);
void parcours_var_indicee(n_var *n);
void parcours_appel(n_appel *n);

int longueur_liste(n_l_dec *l_dec);
int longueur_args(n_l_exp *l_args);

//int trace_abs = 1;

extern int portee;
extern int adresseLocaleCourante;
extern int adresseArgumentCourant;
int adresseGlobaleCourante = 0;

/*-------------------------------------------------------------------------*/

void parcours_n_prog(n_prog *n)
{
  //char *fct = "prog";

  portee = P_VARIABLE_GLOBALE;
  parcours_l_dec(n->variables);

  parcours_l_dec(n->fonctions); 
}

/*-------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/

void parcours_l_instr(n_l_instr *n)
{
  //char *fct = "l_instr";
  if(n){
  parcours_instr(n->tete);
  parcours_l_instr(n->queue);
  }
}

/*-------------------------------------------------------------------------*/

void parcours_instr(n_instr *n)
{
  if(n){
    if(n->type == blocInst) parcours_l_instr(n->u.liste);
    else if(n->type == affecteInst) parcours_instr_affect(n);
    else if(n->type == siInst) parcours_instr_si(n);
    else if(n->type == tantqueInst) parcours_instr_tantque(n);
    else if(n->type == appelInst) parcours_instr_appel(n);
    else if(n->type == retourInst) parcours_instr_retour(n);
    else if(n->type == ecrireInst) parcours_instr_ecrire(n);
  }
}

/*-------------------------------------------------------------------------*/

void parcours_instr_si(n_instr *n)
{  
  //char *fct = "instr_si";

  parcours_exp(n->u.si_.test);
  parcours_instr(n->u.si_.alors);
  if(n->u.si_.sinon){
    parcours_instr(n->u.si_.sinon);
  }
 
}

/*-------------------------------------------------------------------------*/

void parcours_instr_tantque(n_instr *n)
{
  //char *fct = "instr_tantque";

  parcours_exp(n->u.tantque_.test);
  parcours_instr(n->u.tantque_.faire);
}

/*-------------------------------------------------------------------------*/

void parcours_instr_affect(n_instr *n)    /********************/
{
  //char *fct = "instr_affect";

  int indice_identif;
  if((indice_identif = rechercheExecutable(n->u.affecte_.var->nom)) == -1) {
    erreur("La variable doit être déclarée avant de lui affecter une valeur");
  }

  if(n->u.affecte_.var->type == simple){
    if(tabsymboles.tab[indice_identif].type == T_TABLEAU_ENTIER)
      erreur("Une variable simple ne doit pas avoir d'indice");
  }
  else {
    if(tabsymboles.tab[indice_identif].type == T_ENTIER)
      erreur("Un variable de type tableau doit avoir un indice");
  }

  parcours_var(n->u.affecte_.var);
  parcours_exp(n->u.affecte_.exp);
}

/*-------------------------------------------------------------------------*/

void parcours_instr_appel(n_instr *n)
{
  //char *fct = "instr_appel";


  parcours_appel(n->u.appel);
}
/*-------------------------------------------------------------------------*/

void parcours_appel(n_appel *n)   /*********************/
{
  //char *fct = "appel";

  int indice_fct;
  if((indice_fct = rechercheExecutable(n->fonction)) == -1) {
    erreur("Une fonction doit être déclarée avant de l'appeler");
  }

  int nb_args = longueur_args(n->args);
  if(nb_args != tabsymboles.tab[indice_fct].complement) {
    erreur("Mauvais nombre d'arguments pour la fonction");
  }
  
  if((indice_fct = rechercheExecutable("main")) == -1) {
	  erreur("Main inexistant");

    if(tabsymboles.tab[indice_fct].complement !=  0) {
		erreur("Le main doit avoir aucun args");
	  }
  }
  
  //parcours_xml_texte( n->fonction, trace_abs);
  parcours_l_exp(n->args);
  
}

/*-------------------------------------------------------------------------*/

void parcours_instr_retour(n_instr *n)
{
  //char *fct = "instr_retour";
  
  parcours_exp(n->u.retour_.expression);

}

/*-------------------------------------------------------------------------*/

void parcours_instr_ecrire(n_instr *n)
{
  //char *fct = "instr_ecrire";
  
  parcours_exp(n->u.ecrire_.expression);
 
}

/*-------------------------------------------------------------------------*/

void parcours_l_exp(n_l_exp *n)
{
  //char *fct = "l_exp";
 

  if(n){
    parcours_exp(n->tete);
    parcours_l_exp(n->queue);
  }
 
}

/*-------------------------------------------------------------------------*/

void parcours_exp(n_exp *n)
{

  if(n->type == varExp) parcours_varExp(n);
  else if(n->type == opExp) parcours_opExp(n);
  else if(n->type == intExp) parcours_intExp(n);
  else if(n->type == appelExp) parcours_appelExp(n);
  else if(n->type == lireExp) parcours_lireExp(n);
}

/*-------------------------------------------------------------------------*/

void parcours_varExp(n_exp *n)  /****************/
{
  //char *fct = "varExp";

  int indice_var;
  if((indice_var = rechercheExecutable(n->u.var->nom)) == -1) {
    erreur("Une variable doit être déclarée avant utilisation");
  }

  if(n->u.var->type == simple){
    if(tabsymboles.tab[indice_var].type == T_TABLEAU_ENTIER)
      erreur("Une variable simple ne doit pas avoir d'indice");
  }
  else {
    if(tabsymboles.tab[indice_var].type == T_ENTIER)
      erreur("Un variable de type tableau doit avoir un indice");
  }

  parcours_var(n->u.var);
}

/*-------------------------------------------------------------------------*/
void parcours_opExp(n_exp *n)
{
  //char *fct = "opExp";
 
  /*if(n->u.opExp_.op == plus) parcours_xml_texte("plus", trace_abs);
  else if(n->u.opExp_.op == moins) parcours_xml_texte("moins", trace_abs);
  else if(n->u.opExp_.op == fois) parcours_xml_texte("fois", trace_abs);
  else if(n->u.opExp_.op == divise) parcours_xml_texte("divise", trace_abs);
  else if(n->u.opExp_.op == egal) parcours_xml_texte("egal", trace_abs);
  else if(n->u.opExp_.op == inferieur) parcours_xml_texte("inf", trace_abs);
  else if(n->u.opExp_.op == ou) parcours_xml_texte("ou", trace_abs);
  else if(n->u.opExp_.op == et) parcours_xml_texte("et", trace_abs);
  else if(n->u.opExp_.op == non) parcours_xml_texte("non", trace_abs);  */
  if( n->u.opExp_.op1 != NULL ) {
    parcours_exp(n->u.opExp_.op1);
  }
  if( n->u.opExp_.op2 != NULL ) {
    parcours_exp(n->u.opExp_.op2);
  }
 
}

/*-------------------------------------------------------------------------*/

void parcours_intExp(n_exp *n)
{
  char texte[ 50 ]; // Max. 50 chiffres
  sprintf(texte, "%d", n->u.entier);
  //parcours_element( "intExp", texte, trace_abs );
}

/*-------------------------------------------------------------------------*/
void parcours_lireExp(n_exp *n)
{
  //char *fct = "lireExp";

}

/*-------------------------------------------------------------------------*/

void parcours_appelExp(n_exp *n)
{
  //char *fct = "appelExp";
  parcours_appel(n->u.appel);
}

/*-------------------------------------------------------------------------*/

void parcours_l_dec(n_l_dec *n)
{
  //char *fct = "l_dec";

  if( n ){
    
    parcours_dec(n->tete);
    parcours_l_dec(n->queue);
  }
}

/*-------------------------------------------------------------------------*/

void parcours_dec(n_dec *n)
{

  if(n){
    if(n->type == foncDec) {
      parcours_foncDec(n);
    }
    else if(n->type == varDec) {
      parcours_varDec(n);
    }
    else if(n->type == tabDec) { 
      parcours_tabDec(n);
    }
  }
}

/*-------------------------------------------------------------------------*/

void parcours_foncDec(n_dec *n)  /*********************/
{
  //char *fct = "foncDec";
  int complement = longueur_liste(n->u.foncDec_.param);

  entreeFonction();
  ajouteIdentificateur(n->nom, P_VARIABLE_GLOBALE, T_FONCTION, 0, complement);
  //  parcours_xml_texte( n->nom, trace_abs );
  parcours_l_dec(n->u.foncDec_.param);
  portee = P_VARIABLE_LOCALE;
  parcours_l_dec(n->u.foncDec_.variables);
  parcours_instr(n->u.foncDec_.corps);

  sortieFonction(1);
}

/*-------------------------------------------------------------------------*/

void parcours_varDec(n_dec *n)   /**********************/
{
  int address;

  if(rechercheDeclarative(n->nom) == -1) {
    int complement = 1;

    switch(portee) {
      case P_VARIABLE_GLOBALE : address = adresseGlobaleCourante;
                                printf("adressGlobaleCourante = %d\n", address);
                                ajouteIdentificateur(n->nom, portee, T_ENTIER, address, complement);
                                adresseGlobaleCourante += 4;
                                break;
      case P_VARIABLE_LOCALE : address = adresseLocaleCourante;
                               ajouteIdentificateur(n->nom, portee, T_ENTIER, address, complement);
                               adresseLocaleCourante += 4;
                               break;
      case P_ARGUMENT : address = adresseArgumentCourant; 
                                  ajouteIdentificateur(n->nom, portee, T_ENTIER, address, complement);
                                  adresseArgumentCourant += 4;
                                  break;
    }
  }
  else {
	  erreur("Variable simple déjà déclarée");
  }
  //parcours_element("varDec", n->nom, trace_abs);
}

/*-------------------------------------------------------------------------*/

void parcours_tabDec(n_dec *n)     /*******************/
{
  char texte[100]; // Max. 100 chars nom tab + taille
  
  if(rechercheDeclarative(n->nom) == -1) {
	int address;
	int complement = n->u.tabDec_.taille;
	
	switch(portee) {
      case P_VARIABLE_GLOBALE : address = adresseGlobaleCourante;
                                ajouteIdentificateur(n->nom, portee, T_TABLEAU_ENTIER, address, complement);
                                adresseGlobaleCourante += 4*complement;
                                break;
      case P_VARIABLE_LOCALE : address = adresseLocaleCourante;
                               ajouteIdentificateur(n->nom, portee, T_TABLEAU_ENTIER, address, complement);
                               adresseLocaleCourante += 4*complement;
                               break;
      case P_ARGUMENT : address = adresseArgumentCourant; 
                                  ajouteIdentificateur(n->nom, portee, T_TABLEAU_ENTIER, address, complement);
                                  adresseArgumentCourant += 4*complement;
                                  break;
    }
  }
  else 
	erreur("Varaible tableau déjà déclarée");
  
  sprintf(texte, "%s[%d]", n->nom, n->u.tabDec_.taille);
  //parcours_element( "tabDec", texte, trace_abs );
}

/*-------------------------------------------------------------------------*/

void parcours_var(n_var *n)
{
  if(n->type == simple) {
    parcours_var_simple(n);
  }
  else if(n->type == indicee) {
    parcours_var_indicee(n);
  }
}

/*-------------------------------------------------------------------------*/
void parcours_var_simple(n_var *n)  /*********************/
{
  int indice_var;
  if((indice_var = rechercheExecutable(n->nom)) == -1) {
	  erreur("Une variable simple doit être déclarée");
  }
  
  if(tabsymboles.tab[indice_var].type == T_TABLEAU_ENTIER) {
	erreur("Une variable simple ne doit pas être utilisé avec un indice");
  }
  //parcours_element("var_simple", n->nom, trace_abs);
}

/*-------------------------------------------------------------------------*/
void parcours_var_indicee(n_var *n)   /********************/
{
  //char *fct = "var_indicee";
  
  int indice_var;
  if((indice_var = rechercheExecutable(n->nom)) == -1) {
	  erreur("Une variable tableau doit être déclarée avant utilisation");
  }

	if(tabsymboles.tab[indice_var].type == T_ENTIER) {
		erreur("Une variable tableau doit être utilisée avec un indice");
	}

  //parcours_element("var_base_tableau", n->nom, trace_abs);
  parcours_exp( n->u.indicee_.indice );

}

/*-------------------------------------------------------------------------*/
int longueur_liste(n_l_dec *l_dec) 
{
  if(l_dec == NULL)
    return 0;

  return longueur_liste(l_dec->queue) + 1;
}

int longueur_args(n_l_exp *l_args) {
  if(l_args == NULL)
    return 0;
  
  return longueur_args(l_args->queue) + 1;
}
