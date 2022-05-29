#pragma once

// analiza de domeniu

struct Symbol;
typedef struct Symbol Symbol;

typedef enum
{ // tipul de baza
    TB_INT,
    TB_DOUBLE,
    TB_CHAR,
    TB_VOID,
    TB_STRUCT
} TypeBase;

typedef struct
{ // tipul unui simbol
    TypeBase tb;
    Symbol *s; // pentru TB_STRUCT, structura efectiva
    // n - dimensiunea pentru array
    //		n<0 - nu este un array
    //		n==0 - array fara dimensiune specificata: int v[]
    //		n>0 - array cu dimensiune specificata: double v[10]
    int n;
} Type;

// returneaza dimensiunea in octeti a tipului t
int typeSize(Type *t);

typedef enum
{ // felul unui simbol
    SK_VAR,
    SK_PARAM,
    SK_FN,
    SK_STRUCT
} SymKind;

struct Symbol
{
    const char *name; // numele simbolului. Simbolul nu detine acest pointer, ci el este alocat in alta parte (ex: in Token)
    SymKind kind;     // felul
    Type type;        // tipul
    // owner:
    //		- NULL pentru simboluri globale
    //		- o structura pentru variabile definite in acea structura
    //		- o functie pentru parametri/variabile locale acelei functii
    Symbol *owner;
    Symbol *next; // inlantuire la urmatorul simbol din lista
    union
    { // date specifice pentru fiecare fel de simbol
        // indexul in fn.locals pentru variabile locale
        // indexul in globals pentru variabile globale
        // indexul in structura pentru membri de structura
        int varIdx;
        // indexul in fn.params pentru parametri
        int paramIdx;
        // membrii unei structuri
        Symbol *structMembers;
        struct
        {
            Symbol *params;     // parametrii functiei
            Symbol *locals;     // variabilele locale, inclusiv cele din subdomeniile fn
            void (*extFnPtr)(); // !=NULL pentru functii externe
            int instrIdx;
        } fn;
    };
};

// aloca dinamic un nou simbol
Symbol *newSymbol(const char *name, SymKind kind);
// duplica simbolul dat
Symbol *dupSymbol(Symbol *symbol);
// adauga simbolul la sfarsitul listei
// list - adresa listei unde se face adaugarea
Symbol *addSymbolToList(Symbol **list, Symbol *s);
// numarul de simboluri din lista
int symbolsLen(Symbol *list);
// elibereaza memoria ocupata de un simbol
void freeSymbol(Symbol *s);

typedef struct _Domain
{
    struct _Domain *parent; // domeniul parinte
    Symbol *symbols;        // simbolurile din acest domeniu (lista simplu inlantuita)
} Domain;

// domeniul curent (varful stivei de domenii)
extern Domain *symTable;

// adauga un domeniu in varful stivei de domenii
Domain *pushDomain();
// sterge domeniul din varful stivei de domenii
void dropDomain();
// afiseaza continutul domeniului curent
void showDomain(Domain *d, const char *name);
// cauta un simbol in domeniul specificat si il returneaza
// daca nu il gaseste, returneaza NULL
Symbol *findSymbolInDomain(Domain *d, const char *name);
// cauta un simbol in toate domeniile, incepand cu cel curent
Symbol *findSymbol(const char *name);
// adauga un simbol la domeniul curent
Symbol *addSymbolToDomain(Domain *d, Symbol *s);

// memoria in care sunt pastrate valorile variabilelor globale
extern char *globalMemory;
extern int nGlobalMemory; // dimensiunea lui globalMemory, in octeti

// aloca in globalMemory un spatiu de nBytes octeti
// si ii returneaza indexul de inceput
int allocInGlobalMemory(int nBytes);

// adauga in TS o functie externa cu numele, adresa si tipul returnat date
Symbol *addExtFn(const char *name, void (*extFnPtr)(), Type ret);

// adauga la fn data un parametru cu numele si tipul specificate
// nu face verificare de parametrii redefiniti
// returneaza paramentrul adaugat
Symbol *addFnParam(Symbol *fn, const char *name, Type type);