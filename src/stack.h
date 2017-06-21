/** @file
   Interfejs klasy stos.

   @author Anna Kramarska <ak385833@students.mimuw.edu.pl>
   @copyright Uniwersytet Warszawski
   @date 2017-05-13
*/


#include "poly.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utils.h"

/**
 * Struktura przechowujaca element na stosie.
 */
typedef struct StackEl
{
    struct StackEl *next; ///< nastepny element za wierzcholkiem
    Poly poly; ///< wielomian
} StackEl;

/**
 * Struktura reprezentujaca stos.
 */
typedef struct Stack
{
    struct StackEl *top; ///< wierzcholek stosu
} Stack;

/**
 * Tworzy nowy stos.
 * @return stos
 */
Stack *newStack() {
    Stack *s = (Stack *)malloc(sizeof(Stack));
    s -> top = NULL;
    return s;
}

/**
 * Sprawdza, czy stos jest pusty.
 * @param s : stos
 * @return true, jesli pusty
 */
bool isEmpty(Stack *s) {
    return !(s -> top);
}

/**
 * Zwraca element stosu bedacy wierzcholkiem.
 * @param s : stos
 * @return  wierzcholek stosu
 */
StackEl *StackTop(Stack *s) {
    return s -> top;
}

/**
 * Zwraca wielomian znajdujacy sie w wierzcholku stosu.
 * @param s : stos
 * @return wielomian
 */
Poly top(Stack *s) {
    return s -> top -> poly;
}

/**
 * Usuwa wierzcholek ze stosu.
 * @param s : stos
 * @return zwraca wielomian, ktory byl w wierzcholku
 */
Poly pop(Stack *s) {
    StackEl *topEl = StackTop(s);
    Poly p;
    if(topEl != NULL) {
        p = topEl -> poly;
        s -> top = topEl -> next;
        free(topEl);
    }
    return p;
}

/**
 * Wklada na stos wierzcholek z wielomianem.
 * Przejmuje wielomian na wlasnosc.
 * @param s : stos
 * @param poly
 */
void push(Stack *s, Poly *poly) {
    StackEl *el = (StackEl *)malloc(sizeof(StackEl));
    el -> poly = *poly;
    el -> next = s -> top;
    s -> top = el;
}

/**
 * Usuwa kolejne elementy stosu.
 * @param el : wierzcholek
 */
void StackElDestroy(StackEl *el) {
    if(el -> next != NULL) {
        StackElDestroy(el -> next);
        el -> next = NULL;
    }
    PolyDestroy(&(el -> poly));
    free(el);
}

/**
 * Czysci stos.
 * @param s : stos
 */
void clear(Stack *s) {
    if(s -> top != NULL) {
        StackElDestroy(s -> top);
        s -> top = NULL;
    }
}

