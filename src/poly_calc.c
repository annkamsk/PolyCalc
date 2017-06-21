#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include "poly.h"
#include "stack.h"

#define LONG_MAX_LENGTH 19
#define UINT_LENGTH 10
#define INT_LENGTH 10
#define MAX_COMMAND_LENGTH 8
#define NO_ERROR -2
#define ERROR 2
#define N 2

#define UNUSED(x) (void)(x)

Mono parseMono(int line, int *column, bool *isError);
Poly parsePoly(int line, int *column, bool *isError);
long int parseCoeff(int line, int *column, bool *isError);

/**
 * Wskaznik na funkcje wywolywana w przypadku bledu.
 */
typedef void( * error)(int, int);

void zero(Stack *s) {
    Poly p = PolyZero();
    push(s, &p);
}

void isCoeff(Stack *s) {
    Poly p = top(s);
    if(PolyIsZero(&p) || PolyIsCoeff(&p)) {
        printf("%d\n", 1);
    } else {
        printf("%d\n", 0);
    }
}

void isZero(Stack *s) {
    Poly p = top(s);
    if(PolyIsZero(&p)) {
        printf("%d\n", 1);
    } else {
        printf("%d\n", 0);
    }
}

void clone(Stack *s) {
    Poly PolyTop = top(s);
    Poly p = PolyClone(&PolyTop);
    push(s, &p);
}

void add(Stack *s) {
    Poly p = pop(s);
    Poly q = pop(s);
    Poly sum = PolyAdd(&p, &q);
    push(s, &sum);
    PolyDestroy(&p);
    PolyDestroy(&q);
}

void mul(Stack *s) {
    Poly p = pop(s);
    Poly q = pop(s);
    Poly mul = PolyMul(&p, &q);
    push(s, &mul);
    PolyDestroy(&p);
    PolyDestroy(&q);
}

void neg(Stack *s) {
    Poly p = pop(s);
    Poly neg = PolyNeg(&p);
    push(s, &neg);
    PolyDestroy(&p);
}

void sub(Stack *s) {
    Poly p = pop(s);
    Poly q = pop(s);
    Poly sub = PolySub(&p, &q);
    push(s, &sub);
    PolyDestroy(&p);
    PolyDestroy(&q);
}

void isEq(Stack *s) {
    Poly p = pop(s);
    Poly q = top(s);
    if(PolyIsEq(&p, &q)) {
        printf("%d\n", 1);
    } else {
        printf("%d\n", 0);
    }
    push(s, &p);
}

void deg(Stack *s) {
    Poly p = top(s);
    printf("%d\n", PolyDeg(&p));
}

void degBy(Stack *s, int idx) {
    Poly p = top(s);
    printf("%d\n", PolyDegBy(&p, idx));
}

void at(Stack *s, poly_coeff_t x) {
    Poly p = pop(s);
    Poly result = PolyAt(&p, x);
    push(s, &result);
    PolyDestroy(&p);
}

void PolyPrint(Poly *p, poly_coeff_t coeff) {
    if(PolyIsZero(p)) {
        printf("%li\n", p -> coeff);
    } else if(PolyIsCoeff(p)) {
        printf("%li", p -> coeff + coeff);
    } else {
        if(p -> monos -> m -> exp != 0 && (coeff != 0 || p -> coeff != 0)) {
                printf("(%li,%d)", p -> coeff + coeff, 0);
                printf("+");
        }
        for(List *l = p -> monos; l != NULL; l = l -> next) {
            printf("(");
            if(l -> m -> exp == 0) {
                PolyPrint(&(l -> m -> p), coeff + p -> coeff);
            } else {
                PolyPrint(&(l -> m -> p), 0);
            }
            printf(",%d", l -> m -> exp);
            printf(")");
            if(l -> next != NULL) {
                printf("+");
            }
        }
    }
}

void print(Stack *s) {
    Poly p = top(s);
    if(PolyIsZero(&p) || PolyIsCoeff(&p)) {
        printf("%li\n", p.coeff);
    } else {
        PolyPrint(&p, 0);
        printf("\n");
    }
}

/**
 * Wczytuje znaki do konca linii.
 */
void parseToTheEnd() {
    int c = getchar();

    while(c != '\n' && c != EOF) {
        c = getchar();
    }
}

void errorUnderflow(int line, int column) {
    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line);
    UNUSED(column);
    //parseToTheEnd();
}

void errorParsePoly(int line, int column) {
    fprintf(stderr, "ERROR %d %d\n", line, column);
    parseToTheEnd();
}

void errorWrongCommand(int line, int column) {
    fprintf(stderr, "ERROR %d WRONG COMMAND\n", line);
    UNUSED(column);
    parseToTheEnd();
}

void errorWrongValue(int line, int column) {
    fprintf(stderr, "ERROR %d WRONG VALUE\n", line);
    UNUSED(column);
    parseToTheEnd();
}

void errorWrongVariable(int line, int column) {
    fprintf(stderr, "ERROR %d WRONG VARIABLE\n", line);
    UNUSED(column);
    parseToTheEnd();
}

bool isEndOfLine(int line, void(*error)(int line, int column)) {
    int c = getchar();
    if(c != '\n' && c != EOF) {
        (*error)(line, 0);
        return false;
    } else {
        return true;
    }
}

bool isEndOfPoly(int line, int column) {
    int c = getchar();
    column++;
    if(c != '\n' && c != EOF) {
        errorParsePoly(line, column);
        return false;
    } else {
        return true;
    }
}

/**
 * Sprawdza, czy na stosie sa elementy, na ktorych mozna wykonac polecenie.
 * @param s : stos
 * @param n : liczba potrzebnych elementow, 1 albo 2
 * @param line : nr wiersza
 * @return true, jesli sa potrzebne elementy
 */
bool checkForStackUnderflow(Stack *s, int n, int line) {
    StackEl *el1 = StackTop(s);
    if(el1 == NULL) {
        errorUnderflow(line, 0);
        return false;
    }
    if(n == 1) {
        return true;
    }
    if(el1 -> next == NULL) {
        errorUnderflow(line, 0);
        return false;
    }
    return true;
}

/**
 * Sprawdza, czy liczba miesci sie w zakresie [0, ULONG_MAX].
 * Zwraca odpowiedni komunikat w przypadku bledu.
 * @param line : wiersz
 * @param column : kolumna
 * @param k : liczba
 * @return true, jesli liczba miesci sie w zakresie
 */
unsigned int parseUIntNumber(int line, int *column, bool *isError, void(*error)(int line, int column)) {
    unsigned int k = 0;
    char c = getchar();
    (*column)++;
    if(c == '-') {
        (*error)(line, *column);
        *isError = true;
        return ERROR;
    }
    long int n = 0;
    int i = 0;
    while(isdigit(c)) {
        if(i == UINT_LENGTH) {
            (*error)(line, *column);
            *isError = true;
            return ERROR;
        }
        i++;
        n = n * 10 + c - '0';
        k = k * 10 + c - '0';
        c = getchar();
        (*column)++;
    }
    ungetc(c, stdin);
    if(i == 0) {
        (*error)(line, *column);
        *isError = true;
        return ERROR;
    }
    long l = (long) k;
    if(n != l) {
        (*error)(line, *column);
        *isError = true;
        return ERROR;
    }
    *isError = false;
    return k;
}

/**
 * Sprawdza, czy liczba miesci sie w zakresie [LONG_MING, LONG_MAX].
 * Zwraca odpowiedni komunikat w przypadku bledu.
 * @param line : wiersz
 * @param column : kolumna
 * @param k : liczba
 * @return true, jesli liczba miesci sie w zakresie
 */
long int parseLongNumber(int line, int *column, bool *isError, void (*error)(int line, int column)) {
    bool isNegative = false;
    long int k = 0;
    char c = getchar();
    (*column)++;
    if(c == '-') {
        c = getchar();
        (*column)++;
        isNegative = true;
    }
    int i = 0;
    while(isdigit(c)) {
        if(i == LONG_MAX_LENGTH) {
            (*error)(line, *column);
            (*isError) = true;
            return ERROR;
        }
        k = k * 10 + c - '0';
        i++;
        c = getchar();
        (*column)++;
    }
    ungetc(c, stdin);
    (*column)--;
    if(i == 0) {
        (*error)(line, *column);
        (*isError) = true;
        return ERROR;
    }
    if(i == LONG_MAX_LENGTH) {
        if(isNegative && k == LONG_MIN) {
            *isError = false;
        } else {
            long int l = k / 10;
            if(l >= k) {
                (*error)(line, *column);
                (*isError) = true;
                return ERROR;
            }
        }
    }
    if(isNegative) {
        if(k != LONG_MIN) {
            k = k * (-1);
        }
    }
    (*isError) = false;
    return k;
}

/**
 * Podejmuje probe wczytania wartosci zmiennej dla funkcji AT.
 * W przypadku bledu wypisuje stosowny komunikat i zwraca false.
 * @param line : wiersz
 * @param k : wartosc zmiennej
 * @return true, jesli parsowanie sie powiedzie
 */
long int parseValue(int line, bool *isError) {
    error f = errorWrongValue;
    int column = 0;
    return parseLongNumber(line, &column, isError, f);
}

/**
 * Podejmuje probe wczytania indeksu zmiennej dla funkcji DEG_BY.
 * W przypadku bledu wypisuje stosowny komunikat i zwraca false.
 * @param line : wiersz
 * @param k : indeks
 * @return true, jesli parsowanie sie powiedzie
 */
int parseVariable(int line, bool *isError) {
    error f = errorWrongVariable;
    int column = 0;
    return parseUIntNumber(line, &column, isError, f);
}

bool chooseCommand(Stack *s, char *str, int line) {
    if(!strcmp(str, "ZERO")) {
        zero(s);
    } else if(!strcmp(str, "IS_COEFF")) {
        if(checkForStackUnderflow(s, 1, line)) {
            isCoeff(s);
        } else {
            return false;
        }

    } else if(!strcmp(str, "IS_ZERO")) {
        if(checkForStackUnderflow(s, 1, line)) {
            isZero(s);
        } else {
            return false;
        }

    } else if(!strcmp(str, "CLONE")) {
        if(checkForStackUnderflow(s, 1, line)) {
            clone(s);
        } else {
            return false;
        }

    } else if(!strcmp(str, "ADD")) {
        if(checkForStackUnderflow(s, 2, line)) {
            add(s);
        } else {
            return false;
        }

    } else if(!strcmp(str, "MUL")) {
        if(checkForStackUnderflow(s, 2, line)) {
            mul(s);
        } else {
            return false;
        }

    } else if(!strcmp(str, "NEG")) {
        if(checkForStackUnderflow(s, 1, line)) {
            neg(s);
        } else {
            return false;
        }

    } else if(!strcmp(str, "SUB")) {
        if(checkForStackUnderflow(s, 2, line)) {
            sub(s);
        } else {
            return false;
        }

    } else if(!strcmp(str, "IS_EQ")) {
        if(checkForStackUnderflow(s, 2, line)) {
            isEq(s);
        } else {
            return false;
        }

    } else if(!strcmp(str, "DEG")) {
        if(checkForStackUnderflow(s, 1, line)) {
            deg(s);
        } else {
            return false;
        }

    } else if(!strcmp(str, "DEG_BY")) {
        char c = getchar();
        if(c != ' ') {
            errorWrongCommand(line, 0);
        } else{
            bool isError = true;
            unsigned int idx = parseVariable(line, &isError);
            error f = errorWrongVariable;
            if(!isError && isEndOfLine(line, f) && checkForStackUnderflow(s, 1, line)) {
                degBy(s, idx);
            } else {
                return false;
            }
        }
    } else if(!strcmp(str, "AT")) {
        char c = getchar();
        if(c != ' ') {
            errorWrongCommand(line, 0);
        } else {
            bool isError = true;
            long int k = parseValue(line, &isError);
            error f = errorWrongValue;
            if(!isError && isEndOfLine(line, f) && checkForStackUnderflow(s, 1, line)) {
                at(s, k);
            } else {
                return false;
            }
        }

    } else if(!strcmp(str, "PRINT")) {
        if(checkForStackUnderflow(s, 1, line)) {
            print(s);
        } else {
            return false;
        }

    } else if(!strcmp(str, "POP")) {
        if(checkForStackUnderflow(s, 1, line)) {
            Poly p = pop(s);
            PolyDestroy(&p);
        } else {
            return false;
        }

    } else {
        ungetc('\n', stdin);
        errorWrongCommand(line, 0);
        return false;
    }
    return true;
}

/**
 * Podejmuje probe wczytania i wykonania komendy.
 * @param s : stos
 * @param line : czytany wiersz
 */
bool readCommand(Stack *s, int line) {
    char *str = (char *)calloc(MAX_COMMAND_LENGTH + 1, sizeof(char));
    char c = getchar();
    int i = 0;
    while(!isspace(c)) {
        if(i == MAX_COMMAND_LENGTH) {
            errorWrongCommand(line, 0);
            free(str);
            return false;
        }
        str[i] = c;
        i++;
        c = getchar();
    }
    if(c == ' ') {
        if(!strcmp(str, "DEG_BY") || !strcmp(str, "AT")) {
            ungetc(c, stdin);
        } else {
            errorWrongCommand(line, 0);
            free(str);
            return false;
        }
    } else if(c != '\n') {
        errorWrongCommand(line, 0);
        free(str);
        return false;
    } else if(c == '\n' && (!strcmp(str, "DEG_BY") || !strcmp(str, "AT"))) {
        errorWrongCommand(line, 0);
        free(str);
        return false;
    }

    if(!chooseCommand(s, str, line)) {
        free(str);
        return false;
    }

    free(str);
    return true;
}

/**
 * Podejmuje probe parsowania wykladnika jednomianu.
 * W przypadku bledu wypisuje odpowiedni komunikat.
 * @param line : czytany wiersz
 * @param column : kolumna
 * @param k : wykladnik
 * @return true, jesli proba sie powiodla
 */
bool parseExp(int line, int *column, poly_exp_t *k) {
    char c = getchar();
    (*column)++;
    if(c == '-') {
        errorParsePoly(line, *column);
        return false;
    }
    int i = 0;
    while(isdigit(c)) {
        if(i == INT_LENGTH) {
            errorParsePoly(line, *column);
            return false;
        }
        (*k) = (*k) * 10 + c - '0';
        i++;
        c = getchar();
        (*column)++;
    }
    if(i == 0) {
        errorParsePoly(line, *column);
        return false;
    }
    ungetc(c, stdin);
    (*column)--;
    int l = (*k) / 10;
    if(i == INT_LENGTH && l >= (*k)) {
        errorParsePoly(line, *column);
        return false;
    }
    return true;
}

Mono null() {
    Poly p = PolyFromCoeff(0);
    return MonoFromPoly(&p, 0);
}

Mono parseMono(int line, int *column, bool *isError) {
    char c = getchar();
    (*column)++;
    if(c == '(') {
        bool isErrorP = false;
        Poly p = parsePoly(line, column, &isErrorP);
        if(!isErrorP) {
            c = getchar();
            (*column)++;
            if(c == ',') {
                poly_exp_t k = 0;
                if(parseExp(line, column, &k)) {
                    c = getchar();
                    (*column)++;
                    if(c == ')') {
                        *isError = false;
                        return MonoFromPoly(&p, k);
                    } else {
                        if(c == '\n') {
                            ungetc(c, stdin);
                        }
                        errorParsePoly(line, *column);
                        *isError = true;
                        return null();
                    }
                }
                PolyDestroy(&p);
                *isError = true;
                return null();
            } else {
                if(c == '\n') {
                    ungetc(c, stdin);
                }
                PolyDestroy(&p);
                errorParsePoly(line, *column);
                *isError = true;
                return null();
            }
        } else {
            PolyDestroy(&p);
            *isError = true;
            return null();
        }
    } else {
        if(c == '\n') {
            ungetc(c, stdin);
        }
        errorParsePoly(line, *column);
        *isError = true;
        return null();
    }
}

Poly parsePoly(int line, int *column, bool *isError) {
    char c = getchar();
    (*column)++;
    if(isdigit(c) || c == '-') {
        ungetc(c, stdin);
        (*column)--;
        bool isErrorP = true;
        long int k = parseCoeff(line, column, &isErrorP);
        if(!isErrorP) {
            *isError = false;
            return PolyFromCoeff(k);
        } else {
            *isError = true;
            return PolyZero();
        }
    } else if(c == '('){
        int n = N;
        Mono *monos = (Mono *)calloc(n, sizeof(Mono));
        int i = 0;
        while(c == '(') {
            ungetc(c, stdin);
            (*column)--;
            bool isErrorM = true;
            Mono m = parseMono(line, column, &isErrorM);
            if(!isErrorM) {
                if(i == n) {
                    //Mono *dest = (Mono *)calloc(2 * n, sizeof(Mono));
                    monos = (Mono *)realloc(monos, 2 * n * sizeof(Mono));
                    //free(monos);
                    //monos = dest;
                    n *= 2;
                }
                monos[i] = m;
                i++;
            } else {
                for(int j = 0; j < i; j++) {
                    Mono tmp = monos[j];
                    MonoDestroy(&tmp);
                }
                MonoDestroy(&m);
                free(monos);
                *isError = true;
                return PolyZero();
            }
            c = getchar();
            (*column)++;
            if(c == '+') {
                c = getchar();
                (*column)++;
                if(c != '(') {
                    if(c == '\n') {
                        ungetc(c, stdin);
                    }
                    for(int j = 0; j < i; j++) {
                        Mono tmp = monos[j];
                        MonoDestroy(&tmp);
                    }
                    free(monos);
                    errorParsePoly(line, *column);
                    *isError = true;
                    return PolyZero();
                }
            } else if(c != ',' &&  c != '\n') {
                for(int j = 0; j < i; j++) {
                    Mono tmp = monos[j];
                    MonoDestroy(&tmp);
                }
                free(monos);
                errorParsePoly(line, *column);
                *isError = true;
                return PolyZero();
            }
        }
        if(c == ',') {
            ungetc(c, stdin);
            (*column)--;
            *isError = false;
            Poly p = PolyAddMonos(i, monos);
            free(monos);
            return p;
        } else if(c == '\n') {
            ungetc(c, stdin);
            (*column)--;
            *isError = false;
            Poly p = PolyAddMonos(i, monos);
            free(monos);
            return p;
        } else {
            errorParsePoly(line, *column);
            *isError = true;
            for(int j = 0; j < i; j++) {
                Mono tmp = monos[j];
                MonoDestroy(&tmp);
            }
            free(monos);
            return PolyZero();
        }
    } else {
        if(c == '\n') {
            ungetc(c, stdin);
        }
        errorParsePoly(line, *column);
        *isError = true;
        return PolyZero();
    }
}

long int parseCoeff(int line, int *column, bool *isError) {
    error f = errorParsePoly;
    return parseLongNumber(line, column, isError, f);
}

bool readPolynomial(Stack *s, int line) {
    int c = getchar();
    if(c == '(') {
        ungetc(c, stdin);
        bool isError = true;
        int column = 0;
        Poly p = parsePoly(line, &column, &isError);
        if(!isError && isEndOfPoly(line, column)){
            push(s, &p);
            return true;
        } else {
            PolyDestroy(&p);
            return false;
        }
    } else if(isdigit(c) || c == '-') {
        ungetc(c, stdin);
        int column = 0;
        bool isError = true;
        long int k = parseCoeff(line, &column, &isError);
        if(!isError && isEndOfPoly(line, column)) {
            Poly p = PolyFromCoeff(k);
            push(s, &p);
            return true;
        } else {
            return false;
        }
    } else {
        errorParsePoly(line, 1);
        return false;
    }
}

bool readLine(Stack *s, int line) {
    int c = getchar();
    if(isalpha(c)) {
        ungetc(c, stdin);
        if(!readCommand(s, line)){
            return false;
        }
    } else {
        ungetc(c, stdin);
        if(!readPolynomial(s, line)) {
            return false;
        }
    }
    return true;
}

void read(Stack *s) {
    char c = getchar();
    int i = 1;
    while(c != EOF) {
        ungetc(c, stdin);
        readLine(s, i);
        i++;
        c = getchar();
    }
}

int main(void) {
    Stack *s = newStack();
    read(s);
    clear(s);
    free(s);
    return 0;
}
