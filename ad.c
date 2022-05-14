#include <stdbool.h>
#include <stdio.h>
#include "ALEX.c"
#include "ad.h"

Domain *symTable = NULL;
char *globalMemory = NULL;
int nGlobalMemory = 0;

int typeBaseSize(Type *t)
{
    switch (t->tb)
    {
    case TB_INT:
        return sizeof(int);
    case TB_DOUBLE:
        return sizeof(double);
    case TB_CHAR:
        return sizeof(char);
    case TB_VOID:
        return 0;
    default:
    { // TB_STRUCT
        int size = 0;
        for (Symbol *m = t->s->structMembers; m; m = m->next)
        {
            size += typeSize(&m->type);
        }
        return size;
    }
    }
}

int typeSize(Type *t)
{
    if (t->n < 0)
        return typeBaseSize(t);
    if (t->n == 0)
        return sizeof(void *);
    return t->n * typeBaseSize(t);
}

// elibereaza memoria ocupata de o lista de simboluri
void freeSymbols(Symbol *list)
{
    for (Symbol *next; list; list = next)
    {
        next = list->next;
        freeSymbol(list);
    }
}

Symbol *newSymbol(const char *name, SymKind kind)
{
    Symbol *s;
    SAFEALLOC(s, Symbol);
    // seteaza pe 0/NULL toate campurile
    memset(s, 0, sizeof(Symbol));
    s->name = name;
    s->kind = kind;
    return s;
}

Symbol *dupSymbol(Symbol *symbol)
{
    Symbol *s;
    SAFEALLOC(s, Symbol);
    *s = *symbol;
    s->next = NULL;
    return s;
}

// adauga s la sfarsitul listei list
// s->next este NULL de la newSymbol
Symbol *addSymbolToList(Symbol **list, Symbol *s)
{
    Symbol *iter = *list;
    if (iter)
    {
        while (iter->next)
            iter = iter->next;
        iter->next = s;
    }
    else
    {
        *list = s;
    }
    return s;
}

int symbolsLen(Symbol *list)
{
    int n = 0;
    for (; list; list = list->next)
        n++;
    return n;
}

void freeSymbol(Symbol *s)
{
    switch (s->kind)
    {
    case SK_FN:
        freeSymbols(s->fn.params);
        freeSymbols(s->fn.locals);
        break;
    case SK_STRUCT:
        freeSymbols(s->structMembers);
        break;
    }
    free(s);
}

Domain *pushDomain()
{
    Domain *d;
    SAFEALLOC(d, Domain);
    d->symbols = NULL;
    d->parent = symTable;
    symTable = d;
    return d;
}

void dropDomain()
{
    Domain *d = symTable;
    symTable = d->parent;
    freeSymbols(d->symbols);
    free(d);
}

void showNamedType(Type *t, const char *name)
{
    switch (t->tb)
    {
    case TB_INT:
        printf("int");
        break;
    case TB_DOUBLE:
        printf("double");
        break;
    case TB_CHAR:
        printf("char");
        break;
    case TB_VOID:
        printf("void");
        break;
    default: // TB_STRUCT
        printf("struct %s", t->s->name);
    }
    if (name)
        printf(" %s", name);
    if (t->n == 0)
        printf("[]");
    else if (t->n > 0)
        printf("[%d]", t->n);
}

void showSymbol(Symbol *s)
{
    switch (s->kind)
    {
    case SK_VAR:
        showNamedType(&s->type, s->name);
        printf(";\t// size=%d, idx=%d\n", typeSize(&s->type), s->varIdx);
        break;
    case SK_PARAM:
    {
        showNamedType(&s->type, s->name);
        printf(" /*size=%d, idx=%d*/", typeSize(&s->type), s->paramIdx);
    }
    break;
    case SK_FN:
    {
        showNamedType(&s->type, s->name);
        printf("(");
        bool next = false;
        for (Symbol *param = s->fn.params; param; param = param->next)
        {
            if (next)
                printf(", ");
            showSymbol(param);
            next = true;
        }
        printf("){\n");
        for (Symbol *local = s->fn.locals; local; local = local->next)
        {
            printf("\t");
            showSymbol(local);
        }
        printf("\t}\n");
    }
    break;
    case SK_STRUCT:
    {
        printf("struct %s{\n", s->name);
        for (Symbol *m = s->structMembers; m; m = m->next)
        {
            printf("\t");
            showSymbol(m);
        }
        printf("\t};\t// size=%d\n", typeSize(&s->type));
    }
    break;
    }
}

void showDomain(Domain *d, const char *name)
{
    printf("// domain: %s\n", name);
    for (Symbol *s = d->symbols; s; s = s->next)
    {
        showSymbol(s);
    }
    puts("\n");
}

Symbol *findSymbolInDomain(Domain *d, const char *name)
{
    for (Symbol *s = d->symbols; s; s = s->next)
    {
        if (!strcmp(s->name, name))
            return s;
    }
    return NULL;
}

Symbol *findSymbol(const char *name)
{
    for (Domain *d = symTable; d; d = d->parent)
    {
        Symbol *s = findSymbolInDomain(d, name);
        if (s)
            return s;
    }
    return NULL;
}

Symbol *addSymbolToDomain(Domain *d, Symbol *s)
{
    return addSymbolToList(&d->symbols, s);
}

int allocInGlobalMemory(int nBytes)
{
    char *aux = (char *)realloc(globalMemory, nGlobalMemory + nBytes);
    if (!aux)
        err("not enough memory");
    int idx = nGlobalMemory;
    nGlobalMemory += nBytes;
    return idx;
}