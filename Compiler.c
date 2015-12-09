/*
 * Vatthikorn Apiratitham
 * Amanda Forster
 * Taylor Veith
 *
 * COP3402 - Systems Software - Fall 2015
 *
 * A complete PL/0 compiler with lexical analyzer, syntax analyzer and
 * machine code generator with a PM/0 virtual machine
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_SYMBOL_TABLE_SIZE 100
#define CODE_LENGTH 1000
#define MAX_IDENT_LENGTH 11
#define MAX_DIGITS 5
#define MAX_CODE_LENGTH 500
#define MAX_STACK_HEIGHT 2000
#define MAX_LEXI_LEVELS 10

typedef enum {
    nulsym = 1,
    identsym,
    numbersym,
    plussym,
    minussym,
    multsym,
    slashsym,
    oddsym,
    equalsym,
    neqsym,
    lesssym,
    leqsym,
    gtrsym,
    geqsym,
    lparentsym,
    rparentsym,
    commasym,
    semicolonsym,
    periodsym,
    becomessym,
    beginsym,
    endsym,
    ifsym,
    thensym,
    whilesym,
    dosym,
    callsym,
    constsym,
    varsym,
    procsym,
    writesym,
    readsym,
    elsesym
} token_type;

typedef enum {
    LIT = 1, OPR, LOD, STO, CAL, INC, JMP, JPC, SIO1, SIO2, SIO3
} op_code;

typedef enum {
    NEG = 1, ADD, SUB, MUL, DIV, ODD, MOD, EQL, NEQ, LSS, LEQ, GTR, GEQ
} alu_op;

typedef enum {
    constant = 1, variable, procedure
} kind;

typedef struct {
    char lexeme[MAX_IDENT_LENGTH];
    int token_type;
} Lexeme;

typedef struct symbol {
    int kind;      // const = 1, var = 2, proc = 3
    char name[12]; // name up to 11 chars
    int val;       // value
    int level;     // L level
    int addr;      // M address
} symbol;

struct token {
    //a number of the kind of symbol
    int kind;
    int index;
    char name[12];
};

typedef struct instruction {
    int op;
    int l;
    int m;
} instruction;

instruction code[MAX_CODE_LENGTH];

symbol symbol_table[MAX_SYMBOL_TABLE_SIZE];

void program();
int block();
int constDeclaration();
int varDeclaration();
int procDeclaration();
void parameterBlock();
void parameterList();
void statement();
void condition();
void expression();
void term();
void factor();

struct token getNextToken(char *source);
void getName(char *source, struct symbol*, int);
int findSymbol();
void error(int error);
int relation();
void printMCode();
void printSymbolTable();
void emit(int op, int l, int m);
void updateStackSize(int op, int l, int m);

void initArrayStruct(struct instruction a[]);
void ALU(int m, int stack[], int *sp);
int base(int l, int base, int stack[]);
void printInstructions(struct instruction codeStore[], int stackStore[], int sp, int pc, int bp, FILE *fp);
void printStack(int stack[],int start, int sp, FILE *fp);

int tokenlistSwitch = 0;
int symtableSwitch = 0;
int mcodeSwitch = 0;
int acodeSwitch = 0;
int vmstackSwitch = 0;
int userInput = 0;

FILE *ifp;
FILE *ofp;
struct token token;
char charToken[MAX_IDENT_LENGTH];
char source[CODE_LENGTH];
int symTableIndex = 0;
int stack_size = 0;
int level = -1;
int cx = 0;
int addr = 0;
int params = 0;
int space = 0;
int procedureCount = 0;

void printMCode() {
    
    //ofp = fopen("/Users/Zack/Development/UCF/Systems Software/Assignment4/Assignment4/mcode.txt", "w");

    ofp = fopen("mcode.txt", "w");
    int i = 0;
    while(code[i].op != 0) {
        fprintf(ofp, "%d %d %d\n", code[i].op, code[i].l, code[i].m);
        i++;
    }
    fclose(ofp);
}

void printMCodeScreen() {
    printf("\n");
    int i = 0;
    while(code[i].op != 0) {
        printf("%d %d %d\n", code[i].op, code[i].l, code[i].m);
        i++;
    }
    printf("\n");

}

void printSymbolTable() {
    
    //ofp = fopen("/Users/Zack/Development/UCF/Systems Software/Assignment4/Assignment4/symlist.txt", "w");
    
    ofp = fopen("symboltable.txt", "w");
    
    fprintf(ofp, "Name\t\tType\tLevel\tValue\n");
    
    int i;
    for (i = 0; i < symTableIndex; i++) {
        
        fprintf(ofp, "%-10s\t", symbol_table[i].name);
        
        if (symbol_table[i].kind == constant) {
            fprintf(ofp, "%-7s", "const");
        } else if (symbol_table[i].kind == variable) {
            fprintf(ofp, "%-7s", "var");
        } else {
            fprintf(ofp, "%-7s", "proc");
        }
        
        fprintf(ofp, "\t%3d\t%3d\n", symbol_table[i].level, symbol_table[i].val);
    }
    
    fclose(ofp);
}

void printSymbolTableScreen() {
    
    printf("\nName\t\tType\tLevel\tValue\n");
    
    int i;
    for (i = 0; i < symTableIndex; i++) {
        
        printf("%-10s\t", symbol_table[i].name);
        
        if (symbol_table[i].kind == constant) {
            printf("%-7s", "const");
        } else if (symbol_table[i].kind == variable) {
            printf("%-7s", "var");
        } else {
            printf("%-7s", "proc");
        }
        
        printf("\t%3d\t%3d\n", symbol_table[i].level, symbol_table[i].val);
    }
    
    printf("\n");
}

void getTokenList(char *source) {
    
    //ifp = fopen("/Users/Zack/Development/UCF/Systems Software/Assignment4/Assignment4/tokenlist.txt", "r");
    
    ifp = fopen("tokenlist.txt", "r");

    
    //Read in all the characters into the source code array
    int i = 0;
    while (i < CODE_LENGTH) {
        fscanf(ifp, "%c", &source[i]);
        i++;
    }
    
    //close the file
    fclose(ifp);
    
}

void getCode(char *source) {
    
    //ifp = fopen("/Users/Zack/Development/UCF/Systems Software/Assignment4/Assignment4/input.txt", "r");
    
    ifp = fopen("input.txt", "r");
    
    //Read in all the characters into the source code array
    int i = 0;
    while (i < CODE_LENGTH) {
        fscanf(ifp, "%c", &source[i]);
        i++;
    }
    
    //close the file
    fclose(ifp);
}

//Initialize array to all be a null pointer
void initArray(char a[], int length) {
    int i = 0;
    for (i = 0; i < length; i++) {
        a[i] = '\0';
    }
}

void initIntArray(int a[], int length) {
    int i = 0;
    for (i = 0; i < length; i++) {
        a[i] = 0;
    }
}

void initArrayStruct(struct instruction a[]) {
    int i = 0;
    for (i = 0; i < MAX_CODE_LENGTH; i++) {
        a[i].op = 0;
        a[i].l = 0;
        a[i].m = 0;
    }
}

//Removes comments from the source code
void removeComments(char *source) {
    
    int code_ptr = 0;
    int comment_ptr = 0;
    
    //Big loop for the whole program
    while (source[code_ptr] != '\0') {
        
        //Detecting comments
        if (source[code_ptr] == '/' && source[code_ptr + 1] == '*') {
            code_ptr += 2;
            while (source[code_ptr] != '*' || source[code_ptr + 1] != '/') {
                code_ptr++;
            }
            code_ptr += 2;
        }
        
        source[comment_ptr] = source[code_ptr];
        
        comment_ptr++;
        code_ptr++;
    }
    
    //removing comments by moving non commented code forward leaves garbage at the end.
    //overwrite garbage with null terminator
    source[comment_ptr] = '\0';
}

void generateLexemes(char *source, Lexeme *lexemes, int *lexeme_count) {
    
    int src_ptr = 0;
    int line = 1;
    
    while (source[src_ptr] != '\0' && src_ptr < CODE_LENGTH) {
        Lexeme lexeme;
        
        //skip whitespace
        if (source[src_ptr] == ' ' || source[src_ptr] == '\t' || source[src_ptr] == '\v' ||
            source[src_ptr] == '\f') {
            src_ptr++;
            continue;
        }
        
        if (source[src_ptr] == '\n') {
            if (source[src_ptr + 1] == '\r') {
                src_ptr++;
            }
            src_ptr++;
            line++;
            continue;
        }
        
        if (source[src_ptr] == '\r') {
            if (source[src_ptr + 1] == '\n') {
                src_ptr++;
            }
            src_ptr++;
            line++;
            continue;
        }
        
        //begin
        if (source[src_ptr + 0] == 'b' && source[src_ptr + 1] == 'e' && source[src_ptr + 2] == 'g' &&
            source[src_ptr + 3] == 'i' && source[src_ptr + 4] == 'n' && isalpha(source[src_ptr + 5]) == 0 ) {
            lexeme.token_type = beginsym;
            strcpy(lexeme.lexeme, "begin");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 5;
            continue;
        }
        
        //call | const
        if (source[src_ptr + 0] == 'c') {
            if (source[src_ptr + 1] == 'a' && source[src_ptr + 2] == 'l' && source[src_ptr + 3] == 'l' && isalpha(source[src_ptr + 4]) == 0 ) {
                lexeme.token_type = callsym;
                strcpy(lexeme.lexeme, "call");
                lexemes[*lexeme_count] = lexeme;
                (*lexeme_count)++;
                src_ptr += 4;
                continue;
            }
            
            if (source[src_ptr + 1] == 'o' && source[src_ptr + 2] == 'n' && source[src_ptr + 3] == 's' &&
                source[src_ptr + 4] == 't' && isalpha(source[src_ptr + 5]) == 0 ) {
                lexeme.token_type = constsym;
                strcpy(lexeme.lexeme, "const");
                lexemes[*lexeme_count] = lexeme;
                (*lexeme_count)++;
                src_ptr += 5;
                continue;
            }
        }
        
        //do
        if (source[src_ptr + 0] == 'd' && source[src_ptr + 1] == 'o' && isalpha(source[src_ptr + 2]) == 0 ) {
            lexeme.token_type = dosym;
            strcpy(lexeme.lexeme, "do");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 2;
            continue;
        }
        
        //end | else
        if (source[src_ptr + 0] == 'e') {
            if (source[src_ptr + 1] == 'n' && source[src_ptr + 2] == 'd' && isalpha(source[src_ptr + 3]) == 0 ) {
                lexeme.token_type = endsym;
                strcpy(lexeme.lexeme, "end");
                lexemes[*lexeme_count] = lexeme;
                (*lexeme_count)++;
                src_ptr += 3;
                continue;
            }
            
            if (source[src_ptr + 1] == 'l' && source[src_ptr + 2] == 's' && source[src_ptr + 3] == 'e' && isalpha(source[src_ptr + 4]) == 0 ) {
                lexeme.token_type = elsesym;
                strcpy(lexeme.lexeme, "else");
                lexemes[*lexeme_count] = lexeme;
                (*lexeme_count)++;
                src_ptr += 4;
                continue;
            }
        }
        
        //if
        if (source[src_ptr + 0] == 'i' && source[src_ptr + 1] == 'f' && isalpha(source[src_ptr + 2]) == 0 ) {
            lexeme.token_type = ifsym;
            strcpy(lexeme.lexeme, "if");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 2;
            continue;
        }
        
        //then
        if (source[src_ptr + 0] == 't' && source[src_ptr + 1] == 'h' && source[src_ptr + 2] == 'e' && source[src_ptr + 3] == 'n' && isalpha(source[src_ptr + 4]) == 0 ) {
            lexeme.token_type = thensym;
            strcpy(lexeme.lexeme, "then");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 4;
            continue;
        }
        
        //procedure
        if (source[src_ptr + 0] == 'p' && source[src_ptr + 1] == 'r' && source[src_ptr + 2] == 'o' &&
            source[src_ptr + 3] == 'c' && source[src_ptr + 4] == 'e' && source[src_ptr + 5] == 'd' &&
            source[src_ptr + 6] == 'u' && source[src_ptr + 7] == 'r' && source[src_ptr + 8] == 'e' &&
            source[src_ptr + 9] == ' ') {
            lexeme.token_type = procsym;
            strcpy(lexeme.lexeme, "procedure");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 9;
            continue;
        }
        
        //read
        if (source[src_ptr + 0] == 'r' && source[src_ptr + 1] == 'e' && source[src_ptr + 2] == 'a' &&
            source[src_ptr + 3] == 'd' && isalpha(source[src_ptr + 4]) == 0 ) {
            lexeme.token_type = readsym;
            strcpy(lexeme.lexeme, "read");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 4;
            continue;
        }
        
        //var
        if (source[src_ptr + 0] == 'v' && source[src_ptr + 1] == 'a' && source[src_ptr + 2] == 'r' &&
            source[src_ptr + 3] == ' ' ) {
            lexeme.token_type = varsym;
            strcpy(lexeme.lexeme, "var");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 3;
            continue;
        }
        
        //while | write
        if (source[src_ptr + 0] == 'w') {
            if (source[src_ptr + 1] == 'h' && source[src_ptr + 2] == 'i' && source[src_ptr + 3] == 'l' &&
                source[src_ptr + 4] == 'e' && isalpha(source[src_ptr + 5]) == 0 ) {
                lexeme.token_type = whilesym;
                strcpy(lexeme.lexeme, "while");
                lexemes[*lexeme_count] = lexeme;
                (*lexeme_count)++;
                src_ptr += 5;
                continue;
            }
            
            if (source[src_ptr + 1] == 'r' && source[src_ptr + 2] == 'i' && source[src_ptr + 3] == 't' &&
                source[src_ptr + 4] == 'e' && isalpha(source[src_ptr + 5]) == 0 ) {
                lexeme.token_type = writesym;
                strcpy(lexeme.lexeme, "write");
                lexemes[*lexeme_count] = lexeme;
                (*lexeme_count)++;
                src_ptr += 5;
                continue;
            }
        }
        
        //:= becomes
        if (source[src_ptr + 0] == ':' && source[src_ptr + 1] == '=') {
            lexeme.token_type = becomessym;
            strcpy(lexeme.lexeme, ":=");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 2;
            continue;
        }
        
        //. period
        if (source[src_ptr + 0] == '.') {
            lexeme.token_type = periodsym;
            strcpy(lexeme.lexeme, ".");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 1;
            continue;
        }
        
        //; semicolon
        if (source[src_ptr + 0] == ';') {
            lexeme.token_type = semicolonsym;
            strcpy(lexeme.lexeme, ";");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 1;
            continue;
        }
        
        //, comma
        if (source[src_ptr + 0] == ',') {
            lexeme.token_type = commasym;
            strcpy(lexeme.lexeme, ",");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 1;
            continue;
        }
        
        //) right parenthesis
        if (source[src_ptr + 0] == ')') {
            lexeme.token_type = rparentsym;
            strcpy(lexeme.lexeme, ")");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 1;
            continue;
        }
        
        //( left parenthesis
        if (source[src_ptr + 0] == '(') {
            lexeme.token_type = lparentsym;
            strcpy(lexeme.lexeme, "(");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 1;
            continue;
        }
        
        //> | >=
        if (source[src_ptr + 0] == '>') {
            if (source[src_ptr + 1] == '=') {
                lexeme.token_type = geqsym;
                strcpy(lexeme.lexeme, ">=");
                lexemes[*lexeme_count] = lexeme;
                (*lexeme_count)++;
                src_ptr += 2;
                continue;
            }
            
            lexeme.token_type = gtrsym;
            strcpy(lexeme.lexeme, ">");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 1;
            continue;
        }
        
        //< | <=
        if (source[src_ptr + 0] == '<') {
            if (source[src_ptr + 1] == '=') {
                lexeme.token_type = leqsym;
                strcpy(lexeme.lexeme, "<=");
                lexemes[*lexeme_count] = lexeme;
                (*lexeme_count)++;
                src_ptr += 2;
                continue;
            }
            
            
            if (source[src_ptr + 1] == '>') {
                lexeme.token_type = neqsym;
                strcpy(lexeme.lexeme, "<>");
                lexemes[*lexeme_count] = lexeme;
                (*lexeme_count)++;
                src_ptr += 2;
                continue;
            }
            
            lexeme.token_type = lesssym;
            strcpy(lexeme.lexeme, "<");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 1;
            continue;
        }
        
        //= equals
        if (source[src_ptr + 0] == '=') {
            lexeme.token_type = equalsym;
            strcpy(lexeme.lexeme, "=");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 1;
            continue;
        }
        
        //odd
        if (source[src_ptr + 0] == 'o' && source[src_ptr + 1] == 'd' && source[src_ptr + 2] == 'd' ) {
            lexeme.token_type = oddsym;
            strcpy(lexeme.lexeme, "odd");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 3;
            continue;
        }
        
        // / slash
        if (source[src_ptr + 0] == '/') {
            lexeme.token_type = slashsym;
            strcpy(lexeme.lexeme, "/");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 1;
            continue;
        }
        
        //* multiply
        if (source[src_ptr + 0] == '*') {
            lexeme.token_type = multsym;
            strcpy(lexeme.lexeme, "*");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 1;
            continue;
        }
        
        //- minus
        if (source[src_ptr + 0] == '-') {
            lexeme.token_type = minussym;
            strcpy(lexeme.lexeme, "-");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 1;
            continue;
        }
        
        //+ plus
        if (source[src_ptr + 0] == '+') {
            lexeme.token_type = plussym;
            strcpy(lexeme.lexeme, "+");
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            src_ptr += 1;
            continue;
        }
        
        /*
         
         LITERALS AND IDENTIFIERS PAST THIS POINT
         
         */
        
        //literal identifier must start with a letter, anything after that is fair game
        if ((source[src_ptr] >= 'a' && source[src_ptr] <= 'z') || (source[src_ptr] >= 'A' && source[src_ptr] <= 'Z')) {
            int count = 0;
            char identifier[MAX_IDENT_LENGTH];
            initArray(identifier, MAX_IDENT_LENGTH);
            
            identifier[count] = source[src_ptr];
            src_ptr++;
            
            while ((source[src_ptr] >= 'a' && source[src_ptr] <= 'z') ||
                   (source[src_ptr] >= 'A' && source[src_ptr] <= 'Z') ||
                   (source[src_ptr] >= '0' && source[src_ptr] <= '9')) {
                count++;
                if (count > MAX_IDENT_LENGTH) {
                    printf("Error: Variable name is too long: '%s...' at line %d\n", identifier, line);
                    exit(1);
                }
                
                identifier[count] = source[src_ptr];
                src_ptr++;
            }
            
            lexeme.token_type = identsym;
            strcpy(lexeme.lexeme, identifier);
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            continue;
        }
        
        //literal digit of any length less than or equal to MAX_DIGITS
        if (source[src_ptr] >= '0' && source[src_ptr] <= '9') {
            int count = 0;
            char digit[MAX_IDENT_LENGTH];
            initArray(digit, MAX_IDENT_LENGTH);
            
            digit[count] = source[src_ptr];
            src_ptr++;
            
            //if the next character after a digit is a letter, it's an invalid variable name
            //I don't believe there are any valid cases in our language where a letter immediately following
            //the first number in a sequence of numbers is valid. no 0D 0L 0x 0b etc
            if ((source[src_ptr] >= 'a' && source[src_ptr] <= 'z') ||
                (source[src_ptr] >= 'A' && source[src_ptr] <= 'Z')) {
                printf("Error: Bad variable name at line %d\n", line);
                exit(3);
            }
            
            while (source[src_ptr] >= '0' && source[src_ptr] <= '9') {
                count++;
                if (count > MAX_DIGITS) {
                    printf("Error: Digit too long at line %d\n", line);
                    exit(4);
                }
                
                digit[count] = source[src_ptr];
                src_ptr++;
            }
            
            lexeme.token_type = numbersym;
            strcpy(lexeme.lexeme, digit);
            lexemes[*lexeme_count] = lexeme;
            (*lexeme_count)++;
            continue;
        }
        
        
        printf("Error: Invalid symbol '%c' at line %d\n", source[src_ptr], line);
        exit(5);
        
        //src_ptr++;
    }
};

void printSource(char *source) {
    
    //ofp = fopen("/Users/Zack/Development/UCF/Systems Software/Assignment4/Assignment4/cleaninput.txt", "w");
    
    ofp = fopen("cleaninput.txt", "w");
    
    int i = 0;
    while (source[i] != '\0') {
        fprintf(ofp, "%c", source[i]);
        i++;
    }
    fprintf(ofp, "\n");
    
    fclose(ofp);
}

void printLexemeTable(Lexeme *lexemes, int lexeme_count) {
    
    //ofp = fopen("/Users/Zack/Development/UCF/Systems Software/Assignment4/Assignment4/lexemetable.txt", "w");
    
    ofp = fopen("lexemetable.txt", "w");
    
    int i;
    //    printf("%s\t%s\n", "lexeme", "token type");
    fprintf(ofp, "%s\t\t%s\n", "lexeme", "token type");
    for (i = 0; i < lexeme_count; i++) {
        Lexeme lexeme = lexemes[i];
        //        printf("%s\t%d\n", lexeme.lexeme, lexeme.token_type);
        fprintf(ofp, "%-10s\t%d\n", lexeme.lexeme, lexeme.token_type);
    }
    //    printf("\n");
    fprintf(ofp, "\n");
    
    fclose(ofp);
};

void printTokenList(Lexeme *lexemes, int lexeme_count) {
    
    //ofp = fopen("/Users/Zack/Development/UCF/Systems Software/Assignment4/Assignment4/tokenlist.txt", "w");
    
    ofp = fopen("tokenlist.txt", "w");
    
    int i;
    for (i = 0; i < lexeme_count; i++) {
        Lexeme lexeme = lexemes[i];

        fprintf(ofp, "%d ", lexeme.token_type);

        if (lexeme.token_type == identsym || lexeme.token_type == numbersym) {
            fprintf(ofp, "%s ", lexeme.lexeme);
        }
    }
    
    fprintf(ofp, "\n");
    
    fclose(ofp);
};

void printTokenListScreen(Lexeme *lexemes, int lexeme_count) {
    printf("\n");
    int i;
    for (i = 0; i < lexeme_count; i++) {
        Lexeme lexeme = lexemes[i];

        printf("%d ", lexeme.token_type);

        if (lexeme.token_type == identsym || lexeme.token_type == numbersym) {
            printf("%s ", lexeme.lexeme);
        }
    }
    
    printf("\n\n");
};


//PM0 PART

void getmcode(struct instruction codeStore[]) {
    
    //Open the input file
    //ofp = fopen("/Users/Zack/Development/UCF/Systems Software/Assignment4/Assignment4/mcode.txt", "r");
    
    ofp = fopen("mcode.txt", "r");
    
    //ofp = fopen("mcode.txt", "r");
    
    //loop through the array storing all the instructions
    int i = 0;
    while (i < MAX_CODE_LENGTH) {
        fscanf(ofp, "%d", &codeStore[i].op);
        fscanf(ofp, "%d", &codeStore[i].l);
        fscanf(ofp, "%d", &codeStore[i].m);
        i++;
    }
    
    //Done reading. Close the file.
    fclose(ofp);
}

//A function for all ALU operations
void ALU(int m, int stackStore[], int *sp) {
    
    switch (m) {
            //Pop the stack and push the negation of the result.
        case 1:
            //CHECK HERE
            stackStore[*sp] = 0 - stackStore[*sp];
            break;
            
            //Pop the stack twice, add the values, and push the result.
        case 2:
            *sp = *sp - 1;
            stackStore[*sp] = stackStore[*sp] + stackStore[*sp + 1];
            break;
            
            //Pop the stack twice, subtract the top value from the second value, and push the result.
        case 3:
            *sp = *sp - 1;
            stackStore[*sp] = stackStore[*sp] - stackStore[*sp + 1];
            break;
            
            //Pop the stack twice, multiply the values, and push the result.
        case 4:
            *sp = *sp - 1;
            stackStore[*sp] = stackStore[*sp] * stackStore[*sp+1];
            break;
            
            //Pop the stack twice, divide the second value by the top value, and push the quotient.
        case 5:
            *sp = *sp - 1;
            stackStore[*sp] = stackStore[*sp] / stackStore[*sp + 1];
            break;
            
            //Pop the stack, push 1 if the value is odd, and push 0 otherwise.
        case 6:
            if (stackStore[*sp] % 2 != 0)
                stackStore[*sp] = 1;
            else
                stackStore[*sp] = 0;
            break;
            
            //Pop the stack twice, divide the second value by the top value, and push the remainder.
        case 7:
            *sp = *sp - 1;
            stackStore[*sp] = stackStore[*sp] % stackStore[*sp + 1];
            break;
            
            //Pop the stack twice and compare the top value t with the second value s. Push 1 if s = t and 0 otherwise.
        case 8:
            *sp = *sp - 1;
            if (stackStore[*sp] == stackStore[*sp+1])
                stackStore[*sp] = 1;
            else
                stackStore[*sp] = 0;
            break;
            
            //Pop the stack twice and compare the top value t with the second value s. Push 1 if s ≠ t and 0 otherwise.
        case 9:
            *sp = *sp - 1;
            if (stackStore[*sp] != stackStore[*sp + 1])
                stackStore[*sp] = 1;
            else
                stackStore[*sp] = 0;
            
            break;
            
            //Pop the stack twice and compare the top value t with the second value s. Push 1 if s < t and 0 otherwise.
        case 10:
            *sp = *sp - 1;
            if (stackStore[*sp] < stackStore[*sp + 1])
                stackStore[*sp] = 1;
            else
                stackStore[*sp] = 0;
            
            break;
            
            //Pop the stack twice and compare the top value t with the second value s. Push 1 if s ≤ t and 0 otherwise.
        case 11:
            *sp = *sp - 1;
            if (stackStore[*sp] <= stackStore[*sp + 1])
                stackStore[*sp] = 1;
            else
                stackStore[*sp] = 0;
            
            break;
            //Pop the stack twice and compare the top value t with the second value s. Push 1 if s > t and 0 otherwise.
        case 12:
            *sp = *sp - 1;
            if (stackStore[*sp] > stackStore[*sp + 1])
                stackStore[*sp] = 1;
            else
                stackStore[*sp] = 0;
            break;
            //Pop the stack twice and compare the top value t with the second value s. Push 1 if s ≥ t and 0 otherwise.
        case 13:
            *sp = *sp - 1;
            if (stackStore[*sp] >= stackStore[*sp + 1])
                stackStore[*sp] = 1;
            else
                stackStore[*sp] = 0;
            break;
        default:
            break;
    }
    
}

void printInstructions(struct instruction codeStore[], int stackStore[], int sp, int pc, int bp, FILE *fp) {
    int i = 0;
    fprintf(fp, "Line	OP      L       M\n");
    while (codeStore[i].op != 0) {
        
        fprintf(fp, "%3d\t", i);
        
        switch (codeStore[i].op) {
            case 1:
                fprintf(fp, "LIT\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 2:
                fprintf(fp, "OPR\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 3:
                fprintf(fp, "LOD\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 4:
                fprintf(fp, "STO\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 5:
                fprintf(fp, "CAL\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 6:
                fprintf(fp, "INC\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 7:
                fprintf(fp, "JMP\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 8:
                fprintf(fp, "JPC\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 9:
                fprintf(fp, "SIO\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 10:
                fprintf(fp, "SIO\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 11:
                fprintf(fp, "SIO\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            default:
                break;
        }
        i++;
    }
}

void printInstructionsScreen(struct instruction codeStore[], int stackStore[], int sp, int pc, int bp) {
    int i = 0;
    printf("Line	OP      L       M\n");
    while (codeStore[i].op != 0) {
        
        printf("%3d\t", i);
        
        switch (codeStore[i].op) {
            case 1:
                printf("LIT\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 2:
                printf("OPR\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 3:
                printf("LOD\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 4:
                printf("STO\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 5:
                printf("CAL\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 6:
                printf("INC\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 7:
                printf("JMP\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 8:
                printf("JPC\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 9:
                printf("SIO\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 10:
                printf("SIO\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            case 11:
                printf("SIO\t%d\t%d\n", codeStore[i].l, codeStore[i].m);
                break;
            default:
                break;
        }
        i++;
    }
}


int base(int l, int base, int stack[]) {
    int b1 = base; //find base L levels down
    while (l > 0) {
        b1 = stack[b1 + 1];
        l--;
    }
    return b1;
}

void printStack(int stack[], int start, int sp, FILE *fp) {
    int i;
    for (i = start; i <= sp; i++) {
        fprintf(fp, "%d ", stack[i]);
    }
}

void printStackScreen(int stack[], int start, int sp) {
    int i;
    for (i = start; i <= sp; i++) {
        printf("%d ", stack[i]);
    }
}


void runPM0() {
    
    //declar variables
    int sp = 0, bp = 1, pc = 0;
    struct instruction ir;
    
    //Create an array of structs with instructions
    struct instruction codeStore[MAX_CODE_LENGTH];
    int stackStore[MAX_STACK_HEIGHT];
    
    //initialize all of them to be zero
    initArrayStruct(codeStore);
    initIntArray(stackStore, MAX_STACK_HEIGHT);
    
    
    //Read in file
    getmcode(codeStore);
    
    //Create a file pointer for writing
    FILE *fp = fopen("stacktrace.txt", "w");
    
    printInstructions(codeStore, stackStore, sp, pc, bp, fp);
    
    fprintf(fp, "\n                                pc\tbp\tsp\tstack\n");
    
    fprintf(fp, "Initial values                  0\t1\t0\n");
    
    //A variable to flag if the program is inside a procedure call or not
    //and use to add a bar
    int procedureCall = 0;
    //Helper variable when need to print a bar on the stacktrace
    int stackIndex = 0;
    
    //Begin fetch-execute cycles
    while (pc > 0 || bp > 0 || sp > 0) {
        
        //Print the PC
        fprintf(fp, "%3d\t", pc);
        
        //Fetch an instruction from the code store
        ir = codeStore[pc];
        
        //increment program counter
        pc++;
        
        switch (ir.op) {
                
                //Push the literal value M onto the stack.
            case 1:
                sp += 1;
                stackStore[sp] = ir.m;
                fprintf(fp, "LIT\t%d\t%d\t%d\t%d\t%d\t", ir.l, ir.m, pc, bp, sp);
                if (procedureCall == 0) {
                    printStack(stackStore, 1, sp, fp);
                    fprintf(fp, "\n");
                } else {
                    printStack(stackStore, 1, stackIndex, fp);
                    fprintf(fp, "| ");
                    printStack(stackStore, stackIndex + 1, sp, fp);
                    fprintf(fp, "\n");
                    
                }
                break;
                
                //02 OPR 0, 0 Return from a procedure call. or if M exists: Perform an ALU operation, specified by M.
            case 2:
                //if it's a return instruction
                if (ir.l == 0 && ir.m == 0) {
                    sp = bp - 1;
                    pc = stackStore[sp + 4];
                    bp = stackStore[sp + 3];
                    procedureCall = 0;
                } else {
                    //Perform an ALU operation specified by M
                    ALU(ir.m, stackStore, &sp);
                    
                }
                fprintf(fp, "OPR\t%d\t%d\t%d\t%d\t%d\t", ir.l, ir.m, pc, bp, sp);
                if (procedureCall == 0) {
                    printStack(stackStore, 1, sp, fp);
                    fprintf(fp, "\n");
                } else {
                    printStack(stackStore, 1, stackIndex, fp);
                    fprintf(fp, "| ");
                    printStack(stackStore, stackIndex + 1, sp, fp);
                    fprintf(fp, "\n");
                }
                break;
                
                //Read the value at offset M from L levels down (if L=0, our own frame) and push it onto the stack.
            case 3:
                sp += 1;
                stackStore[sp] = stackStore[base(ir.l, bp, stackStore) + ir.m];
                fprintf(fp, "LOD\t%d\t%d\t%d\t%d\t%d\t", ir.l, ir.m, pc, bp, sp);
                if (procedureCall == 0) {
                    printStack(stackStore, 1, sp, fp);
                    fprintf(fp, "\n");
                } else {
                    printStack(stackStore, 1, stackIndex, fp);
                    fprintf(fp, "| ");
                    printStack(stackStore, stackIndex + 1, sp, fp);
                    fprintf(fp, "\n");
                }
                break;
                //Pop the stack and write the value into offset M from L levels down – if L=0, our own frame.
            case 4:
                stackStore[base(ir.l, bp, stackStore) + ir.m] = stackStore[sp];
                sp -= 1;
                fprintf(fp, "STO\t%d\t%d\t%d\t%d\t%d\t", ir.l, ir.m, pc, bp, sp);
                if (procedureCall == 0) {
                    printStack(stackStore, 1, sp, fp);
                    fprintf(fp, "\n");
                } else {
                    printStack(stackStore, 1, stackIndex, fp);
                    fprintf(fp, "| ");
                    printStack(stackStore, stackIndex + 1, sp, fp);
                    fprintf(fp, "\n");
                    
                }
                break;
                
                //Call the procedure at M.
            case 5:
                stackStore[sp + 1] = 0;
                stackStore[sp + 2] = base(ir.l, bp, stackStore);
                stackStore[sp + 3] = bp;
                stackStore[sp + 4] = pc;
                bp = sp + 1;
                pc = ir.m;
                
                fprintf(fp, "CAL\t%d\t%d\t%d\t%d\t%d\t", ir.l, ir.m, pc, bp, sp);
                printStack(stackStore, 1, sp, fp);
                fprintf(fp, "\n");
                
                stackIndex = sp;
                
                //Set the flag for the procedure call
                procedureCall = 1;
                break;
                
                //Allocate enough space for M local variables. We will always allocate at least four.
            case 6:
                
                sp += ir.m;
                
                fprintf(fp, "INC\t%d\t%d\t%d\t%d\t%d\t", ir.l, ir.m, pc, bp, sp);
                
                if (procedureCall == 0) {
                    printStack(stackStore, 1, sp, fp);
                    fprintf(fp, "\n");
                } else {
                    printStack(stackStore, 1, stackIndex, fp);
                    fprintf(fp, "| ");
                    printStack(stackStore, stackIndex + 1, sp, fp);
                    fprintf(fp, "\n");
                }
                
                break;
                //Branch to M.
            case 7:
                pc = ir.m;
                fprintf(fp, "JMP\t%d\t%d\t%d\t%d\t%d\n", ir.l, ir.m, pc, bp, sp);
                break;
                //Pop the stack and branch to M if the result is 0.
            case 8:
                
                if (stackStore[sp] == 0) {
                    pc = ir.m;
                }
                sp--;
                
                fprintf(fp, "JPC\t%d\t%d\t%d\t%d\t%d\t", ir.l, ir.m, pc, bp, sp);
                if (procedureCall == 0) {
                    printStack(stackStore, 1, sp, fp);
                    fprintf(fp, "\n");
                } else {
                    printStack(stackStore, 1, stackIndex, fp);
                    fprintf(fp, "| ");
                    printStack(stackStore, stackIndex + 1, sp, fp);
                    fprintf(fp, "\n");
                }
                break;
                //Pop the stack and write the result to the screen.
            case 9:
                printf("%d\n", stackStore[sp]);
                sp--;
                
                fprintf(fp, "SIO\t%d\t%d\t%d\t%d\t%d\t", ir.l, ir.m, pc, bp, sp);
                if (procedureCall == 0) {
                    printStack(stackStore, 1, sp, fp);
                    fprintf(fp, "\n");
                } else {
                    printStack(stackStore, 1, stackIndex, fp);
                    fprintf(fp, "| ");
                    printStack(stackStore, stackIndex + 1, sp, fp);
                    fprintf(fp, "\n");
                }
                break;
                //Read an input from the user and store it at the top of the stack.
            case 10:
                sp++;
                scanf("%d", &stackStore[sp]);
                userInput = stackStore[sp];
                
                fprintf(fp, "SIO\t%d\t%d\t%d\t%d\t%d\t", ir.l, ir.m, pc, bp, sp);
                if (procedureCall == 0) {
                    printStack(stackStore, 1, sp, fp);
                    fprintf(fp, "\n");
                } else {
                    printStack(stackStore, 1, stackIndex, fp);
                    fprintf(fp, "| ");
                    printStack(stackStore, stackIndex + 1, sp, fp);
                    fprintf(fp, "\n");
                    
                }
                break;
                //Stop the machine.
            case 11:
                pc = 0;
                bp = 0;
                sp = 0;
                
                fprintf(fp, "SIO\t%d\t%d\n", ir.l, ir.m);
                
                break;
                
            default:
                break;
        }
    }
    
    fprintf(fp, "Successfully halted.\n");

}

void printStackTraceScreen() {
    
    //declar variables
    int sp = 0, bp = 1, pc = 0;
    struct instruction ir;
    
    //Create an array of structs with instructions
    struct instruction codeStore[MAX_CODE_LENGTH];
    int stackStore[MAX_STACK_HEIGHT];
    
    //initialize all of them to be zero
    initArrayStruct(codeStore);
    initIntArray(stackStore, MAX_STACK_HEIGHT);
    
    
    //Read in file
    getmcode(codeStore);
    
    //Create a file pointer for writing
    //FILE *fp = fopen("stacktrace.txt", "w");
    
    printInstructionsScreen(codeStore, stackStore, sp, pc, bp);
    
    printf("\n                                pc\tbp\tsp\tstack\n");
    
    printf("Initial values                  0\t1\t0\n");
    
    //A variable to flag if the program is inside a procedure call or not
    //and use to add a bar
    int procedureCall = 0;
    //Helper variable when need to print a bar on the stacktrace
    int stackIndex = 0;
    
    //Begin fetch-execute cycles
    while (pc > 0 || bp > 0 || sp > 0) {
        
        //Print the PC
        printf("%3d\t", pc);
        
        //Fetch an instruction from the code store
        ir = codeStore[pc];
        
        //increment program counter
        pc++;
        
        switch (ir.op) {
                
                //Push the literal value M onto the stack.
            case 1:
                sp += 1;
                stackStore[sp] = ir.m;
                printf("LIT\t%d\t%d\t%d\t%d\t%d\t", ir.l, ir.m, pc, bp, sp);
                if (procedureCall == 0) {
                    printStackScreen(stackStore, 1, sp);
                    printf("\n");
                } else {
                    printStackScreen(stackStore, 1, stackIndex);
                    printf("| ");
                    printStackScreen(stackStore, stackIndex + 1, sp);
                    printf("\n");
                    
                }
                break;
                
                //02 OPR 0, 0 Return from a procedure call. or if M exists: Perform an ALU operation, specified by M.
            case 2:
                //if it's a return instruction
                if (ir.l == 0 && ir.m == 0) {
                    sp = bp - 1;
                    pc = stackStore[sp + 4];
                    bp = stackStore[sp + 3];
                    procedureCall = 0;
                } else {
                    //Perform an ALU operation specified by M
                    ALU(ir.m, stackStore, &sp);
                    
                }
                printf("OPR\t%d\t%d\t%d\t%d\t%d\t", ir.l, ir.m, pc, bp, sp);
                if (procedureCall == 0) {
                    printStackScreen(stackStore, 1, sp);
                    printf("\n");
                } else {
                    printStackScreen(stackStore, 1, stackIndex);
                    printf("| ");
                    printStackScreen(stackStore, stackIndex + 1, sp);
                    printf("\n");
                }
                break;
                
                //Read the value at offset M from L levels down (if L=0, our own frame) and push it onto the stack.
            case 3:
                sp += 1;
                stackStore[sp] = stackStore[base(ir.l, bp, stackStore) + ir.m];
                printf("LOD\t%d\t%d\t%d\t%d\t%d\t", ir.l, ir.m, pc, bp, sp);
                if (procedureCall == 0) {
                    printStackScreen(stackStore, 1, sp);
                    printf("\n");
                } else {
                    printStackScreen(stackStore, 1, stackIndex);
                    printf("| ");
                    printStackScreen(stackStore, stackIndex + 1, sp);
                    printf("\n");
                }
                break;
                //Pop the stack and write the value into offset M from L levels down – if L=0, our own frame.
            case 4:
                stackStore[base(ir.l, bp, stackStore) + ir.m] = stackStore[sp];
                sp -= 1;
                printf("STO\t%d\t%d\t%d\t%d\t%d\t", ir.l, ir.m, pc, bp, sp);
                if (procedureCall == 0) {
                    printStackScreen(stackStore, 1, sp);
                    printf("\n");
                } else {
                    printStackScreen(stackStore, 1, stackIndex);
                    printf("| ");
                    printStackScreen(stackStore, stackIndex + 1, sp);
                    printf("\n");
                    
                }
                break;
                
                //Call the procedure at M.
            case 5:
                stackStore[sp + 1] = 0;
                stackStore[sp + 2] = base(ir.l, bp, stackStore);
                stackStore[sp + 3] = bp;
                stackStore[sp + 4] = pc;
                bp = sp + 1;
                pc = ir.m;
                
                printf("CAL\t%d\t%d\t%d\t%d\t%d\t", ir.l, ir.m, pc, bp, sp);
                printStackScreen(stackStore, 1, sp);
                printf("\n");
                
                stackIndex = sp;
                
                //Set the flag for the procedure call
                procedureCall = 1;
                break;
                
                //Allocate enough space for M local variables. We will always allocate at least four.
            case 6:
                
                sp += ir.m;
                
                printf("INC\t%d\t%d\t%d\t%d\t%d\t", ir.l, ir.m, pc, bp, sp);
                
                if (procedureCall == 0) {
                    printStackScreen(stackStore, 1, sp);
                    printf("\n");
                } else {
                    printStackScreen(stackStore, 1, stackIndex);
                    printf("| ");
                    printStackScreen(stackStore, stackIndex + 1, sp);
                    printf("\n");
                }
                
                break;
                //Branch to M.
            case 7:
                pc = ir.m;
                printf("JMP\t%d\t%d\t%d\t%d\t%d\n", ir.l, ir.m, pc, bp, sp);
                break;
                //Pop the stack and branch to M if the result is 0.
            case 8:
                
                if (stackStore[sp] == 0) {
                    pc = ir.m;
                }
                sp--;
                
                printf("JPC\t%d\t%d\t%d\t%d\t%d\t", ir.l, ir.m, pc, bp, sp);
                if (procedureCall == 0) {
                    printStackScreen(stackStore, 1, sp);
                    printf("\n");
                } else {
                    printStackScreen(stackStore, 1, stackIndex);
                    printf("| ");
                    printStackScreen(stackStore, stackIndex + 1, sp);
                    printf("\n");
                }
                break;
                
                //Pop the stack and write the result to the screen.
            case 9:
                //printf("%d\n", stackStore[sp]);
                sp--;
                
                printf("SIO\t%d\t%d\t%d\t%d\t%d\t", ir.l, ir.m, pc, bp, sp);
                if (procedureCall == 0) {
                    printStackScreen(stackStore, 1, sp);
                    printf("\n");
                } else {
                    printStackScreen(stackStore, 1, stackIndex);
                    printf("| ");
                    printStackScreen(stackStore, stackIndex + 1, sp);
                    printf("\n");
                }
                break;
                //Read an input from the user and store it at the top of the stack.
            case 10:
                sp++;
                //scanf("%d", &stackStore[sp]);
                stackStore[sp] = userInput;
                userInput = 0;
                
                printf("SIO\t%d\t%d\t%d\t%d\t%d\t", ir.l, ir.m, pc, bp, sp);
                if (procedureCall == 0) {
                    printStackScreen(stackStore, 1, sp);
                    printf("\n");
                } else {
                    printStackScreen(stackStore, 1, stackIndex);
                    printf("| ");
                    printStackScreen(stackStore, stackIndex + 1, sp);
                    printf("\n");
                    
                }
                break;
                //Stop the machine.
            case 11:
                pc = 0;
                bp = 0;
                sp = 0;
                
                printf("SIO\t%d\t%d\n", ir.l, ir.m);
                
                break;
                
            default:
                break;
        }
    }
    
    printf("Successfully halted.\n");
    
}


int main(int argc, const char * argv[]) {
    
    int i;
    
    //const char *input = argv[1];
    
    for (i = 1; i < argc; i++) {
        
        if (strcmp("-t", argv[i]) == 0) {
            tokenlistSwitch = 1;
        } else if (strcmp("-s", argv[i]) == 0) {
            symtableSwitch = 1;
        } else if (strcmp("-m", argv[i]) == 0) {
            mcodeSwitch = 1;
        } else if (strcmp("-a", argv[i]) == 0) {
            acodeSwitch = 1;
        } else if (strcmp("-v", argv[i]) == 0) {
            vmstackSwitch = 1;
        } else {
            printf("Invalid command line argument: '%s'.\n", argv[i]);
            //exit(1);
        }
    }
    
    initArray(source, CODE_LENGTH);
    
    getCode(source);
    
    removeComments(source);
    
    int lexemeCount = 0;
    Lexeme lexemes[CODE_LENGTH];
    
    generateLexemes(source, lexemes, &lexemeCount);
    
    printSource(source);
    
    printLexemeTable(lexemes, lexemeCount);
    
    printTokenList(lexemes, lexemeCount);
    
    if (tokenlistSwitch == 1) {
        printTokenListScreen(lexemes, lexemeCount);
    }
    
    //Clear out the source array
    initArray(source, CODE_LENGTH);
    getTokenList(source);
    
    token.index = 0;
    token.kind = 0;
    
    token = getNextToken(source);
    
    program();
    
    printSymbolTable();
    
    if (symtableSwitch == 1)
        printSymbolTableScreen();
    
    printMCode();
    
    if (mcodeSwitch == 1)
        printMCodeScreen();
    
    runPM0();
    if (vmstackSwitch == 1)
        printStackTraceScreen();
    
    return 0;
}

void emit(int op, int l, int m)
{
    if (cx > MAX_CODE_LENGTH) {
        exit(25);
    } else {
        code[cx].op = op; // opcode
        code[cx].l = l;   // lex level
        code[cx].m = m;   // modifier
        updateStackSize(op, l, m);
        cx++;
    }
}

void updateStackSize(int op, int l, int m) {
    
    if (op == LIT || op == LOD || op == SIO2) {
        stack_size++;
    } else if (op == STO || op == JPC || op == SIO1) {
        stack_size--;
    } else if (op == INC) {
        stack_size += m;
    } else if (op == OPR) {
        if (m == 0) {
            stack_size = 0;
        } else if (m != NEG || m != ODD) {
            stack_size--;
        }
    }
    
}

//This function is called after we found an identifier, to get the name of the ident to put in the symbol table
void getName(char *source, struct symbol *sym, int num) {
    
    initArray((*sym).name, 12);
    
    int index = num;
    int count = 0;
    
    while (source[index] == ' ') {
        index++;
    }
    
    //literal identifier must start with a letter, anything after that is fair game
    if ((source[index] >= 'a' && source[index] <= 'z') || (source[index] >= 'A' && source[index] <= 'Z')) {
        
        char identifier[MAX_IDENT_LENGTH];
        initArray(identifier, MAX_IDENT_LENGTH);
        
        identifier[count] = source[index];
        index++;
        count++;
        
        while ((source[index] >= 'a' && source[index] <= 'z') ||
               (source[index] >= 'A' && source[index] <= 'Z') ||
               (source[index] >= '0' && source[index] <= '9')) {
            
            identifier[count] = source[index];
            index++;
            count++;
        }
        
        strcpy((*sym).name, identifier);
    }
}

//This function reads in the number token
struct token getNextToken(char *source) {
    
    struct token tok;
    int index = token.index;
    
    if (source[index] == ' ') {
        index++;
    }
    
    //If it's a variable name
    while (isdigit(source[index]) == 0) {
        index++;
    }
    
    if (source[index] == ' ') {
        index++;
    }
    
    if (source[index] >= '0' && source[index] <= '9') {
        
        int count = 0;
        char digit[MAX_IDENT_LENGTH];
        initArray(digit, MAX_IDENT_LENGTH);
        
        digit[count] = source[index];
        index++;
        
        while (source[index] >= '0' && source[index] <= '9') {
            count++;
            
            digit[count] = source[index];
            index++;
        }
        
        tok.kind = atoi(digit);
        tok.index = index;
    }
    
    return tok;
}

void program() {
    
    block();
    
    if(token.kind != periodsym) {
        error(9);
    } else {
        //halt the program
        emit(SIO3, 0, 3);
    }
}

int block() {
    
    level++;
    
    int space = 0, numVars = 0, numConsts = 0, numProcs = 0, jmpAddr = 0;
    
    //Save the jump address for update later
    jmpAddr = cx;
    emit(JMP, 0, 0);
    
    space = 4;
    
    if(token.kind == constsym) {
        numConsts = constDeclaration();
    }
    
    if (token.kind == varsym) {
        numVars = varDeclaration();
    }
    
    space += numVars;
    
    while (token.kind == procsym) {
        numProcs = procDeclaration();
    }
    
    code[jmpAddr].m = cx;
    
    emit(INC, 0, space);
    
    statement();
    
    //If inside other level, do a return statement
    if (level != 0) {
        emit(OPR, 0, 0);
    }
    
    level--;
    return space;
}

int constDeclaration() {
    
    int constCount = 0;
    
    do {
        //Declare a new symbol
        symbol sym;
        
        sym.kind = constant;
        sym.val = 0;
        sym.addr = 0;
        sym.level = level;
        
        token = getNextToken(source);
        
        if (token.kind != identsym)
            error(4);
        
        //Save the name
        getName(source, &sym, token.index);
        
        token = getNextToken(source);
        
        if (token.kind != equalsym)
            error(27);
        
        token = getNextToken(source);
        
        if ( token.kind != numbersym )
            error(2);
        
        
        //This call will return the value of the const
        token = getNextToken(source);
        
        //Store the value of that const into the symbol
        sym.val = token.kind;
        
        
        symbol_table[symTableIndex] = sym;
        symTableIndex++;
        
        token = getNextToken(source);
        
        constCount++;
        
    } while(token.kind == commasym);
    
    if (token.kind != semicolonsym)
        error(5);
    
    token = getNextToken(source);
    
    return constCount;
}

int varDeclaration() {
    int varCount = 0;
    
    do {
        //Declare a new symbol
        symbol sym;
        
        sym.kind = variable;
        sym.val = 4 + varCount;
        sym.addr = 4 + varCount;
        sym.level = level;
        
        token = getNextToken(source);
        
        if(token.kind != identsym)
            error(4);
        
        //Save the name
        getName(source, &sym, token.index);
        
        symbol_table[symTableIndex] = sym;
        symTableIndex++;
        
        varCount++;
        
        //Get next token
        token = getNextToken(source);
        
    } while(token.kind == commasym);
    
    if (token.kind != semicolonsym)
        error(5);
    
    token = getNextToken(source);
    
    return varCount;
}

int procDeclaration() {
    
    procedureCount++;
    
    //Declare a new symbol
    symbol sym;
    
    sym.kind = procedure;
    sym.val = procedureCount;
    sym.addr = procedureCount;
    sym.level = level;
    
    token = getNextToken(source);
    
    if (token.kind != identsym)
    {
        error(4);
        
    }
    
    //Save the name
    getName(source, &sym, token.index);
    
    //Store the procedure to the symtbol table
    symbol_table[symTableIndex] = sym;
    symTableIndex++;
    
    //Call parameter block after getting the name
    parameterBlock();
    
    token = getNextToken(source);
    
    if (token.kind != semicolonsym)
        error(5);
    
    
    token = getNextToken(source);
    
    procedureCount += block();
    
    if (token.kind != semicolonsym)
        error(5);
    
    token = getNextToken(source);
    
    return procedureCount;
}

void parameterBlock() {
    
    addr = 4;
    
    token = getNextToken(source);
    
    if (token.kind != lparentsym)
        error(29);
    
    token = getNextToken(source);
    
    if (token.kind == identsym) {
        
        //add the identifier to the symbol table
        //enter (ident, level+1, addr)
        
        symbol sym;
        
        sym.kind = variable;
        sym.level = level + 1;
        sym.addr = addr;
        sym.val = addr;
        
        getName(source, &sym, token.index);
        
        symbol_table[symTableIndex] = sym;
        symTableIndex++;
        
        addr++;
        
        token = getNextToken(source);
        
        while (token.kind == commasym) {
            token = getNextToken(source);
            
            if (token.kind != identsym)
                error(30);
            
            symbol sym;
            
            sym.kind = variable;
            sym.level = level + 1;
            sym.addr = addr;
            sym.val = addr;
            
            getName(source, &sym, token.index);
            
            symbol_table[symTableIndex] = sym;
            symTableIndex++;

            addr++;
            
            token = getNextToken(source);
        }
        
        
    }
    
    if (token.kind != rparentsym)
        error(31);
    
    //Add the variable 'return'
    
    symbol sym;
    
    sym.kind = variable;
    sym.level = level + 1;
    sym.addr = 0;
    sym.val = 0;
    
    strcpy(sym.name, "return");
    
    symbol_table[symTableIndex] = sym;
    symTableIndex++;
    
    
    
    
    
}

void parameterList() {
    
    params = 0;
    
    if (token.kind != lparentsym)
        error(32);
    
    
    token = getNextToken(source);
    
    if (token.kind != rparentsym) {
        expression();
        params++;
    }
    
    while (token.kind == commasym) {
        token = getNextToken(source);
        expression();
        params++;
    }
    
    while (params > 0) {
        
        //need to do stack size
        emit(STO, 0, stack_size + 4 - 1);
        
        params--;
    }
    
    if (token.kind != rparentsym)
        error(33);
    
    
}

void statement() {
    
    int index, ctemp, cx1, cx2;
    
    if (token.kind == identsym) {
        
        index = findSymbol();
        

        
        if (index == -1 || symbol_table[index].level > level) {
            
            if (strcmp(symbol_table[index].name, "return") == 0) {
                symbol sym;
                
                getName(source, &sym, token.index);
                
                //Check in the symbol table for the name retrieved
                int i;
                for (i = symTableIndex - 2; i >= 0; i--) {
                    
                    //Check for any var, const, or proc name that have been declared
                    if (strcmp(symbol_table[i].name, sym.name) == 0) {
                        index = i;
                        break;
                    }
                    
                }

            } else {
                error(11); //undeclared ident
            }
        }
        
        if (symbol_table[index].kind != variable) {
            error(12);
        }
        
        
        token = getNextToken(source);
        
        if (token.kind != becomessym)
            error(3);
        
        
        token = getNextToken(source);
        
        expression();
        
        emit(STO, level - symbol_table[index].level, symbol_table[index].addr);
        
    } else if (token.kind == callsym) {
        
        token = getNextToken(source);
        
        if (token.kind != identsym)
            error(14);
        
        //Check inside the symbol table if the name exists
        index = findSymbol();
        
        if (index == -1) {
            error(11); //undeclared identifier
        }
//        
//        //Then check if it's a procedure name
//        if (symbol_table[index].kind == procedure) {
//            
//            emit(CAL, level - symbol_table[index].level, symbol_table[index].addr);
//            
//        } else {
//            
//            error(15);
//        }
//        
//        token = getNextToken(source);
//        
//        parameterList();
//        
//        token = getNextToken(source);
        
        if (symbol_table[index].kind != procedure) {
            error(15);
        }
        
        
        
        token = getNextToken(source);
        
        parameterList();
        
        //This goes after the param block
        emit(CAL, level - symbol_table[index].level, symbol_table[index].addr);
        
        //emit(INC, 0, 1);
        
        token = getNextToken(source);
        
        
    } else if (token.kind == beginsym) {
        
        token = getNextToken(source);
        
        
        statement();
        
        while(token.kind == semicolonsym) {
            token = getNextToken(source);
            
            statement();
        }
        
        if (token.kind != endsym)
            error(28);
        
        token = getNextToken(source);
        
    } else if (token.kind == ifsym) {
        
        //Get next token then go into condition block
        token = getNextToken(source);
        
        condition();
        
        //Check for 'then'
        if (token.kind != thensym)
            error(16);
        
        token = getNextToken(source);
        
        //Save the index of the jump instruction inside the symbol table
        ctemp = cx;
        
        emit(JPC, 0, 0);
        
        statement();
        
        //Check for 'else' after the statement
        if (token.kind == elsesym) {
            
            token = getNextToken(source);
            
            cx2 = cx;
            
            emit(JMP, 0, 0);
            
            code[ctemp].m = cx;
            
            statement();
            
            code[cx2].m = cx;
            
        } else {
            
            //Somehow the cx emit here contains an off-by-one error
            emit(JMP, 0, cx+1);
            
            code[ctemp].m = cx;
        }
        
    } else if (token.kind == whilesym) {
        
        cx1 = cx;
        
        token = getNextToken(source);
        
        condition();
        
        cx2 = cx;
        
        emit(JPC, 0, 0);
        
        if (token.kind != dosym)
        {
            error(18);
            
        } else {
            
            token = getNextToken(source);
        }
        
        statement();
        
        emit(JMP, 0, cx1);
        
        code[cx2].m = cx;
    } else if (token.kind == readsym) {
        
        token = getNextToken(source);
        
        if (token.kind != identsym) {
            error(26);
        }
        
        index = findSymbol();
        
        //token = getNextToken(source);
        
        if (index == -1) {
            error(11);
        }
        
        if (symbol_table[index].kind != variable) {
            error(26);
        }
        
        //emite read
        emit(SIO2, 0, 2);
        
        //Store the value on top of the stack
        emit(STO, level - symbol_table[index].level, symbol_table[index].addr);
        
        //Get next token
        token = getNextToken(source);
        
    } else if (token.kind == writesym) {
        
        token = getNextToken(source);
        
        if (token.kind != identsym)
            error(26);
        
        
        index = findSymbol();
        
        token = getNextToken(source);
        
        
        if (index == -1) {
            error(11);
        }
        
        if (symbol_table[index].kind != variable) {
            error(26);
        }
        
        //Load the variable from the symbol table
        emit(LOD, level - symbol_table[index].level, symbol_table[index].addr);
        
        //emit write
        emit(SIO1, 0, 1);
        
        //token = getNextToken(source);
    }
}

void condition() {
    int relationOpCode;
    
    if (token.kind == oddsym) {
        
        token = getNextToken(source);
        
        expression();
        
        emit(OPR, 0, ODD);
    } else {
        expression();
        
        relationOpCode = relation();
        
        if (relationOpCode == 0)
            error(20);
        
        token = getNextToken(source);
        
        expression();
        
        emit(OPR, 0, relationOpCode);
    }
}

//This function return the relation operation
int relation() {
    
    switch (token.kind) {
        case equalsym:
            return EQL;
            break;
        case neqsym:
            return NEQ;
            break;
        case lesssym:
            return LSS;
            break;
        case leqsym:
            return LEQ;
            break;
        case gtrsym:
            return GTR;
            break;
        case geqsym:
            return GEQ;
            break;
        default:
            return 0;
            break;
    }
}

void expression() {
    
    struct token addop;
    if ((token.kind == plussym) ||  (token.kind == minussym)) {
        addop.kind = token.kind;
        
        token = getNextToken(source);
        
        term();
        
        if (addop.kind == minussym) {
            emit(OPR, 0, NEG);
        }
        
    } else {
        term();
    }
    
    while((token.kind == plussym) ||  (token.kind == minussym)) {
        
        addop.kind = token.kind;
        
        token = getNextToken(source);
        
        term();
        
        if (addop.kind == plussym) {
            emit(OPR, 0, ADD);
        } else {
            emit(OPR, 0, SUB);
        }
    }
    
}

void term() {
    struct token mulop;
    
    factor();
    
    while((token.kind == multsym) || (token.kind == slashsym)) {
        //Save the kind of mul or div
        mulop.kind = token.kind;
        
        token = getNextToken(source);
        factor();
        
        if (mulop.kind == multsym) {
            emit(OPR, 0, MUL);
        } else {
            emit(OPR, 0, DIV);
        }
        
    }
}

void factor() {
    
    int value;
    int index;
    
    if (token.kind == identsym) {
        
        //Find the token in the symbol table
        index = findSymbol();
        
        //Error if not found or token was declared out of scope
        if (index == -1 || symbol_table[index].level > level) {
            error(11);
        }
        
        
        if (symbol_table[index].kind == variable) {
            
            //The address generated is still wrong
            emit(LOD, level - symbol_table[index].level, symbol_table[index].addr);
            
        } else if (symbol_table[index].kind == constant) {
            
            //If it's contant then we just push that value onto the stack
            emit(LIT, 0, symbol_table[index].val);
            
            
        } else {
            //otherwise the expression contains a procedure then error out
            error(21);
        }
        
        token = getNextToken(source);
        
    } else if (token.kind == numbersym) {
        
        //This call will get the value of the number
        token = getNextToken(source);
        
        value = token.kind;
        
        emit(LIT, 0, value); //push the value to the top of stack
        
        token = getNextToken(source);
    } else if (token.kind == lparentsym ) {
        token = getNextToken(source);
        
        expression();
        
        if(token.kind != rparentsym)
        {
            error(22);
        }
        
        token = getNextToken(source);
        
    } else if (token.kind == callsym) {
      
        token = getNextToken(source);
        
        if (token.kind != identsym)
            error(14);
        
        index = findSymbol();
        
        if (index == -1 || symbol_table[index].level > level)
            error(11);
        
        if (symbol_table[index].kind != procedure) {
            error(15);
        }
        
     
        
        token = getNextToken(source);
        
        parameterList();
        
        //This goes after the param block
        emit(CAL, level - symbol_table[index].level, symbol_table[index].addr);
        
        emit(INC, 0, 1);
        
        token = getNextToken(source);
        
    } else {
        error(23);
    }
}

//This function returns the index of the current symbol (const, var, proc)
int findSymbol() {
    
    //Get the name and kind of the identifier
    symbol sym;
    
    getName(source, &sym, token.index);
    
    //Check in the symbol table for the name retrieved
//    int i;
//    for (i = symTableIndex; i >= 0; i--) {
//        
//        //Check for any var, const, or proc name that have been declared
//        if (strcmp(symbol_table[i].name, sym.name) == 0) {
//            return i;
//        }
//        
//    }
    
    int i;
    for (i = 0; i <= symTableIndex; i++) {
        
        //Check for any var, const, or proc name that have been declared
        if (strcmp(symbol_table[i].name, sym.name) == 0) {
            return i;
        }
        
    }
    
    //if not found return -1. Do error checking after the function call
    return -1;
}

void error(int error) {
    
    printf("Error: ");
    
    switch (error) {
        case 1:
            printf("Use = instead of :=.");
            break;
        case 2:
            printf("= must be followed by a number.");
            break;
        case 3:
            printf("Identifier must be followed by :=.");
            break;
        case 4:
            printf("const, var, procedure must be followed by identifier.");
            break;
        case 5:
            printf("Semicolon or comma missing.");
            break;
        case 6:
            printf("Incorrect symbol after procedure declaration.");
            break;
        case 7:
            printf("Statement expected.");
            break;
        case 8:
            printf("Incorrect symbol after statement part in block.");
            break;
        case 9:
            printf("Period expected.");
            break;
        case 10:
            printf("Semicolon between statements missing.");
            break;
        case 11:
            printf("Undeclared identifier.");
            break;
        case 12:
            printf("Assignment to constant or procedure is not allowed.");
            break;
        case 13:
            printf("Assignment operator expected.");
            break;
        case 14:
            printf("call must be followed by an identifier.");
            break;
        case 15:
            printf("Call of a constant or variable is meaningless.");
            break;
        case 16:
            printf("then expected.");
            break;
        case 17:
            printf("Semicolon or } expected.");
            break;
        case 18:
            printf("do expected.");
            break;
        case 19:
            printf("Incorrect symbol following statement.");
            break;
        case 20:
            printf("Relational operator expected.");
            break;
        case 21:
            printf("Expression must not contain a procedure identifier.");
            break;
        case 22:
            printf("Right parenthesis missing.");
            break;
        case 23:
            printf("The preceding factor cannot begin with this symbol.");
            break;
        case 24:
            printf("An expression cannot begin with this symbol.");
            break;
        case 25:
            printf("This number is too large.");
            break;
        case 26:
            printf("read or write must be followed by an identifier.");
            break;
        case 27:
            printf("= expected.");
            break;
        case 28:
            printf("end expected.");
            break;
        case 29:
            printf("Procedure must have parameters");
            break;
        case 30:
            printf("Identifier expected after comma in parameter block");
            break;
        case 31:
            printf("Bad procedure declaration");
            break;
        case 32:
            printf("Missing parameter list at call");
            break;
        case 33:
            printf("Bad calling formatting");
            break;
    }
    printf("\n");
    exit(1);
}

