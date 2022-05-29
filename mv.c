#include "ad.h"
#include "mv.h"
#include "ad.c"

Instr *instructions = NULL;
int nInstructions = 0;

int addInstr(int op)
{
    Instr *p = (Instr *)realloc(instructions, (nInstructions + 1) * sizeof(Instr));
    if (!p)
        err("not enough memory");
    instructions = p;
    Instr *i = instructions + nInstructions;
    i->op = op;
    return nInstructions++;
}

int addInstrWithInt(int op, int i)
{
    int pos = addInstr(op);
    instructions[pos].arg.i = i;
    return pos;
}

int addInstrWithDouble(int op, double d)
{
    int pos = addInstr(op);
    instructions[pos].arg.f = d;
    return pos;
}

Val stack[10000]; // stiva
Val *SP;          // Stack pointer - varful stivei - indica intotdeauna valoarea din varful stivei

void pushv(Val v)
{
    if (SP + 1 == stack + 10000)
        err("trying to push into a full stack");
    *++SP = v;
}

Val popv()
{
    if (SP == stack - 1)
        err("trying to pop from empty stack");
    return *SP--;
}

void pushi(int i)
{
    if (SP + 1 == stack + 10000)
        err("trying to push into a full stack");
    (++SP)->i = i;
}

int popi()
{
    if (SP == stack - 1)
        err("trying to pop from empty stack");
    return SP--->i;
}

double popf()
{
    if (SP == stack - 1)
        err("trying to pop from empty stack");
    return SP--->f;
}

void pushf(double f)
{
    if (SP + 1 == stack + 10000)
        err("trying to push into a full stack");
    (++SP)->f = f;
}

void pushp(void *p)
{
    if (SP + 1 == stack + 10000)
        err("trying to push into a full stack");
    (++SP)->p = p;
}

void *popp()
{
    if (SP == stack - 1)
        err("trying to pop from empty stack");
    return SP--->p;
}

void put_i()
{
    printf("=> %d", popi());
}

void put_d()
{
    printf("=> %lg", popf());
}

void mvInit()
{
    Symbol *fn = addExtFn("put_i", put_i, (Type){TB_VOID, NULL, -1});
    addFnParam(fn, "i", (Type){TB_INT, NULL, -1});

    Symbol *fn2 = addExtFn("put_d", put_d, (Type){TB_VOID, NULL, -1});
    addFnParam(fn2, "d", (Type){TB_DOUBLE, NULL, -1});
}

void run()
{
    SP = stack - 1;
    Val *FP = NULL;           // valoarea initiala nu conteaza
    Instr *IP = instructions; // Instruction pointer - pointer la instructiunea curenta
    Val v;
    int iArg, iTop, iBefore;
    double dArg, dTop, dBefore;
    void (*extFnPtr)();
    for (;;)
    {
        // afiseaza indexul instructiunii curente
        // si numarul de valori din stiva
        printf("%03d/%d\t", (int)(IP - instructions), (int)(SP - stack + 1));
        switch (IP->op)
        {
        case OP_HALT:
            printf("HALT");
            return;
        case OP_PUSH_I:
            printf("PUSH.i\t%d", IP->arg.i);
            pushi(IP->arg.i);
            IP++;
            break;
        case OP_CALL:
            pushp(IP + 1);
            printf("CALL\t%d", IP->arg.i);
            IP = instructions + IP->arg.i;
            break;
        case OP_CALL_EXT:
            extFnPtr = IP->arg.extFnPtr;
            printf("CALL_EXT\t%p\n", extFnPtr);
            extFnPtr();
            IP++;
            break;
        case OP_ENTER:
            pushp(FP);
            FP = SP;
            SP += IP->arg.i;
            printf("ENTER\t%d", IP->arg.i);
            IP++;
            break;
        case OP_RET_VOID:
            iArg = IP->arg.i;
            printf("RET_VOID\t%d", iArg);
            IP = FP[-1].p;
            SP = FP - iArg - 2;
            FP = FP[0].p;
            break;
        case OP_JMP:
            printf("JMP\t%d", IP->arg.i);
            IP = instructions + IP->arg.i;
            break;
        case OP_JF:
            iTop = popi();
            printf("JF\t%d\t// %d", IP->arg.i, iTop);
            IP = iTop ? IP + 1 : instructions + IP->arg.i;
            break;
        case OP_FPLOAD:
            v = FP[IP->arg.i];
            pushv(v);
            printf("FPLOAD\t%d\t// i:%d, f:%g", IP->arg.i, v.i, v.f);
            IP++;
            break;
        case OP_FPSTORE:
            v = popv();
            FP[IP->arg.i] = v;
            printf("FPSTORE\t%d\t// i:%d, f:%g", IP->arg.i, v.i, v.f);
            IP++;
            break;
        case OP_ADD_I:
            iTop = popi();
            iBefore = popi();
            pushi(iBefore + iTop);
            printf("ADD.i\t// %d+%d -> %d", iBefore, iTop, iBefore + iTop);
            IP++;
            break;
        case OP_LESS_I:
            iTop = popi();
            iBefore = popi();
            pushi(iBefore < iTop);
            printf("LESS.i\t// %d<%d -> %d", iBefore, iTop, iBefore < iTop);
            IP++;
            break;
        case OP_PUSH_F:
            printf("PUSH.f\t%g", IP->arg.f);
            pushf(IP->arg.f);
            IP++;
            break;
        case OP_LESS_F:
            dTop = popf();
            dBefore = popf();
            pushi(dBefore < dTop);
            printf("LESS.f\t// %lg<%lg -> %d", dBefore, dTop, dBefore < dTop);
            IP++;
            break;
        case OP_ADD_F:
            dTop = popf();
            dBefore = popf();
            pushf(dTop + dBefore);
            printf("ADD.f\t// %lg+%lg -> %lg", dBefore, dTop, dBefore + dTop);
            IP++;
            break;
        default:
            err("run: instructiune neimplementata: %d", IP->op);
        }
        putchar('\n');
    }
}

/* Programul codifica urmatorul cod sursa:
f(2);
void f(int n){		// cadru: n[-2] ret[-1] oldFP[0] i[1]
    int i;
    for(i=0;i<n;i=i+1){
        put_i(i);
        }
    }
*/
void genTestProgram()
{
    addInstrWithInt(OP_PUSH_I, 2);
    int callPos = addInstr(OP_CALL);
    addInstr(OP_HALT);
    instructions[callPos].arg.i = nInstructions;
    addInstrWithInt(OP_ENTER, 1);
    // i=0;  (componentele lui for sunt implementate sub forma unui while)
    addInstrWithInt(OP_PUSH_I, 0);
    addInstrWithInt(OP_FPSTORE, 1);
    // while(i<n){
    int whilePos = addInstrWithInt(OP_FPLOAD, 1);
    addInstrWithInt(OP_FPLOAD, -2);
    addInstr(OP_LESS_I);
    int jfAfterFor = addInstr(OP_JF);
    // put_i(i);
    addInstrWithInt(OP_FPLOAD, 1);
    Symbol *s = findSymbol("put_i");
    if (!s)
        err("undefined: put_i");
    int putiCall = addInstr(OP_CALL_EXT);
    instructions[putiCall].arg.extFnPtr = s->fn.extFnPtr;
    // i=i+1
    addInstrWithInt(OP_FPLOAD, 1);
    addInstrWithInt(OP_PUSH_I, 1);
    addInstr(OP_ADD_I);
    addInstrWithInt(OP_FPSTORE, 1);
    // } ( urmatoarea iteratie )
    addInstrWithInt(OP_JMP, whilePos);
    // revenire din functie
    instructions[jfAfterFor].arg.i = nInstructions;
    addInstrWithInt(OP_RET_VOID, 1);
}

/*
f(2.0);
void f(double n)
{
    double i;
    for (i = 0.0; i < n; i = i + 0.5)
    {
        put_d(i);
    }
}
*/
void genTestFloat()
{
    addInstrWithDouble(OP_PUSH_F, 2.0);
    int callPos = addInstr(OP_CALL);
    addInstr(OP_HALT);
    instructions[callPos].arg.i = nInstructions;
    addInstrWithInt(OP_ENTER, 1);
    // i=0.0;  (componentele lui for sunt implementate sub forma unui while)
    addInstrWithDouble(OP_PUSH_F, 0.0);
    addInstrWithInt(OP_FPSTORE, 1);
    // while(i<n){
    int whilePos = addInstrWithInt(OP_FPLOAD, 1);
    addInstrWithInt(OP_FPLOAD, -2);
    addInstr(OP_LESS_F);
    int jfAfterFor = addInstr(OP_JF);
    // put_d(i);
    addInstrWithInt(OP_FPLOAD, 1);
    Symbol *s = findSymbol("put_d");
    if (!s)
        err("undefined: put_d");
    int putdCall = addInstr(OP_CALL_EXT);
    instructions[putdCall].arg.extFnPtr = s->fn.extFnPtr;
    // i=i+0.5
    addInstrWithInt(OP_FPLOAD, 1);
    addInstrWithDouble(OP_PUSH_F, 0.5);
    addInstr(OP_ADD_F);
    addInstrWithInt(OP_FPSTORE, 1);
    // } ( urmatoarea iteratie )
    addInstrWithInt(OP_JMP, whilePos);
    // revenire din functie
    instructions[jfAfterFor].arg.i = nInstructions;
    addInstrWithInt(OP_RET_VOID, 1);
}