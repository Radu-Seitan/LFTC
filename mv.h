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

    // instructiuni adaugate pentru generarea de cod
    OP_PUSH_F // [ct.i] depune pe stiva constanta ct.i
    ,
    OP_CONV_F_I // converteste valoarea de pe stiva de la double la int
    ,
    OP_LOAD_I // preia o adresa din stiva si depune in locul ei valoarea intreaga de la aceasta adresa
    ,
    OP_LOAD_F // preia o adresa din stiva si depune in locul ei valoarea reala de la aceasta adresa
    ,
    OP_STORE_I // preia din stiva o adresa si o valoare intreaga si depune valoarea la adresa specificata. Lasa pe stiva valoarea.
    ,
    OP_STORE_F // preia din stiva o adresa si o valoare reala si depune valoarea la adresa specificata. Lasa pe stiva valoarea.
    ,
    OP_ADDR // [idx] depune pe stiva adresa lui globalMemory[idx]
    ,
    OP_FPADDR_I // [idx] depune pe stiva adresa lui FP[idx].i
    ,
    OP_FPADDR_F // [idx] depune pe stiva adresa lui FP[idx].f
    ,
    OP_ADD_F // aduna 2 valori reale din varful stivei si depune rezultatul in stiva
    ,
    OP_SUB_I // scade 2 valori intregi din varful stivei si depune rezultatul in stiva
    ,
    OP_SUB_F // scade 2 valori reale din varful stivei si depune rezultatul in stiva
    ,
    OP_MUL_I // inmulteste 2 valori intregi din varful stivei si depune rezultatul in stiva
    ,
    OP_MUL_F // inmulteste 2 valori reale din varful stivei si depune rezultatul in stiva
    ,
    OP_DIV_I // imparte 2 valori intregi din varful stivei si depune rezultatul in stiva
    ,
    OP_DIV_F // imparte 2 valori reale din varful stivei si depune rezultatul in stiva
    ,
    OP_LESS_F // compara 2 valori reale din varful stivei si depune rezultatul in stiva ca int
    ,
    OP_DROP // sterge valoarea din varful stivei

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

// insereaza la pozitia specificata o noua instructiune in vectorul de instructiuni
// si ii seteaza campul "op".
// Returneaza "pos".
int insertInstr(int pos, int op);

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