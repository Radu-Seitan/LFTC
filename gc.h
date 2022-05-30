#pragma once

// generarea de cod

#include "at.h"
#include "mv.c"

// insereaza la pozitia specificata o instructiune de conversie,
// doar daca este necesar
void insertConvIfNeeded(int pos, Type *srcType, Type *dstType);

// daca lval este true, se genereaza rval din valoarea curenta de pe stiva
void addRVal(bool lval, Type *type);