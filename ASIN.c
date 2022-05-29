#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "at.c"
#include "mv.c"

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
bool expr(Ret *r);
bool exprAssign(Ret *r);
bool exprOr(Ret *r);
bool exprAnd(Ret *r);
bool exprEq(Ret *r);
bool exprRel(Ret *r);
bool exprAdd(Ret *r);
bool exprMul(Ret *r);
bool exprCast(Ret *r);
bool exprUnary(Ret *r);
bool exprPostfix(Ret *r);
bool exprPrimary(Ret *r);

bool exprPostfixAux();
bool exprMulAux(Ret *r);
bool exprAddAux(Ret *r);
bool exprRelAux(Ret *r);
bool exprEqAux(Ret *r);
bool exprAndAux(Ret *r);
bool exprOrAux(Ret *r);

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
    Ret rInit, rCond, rStep, rExpr;

    if (stmCompound(true))
    {
        return true;
    }

    if (consume(IF))
    {
        if (consume(LPAR))
        {
            if (expr(&rCond))
            {
                if (!canBeScalar(&rCond))
                    tkerr(iTk, "The if condition must be a scalar value");
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
            if (expr(&rCond))
            {
                if (!canBeScalar(&rCond))
                    tkerr(iTk, "The while condition must be a scalar value");
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
            if (expr(&rInit))
            {
            }
            if (consume(SEMICOLON))
            {
                if (expr(&rCond))
                {
                    if (!canBeScalar(&rCond))
                        tkerr(iTk, "The for condition must be a scalar value");
                }
                if (consume(SEMICOLON))
                {
                    if (expr(&rStep))
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
        if (expr(&rExpr))
        {
            if (owner->type.tb == TB_VOID)
                tkerr(iTk, "A void function cannot return a value");
            if (!canBeScalar(&rExpr))
                tkerr(iTk, "The return value must be a scalar value");
            if (!convTo(&rExpr.type, &owner->type))
                tkerr(iTk, "Cannot convert the return expression type to the function return type");
        }
        else
        {
            if (owner->type.tb != TB_VOID)
                tkerr(iTk, "a non-void function must return a value");
        }
        if (consume(SEMICOLON))
        {
            return true;
        }
        else
            tkerr(iTk, "Missing ; after the return instruction");
    }

    if (expr(&rExpr))
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
bool expr(Ret *r)
{
    Token *start = iTk;

    if (exprAssign(r))
    {
        return true;
    }

    iTk = start;
    return false;
}

// exprAssign: exprUnary ASSIGN exprAssign | exprOr
bool exprAssign(Ret *r)
{
    Token *start = iTk;
    Ret rDst;

    if (exprUnary(&rDst))
    {
        if (consume(ASSIGN))
        {
            if (exprAssign(r))
            {
                if (!rDst.lval)
                    tkerr(iTk, "The assign destination must be a left-value");
                if (rDst.ct)
                    tkerr(iTk, "The assign destination cannot be constant");
                if (!canBeScalar(&rDst))
                    tkerr(iTk, "The assign destination must be scalar");
                if (!canBeScalar(r))
                    tkerr(iTk, "The assign source must be scalar");
                if (!convTo(&r->type, &rDst.type))
                    tkerr(iTk, "The assign source cannot be converted to destination");
                r->lval = false;
                r->ct = true;
                return true;
            }
            else
                tkerr(iTk, "Invalid expression after = \n");
        }
    }

    iTk = start;
    if (exprOr(r))
    {
        return true;
    }

    iTk = start;
    return false;
}

// exprOr: exprOr OR exprAnd | exprAnd
// exprOr : exprAnd exprOrAux
bool exprOr(Ret *r)
{
    Token *start = iTk;

    if (exprAnd(r))
    {
        if (exprOrAux(r))
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
bool exprOrAux(Ret *r)
{
    Token *start = iTk;

    if (consume(OR))
    {
        Ret right;
        if (exprAnd(&right))
        {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst))
                tkerr(iTk, "Invalid operand type for ||");
            *r = (Ret){{TB_INT, NULL, -1}, false, true};
            if (exprOrAux(r))
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
bool exprAnd(Ret *r)
{
    Token *start = iTk;

    if (exprEq(r))
    {
        if (exprAndAux(r))
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
bool exprAndAux(Ret *r)
{
    Token *start = iTk;

    if (consume(AND))
    {
        Ret right;
        if (exprEq(&right))
        {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst))
                tkerr(iTk, "Invalid operand type for &&");
            *r = (Ret){{TB_INT, NULL, -1}, false, true};
            if (exprAndAux(&right))
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
bool exprEq(Ret *r)
{
    Token *start = iTk;

    if (exprRel(r))
    {
        if (exprEqAux(r))
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
bool exprEqAux(Ret *r)
{
    Token *start = iTk;

    if (consume(EQUAL))
    {
        Ret right;
        if (exprRel(&right))
        {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst))
                tkerr(iTk, "Invalid operand type for ==");
            *r = (Ret){{TB_INT, NULL, -1}, false, true};
            if (exprEqAux(r))
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
        Ret right;
        if (exprRel(&right))
        {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst))
                tkerr(iTk, "Invalid operand type for !=");
            *r = (Ret){{TB_INT, NULL, -1}, false, true};
            if (exprEqAux(r))
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
bool exprRel(Ret *r)
{
    Token *start = iTk;

    if (exprAdd(r))
    {
        if (exprRelAux(r))
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
bool exprRelAux(Ret *r)
{
    Token *start = iTk;

    if (consume(LESS))
    {
        Ret right;

        if (exprAdd(&right))
        {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst))
                tkerr(iTk, "Invalid operand type for <");
            *r = (Ret){{TB_INT, NULL, -1}, false, true};
            if (exprRelAux(r))
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
        Ret right;

        if (exprAdd(&right))
        {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst))
                tkerr(iTk, "Invalid operand type for <=");
            *r = (Ret){{TB_INT, NULL, -1}, false, true};
            if (exprRelAux(r))
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
        Ret right;
        if (exprAdd(&right))
        {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst))
                tkerr(iTk, "Invalid operand type for >");
            *r = (Ret){{TB_INT, NULL, -1}, false, true};
            if (exprRelAux(r))
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
        Ret right;

        if (exprAdd(&right))
        {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst))
                tkerr(iTk, "Invalid operand type for >=");
            *r = (Ret){{TB_INT, NULL, -1}, false, true};
            if (exprRelAux(r))
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
bool exprAdd(Ret *r)
{
    Token *start = iTk;

    if (exprMul(r))
    {
        if (exprAddAux(r))
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
bool exprAddAux(Ret *r)
{
    Token *start = iTk;

    if (consume(ADD))
    {
        Ret right;
        if (exprMul(&right))
        {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst))
                tkerr(iTk, "Invalid operand type for +");
            *r = (Ret){tDst, false, true};
            if (exprAddAux(r))
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
        Ret right;
        if (exprMul(&right))
        {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst))
                tkerr(iTk, "Invalid operand type for -");
            *r = (Ret){tDst, false, true};
            if (exprAddAux(r))
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
bool exprMul(Ret *r)
{
    Token *start = iTk;

    if (exprCast(r))
    {
        if (exprMulAux(r))
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
bool exprMulAux(Ret *r)
{
    Token *start = iTk;

    if (consume(MUL))
    {
        Ret right;

        if (exprCast(&right))
        {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst))
                tkerr(iTk, "Invalid operand type for *");
            *r = (Ret){tDst, false, true};
            if (exprMulAux(r))
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
        Ret right;

        if (exprCast(&right))
        {
            Type tDst;
            if (!arithTypeTo(&r->type, &right.type, &tDst))
                tkerr(iTk, "Invalid operand type for * or /");
            *r = (Ret){tDst, false, true};
            if (exprMulAux(r))
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
bool exprCast(Ret *r)
{
    Token *start = iTk;

    if (consume(LPAR))
    {
        Type t;
        Ret op;

        if (typeBase(&t))
        {
            if (arrayDecl(&t))
            {
            }
            if (consume(RPAR))
            {
                if (exprCast(&op))
                {
                    if (t.tb == TB_STRUCT)
                        tkerr(iTk, "Cannot convert to a struct type");
                    if (op.type.tb == TB_STRUCT)
                        tkerr(iTk, "Cannot convert a struct");
                    if (op.type.n >= 0 && t.n < 0)
                        tkerr(iTk, "An array can be converted only to another array");
                    if (op.type.n < 0 && t.n >= 0)
                        tkerr(iTk, "A scalar can be converted only to another scalar");
                    *r = (Ret){t, false, true};
                    return true;
                }
                else
                    tkerr(iTk, "Invalid expression after ) in cast expression \n");
            }
            else
                tkerr(iTk, "Missing ) from cast expression \n");
        }
    }

    if (exprUnary(r))
    {
        return true;
    }

    iTk = start;
    return false;
}

// exprUnary: ( SUB | NOT ) exprUnary | exprPostfix
bool exprUnary(Ret *r)
{
    Token *start = iTk;

    if (consume(SUB))
    {
        if (exprUnary(r))
        {
            if (!canBeScalar(r))
                tkerr(iTk, "Unary - must have a scalar operand");
            r->lval = false;
            r->ct = true;
            return true;
        }
        else
            tkerr(iTk, "Invalid expression after - \n");
    }

    else if (consume(NOT))
    {
        if (exprUnary(r))
        {
            if (!canBeScalar(r))
                tkerr(iTk, "unary - must have a scalar operand");
            r->lval = false;
            r->ct = true;
            return true;
        }
        else
            tkerr(iTk, "Invalid expression after ! \n");
    }

    if (exprPostfix(r))
    {
        return true;
    }

    iTk = start;
    return false;
}

// exprPostfix: exprPostfix LBRACKET expr RBRACKET | exprPostfix DOT ID | exprPrimary
// exprPostfix: exprPrimary exprPostfixAux
bool exprPostfix(Ret *r)
{
    Token *start = iTk;

    if (exprPrimary(r))
    {
        if (exprPostfixAux(r))
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
bool exprPostfixAux(Ret *r)
{
    Token *start = iTk;

    if (consume(LBRACKET))
    {
        Ret idx;
        if (expr(&idx))
        {
            if (consume(RBRACKET))
            {
                if (r->type.n < 0)
                    tkerr(iTk, "Only an array can be indexed");
                Type tInt = {TB_INT, NULL, -1};
                if (!convTo(&idx.type, &tInt))
                    tkerr(iTk, "The index is not convertible to int");
                r->type.n = -1;
                r->lval = true;
                r->ct = false;
                if (exprPostfixAux(r))
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
            Token *tkName = consumedTk;
            if (r->type.tb != TB_STRUCT)
                tkerr(iTk, "A field can only be selected from a struct");
            Symbol *s = findSymbolInList(r->type.s->structMembers, tkName->text);
            if (!s)
                tkerr(iTk, "The structure %s does not have a field %s", r->type.s->name, tkName->text);
            *r = (Ret){s->type, true, s->type.n >= 0};
            if (exprPostfixAux(r))
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
bool exprPrimary(Ret *r)
{
    Token *start = iTk;

    if (consume(ID))
    {
        Token *tkName = consumedTk;
        Symbol *s = findSymbol(tkName->text);
        if (!s)
            tkerr(iTk, "Undefined id: %s", tkName->text);
        if (consume(LPAR))
        {
            if (s->kind != SK_FN)
                tkerr(iTk, "Only a function can be called");
            Ret rArg;
            Symbol *param = s->fn.params;
            if (expr(&rArg))
            {
                if (!param)
                    tkerr(iTk, "Too many arguments in function call");
                if (!convTo(&rArg.type, &param->type))
                    tkerr(iTk, "In call, cannot convert the argument type to the parameter type");
                param = param->next;
                for (;;)
                {
                    if (consume(COMMA))
                    {
                        if (expr(&rArg))
                        {
                            if (!param)
                                tkerr(iTk, "Too many arguments in function call");
                            if (!convTo(&rArg.type, &param->type))
                                tkerr(iTk, "In call, cannot convert the argument type to the parameter type");
                            param = param->next;
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
                if (param)
                    tkerr(iTk, "too few arguments in function call");
                *r = (Ret){s->type, false, true};
                return true;
            }
            else
                tkerr(iTk, "Missing ) after expression \n");
        }

        if (s->kind == SK_FN)
            tkerr(iTk, "A function can only be called");
        *r = (Ret){s->type, true, s->type.n >= 0};
        return true;
    }

    if (consume(CT_INT))
    {
        *r = (Ret){{TB_INT, NULL, -1}, false, true};
        return true;
    }

    if (consume(CT_REAL))
    {
        *r = (Ret){{TB_DOUBLE, NULL, -1}, false, true};
        return true;
    }

    if (consume(CT_CHAR))
    {
        *r = (Ret){{TB_CHAR, NULL, -1}, false, true};
        return true;
    }

    if (consume(CT_STRING))
    {
        *r = (Ret){{TB_CHAR, NULL, 0}, false, true};
        return true;
    }

    if (consume(LPAR))
    {
        if (expr(r))
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
    mvInit();
    iTk = tokens;
    if (unit())
    {
        printf("Syntax check has completed successfully\n");
    }
    // showDomain(symTable, "global");
    // genTestProgram();
    genTestFloat();
    run();
    dropDomain();
    return 0;
}
