#include "gc.h"

void insertConvIfNeeded(int pos, Type *srcType, Type *dstType)
{
    switch (srcType->tb)
    {
    case TB_INT:
        switch (dstType->tb)
        {
        case TB_DOUBLE:
            insertInstr(pos, OP_CONV_I_F);
            break;
        }
        break;
    case TB_DOUBLE:
        switch (dstType->tb)
        {
        case TB_INT:
            insertInstr(pos, OP_CONV_F_I);
            break;
        }
        break;
    }
}

void addRVal(bool lval, Type *type)
{
    if (!lval)
        return;
    switch (type->tb)
    {
    case TB_INT:
        addInstr(OP_LOAD_I);
        break;
    case TB_DOUBLE:
        addInstr(OP_LOAD_F);
        break;
    }
}