/** @file
   Implementacja kalkulatora.

   @author Anna Kramarska <ak385833@students.mimuw.edu.pl>
   @copyright Uniwersytet Warszawski
   @date 2017-05-13
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include "poly.h"
#include "stack.h"
#include "utils.h"


/** Dlugosc liczby LONG_MAX */
#define LONG_MAX_LENGTH 19

/** Dlugosc liczby UINT_MAX */
#define UINT_LENGTH 10

/** Dlugosc liczby INT_MAX */
#define INT_LENGTH 10

/** Maksymalna dlugosc komendy */
#define MAX_COMMAND_LENGTH 8

/** Wartosc zwracana przy braku bledu */
#define NO_ERROR -2

/** Wartosc zwracana w przypadku bledu */
#define ERROR 2

/** Domyslna dlugosc tablicy jednomianow */
#define N 2

/** Funkcja obslugujaca nieuzywane zmienne */
#define UNUSED(x) (void)(x)

Mono parseMono(int line, int *column, bool *isError);
Poly parsePoly(int line, int *column, bool *isError);
long int parseCoeff(int line, int *column, bool *isError);

/**
 * Wskaznik na funkcje wywolywana w przypadku bledu.
 */
typedef void( * error)(int, int);

/**
 * Wstawia na stos wielomian zerowy.
 * @param s : stos
 */
void zero(Stack *s) {
    Poly p = PolyZero();
    push(s, &p);
}

/**
 * Sprawdza, czy wielomian w wierzcholku stosu jest staly.
 * Wypisuje 1, jesli tak, 0 wpp.
 * @param s : stos
 */
void isCoeff(Stack *s) {
    Poly p = top(s);
    if(PolyIsZero(&p) || PolyIsCoeff(&p)) {
        printf("%d\n", 1);
    } else {
        printf("%d\n", 0);
    }
}

/**
 * Sprawdza, czy wielomian w wierzcholku stosu jest zerowy.
 * Wypisuje 1, jesli tak, 0 wpp.
 * @param s : stos
 */
void isZero(Stack *s) {
    Poly p = top(s);
    if(PolyIsZero(&p)) {
        printf("%d\n", 1);
    } else {
        printf("%d\n", 0);
    }
}

/**
 * Wstawia na stos kopie wielomianu z wierzcholka.
 * @param s : stos
 */
void clone(Stack *s) {
    Poly PolyTop = top(s);
    Poly p = PolyClone(&PolyTop);
    push(s, &p);
}

/**
 * Zdejmuje ze stosu dwa wielomiany i dodaje na stos ich sume.
 * @param s : stos
 */
void add(Stack *s) {
    Poly p = pop(s);
    Poly q = pop(s);
    Poly sum = PolyAdd(&p, &q);
    push(s, &sum);
    PolyDestroy(&p);
    PolyDestroy(&q);
}


/**
 * Zdejmuje ze stosu dwa wielomiany i dodaje na stos ich iloczyn.
 * @param s : stos
 */
void mul(Stack *s) {
    Poly p = pop(s);
    Poly q = pop(s);
    Poly mul = PolyMul(&p, &q);
    push(s, &mul);
    PolyDestroy(&p);
    PolyDestroy(&q);
}


/**
 * Zdejmuje ze stosu wielomian i dodaje na stos wielomian przeciwny.
 * @param s : stos
 */
void neg(Stack *s) {
    Poly p = pop(s);
    Poly neg = PolyNeg(&p);
    push(s, &neg);
    PolyDestroy(&p);
}


/**
 * Zdejmuje ze stosu dwa wielomiany i dodaje na stos ich roznice.
 * @param s : stos
 */
void sub(Stack *s) {
    Poly p = pop(s);
    Poly q = pop(s);
    Poly sub = PolySub(&p, &q);
    push(s, &sub);
    PolyDestroy(&p);
    PolyDestroy(&q);
}

/**
 * Zdejmuje ze stosu wielomian, sprawdza, czy jest rowny aktualnemu wierzcholkowi i wklada go z powrotem.
 * Jest wielomiany sa rowne, wypisuje 1, wpp wypisuje 0.
 * @param s : stos
 */
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

/**
 * Wypisuje na standardowe wyjscie stopien wielomianu z wierzcholka.
 * @param s : stos
 */
void deg(Stack *s) {
    Poly p = top(s);
    printf("%d\n", PolyDeg(&p));
}

/**
 * Wypisuje na standardowe wyjscie stopien wielomianu z wierzcholka ze wzgledu na zadana zmienna.
 * @param s : stos
 * @param idx : indeks zmiennej
 */
void degBy(Stack *s, int idx) {
    Poly p = top(s);
    printf("%d\n", PolyDegBy(&p, idx));
}

/**
 * Wypisuje na standardowe wyjscie wartosc wielomianu z wierzcholka dla wartosci x.
 * @param s : stos
 * @param x : wartosc zmiennej
 */
void at(Stack *s, poly_coeff_t x) {
    Poly p = pop(s);
    Poly result = PolyAt(&p, x);
    push(s, &result);
    PolyDestroy(&p);
}

/**
 * Zdejmuje ze stosu wielomian i podstawia pod jego zmienne o indeksach [0, count) kolejne count wielomianow ze stosu.
 * Wklada na stos wynik.
 * @param s : stos
 * @param count : liczba wielomianow do zdjecia i podstawienia
 */
void compose(Stack *s, unsigned count) {
    Poly p = pop(s);
    Poly *x = (Poly *)calloc(count, sizeof(Poly));
    unsigned i = 0;
    while(i != count) {
        x[i] = pop(s);
        i++;
    }
    Poly result = PolyCompose(&p, count, x);
    push(s, &result);
    for(i = 0; i < count; i++) {
        Poly tmp = x[i];
        PolyDestroy(&tmp);
    }
    free(x);
    PolyDestroy(&p);
}

/**
 * Wypisuje wielomian rekurencyjnie po kolejnych indeksach zmiennych na standardowe wyjscie.
 * @param p : wielomian
 * @param coeff : suma wyrazow stalych wielomianow zmiennych wyzszych indeksow
 */
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

/**
 * Wypisuje wielomian z wierzcholka stosu na standardowe wyjscie.
 * @param s : stos
 */
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

/**
 *  Wypisuje komunikat na wyjscie diagnostyczne w przypadku bledu.
 * @param line : wiersz
 * @param column : kolumna
 */
void errorUnderflow(int line, int column) {
    fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line);
    UNUSED(column);
}

/**
 *  Wypisuje komunikat na wyjscie diagnostyczne w przypadku bledu.
 * @param line : wiersz
 * @param column : kolumna
 */
void errorParsePoly(int line, int column) {
    fprintf(stderr, "ERROR %d %d\n", line, column);
    parseToTheEnd();
}

/**
 *  Wypisuje komunikat na wyjscie diagnostyczne w przypadku bledu.
 * @param line : wiersz
 * @param column : kolumna
 */
void errorWrongCommand(int line, int column) {
    fprintf(stderr, "ERROR %d WRONG COMMAND\n", line);
    UNUSED(column);
    parseToTheEnd();
}

/**
 *  Wypisuje komunikat na wyjscie diagnostyczne w przypadku bledu.
 * @param line : wiersz
 * @param column : kolumna
 */
void errorWrongValue(int line, int column) {
    fprintf(stderr, "ERROR %d WRONG VALUE\n", line);
    UNUSED(column);
    parseToTheEnd();
}

/**
 *  Wypisuje komunikat na wyjscie diagnostyczne w przypadku bledu.
 * @param line : wiersz
 * @param column : kolumna
 */
void errorWrongVariable(int line, int column) {
    fprintf(stderr, "ERROR %d WRONG VARIABLE\n", line);
    UNUSED(column);
    parseToTheEnd();
}

/**
 *  Wypisuje komunikat na wyjscie diagnostyczne w przypadku bledu.
 * @param line : wiersz
 * @param column : kolumna
 */
void errorWrongCount(int line, int column) {
    fprintf(stderr, "ERROR %d WRONG COUNT\n", line);
    UNUSED(column);
    parseToTheEnd();
}

/**
 * Sprawdza, czy jest juz koniec linii.
 * Wypisuje stosowny komunikat w przypadku bledu.
 * @param line : wiersz
 * @param error : funkcja zwracajaca blad
 * @return true w przypadku braku bledu
 */
bool isEndOfLine(int line, void(*error)(int line, int column)) {
    int c = getchar();
    if(c != '\n' && c != EOF) {
        (*error)(line, 0);
        return false;
    } else {
        return true;
    }
}

/**
 * Sprawdza, czy jest juz koniec linii po wczytywaniu wielomianu.
 * Wypisuje stosowny komunikat w przypadku bledu.
 * @param line : wiersz
 * @param column: kolumna
 * @return true w przypadku braku bledu
 */
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
 * @param n : liczba potrzebnych elementow
 * @param line : nr wiersza
 * @return true, jesli sa potrzebne elementy
 */
bool checkForStackUnderflow(Stack *s, int n, int line) {
    if(n == 0) {
        return true;
    }
    StackEl *el1 = StackTop(s);
    if(el1 == NULL) {
        errorUnderflow(line, 0);
        return false;
    }
    if(n == 1) {
        return true;
    }
    int count = 0;
    for(StackEl *el = el1; el != NULL && count != n; el = el -> next) {
        count++;
    }
    if(count != n) {
        errorUnderflow(line, 0);
        return false;
    }
    return true;
}

/**
 * Sprawdza, czy liczba miesci sie w zakresie [0, UINT_MAX].
 * Zwraca odpowiedni komunikat w przypadku bledu.
 * @param line : wiersz
 * @param column : kolumna
 * @param isError : zmienna informujaca o pojawieniu sie bledu
 * @param error : funkcja zwracajaca komunikat w przypadku bledu
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
 * Sprawdza, czy liczba miesci sie w zakresie [LONG_MIN, LONG_MAX].
 * Zwraca odpowiedni komunikat w przypadku bledu.
 * @param line : wiersz
 * @param column : kolumna
 * @param isError : zmienna informujaca o pojawieniu sie bledu
 * @param error : funkcja zwracajaca komunikat w przypadku bledu
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
 * @param isError : zmienna informujaca o pojawieniu sie bledu
 * @return true, jesli wczytywanie sie powiedzie
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
 * @param isError : zmienna informujaca o pojawieniu sie bledu
 * @return true, jesli wczytywanie sie powiedzie
 */
unsigned int parseVariable(int line, bool *isError) {
    error f = errorWrongVariable;
    int column = 0;
    return parseUIntNumber(line, &column, isError, f);
}

/**
 * Podejmuje probe wczytania liczby zmiennych dla funkcji COMPOSE.
 * W przypadku bledu wypisuje stosowny komunikat i zwraca false.
 * @param line : wiersz
 * @param isError : zmienna informujaca o pojawieniu sie bledu
 * @return true, jesli wczytywanie sie powiedzie
 */
unsigned int parseCount(int line, bool *isError) {
    error f = errorWrongCount;
    int column = 0;
    return parseUIntNumber(line, &column, isError, f);
}

/**
 * Sprawdza, jaki komunikat zostal wyslany do kalkulatora i podejmuje akcje.
 * W przypadku bledu wyswietla komunikat o bledzie.
 * @param s : stos
 * @param str : komunikat
 * @param line : wiersz
 * @return true, jesli nie pojawi sie blad
 */
void chooseCommand(Stack *s, char *str, int line) {
    if(!strcmp(str, "ZERO")) {
        zero(s);
    } else if(!strcmp(str, "IS_COEFF")) {
        if(checkForStackUnderflow(s, 1, line)) {
            isCoeff(s);
        }

    } else if(!strcmp(str, "IS_ZERO")) {
        if(checkForStackUnderflow(s, 1, line)) {
            isZero(s);
        }

    } else if(!strcmp(str, "CLONE")) {
        if(checkForStackUnderflow(s, 1, line)) {
            clone(s);
        }

    } else if(!strcmp(str, "ADD")) {
        if(checkForStackUnderflow(s, 2, line)) {
            add(s);
        }

    } else if(!strcmp(str, "MUL")) {
        if(checkForStackUnderflow(s, 2, line)) {
            mul(s);
        }

    } else if(!strcmp(str, "NEG")) {
        if(checkForStackUnderflow(s, 1, line)) {
            neg(s);
        }

    } else if(!strcmp(str, "SUB")) {
        if(checkForStackUnderflow(s, 2, line)) {
            sub(s);
        }

    } else if(!strcmp(str, "IS_EQ")) {
        if(checkForStackUnderflow(s, 2, line)) {
            isEq(s);
        }

    } else if(!strcmp(str, "DEG")) {
        if(checkForStackUnderflow(s, 1, line)) {
            deg(s);
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
            }
        }

    } else if(!strcmp(str, "PRINT")) {
        if(checkForStackUnderflow(s, 1, line)) {
            print(s);
        }
    } else if(!strcmp(str, "POP")) {
        if(checkForStackUnderflow(s, 1, line)) {
            Poly p = pop(s);
            PolyDestroy(&p);
        }
    } else if(!strcmp(str, "COMPOSE")) {
        char c = getchar();
        if(c != ' ') {
            errorWrongCommand(line, 0);
        } else {
            bool isError = true;
             error f = errorWrongCount;
            unsigned int count = parseCount(line, &isError);
            if(count == UINT_MAX) {
                if(isEndOfLine(line, f)) {
                    errorUnderflow(line, 0);
                }
            } else if(!isError && isEndOfLine(line, f) && checkForStackUnderflow(s, count + 1, line)) {
                compose(s, count);
            }
        }
    } else {
        ungetc('\n', stdin);
        errorWrongCommand(line, 0);
    }
}

/**
 * Podejmuje probe wczytania komendy.
 * @param s : stos
 * @param line : czytany wiersz
 */
void readCommand(Stack *s, int line) {
    char *str = (char *)calloc(MAX_COMMAND_LENGTH + 1, sizeof(char));
    assert(str != NULL);
    char c = getchar();
    int i = 0;
    while(!isspace(c)) {
        if(i == MAX_COMMAND_LENGTH) {
            errorWrongCommand(line, 0);
            free(str);
            return;
        }
        str[i] = c;
        i++;
        c = getchar();
    }
    if(c == ' ') {
        if(!strcmp(str, "DEG_BY") || !strcmp(str, "AT") || !strcmp(str, "COMPOSE")) {
            ungetc(c, stdin);
        } else {
            errorWrongCommand(line, 0);
            free(str);
            return;
        }
    } else if(c != '\n') {
        errorWrongCommand(line, 0);
        free(str);
        return;
    } else if(c == '\n' && (!strcmp(str, "DEG_BY") || !strcmp(str, "AT") || !strcmp(str, "COMPOSE"))) {
        errorWrongCommand(line, 0);
        free(str);
        return;
    }

    chooseCommand(s, str, line);
    free(str);
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

/**
 * Tworzy jednoomian tozsamosciowo rowny zeru.
 * @return jednomian
 */
Mono null() {
    Poly p = PolyFromCoeff(0);
    return MonoFromPoly(&p, 0);
}

/**
 * Wczytuje pojedynczy jednomian i sprawdza jego poprawnosc.
 * @param line : czytany wiersz
 * @param column : czytana kolumna
 * @param isError : zmienna informujaca o bledzie
 * @return jednomian
 */
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

/**
 * Wczytuje pojedynczy wielomian i sprawdza jego poprawnosc.
 * @param line : czytany wiersz
 * @param column : czytana kolumna
 * @param isError : zmienna informujaca o bledzie
 * @return wielomian
 */
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
        assert(monos != NULL);
        int i = 0;
        while(c == '(') {
            ungetc(c, stdin);
            (*column)--;
            bool isErrorM = true;
            Mono m = parseMono(line, column, &isErrorM);
            if(!isErrorM) {
                if(i == n) {
                    monos = (Mono *)realloc(monos, 2 * n * sizeof(Mono));
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

/**
 * Wczytuje wyraz wolny wielomianu.
 * @param line : wczytywany wiersz
 * @param column : wczytywana kolumna
 * @param isError : zmienna informujaca o bledzie
 * @return wyraz wolny
 */
long int parseCoeff(int line, int *column, bool *isError) {
    error f = errorParsePoly;
    return parseLongNumber(line, column, isError, f);
}

/**
 * Podejmuje probe wczytania wielomianu i w razie powodzenia wrzuca go na stos.
 * @param s : stos
 * @param line : czytany wiersz
 * @return true, jesli nie bylo bledow
 */
void readPolynomial(Stack *s, int line) {
    int c = getchar();
    if(c == '(') {
        ungetc(c, stdin);
        bool isError = true;
        int column = 0;
        Poly p = parsePoly(line, &column, &isError);
        if(!isError && isEndOfPoly(line, column)){
            push(s, &p);
        } else {
            PolyDestroy(&p);
        }
    } else if(isdigit(c) || c == '-') {
        ungetc(c, stdin);
        int column = 0;
        bool isError = true;
        long int k = parseCoeff(line, &column, &isError);
        if(!isError && isEndOfPoly(line, column)) {
            Poly p = PolyFromCoeff(k);
            push(s, &p);
        } else {
        }
    } else {
        errorParsePoly(line, 1);
    }
}

/**
 * Wczytuje pojedyncza linie z wejscia.
 * @param s : stos
 * @param line : linia
 */
void readLine(Stack *s, int line) {
    int c = getchar();
    if(isalpha(c)) {
        ungetc(c, stdin);
        readCommand(s, line);
    } else {
        ungetc(c, stdin);
        readPolynomial(s, line);
    }
}

/**
 * Wczytuje cale wejscie standardowe.
 * @param s : stos
 */
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

/**
 * Funkcja main implementacji kalkulatora.
 */
int main() {
    Stack *s = newStack();
    read(s);
    clear(s);
    free(s);
    return 0;
}
