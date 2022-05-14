#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "ad.c"

Token *iTk;
Token *consumedTk;

Symbol *owner = NULL;

bool unit();
bool structDef();
bool varDef();
bool typeBase(Type *t);
bool arrayDecl(Type *t);
bool fnDef();
bool fnParam();
bool stm();
bool stmCompound(bool newDomain);
bool expr();
bool exprAssign();
bool exprOr();
bool exprAnd();
bool exprEq();
bool exprRel();
bool exprAdd();
bool exprMul();
bool exprCast();
bool exprUnary();
bool exprPostfix();
bool exprPrimary();

bool exprPostfixAux();
bool exprMulAux();
bool exprAddAux();
bool exprRelAux();
bool exprEqAux();
bool exprAndAux();
bool exprOrAux();

bool consume(int code)
{
    if (iTk->code == code)
    {
        consumedTk = iTk;
        iTk = iTk->next;
        // printToken(code);
        // printf("\n");
        return true;
    }
    return false;
}

// unit: ( structDef | fnDef | varDef )* END
bool unit()
{
    Token *start = iTk;

    for (;;)
    {
        if (structDef())
        {
        }
        else if (fnDef())
        {
        }
        else if (varDef())
        {
        }
        else
            break;
    }
    if (consume(END))
    {
        return true;
    }

    tkerr(iTk, "Syntax error\n");
    iTk = start;
    return false;
}

// structDef: STRUCT ID LACC varDef* RACC SEMICOLON
bool structDef()
{
    Token *start = iTk;

    if (consume(STRUCT))
    {
        if (consume(ID))
        {
            Token *tkName = consumedTk;
            if (consume(LACC))
            {
                Symbol *s = findSymbolInDomain(symTable, tkName->text);
                if (s)
                    tkerr(iTk, "symbol redefinition: %s", tkName->text);
                s = addSymbolToDomain(symTable, newSymbol(tkName->text, SK_STRUCT));
                s->type.tb = TB_STRUCT;
                s->type.s = s;
                s->type.n = -1;
                pushDomain();
                owner = s;
                for (;;)
                {
                    if (varDef())
                    {
                    }
                    else
                        break;
                }
                if (consume(RACC))
                {
                    if (consume(SEMICOLON))
                    {
                        owner = NULL;
                        dropDomain();
                        return true;
                    }
                    else
                        tkerr(iTk, "Missing ; after struct declaration\n");
                }
                else
                    tkerr(iTk, "Missing } after struct body\n");
            }
        }
        else
            tkerr(iTk, "Missing struct name\n");
    }

    iTk = start;
    return false;
}

// varDef: typeBase ID arrayDecl? SEMICOLON
bool varDef()
{
    Token *start = iTk;
    Type t;
    if (typeBase(&t))
    {
        if (consume(ID))
        {
            Token *tkName = consumedTk;
            if (arrayDecl(&t))
            {
                if (t.n == 0)
                    tkerr(iTk, "a vector variable must have a specified dimension");
            }
            if (consume(SEMICOLON))
            {
                Symbol *var = findSymbolInDomain(symTable, tkName->text);
                if (var)
                    tkerr(iTk, "symbol redefinition: %s", tkName->text);
                var = newSymbol(tkName->text, SK_VAR);
                var->type = t;
                var->owner = owner;
                addSymbolToDomain(symTable, var);
                if (owner)
                {
                    switch (owner->kind)
                    {
                    case SK_FN:
                        var->varIdx = symbolsLen(owner->fn.locals);
                        addSymbolToList(&owner->fn.locals, dupSymbol(var));
                        break;
                    case SK_STRUCT:
                        var->varIdx = typeSize(&owner->type);
                        addSymbolToList(&owner->structMembers, dupSymbol(var));
                        break;
                    }
                }
                else
                {
                    var->varIdx = allocInGlobalMemory(typeSize(&t));
                }
                return true;
            }
            else
                tkerr(iTk, "Missing ; after variable declaration\n");
        }
        else
            tkerr(iTk, "Missing variable name\n");
    }

    iTk = start;
    return false;
}

// typeBase: INT | DOUBLE | CHAR | STRUCT ID
bool typeBase(Type *t)
{
    Token *start = iTk;
    t->n = -1;

    if (consume(INT))
    {
        t->tb = TB_INT;
        return true;
    }
    if (consume(DOUBLE))
    {
        t->tb = TB_DOUBLE;
        return true;
    }
    if (consume(CHAR))
    {
        t->tb = TB_CHAR;
        return true;
    }
    if (consume(STRUCT))
    {
        if (consume(ID))
        {
            Token *tkName = consumedTk;
            t->tb = TB_STRUCT;
            t->s = findSymbol(tkName->text);
            if (!t->s)
                tkerr(iTk, "structura nedefinita: %s", tkName->text);
            return true;
        }
        else
            tkerr(iTk, "Missing struct name\n");
    }

    iTk = start;
    return false;
}

// arrayDecl: LBRACKET CT_INT? RBRACKET
bool arrayDecl(Type *t)
{
    Token *start = iTk;

    if (consume(LBRACKET))
    {
        if (consume(CT_INT))
        {
            Token *tkSize = consumedTk;
            t->n = tkSize->i;
        }
        else
        {
            t->n = 0;
        }
        if (consume(RBRACKET))
        {
            return true;
        }
        else
            tkerr(iTk, "Missing ] after array declaration\n");
    }

    iTk = start;
    return false;
}

// fnDef: ( typeBase | VOID ) ID LPAR ( fnParam (COMMA fnParam)* )? RPAR stmCompound
bool fnDef()
{
    Token *start = iTk;
    Type t;

    if (typeBase(&t))
    {
        if (consume(ID))
        {
            Token *tkName = consumedTk;
            if (consume(LPAR))
            {
                Symbol *fn = findSymbolInDomain(symTable, tkName->text);
                if (fn)
                    tkerr(iTk, "symbol redefinition: %s", tkName->text);
                fn = newSymbol(tkName->text, SK_FN);
                fn->type = t;
                addSymbolToDomain(symTable, fn);
                owner = fn;
                pushDomain();

                if (fnParam())
                {
                    for (;;)
                    {
                        if (consume(COMMA))
                        {
                            if (fnParam())
                            {
                            }
                            else
                                tkerr(iTk, "Missing argument from the function header\n");
                        }
                        else
                            break;
                    }
                }
                if (consume(RPAR))
                {
                    if (stmCompound(false))
                    {
                        dropDomain();
                        owner = NULL;
                        return true;
                    }
                    else
                        tkerr(iTk, "Function has no body\n");
                }
                else
                    tkerr(iTk, "Missing ) after the function arguments\n");
            }
        }
        else
            tkerr(iTk, "Missing function name\n");
    }

    if (consume(VOID))
    {
        t.tb = TB_VOID;
        if (consume(ID))
        {
            Token *tkName = consumedTk;
            if (consume(LPAR))
            {
                Symbol *fn = findSymbolInDomain(symTable, tkName->text);
                if (fn)
                    tkerr(iTk, "symbol redefinition: %s", tkName->text);
                fn = newSymbol(tkName->text, SK_FN);
                fn->type = t;
                addSymbolToDomain(symTable, fn);
                owner = fn;
                pushDomain();

                if (fnParam())
                {
                    for (;;)
                    {
                        if (consume(COMMA))
                        {
                            if (fnParam())
                            {
                            }
                            else
                                tkerr(iTk, "Missing argument from the function header\n");
                        }
                        else
                            break;
                    }
                }
                if (consume(RPAR))
                {
                    if (stmCompound(false))
                    {
                        dropDomain();
                        owner = NULL;
                        return true;
                    }
                    else
                        tkerr(iTk, "Function has no body\n");
                }
                else
                    tkerr(iTk, "Missing ) after the function arguments\n");
            }
        }
        else
            tkerr(iTk, "Missing function name\n");
    }

    iTk = start;
    return false;
}

// fnParam: typeBase ID arrayDecl?
bool fnParam()
{
    Type t;
    Token *start = iTk;

    if (typeBase(&t))
    {
        if (consume(ID))
        {
            Token *tkName = consumedTk;
            if (arrayDecl(&t))
            {
                t.n = 0;
            }
            Symbol *param = findSymbolInDomain(symTable, tkName->text);
            if (param)
                tkerr(iTk, "symbol redefinition: %s", tkName->text);
            param = newSymbol(tkName->text, SK_PARAM);
            param->type = t;
            param->paramIdx = symbolsLen(owner->fn.params);

            // parametrul este adaugat atat la domeniul curent, cat si la parametrii fn
            addSymbolToDomain(symTable, param);
            addSymbolToList(&owner->fn.params, dupSymbol(param));
            return true;
        }
        else
            tkerr(iTk, "Missing argument name\n");
    }

    iTk = start;
    return false;
}

/*stm: stmCompound
| IF LPAR expr RPAR stm ( ELSE stm )?
| WHILE LPAR expr RPAR stm
| FOR LPAR expr? SEMICOLON expr? SEMICOLON expr? RPAR stm
| BREAK SEMICOLON
| RETURN expr? SEMICOLON
| expr? SEMICOLON
*/
bool stm()
{
    Token *start = iTk;

    if (stmCompound(true))
    {
        return true;
    }

    if (consume(IF))
    {
        if (consume(LPAR))
        {
            if (expr())
            {
                if (consume(RPAR))
                {
                    if (stm())
                    {
                        if (consume(ELSE))
                        {
                            if (stm())
                            {
                                return true;
                            }
                            else
                                tkerr(iTk, "The else instruction cannot have an empty body\n");
                        }
                        return true;
                    }
                    else
                        tkerr(iTk, "The if instruction must have a body\n");
                }
                else
                    tkerr(iTk, "Missing ) after the if condition\n");
            }
            else
                tkerr(iTk, "Missing condition from the if instruction\n");
        }
        else
            tkerr(iTk, "Missing ( after the if instruction\n");
    }

    if (consume(WHILE))
    {
        if (consume(LPAR))
        {
            if (expr())
            {
                if (consume(RPAR))
                {
                    if (stm())
                    {
                        return true;
                    }
                    else
                        tkerr(iTk, "The while instruction must have a body\n");
                }
                else
                    tkerr(iTk, "Missing ) after the while condition\n");
            }
        }
        else
            tkerr(iTk, "Missing ( after the while instruction\n");
    }

    if (consume(FOR))
    {
        if (consume(LPAR))
        {
            if (expr())
            {
            }
            if (consume(SEMICOLON))
            {
                if (expr())
                {
                }
                if (consume(SEMICOLON))
                {
                    if (expr())
                    {
                    }
                    if (consume(RPAR))
                    {
                        if (stm())
                        {
                            return true;
                        }
                        else
                            tkerr(iTk, "The for instruction must have a body\n");
                    }
                    else
                        tkerr(iTk, "Missing ) after the for condition\n");
                }
                else
                    tkerr(iTk, "Missing second ; from the for condition\n");
            }
            else
                tkerr(iTk, "Missing first ; from the for condition\n");
        }
        else
            tkerr(iTk, "Missing ( after the for instruction\n");
    }

    if (consume(BREAK))
    {
        if (consume(SEMICOLON))
        {
            return true;
        }
        else
            tkerr(iTk, "Missing ; after the break instruction");
    }

    if (consume(RETURN))
    {
        if (expr())
        {
        }
        if (consume(SEMICOLON))
        {
            return true;
        }
        else
            tkerr(iTk, "Missing ; after the return instruction");
    }

    if (expr())
    {
        if (consume(SEMICOLON))
        {
            return true;
        }
        else
            tkerr(iTk, "Missing ; after expression");
    }

    if (consume(SEMICOLON))
    {
        return true;
    }

    iTk = start;
    return false;
}

// stmCompound: LACC ( varDef | stm )* RACC
bool stmCompound(bool newDomain)
{
    Token *start = iTk;

    if (consume(LACC))
    {
        if (newDomain)
            pushDomain();
        for (;;)
        {
            if (varDef())
            {
            }
            else if (stm())
            {
            }
            else
                break;
        }
        if (consume(RACC))
        {
            if (newDomain)
                dropDomain();
            return true;
        }
        else
            tkerr(iTk, "Missing } after body");
    }

    iTk = start;
    return false;
}

// expr: exprAssign
bool expr()
{
    Token *start = iTk;

    if (exprAssign())
    {
        return true;
    }

    iTk = start;
    return false;
}

// exprAssign: exprUnary ASSIGN exprAssign | exprOr
bool exprAssign()
{
    Token *start = iTk;

    if (exprUnary())
    {
        if (consume(ASSIGN))
        {
            if (exprAssign())
            {
                return true;
            }
            else
                tkerr(iTk, "Invalid expression after = \n");
        }
    }

    iTk = start;
    if (exprOr())
    {
        return true;
    }

    iTk = start;
    return false;
}

// exprOr: exprOr OR exprAnd | exprAnd
// exprOr : exprAnd exprOrAux
bool exprOr()
{
    Token *start = iTk;

    if (exprAnd())
    {
        if (exprOrAux())
        {
            return true;
        }
        else
            tkerr(iTk, "Invalid expression after && \n");
    }

    iTk = start;
    return false;
}

// exprOrAux : OR exprAnd exprOrAux | epsilon
bool exprOrAux()
{
    Token *start = iTk;

    if (consume(OR))
    {
        if (exprAnd())
        {
            if (exprOrAux())
            {
                return true;
            }
            else
                tkerr(iTk, "Invalid expression after && \n");
        }
        else
            tkerr(iTk, "Invalid expression after || \n");
    }

    iTk = start;
    return true;
}

// exprAnd: exprAnd AND exprEq | exprEq
// exprAnd: exprEq exprAndAux
bool exprAnd()
{
    Token *start = iTk;

    if (exprEq())
    {
        if (exprAndAux())
        {
            return true;
        }
        else
            tkerr(iTk, "Invalid expression after == \n");
    }

    iTk = start;
    return false;
}

// exprAndAux: AND exprEq exprAndAux | epsilon
bool exprAndAux()
{
    Token *start = iTk;

    if (consume(AND))
    {
        if (exprEq())
        {
            if (exprAndAux())
            {
                return true;
            }
            else
                tkerr(iTk, "Invalid expression after == \n");
        }
        else
            tkerr(iTk, "Invalid expression after && \n");
    }

    iTk = start;
    return true;
}

// exprEq: exprEq ( EQUAL | NOTEQ ) exprRel | exprRel
// exprEq: exprRel exprEqAux
bool exprEq()
{
    Token *start = iTk;

    if (exprRel())
    {
        if (exprEqAux())
        {
            return true;
        }
        else
            tkerr(iTk, "Invalid expression \n");
    }

    iTk = start;
    return false;
}

// exprEqAux: ( EQUAL | NOTEQ ) exprRel exprEqAux
bool exprEqAux()
{
    Token *start = iTk;

    if (consume(EQUAL))
    {
        if (exprRel())
        {
            if (exprEqAux())
            {
                return true;
            }
            else
                tkerr(iTk, "Invalid expression \n");
        }
        else
            tkerr(iTk, "Invalid expression after == \n");
    }
    else if (consume(NOTEQ))
    {
        if (exprRel())
        {
            if (exprEqAux())
            {
                return true;
            }
            else
                tkerr(iTk, "Invalid expression \n");
        }
        else
            tkerr(iTk, "Invalid expression after != \n");
    }

    iTk = start;
    return true;
}

// exprRel: exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd
// exprRel: exprAdd exprRelAux
bool exprRel()
{
    Token *start = iTk;

    if (exprAdd())
    {
        if (exprRelAux())
        {
            return true;
        }
        else
            tkerr(iTk, "Invalid expression \n");
    }

    iTk = start;
    return false;
}

// exprRelAux: ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd exprRelAux | epsilon
bool exprRelAux()
{
    Token *start = iTk;

    if (consume(LESS))
    {
        if (exprAdd())
        {
            if (exprRelAux())
            {
                return true;
            }
            else
                tkerr(iTk, "Invalid expression \n");
        }
        else
            tkerr(iTk, "Invalid expression after < \n");
    }

    if (consume(LESSEQ))
    {
        if (exprAdd())
        {
            if (exprRelAux())
            {
                return true;
            }
            else
                tkerr(iTk, "Invalid expression \n");
        }
        else
            tkerr(iTk, "Invalid expression after <= \n");
    }

    if (consume(GREATER))
    {
        if (exprAdd())
        {
            if (exprRelAux())
            {
                return true;
            }
            else
                tkerr(iTk, "Invalid expression \n");
        }
        else
            tkerr(iTk, "Invalid expression after > \n");
    }

    if (consume(GREATEREQ))
    {
        if (exprAdd())
        {
            if (exprRelAux())
            {
                return true;
            }
            else
                tkerr(iTk, "Invalid expression \n");
        }
        else
            tkerr(iTk, "Invalid expression after >= \n");
    }

    iTk = start;
    return true;
}

// exprAdd: exprAdd ( ADD | SUB ) exprMul | exprMul
// exprAdd: exprMul exprAddAux
bool exprAdd()
{
    Token *start = iTk;

    if (exprMul())
    {
        if (exprAddAux())
        {
            return true;
        }
        else
            tkerr(iTk, "Invalid expression \n");
    }

    iTk = start;
    return false;
}

// exprAddAux: ( ADD | SUB ) exprMul exprAddAux | epsilon
bool exprAddAux()
{
    Token *start = iTk;

    if (consume(ADD))
    {
        if (exprMul())
        {
            if (exprAddAux())
            {
                return true;
            }
            else
                tkerr(iTk, "Invalid expression \n");
        }
        else
            tkerr(iTk, "Invalid expression after + \n");
    }
    if (consume(SUB))
    {
        if (exprMul())
        {
            if (exprAddAux())
            {
                return true;
            }
            else
                tkerr(iTk, "Invalid expression \n");
        }
        else
            tkerr(iTk, "Invalid expression after - \n");
    }

    iTk = start;
    return true;
}

// exprMul: exprMul ( MUL | DIV ) exprCast | exprCast
// exprMul: exprCast exprMulAux
bool exprMul()
{
    Token *start = iTk;

    if (exprCast())
    {
        if (exprMulAux())
        {
            return true;
        }
        else
            tkerr(iTk, "Invalid expression after cast\n");
    }

    iTk = start;
    return false;
}

// exprMulAux: ( MUL | DIV ) exprCast exprMulAux | epsilon
bool exprMulAux()
{
    Token *start = iTk;

    if (consume(MUL))
    {
        if (exprCast())
        {
            if (exprMulAux())
            {
                return true;
            }
            else
                tkerr(iTk, "Invalid expression after cast \n");
        }
        else
            tkerr(iTk, "Invalid expression after * \n");
    }

    if (consume(DIV))
    {
        if (exprCast())
        {
            if (exprMulAux())
            {
                return true;
            }
            else
                tkerr(iTk, "Invalid expression after cast \n");
        }
        else
            tkerr(iTk, "Invalid expression after / \n");
    }

    iTk = start;
    return true;
}

// exprCast: LPAR typeBase arrayDecl? RPAR exprCast | exprUnary
bool exprCast()
{
    Token *start = iTk;
    Type t;
    if (consume(LPAR))
    {
        if (typeBase(&t))
        {
            if (arrayDecl(&t))
            {
            }
            if (consume(RPAR))
            {
                if (exprCast())
                {
                    return true;
                }
                else
                    tkerr(iTk, "Invalid expression after ) in cast expression \n");
            }
            else
                tkerr(iTk, "Missing ) from cast expression \n");
        }
    }

    if (exprUnary())
    {
        return true;
    }

    iTk = start;
    return false;
}

// exprUnary: ( SUB | NOT ) exprUnary | exprPostfix
bool exprUnary()
{
    Token *start = iTk;

    if (consume(SUB))
    {
        if (exprUnary())
        {
            return true;
        }
        else
            tkerr(iTk, "Invalid expression after - \n");
    }

    else if (consume(NOT))
    {
        if (exprUnary())
        {
            return true;
        }
        else
            tkerr(iTk, "Invalid expression after ! \n");
    }

    if (exprPostfix())
    {
        return true;
    }

    iTk = start;
    return false;
}

// exprPostfix: exprPostfix LBRACKET expr RBRACKET | exprPostfix DOT ID | exprPrimary
// exprPostfix: exprPrimary exprPostfixAux
bool exprPostfix()
{
    Token *start = iTk;

    if (exprPrimary())
    {
        if (exprPostfixAux())
        {
            return true;
        }
        else
            tkerr(iTk, "Invalid expression \n");
    }

    iTk = start;
    return false;
}

// exprPostfixAux: LBRACKET expr RBRACKET exprPostfixAux | DOT ID exprPostfixAux | epsilon
bool exprPostfixAux()
{
    Token *start = iTk;

    if (consume(LBRACKET))
    {
        if (expr())
        {
            if (consume(RBRACKET))
            {
                if (exprPostfixAux())
                {
                    return true;
                }
                else
                    tkerr(iTk, "Invalid expression after ] \n");
            }
            else
                tkerr(iTk, "Missing ] after expression\n");
        }
        else
            tkerr(iTk, "Invalid expression after [ \n");
    }

    else if (consume(DOT))
    {
        if (consume(ID))
        {
            if (exprPostfixAux())
            {
                return true;
            }
            else
                tkerr(iTk, "Invalid expression after ID \n");
        }
        else
            tkerr(iTk, "Invalid expression after . \n");
    }

    iTk = start;
    return true;
}

// exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )? | CT_INT | CT_REAL | CT_CHAR | CT_STRING | LPAR expr RPAR
bool exprPrimary()
{
    Token *start = iTk;

    if (consume(ID))
    {
        if (consume(LPAR))
        {
            if (expr())
            {
                for (;;)
                {
                    if (consume(COMMA))
                    {
                        if (expr())
                        {
                        }
                        else
                            tkerr(iTk, "Missing expression after , \n");
                    }
                    else
                        break;
                }
            }
            if (consume(RPAR))
            {
                return true;
            }
            else
                tkerr(iTk, "Missing ) after expression \n");
        }

        return true;
    }

    if (consume(CT_INT))
    {
        return true;
    }

    if (consume(CT_REAL))
    {
        return true;
    }

    if (consume(CT_CHAR))
    {
        return true;
    }

    if (consume(CT_STRING))
    {
        return true;
    }

    if (consume(LPAR))
    {
        if (expr())
        {
            if (consume(RPAR))
            {
                return true;
            }
            else
                tkerr(iTk, "Missing ) after expression \n");
        }
        else
            tkerr(iTk, "Missing expression after (");
    }

    iTk = start;
    return false;
}

int main(int argc, char const *argv[])
{
    readFile(argv[1]);
    pCrtCh = inBuffer;
    while (getNextToken() != END)
    {
    }

    pushDomain();
    iTk = tokens;
    if (unit())
    {
        printf("Syntax check has completed successfully\n");
    }
    showDomain(symTable,"global");
    dropDomain();
    return 0;
}
