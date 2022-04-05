#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "AtomiLexicali.h"

Token *iTk;
Token *consumedTk;

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
        else err("Struct type declaration error");
    }

    iTk = start;
    return false;
}

bool fnParam();