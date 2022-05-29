#pragma once

// masina virtuala stiva

// instructiunile masinii virtuale
// FORMAT: OP_<name>.<data_type>    // [argument] effect
//		OP_ - prefix comun (operation code)
//		<name> - numele instructiunii
//		<data_type> - daca este prezent, tipul de date asupra caruia actioneaza instructiunea
//			.i - int
//			.f - double
//			.c - char
//			.p - pointer
//		[argument] - daca este prezent, argumentul instructiunii
//		effect - efectul instructiunii
enum
{
    OP_HALT // incheie executia codului
    ,
    OP_PUSH_I // [ct.i] depune pe stiva constanta ct.i
    ,
    OP_CALL // [idx] apeleaza o functie care se afla la indexul idx in vectorul de instructiuni
    ,
    OP_CALL_EXT // [addr] apeleaza o functie din interiorul compilatorului, de la adresa addr
    ,
    OP_ENTER // [nb_locals] creaza cadrul unei functii cu nb variabile locale
    ,
    OP_RET // [nb_params] revine dintr-o functie care are nb parametri si returneaza o valoare
    ,
    OP_RET_VOID // [nb_params] revine dintr-o functie care are nb parametri, fara sa returneze o valoare
    ,
    OP_CONV_I_F // converteste valoarea de pe stiva de la int la double
    ,
    OP_JMP // salt neconditionat la indexul specificat
    ,
    OP_JF // salt la indexul specificat daca pe stiva este false
    ,
    OP_JT // salt la indexul specificat daca pe stiva este true
    ,
    OP_FPLOAD // [idx] depune pe stiva valoarea de la FP[idx]
    ,
    OP_FPSTORE // [idx] stocheaza la FP[idx] valoarea din varful stivei
    ,
    OP_ADD_I // aduna 2 valori intregi din varful stivei si depune rezultatul in stiva
    ,
    OP_LESS_I // compara 2 valori intregi din varful stivei si depune rezultatul in stiva ca int
    ,

    // instructiuni adaugate pentru a putea lucra mai bine cu double
    OP_PUSH_F,
    OP_LESS_F,
    OP_ADD_F,

};

// o valoare de pe stiva sau argumentul unei instructiuni
typedef union
{
    int i;              // valori intregi si indecsi
    double f;           // valori float
    void *p;            // pointeri
    void (*extFnPtr)(); // pointer la o functie externa
} Val;

// o instructiune a masinii virtuale
typedef struct
{
    int op; // opcode: OP_*
    Val arg;
} Instr;

// vector de instructiuni
extern Instr *instructions;
// nr de instructiuni
extern int nInstructions;

// adauga o noua instructiune in vectorul de instructiuni
// si ii seteaza campul "op".
// Returneaza indexul instructiunii adaugate.
int addInstr(int op);

// adauga o instructiune care are un argument de tip int
int addInstrWithInt(int op, int i);

// adauga o instructiune care are un argument de tip double
int addInstrWithDouble(int op, double d);

// initializeaza masina virtuala
void mvInit();

// Executa codul generat
void run();

// genereaza un program de test
void genTestProgram();

// genereaza program de test
void genTestFloat();