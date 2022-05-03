#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "ALEX.c"

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
            tkerr(iTk, "Missing struct name\n");
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
                            else
                                tkerr(iTk, "Missing argument from the function header\n");
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

    if (consume(LPAR))
    {
        if (typeBase())
        {
            if (arrayDecl())
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

    iTk = tokens;
    if (unit())
    {
        printf("Syntax is good\n");
    }
    return 0;
}
