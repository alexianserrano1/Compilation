#include "code3a.h"
#include "tabsymboles.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "util.h"

#define printverb(format) if(nasm_verbose){ printf(format); }
#define printverba(format, ...) if(nasm_verbose){ printf(format, __VA_ARGS__); }

#define NB_REGISTRES 4

#define REG_NONE 0
#define REG_EBX 1
#define REG_ECX 2
#define REG_EDX 3 // last registers used -> needed for operations (e.g. idiv)
#define REG_EAX 4 // last registers used -> needed for operations (e.g. idiv)

void _nasm_instr(char *opcode, char *op1, char *op2, char *op3, char *comment);
void _nasm_instr_relative(char *opcode,  char *op1, char *op2, int offset, char *comment);
void _nasm_etiquette(char* etiquette);

// tableau global avec code à 3 adresses
extern code3a_ code3a;
//extern operande **desctemp;

int varlocs = 0;
int arguments = 0;
int i_ligne; // ligne courante du code
operande *desc_reg[NB_REGISTRES+1] = {NULL,NULL,NULL,NULL,NULL}; //1st unused
char *nomreg[NB_REGISTRES+1] = {NULL,"ebx","ecx","edx","eax"}; //1st unused
int nbparam = 0;
int nasm_verbose = 0;

/******************************************************************************/

int _is_reg_free(int regnum){
  return  desc_reg[regnum] == NULL ||
          (desc_reg[regnum]->oper_type == O_TEMPORAIRE && // temp dans reg et
          desc_reg[regnum]->u.oper_temp.last_use < i_ligne); // temp obsolète
}

/******************************************************************************/

void _affiche_desc_registres(){
  int regnum;
  printverba(";            => Line = %d, Registres: [", i_ligne);
  for(regnum = REG_EBX; regnum <= REG_EAX; regnum++){
    if(desc_reg[regnum]==NULL){
      printverb("free, ");
    }
    else{
      printverba("t%d{l=%d",(desc_reg[regnum])->u.oper_temp.oper_tempnum,(desc_reg[regnum])->u.oper_temp.last_use);
      if(_is_reg_free(regnum)){ // temp obsolète
        printverb(",dead}, ");
      }
      else{
        printverb("}, ");
      }
    }
  }
  printverb("]\n");
}

/******************************************************************************/

// ALLOCATION DE REGISTRES SIMPLE. oper est forcément un temporaire
int new_registre(operande *oper){
  int regnum;
  printverb(";            Before alloc reg:\n");
  _affiche_desc_registres();
  for(regnum = REG_EBX; regnum <= REG_EAX; regnum++){
    if( _is_reg_free(regnum) ) {
      desc_reg[regnum] = oper;
      printverb(";            After alloc reg:\n");
      _affiche_desc_registres(); //FIXME
      return regnum;
    }
  }
  // TODO FIXME!
  erreur("Pas de registre disponible");
  return REG_NONE;
}

/******************************************************************************/

/* Cas "faciles" :
   * constante renvoyée sous forme de chaîne de caractères ou
   * variable renvoyée sous forme de décalage par rapport à étiquette / registre
*/
char *varconst2nasm(operande *oper){
  if(oper->oper_type != O_VARIABLE && oper->oper_type != O_CONSTANTE){
    erreur("Opérande résultat d'une affectation n'est pas une variable");
  }
  char *result = malloc(sizeof(char)*150);
  if(oper->oper_type == O_CONSTANTE){
    sprintf(result,"%d",oper->u.oper_valeur);
  }
  else if(oper->oper_type == O_VARIABLE){

    
    /* Code que j'ai ajouté (Alexian SERRANO) pour palier au problème des indices dans l'affectation sur les tableaux */
    /******************************************************************************************************************/
    if(oper->u.oper_var.oper_indice != NULL) {
      switch(oper->u.oper_var.oper_indice->oper_type) {
        case O_CONSTANTE : sprintf(result,"dword [%s+%d]", oper->u.oper_var.oper_nom, oper->u.oper_var.oper_indice->u.oper_valeur*4);
                           break;
        /*case O_ETIQUETTE : sprintf(result,"dword [%s+%s]", oper->u.oper_var.oper_nom, oper->u.oper_var.oper_indice->u.oper_nom);
                           break;
        case O_TEMPORAIRE : sprintf(result,"dword [%s+%d]", oper->u.oper_var.oper_nom, oper->u.oper_var.oper_indice->u.oper_valeur*4);
                            break;
        case O_VARIABLE : sprintf(result,"dword [%s+%d]", oper->u.oper_var.oper_nom, oper->u.oper_var.oper_indice->u.oper_valeur*4);
                          break; */
        default : sprintf(result,"dword [%s]", oper->u.oper_var.oper_nom);
                  break;
      }
      /*****************************************************************************************************************/
      /* Fin du code ajouté */


    }
    else if(oper->u.oper_var.oper_portee == P_VARIABLE_GLOBALE) {
      sprintf(result,"dword [%s]", oper->u.oper_var.oper_nom);
    }
    else{
      int adresse = oper->u.oper_var.oper_adresse;
      if (oper->u.oper_var.oper_portee == P_VARIABLE_LOCALE){
        sprintf(result,"dword [ebp - %d]", 4 + adresse );
      }
      else{ // P_ARGUMENT
        sprintf(result,"dword [ebp + %d]", 4 + 4 * (arguments) - adresse);
      }
    }
  }
  return result;
}

/******************************************************************************/

int oper2reg(operande *oper, int regnum){ // regnum can be REG_NONE
  int result;
  if(regnum != REG_NONE && !_is_reg_free(regnum)){
    warning_1s("; register %s not free!", nomreg[regnum]);
  }
  if(oper->oper_type == O_CONSTANTE || oper->oper_type == O_VARIABLE){
    operande *constvartemp = code3a_new_temporaire();
    constvartemp->u.oper_temp.last_use = i_ligne;
    if(regnum == REG_NONE){ result = new_registre(constvartemp); }
    else{ result = regnum; }
    _nasm_instr("mov", nomreg[result],
                      varconst2nasm(oper), NULL, NULL);
    return result;
  }
  else{ // oper is a temporary
    int tempreg = oper->u.oper_temp.emplacement;
    if(tempreg == REG_NONE ){
      erreur("Temporaire non initialisé !");
      return REG_NONE; // remove warning
    }
    // FIXME beaks if register constant values change
    else if(tempreg > REG_NONE){
      if(regnum != REG_NONE && tempreg != regnum){
        _nasm_instr("mov", nomreg[regnum], nomreg[tempreg], NULL, "new reg for t");
        desc_reg[regnum]=desc_reg[tempreg];
        desc_reg[tempreg]=NULL;
        oper->u.oper_temp.emplacement = regnum;
      }
      return tempreg;
    }
    else{
      erreur("TODO: récupérer temp de la pile");
      return REG_NONE; // remove warning
    }
  }
}

/******************************************************************************/

void c3a2nasm_division(operande *oper1, operande *oper2, operande *result){
  //int restoreeax = 0;
  int restoreedx = 0;
  char *oper2string;
   if(!_is_reg_free(REG_EAX)){
     erreur("TODO: free eax before division");
   }
  //   restoreeax = 1;
  //   _nasm_instr("push", "eax", NULL,NULL,NULL,"push eax avant division");
  // }
  if(!_is_reg_free(REG_EDX)){
    restoreedx = 1;
    _nasm_instr("push", "edx", NULL, NULL,"push eax avant division");
  }
  oper2reg(oper1, REG_EAX);
  _nasm_instr("mov", "edx", "0", NULL, "oper1 idiv -> eax");
  if(oper2->oper_type == O_CONSTANTE){
    oper2string = nomreg[oper2reg(oper2, REG_NONE)];
  }
  else if (oper2->oper_type == O_TEMPORAIRE) {
    oper2string = nomreg[oper2->u.oper_temp.emplacement];
  }
  else { // O_VARIABLE
    oper2string = varconst2nasm(oper2);
  }
  _nasm_instr("idiv", oper2string, NULL, NULL, "effectue division");
  result->u.oper_temp.emplacement = REG_EAX;
  if(restoreedx){
    _nasm_instr("pop", "edx", NULL, NULL, "restore edx");
  }
}

/******************************************************************************/

void c3a2nasm_arith(char *opcode, operande *oper1, operande *oper2, operande *result){
  // result is always temp in arith operation (by construction of c3a)
  //TODO optimisation - echanger oper1 et oper2 pour add et imul
  //TODO optimisation - calculer const op const directement
  printverb(";            Translating ");
  if(nasm_verbose){
    code3a_affiche_ligne_code(&(code3a.liste[i_ligne]));
  }
  printverb("\n");
  int oper1reg = oper2reg(oper1, REG_NONE);
  char *oper2string;
  if(oper2->oper_type == O_TEMPORAIRE){
    oper2string = nomreg[oper2reg(oper2, REG_NONE)]; //temp in reg
  }
  else{
    oper2string = varconst2nasm(oper2);
  }
  _nasm_instr(opcode, nomreg[oper1reg], oper2string, NULL, NULL);
  desc_reg[oper1reg] = result;
  printverb(";            After oper arith:\n");
  _affiche_desc_registres(); //FIXME
  result->u.oper_temp.emplacement = oper1reg;
}

/******************************************************************************/

void c3a2nasm_affect(operande *result, operande *oper1){
  char *oper1char, *resultchar;
  if(result->oper_type == O_VARIABLE){
    resultchar = varconst2nasm(result);
  }
  else{ // result == O_TEMPORAIRE
    if(result->u.oper_temp.emplacement == REG_NONE) {
      result->u.oper_temp.emplacement = new_registre(result);
    }
    resultchar = nomreg[result->u.oper_temp.emplacement];
  }
  if(result->oper_type == O_VARIABLE && oper1->oper_type == O_VARIABLE){
    operande *tempvar = code3a_new_temporaire();
    tempvar->u.oper_temp.last_use = i_ligne;
    oper1char = nomreg[new_registre(tempvar)];
    _nasm_instr("mov", oper1char, varconst2nasm(oper1),
                             NULL, "affectation - stocke valeur");
  }
  else if (oper1->oper_type == O_CONSTANTE){
    oper1char = varconst2nasm(oper1);
  }
  else{
    oper1char = nomreg[oper2reg(oper1, REG_NONE)];
  }
  _nasm_instr("mov", resultchar, oper1char, NULL, "affectation stocke valeur");
}

/******************************************************************************/

void c3a2nasm_finfonction(int varlocs){
  if(varlocs){
    printf("\tadd\tesp, %d\t; desallocation variables locales\n", varlocs);
  }
  _nasm_instr("pop", "ebp", NULL, NULL, "restaure la valeur de ebp") ;
  _nasm_instr("ret", NULL, NULL, NULL, NULL);
}

/******************************************************************************/

void c3a2nasm_allouer(operande *var){
  if(!var) { // valeur de retour d'une fonction
    _nasm_instr("sub", "esp", "4", NULL, "allocation valeur de retour");
  }
  else{ // variable locale, 4 octets (dword)
    printf("\tsub\tesp, %d\t; allocation variable locale %s\n", 4, var->u.oper_var.oper_nom);
    varlocs += 4;
  }
}

/******************************************************************************/

void c3a2nasm_debutfonction(char *nomfonction){
  varlocs = 0;
  arguments = tabsymboles.tab[rechercheExecutable(nomfonction)].complement;
  _nasm_instr("push", "ebp", NULL, NULL, "sauvegarde la valeur de ebp") ;
  _nasm_instr("mov", "ebp", "esp", NULL, "nouvelle valeur de ebp");
}


/******************************************************************************/

void c3a2nasm_write(operande *oper){
  //pas besoin de sauvegarder eax. "ecrire" est une instruction, donc on est sûr
  //qu'on n'est pas en plein milieu d'une évaluation d'expression
  if(oper->oper_type == O_TEMPORAIRE && oper->u.oper_temp.emplacement != REG_EAX){
    oper2reg(oper, REG_EAX);
  }
  else{
    _nasm_instr("mov", "eax", varconst2nasm(oper), NULL, NULL) ;
  }
  _nasm_instr("call", "iprintLF", NULL, NULL, NULL);
}

/******************************************************************************/

void c3a2nasm_read(operande *result){
  int restoreeax = 0;
  if(!_is_reg_free(REG_EAX)){
    restoreeax = 1;
    _nasm_instr("push", "eax", NULL, NULL, "sauvegarder eax - TODO si besoin") ;
  }
  _nasm_instr("mov", "eax", "sinput", NULL, NULL);
  _nasm_instr("call", "readline", NULL, NULL, NULL);
  _nasm_instr("mov", "eax", "sinput", NULL, NULL);
  _nasm_instr("call", "atoi", NULL, NULL, NULL);
  if(restoreeax){ // eax wasn't free
    result->u.oper_temp.emplacement = new_registre(result); // will not be REG_EAX
    _nasm_instr("mov", nomreg[result->u.oper_temp.emplacement], "eax", NULL, NULL);
    _nasm_instr("pop", "eax", NULL, NULL, "rétablir eax") ;
  }
  else{
    result->u.oper_temp.emplacement = REG_EAX;
    desc_reg[REG_EAX]=result;
  }
}

/******************************************************************************/

void c3a2nasm_jump(char *opcode, operande *oper1, operande *oper2, operande *cible){
  char *oper1string;
  if((oper1->oper_type == O_VARIABLE && oper2->oper_type == O_VARIABLE) ||
      oper1->oper_type == O_CONSTANTE || oper1->oper_type == O_TEMPORAIRE
    ){
    oper1string = nomreg[oper2reg(oper1, REG_NONE)];
  }
  else{
    oper1string = varconst2nasm(oper1);
  }
  char *oper2string;
  if(oper2->oper_type == O_TEMPORAIRE){
    oper2string = nomreg[oper2reg(oper2, REG_NONE)]; //temp in reg
  }
  else{
    oper2string = varconst2nasm(oper2);
  }
  printverb(";");
  if(nasm_verbose){ code3a_affiche_ligne_code(&(code3a.liste[i_ligne]));}
  printverb("\n");
  _nasm_instr("cmp", oper1string, oper2string, NULL, NULL);
  _nasm_instr(opcode, cible->u.oper_nom, NULL, NULL, "saut");
}

/******************************************************************************/

void c3a2nasm_appel(operande *foncname, operande *result){
  _nasm_instr("call", foncname->u.oper_nom, NULL, NULL, NULL);
  if(nbparam != 0) { // desallouer les arguments
    printf("\tadd\tesp, %d\t\t; desallocation parametres\n", 4 * nbparam);
    nbparam = 0;
  }
  if(result){
    result->u.oper_temp.emplacement = new_registre(result);
    _nasm_instr("pop",nomreg[result->u.oper_temp.emplacement],NULL,NULL,"récupère valeur de retour");
  }
  else{
    _nasm_instr("add","esp","4",NULL,"desalloue valeur de retour ignorée");
  }
}

/******************************************************************************/

void c3a2nasm_param(operande *oper){
  char *argchar;
  if(oper->oper_type == O_TEMPORAIRE) {
    argchar = nomreg[oper->u.oper_temp.emplacement];
  }
  else{
    argchar = varconst2nasm(oper);
  }
  _nasm_instr("push", argchar, NULL, NULL, "empile argument");
  nbparam++;
}

/******************************************************************************/

void c3a2nasm_val_ret(operande *oper){
  char *argchar;
  if(oper->oper_type == O_TEMPORAIRE) {
    argchar = nomreg[oper->u.oper_temp.emplacement];
  }
  else{
    argchar = varconst2nasm(oper);
  }
  _nasm_instr_relative("mov", argchar, "ebp", (arguments)*4 + 8, "ecriture de la valeur de retour");
}

/******************************************************************************/

void c3a2nasm_generer(){
  operation_3a i_oper;
  printf("%%include\t'%s'\n","io.asm");
  /* Variables globales */
  printf("%s","\nsection\t.bss\n");
  printf("%s", "sinput:\tresb\t255\t;reserve a 255 byte space in memory for the users input string\n");
  //i_oper = code3a.liste[0];
  for(i_ligne=0; i_ligne < code3a.next &&
                 code3a.liste[i_ligne].op_code == alloc; i_ligne++){
    i_oper = code3a.liste[i_ligne];
    printf("%s:\tresd\t%d\n", i_oper.op_oper2->u.oper_nom, i_oper.op_oper1->u.oper_valeur);
  }
  printf("%s","\nsection\t.text\n");
  printf("%s","global _start\n");
  printf("%s","_start:\n");
  _nasm_instr("call", "fmain", NULL, NULL, NULL);
  _nasm_instr("mov", "eax", "1" , NULL, "1 est le code de SYS_EXIT");
  _nasm_instr("int", "0x80", NULL, NULL, "exit");
  for(; i_ligne < code3a.next; i_ligne++){ // liste de déc. fonctions
    i_oper = code3a.liste[i_ligne];
    if(i_oper.op_etiq){
      printf("%s:\n",i_oper.op_etiq);
    }
    switch(i_oper.op_code) {
      case func_begin : // début de fonction
        c3a2nasm_debutfonction(i_oper.op_etiq+1);
        break;
      case func_end : // fin de fonction
        c3a2nasm_finfonction(varlocs);
        break;
      case func_call : // appel de fonction
        c3a2nasm_appel(i_oper.op_oper1,i_oper.op_result);
        break;
      case jump :
        _nasm_instr("jmp", i_oper.op_oper1->u.oper_nom, NULL, NULL, NULL);
        break;
      case func_val_ret : // appel de fonction
        c3a2nasm_val_ret(i_oper.op_oper1);

        break;
      case func_param :
        c3a2nasm_param(i_oper.op_oper1);
        break;
      case arith_add :
        c3a2nasm_arith("add",i_oper.op_oper1, i_oper.op_oper2, i_oper.op_result);
        break;
      case arith_sub :
        c3a2nasm_arith("sub",i_oper.op_oper1, i_oper.op_oper2, i_oper.op_result);
        break;
      case arith_mult :
        c3a2nasm_arith("imul",i_oper.op_oper1, i_oper.op_oper2, i_oper.op_result);
        break;
      case arith_div : // TODO FIXME
        c3a2nasm_division(i_oper.op_oper1, i_oper.op_oper2, i_oper.op_result);
        break;
      case alloc :
        c3a2nasm_allouer(i_oper.op_oper2);
        break;
      case assign : // affectation i_oper.op_result <- i_oper.op_oper1
        c3a2nasm_affect(i_oper.op_result, i_oper.op_oper1);
        break;
      case sys_write :
        c3a2nasm_write(i_oper.op_oper1);
        break;
      case sys_read :
        c3a2nasm_read(i_oper.op_result);
        break;
      case jump_if_equal :
        c3a2nasm_jump("je",i_oper.op_oper1,i_oper.op_oper2,i_oper.op_result);
        break;
      case jump_if_not_equal :
        c3a2nasm_jump("jne",i_oper.op_oper1,i_oper.op_oper2,i_oper.op_result);
        break;
      case jump_if_less :
        c3a2nasm_jump("jl",i_oper.op_oper1,i_oper.op_oper2,i_oper.op_result);
        break;
      case jump_if_greater :
        c3a2nasm_jump("jg",i_oper.op_oper1,i_oper.op_oper2,i_oper.op_result);
        break;
      case jump_if_greater_or_equal :
        c3a2nasm_jump("jge",i_oper.op_oper1,i_oper.op_oper2,i_oper.op_result);
        break;
      case jump_if_less_or_equal :
        c3a2nasm_jump("jle",i_oper.op_oper1,i_oper.op_oper2,i_oper.op_result);
        break;
      default:
        code3a_affiche_ligne_code(&i_oper);
        erreur("Opération en code 3 adresses non reconnue");
    }
  }
}

/******************************************************************************/

void _nasm_comment(char *comment) {
  if(comment != NULL) {
    printf("\t\t ; %s", comment);
  }
  printf("%s","\n");
}

/******************************************************************************/

/* Par convention, les derniers opérandes sont nuls si l'opération a moins de
   3 arguments */
void _nasm_instr(char *opcode, char *op1, char *op2, char *op3, char *comment) {
  printf("\t%s", opcode);
  if(op1 != NULL) {
    printf("\t%s", op1);
    if(op2 != NULL) {
      printf(", %s", op2);
      if(op3 != NULL) {
        printf(", %s", op3);
      }
    }
  }
  _nasm_comment(comment);
}

/******************************************************************************/

void _nasm_instr_relative(char *opcode,  char *op1, char *op2, int offset, char *comment) {
  if(offset < 0){
    printf("\t%s\t[%s - %d], %s", opcode, op2, -offset, op1);
  }
  else{
    printf("\t%s\t[%s + %d], %s", opcode, op2, offset, op1);
  }
  /* printifm("\t%s\t[%s + %d], %s", opcode, op2, offset, op1); */
  _nasm_comment(comment);
}

/******************************************************************************/

void _nasm_etiquette(char* etiquette) {
  printf("%s:\n", etiquette);
}
