#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "AtomiLexicali.h"

Token *tokens = NULL;
Token *lastToken = NULL;
char inBuffer[30001];
int line = 1;
char *pCrtCh;

Token *addTk(int code)
{
    Token *tk;
    SAFEALLOC(tk, Token);
    tk->code = code;
    tk->line = line;
    tk->next = NULL;
    if (lastToken)
    {
        lastToken->next = tk;
    }
    else
    {
        tokens = tk;
    }
    lastToken = tk;
    return tk;
}

void readFile(char *filename)
{

    FILE *file;

    if ((file = fopen(filename, "r")) == NULL)
    {
        printf("Eroare la deschidere fisier");
        return EXIT_FAILURE;
    }

    int n = fread(inBuffer, 1, 30000, file);
    inBuffer[n + 1] = '\0';
    fclose(file);
}

char *createString(const char *begin, const char *after)
{
    size_t n = (after - begin);
    char *p;
    SAFEALLOCN(p, char, n + 1);
    memcpy(p, begin, n);
    p[n] = '\0';
    return p;
}

int getNextToken()
{
    int state = 0, nCh;
    char ch;
    const char *pStartCh;
    Token *tk;
    char *intValue;
    char *doubleValue;

    for (;;)
    {
        ch = *pCrtCh;
        switch (state)
        {
        case 0:
            if (isalpha(ch) || ch == '_')
            {
                pStartCh = pCrtCh; // memoreaza inceputul ID-ului
                pCrtCh++;
                state = 1;
            }
            else if (isdigit(ch))
            {
                pStartCh = pCrtCh; // memoreaza inceputul INT-ului
                pCrtCh++;
                state = 3;
            }
            else if (ch == ' ' || ch == '\r' || ch == '\t')
            {
                pCrtCh++;
            }
            else if (ch == '\n')
            {
                line++;
                pCrtCh++;
            }
            else if (ch == '\'')
            {
                pStartCh = pCrtCh + 1; // memoreaza inceputul CHAR-ului
                pCrtCh++;
                state = 11;
            }
            else if (ch == '\"')
            {
                pStartCh = pCrtCh + 1; // memoreaza inceputul STRING-ului
                pCrtCh++;
                state = 14;
            }
            else if (ch == '\/')
            {
                pCrtCh++;
                state = 16;
            }
            else if (ch == ',')
            {
                pCrtCh++;
                state = 18;
            }
            else if (ch == ';')
            {
                pCrtCh++;
                state = 19;
            }
            else if (ch == '(')
            {
                pCrtCh++;
                state = 20;
            }
            else if (ch == ')')
            {
                pCrtCh++;
                state = 21;
            }
            else if (ch == '[')
            {
                pCrtCh++;
                state = 22;
            }
            else if (ch == ']')
            {
                pCrtCh++;
                state = 23;
            }
            else if (ch == '{')
            {
                pCrtCh++;
                state = 24;
            }
            else if (ch == '}')
            {
                pCrtCh++;
                state = 25;
            }
            else if (ch == '\0' || ch == EOF)
            {
                pCrtCh++;
                state = 26;
            }
            else if (ch == '+')
            {
                pCrtCh++;
                state = 27;
            }
            else if (ch == '-')
            {
                pCrtCh++;
                state = 28;
            }
            else if (ch == '*')
            {
                pCrtCh++;
                state = 29;
            }
            else if (ch == '.')
            {
                pCrtCh++;
                state = 31;
            }
            else if (ch == '&')
            {
                pCrtCh++;
                state = 32;
            }
            else if (ch == '|')
            {
                pCrtCh++;
                state = 34;
            }
            else if (ch == '!')
            {
                pCrtCh++;
                state = 36;
            }
            else if (ch == '=')
            {
                pCrtCh++;
                state = 39;
            }
            else if (ch == '<')
            {
                pCrtCh++;
                state = 42;
            }
            else if (ch == '>')
            {
                pCrtCh++;
                state = 45;
            }
            break;
        case 1:
            if (isalnum(ch) || ch == '_')
            {
                pCrtCh++;
            }
            else
                state = 2;
            break;
        case 2:
            nCh = pCrtCh - pStartCh;
            // Teste cuvinte cheie
            if (nCh == 5 && !memcmp(pStartCh, "break", 5))
                tk = addTk(BREAK);
            else if (nCh == 4 && !memcmp(pStartCh, "char", 4))
                tk = addTk(CHAR);
            else if (nCh == 6 && !memcmp(pStartCh, "double", 6))
                tk = addTk(DOUBLE);
            else if (nCh == 4 && !memcmp(pStartCh, "else", 4))
                tk = addTk(ELSE);
            else if (nCh == 3 && !memcmp(pStartCh, "for", 3))
                tk = addTk(FOR);
            else if (nCh == 2 && !memcmp(pStartCh, "if", 2))
                tk = addTk(IF);
            else if (nCh == 3 && !memcmp(pStartCh, "int", 3))
                tk = addTk(INT);
            else if (nCh == 6 && !memcmp(pStartCh, "return", 6))
                tk = addTk(RETURN);
            else if (nCh == 6 && !memcmp(pStartCh, "struct", 6))
                tk = addTk(STRUCT);
            else if (nCh == 4 && !memcmp(pStartCh, "void", 4))
                tk = addTk(VOID);
            else if (nCh == 5 && !memcmp(pStartCh, "while", 5))
                tk = addTk(WHILE);
            else
            {
                tk = addTk(ID);
                tk->text = createString(pStartCh, pCrtCh);
            }
            return tk->code;
        case 3:
            if (isdigit(ch))
            {
                pCrtCh++;
            }
            else if (ch == '.')
            {
                pCrtCh++;
                state = 5;
            }
            else if (ch == 'e' || ch == 'E')
            {
                pCrtCh++;
                state = 8;
            }
            else
                state = 4;
            break;
        case 4:
            tk = addTk(CT_INT);
            intValue = createString(pStartCh, pCrtCh);
            tk->i = strtol(intValue, NULL, 10);
            return CT_INT;
        case 5:
            if (isdigit(ch))
            {
                pCrtCh++;
                state = 6;
            }
            else
                err("Format invalid");
            break;
        case 6:
            if (isdigit(ch))
            {
                pCrtCh++;
            }
            else if (ch == 'e' || ch == 'E')
            {
                pCrtCh++;
                state = 8;
            }
            else
                state = 7;
            break;
        case 7:
            tk = addTk(CT_REAL);
            doubleValue = createString(pStartCh, pCrtCh);
            tk->r = atof(doubleValue);
            return CT_REAL;
        case 8:
            if (ch == '+' || ch == '-')
            {
                pCrtCh++;
                state = 9;
            }
            else
                state = 9;
            break;
        case 9:
            if (isdigit(ch))
            {
                pCrtCh++;
                state = 10;
            }
            break;
        case 10:
            if (isdigit(ch))
            {
                pCrtCh++;
            }
            else
                state = 7;
            break;
        case 11:
            if (ch != '\'')
            {
                pCrtCh++;
                state = 12;
            }
            break;
        case 12:
            if (ch == '\'')
            {
                pCrtCh++;
                state = 13;
            }
            break;
        case 13:
            tk = addTk(CT_CHAR);
            intValue = createString(pStartCh, pCrtCh - 1);
            tk->i = *intValue;
            return CT_CHAR;
        case 14:
            if (ch != '\"')
            {
                pCrtCh++;
            }
            else
            {
                pCrtCh++;
                state = 15;
            }
            break;
        case 15:;
            tk = addTk(CT_STRING);
            tk->text = createString(pStartCh, pCrtCh - 1);
            return CT_STRING;
        case 16:
            if (ch == '\/')
            {
                pCrtCh++;
                state = 17;
            }
            else
                state = 30;
            break;
        case 17:
            if (!(ch == '\n' || ch == '\r' || ch == '\t'))
            {
                pCrtCh++;
            }
            else
                state = 0;
            break;
        case 18:
            addTk(COMMA);
            return COMMA;
        case 19:
            addTk(SEMICOLON);
            return SEMICOLON;
        case 20:
            addTk(LPAR);
            return LPAR;
        case 21:
            addTk(RPAR);
            return RPAR;
        case 22:
            addTk(LBRACKET);
            return LBRACKET;
        case 23:
            addTk(RBRACKET);
            return RBRACKET;
        case 24:
            addTk(LACC);
            return LACC;
        case 25:
            addTk(RACC);
            return RACC;
        case 26:
            addTk(END);
            return END;
        case 27:
            addTk(ADD);
            return ADD;
        case 28:
            addTk(SUB);
            return SUB;
        case 29:
            addTk(MUL);
            return MUL;
        case 30:
            addTk(DIV);
            return DIV;
        case 31:
            addTk(DOT);
            return DOT;
        case 32:
            if (ch == '&')
            {
                pCrtCh++;
                state = 33;
            }
            else
                err("Caracter invalid");
            break;
        case 33:
            addTk(AND);
            return AND;
        case 34:
            if (ch == '|')
            {
                pCrtCh++;
                state = 35;
            }
            else
                err("Caracter invalid");
            break;
        case 35:
            addTk(OR);
            return OR;
        case 36:
            if (ch == '=')
            {
                pCrtCh++;
                state = 38;
            }
            else
                state = 37;
            break;
        case 37:
            addTk(NOT);
            return NOT;
        case 38:
            addTk(NOTEQ);
            return NOTEQ;
        case 39:
            if (ch == '=')
            {
                pCrtCh++;
                state = 41;
            }
            else
                state = 40;
            break;
        case 40:
            addTk(ASSIGN);
            return ASSIGN;
        case 41:
            addTk(EQUAL);
            return EQUAL;
        case 42:
            if (ch == '=')
            {
                pCrtCh++;
                state = 44;
            }
            else
                state = 43;
            break;
        case 43:
            addTk(LESS);
            return LESS;
        case 44:
            addTk(LESSEQ);
            return LESSEQ;
        case 45:
            if (ch == '=')
            {
                pCrtCh++;
                state = 47;
            }
            else
                state = 46;
            break;
        case 46:
            addTk(GREATER);
            return GREATER;
        case 47:
            addTk(GREATEREQ);
            return GREATEREQ;
        default:
            err("Stare necunoscuta");
        }
    }
}

void printToken(int code)
{
    switch (code)
    {
    case 0:
        printf("ID");
        break;
    case 1:
        printf("CT_INT");
        break;
    case 2:
        printf("CT_REAL");
        break;
    case 3:
        printf("CT_CHAR");
        break;
    case 4:
        printf("CT_STRING");
        break;
    case 5:
        printf("END");
        break;
    case 6:
        printf("BREAK");
        break;
    case 7:
        printf("CHAR");
        break;
    case 8:
        printf("DOUBLE");
        break;
    case 9:
        printf("ELSE");
        break;
    case 10:
        printf("FOR");
        break;
    case 11:
        printf("IF");
        break;
    case 12:
        printf("INT");
        break;
    case 13:
        printf("RETURN");
        break;
    case 14:
        printf("STRUCT");
        break;
    case 15:
        printf("VOID");
        break;
    case 16:
        printf("WHILE");
        break;
    case 17:
        printf("COMMA");
        break;
    case 18:
        printf("SEMICOLON");
        break;
    case 19:
        printf("LPAR");
        break;
    case 20:
        printf("RPAR");
        break;
    case 21:
        printf("LBRACKET");
        break;
    case 22:
        printf("RBRACKET");
        break;
    case 23:
        printf("LACC");
        break;
    case 24:
        printf("RACC");
        break;
    case 25:
        printf("ADD");
        break;
    case 26:
        printf("SUB");
        break;
    case 27:
        printf("MUL");
        break;
    case 28:
        printf("DIV");
        break;
    case 29:
        printf("DOT");
        break;
    case 30:
        printf("AND");
        break;
    case 31:
        printf("OR");
        break;
    case 32:
        printf("NOT");
        break;
    case 33:
        printf("ASSIGN");
        break;
    case 34:
        printf("EQUAL");
        break;
    case 35:
        printf("NOTEQ");
        break;
    case 36:
        printf("LESS");
        break;
    case 37:
        printf("LESSEQ");
        break;
    case 38:
        printf("GREATER");
        break;
    case 39:
        printf("GREATEREQ");
        break;
    }
}

void showAtoms()
{
    Token *atom = tokens;
    while (atom != NULL)
    {
        printf("%d\t", atom->line);
        if (atom->code == CT_INT)
        {
            printToken(atom->code);
            printf(":%ld\n", atom->i);
        }
        else if (atom->code == CT_REAL)
        {
            printToken(atom->code);
            printf(":%lf\n", atom->r);
        }
        else if (atom->code == ID)
        {
            printToken(atom->code);
            printf(":%s\n", atom->text);
        }
        else if (atom->code == CT_STRING)
        {
            printToken(atom->code);
            printf(":%s\n", atom->text);
        }
        else if (atom->code == CT_CHAR)
        {
            printToken(atom->code);
            printf(":%c\n", (char)atom->i);
        }
        else
        {
            printToken(atom->code);
            printf("\n");
        }
        atom = atom->next;
    }
}

// int main(int argc, char const *argv[])
// {
//     readFile(argv[1]);
//     pCrtCh = inBuffer;
//     while (getNextToken() != END)
//     {
//     }
//     showAtoms();
//     return 0;
// }
