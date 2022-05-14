#define SAFEALLOC(var, Type)                          \
    if ((var = (Type *)malloc(sizeof(Type))) == NULL) \
        err("not enough memory ");

#define SAFEALLOCN(var, Type, n)                            \
    if ((var = (Type *)malloc(sizeof(Type) * (n))) == NULL) \
        err("not enough memory ");
enum
{
    ID,
    CT_INT,
    CT_REAL,
    CT_CHAR,
    CT_STRING,
    END,
    BREAK,
    CHAR,
    DOUBLE,
    ELSE,
    FOR,
    IF,
    INT,
    RETURN,
    STRUCT,
    VOID,
    WHILE,
    COMMA,
    SEMICOLON,
    LPAR,
    RPAR,
    LBRACKET,
    RBRACKET,
    LACC,
    RACC,
    ADD,
    SUB,
    MUL,
    DIV,
    DOT,
    AND,
    OR,
    NOT,
    ASSIGN,
    EQUAL,
    NOTEQ,
    LESS,
    LESSEQ,
    GREATER,
    GREATEREQ
};

typedef struct _Token
{
    int code;
    union
    {
        char *text;
        int i;
        double r;
    };
    int line;
    struct _Token *next;
} Token;

void err(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}

void tkerr(const Token *tk, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error in line %d: ", tk->line);
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}
