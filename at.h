#pragma once

// analiza de tipuri

#include "ad.h"

typedef struct{
	Type type;		// tipul returnat
	bool lval;			// true daca este left-value
	bool ct;				// true daca este constanta
	}Ret;

// returneaza true daca r->type poate fi convertit
// la o valoare scalara: int, double, char sau adresa
bool canBeScalar(Ret* r);

// verifica daca se poate converti tipul sursa la tipul destinatie
// daca da, returneaza true
bool convTo(Type *src,Type *dst);

// seteaza in dst tipul rezultat de o operatie aritmetica 
// avand ca operanzi tipurile t1 si t2
// returneaza true daca t1 si t2 pot fi operanzi pentru o operatie aritmetica
// ex: double + int -> double
bool arithTypeTo(Type *t1,Type *t2,Type *dst);

// cauta un nume intr-o lista de simboluri
// daca il gaseste, returneaza simbolul corespondent, altfel NULL
Symbol *findSymbolInList(Symbol *list,const char *name);