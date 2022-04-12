#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "AtomiLexicali.h"

Token *iTk;
Token *consumedTk;

bool unit();
bool structDef();
bool varDef();
bool typeBase();
bool arrayDecl();
bool fnDef();
bool fnParam();
bool stm();
bool stmCompound();
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
            if (consume(LACC))
            {
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
                        return true;
                    }
                }
            }
        }
    }

    iTk = start;
    return false;
}

// varDef: typeBase ID arrayDecl? SEMICOLON
bool varDef()
{
    Token *start = iTk;

    if (typeBase())
    {
        if (consume(ID))
        {
            if (arrayDecl())
            {
            }
            if (consume(SEMICOLON))
            {
                return true;
            }
        }
    }

    iTk = start;
    return false;
}

// typeBase: INT | DOUBLE | CHAR | STRUCT ID
bool typeBase()
{
    Token *start = iTk;

    if (consume(INT))
    {
        return true;
    }
    if (consume(DOUBLE))
    {
        return true;
    }
    if (consume(CHAR))
    {
        return true;
    }
    if (consume(STRUCT))
    {
        if (consume(ID))
        {
            return true;
        }
        else
            tkerr(iTk, "Struct type declaration error");
    }

    iTk = start;
    return false;
}

// arrayDecl: LBRACKET CT_INT? RBRACKET
bool arrayDecl()
{
    Token *start = iTk;

    if (consume(LBRACKET))
    {
        if (consume(CT_INT))
        {
        }
        if (consume(RBRACKET))
        {
            return true;
        }
    }

    iTk = start;
    return false;
}

// fnDef: ( typeBase | VOID ) ID LPAR ( fnParam (COMMA fnParam)* )? RPAR stmCompound
bool fnDef()
{
    Token *start = iTk;

    if (typeBase() || consume(VOID))
    {
        if (consume(ID))
        {
            if (consume(LPAR))
            {
                if (fnParam())
                {
                    for (;;)
                    {
                        if (consume(COMMA))
                        {
                            if (fnParam())
                            {
                            }
                        }
                        else
                            break;
                    }
                }
                if (consume(RPAR))
                {
                    if (stmCompound())
                    {
                        return true;
                    }
                }
            }
        }
    }

    iTk = start;
    return false;
}

// fnParam: typeBase ID arrayDecl?
bool fnParam()
{
    Token *start = iTk;

    if (typeBase())
    {
        if (consume(ID))
        {
            if (arrayDecl())
            {
            }
            return true;
        }
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

    if (stmCompound())
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
                            }
                        }

                        return true;
                    }
                }
            }
        }
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
                }
            }
        }
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
                    }
                }
            }
        }
    }

    if (consume(BREAK))
    {
        if (consume(SEMICOLON))
        {
            return true;
        }
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
    }

    if (expr())
    {
    }
    if (consume(SEMICOLON))
    {
        return true;
    }

    iTk = start;
    return false;
}

// stmCompound: LACC ( varDef | stm )* RACC
bool stmCompound()
{
    Token *start = iTk;

    if (consume(LACC))
    {
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
            return true;
        }
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
        }
    }

    if (exprOr())
    {
        return true;
    }

    iTk = start;
    return false;
}
