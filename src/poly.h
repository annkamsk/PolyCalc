/** @file
   Interfejs klasy wielomianów

   @author Jakub Pawlewicz <pan@mimuw.edu.pl>, TODO
   @copyright Uniwersytet Warszawski
   @date 2017-04-09, TODO
*/

#ifndef __POLY_H__
#define __POLY_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

#define UNUSED(x) (void)(x)

/** Typ współczynników wielomianu */
typedef long poly_coeff_t;

/** Typ wykładników wielomianu */
typedef int poly_exp_t;

/**
 * Struktura przechowująca wielomian
 * TODO
 */
typedef struct Poly
{
    struct List *monos;
    poly_coeff_t coeff;
} Poly;

/**
  * Struktura przechowująca jednomian
  * Jednomian ma postać `p * x^e`.
  * Współczynnik `p` może też być wielomianem.
  * Będzie on traktowany jako wielomian nad kolejną zmienną (nie nad x).
  */
typedef struct Mono
{
    Poly p; ///< współczynnik
    poly_exp_t exp; ///< wykładnik
    /* TODO */
} Mono;

/** Struktura będąca listą */
typedef struct List
{
    struct Mono *m;
    struct List *next;
} List;

/*
 * TO DELETE
 */
void PolyToString(Poly *p);

void PolyToStringHelp(Poly *p, int ind);

/**
 * Tworzy wielomian, który jest współczynnikiem.
 * @param[in] c : wartość współczynnika
 * @return wielomian
 */
static inline Poly PolyFromCoeff(poly_coeff_t c) {
    /* TODO */
    return (Poly) {.monos = NULL, .coeff = c};
}

/**
 * Tworzy wielomian tożsamościowo równy zeru.
 * @return wielomian
 */
static inline Poly PolyZero() {
    /* TODO */
    return (Poly) {.monos = NULL, .coeff = 0};
}

/**
 * Tworzy jednomian `p * x^e`.
 * Tworzony jednomian przejmuje na własność (kopiuje) wielomian @p p.
 * @param[in] p : wielomian - współczynnik jednomianu
 * @param[in] e : wykładnik
 * @return jednomian `p * x^e`
 */
static inline Mono MonoFromPoly(Poly *p, poly_exp_t e) {
    return (Mono) {.p = *p, .exp = e};
}

/**
 * Sprawdza, czy wielomian jest współczynnikiem.
 * @param[in] p : wielomian
 * @return Czy wielomian jest współczynnikiem?
 */
static inline bool PolyIsCoeff(const Poly *p) {
    /* TODO */
    if(p -> monos == NULL && p -> coeff != 0) {
        return true;
    } else {
        return false;
    }
}

/**
 * Sprawdza, czy wielomian jest tożsamościowo równy zeru.
 * @param[in] p : wielomian
 * @return Czy wielomian jest równy zero?
 */
static inline bool PolyIsZero(const Poly *p) {
    /* TODO */
    if(p -> monos == NULL && p -> coeff == 0) {
        return true;
    } else {
        return false;
    }
}

/**
 * Usuwa listę z pamięci.
 * @param[in] l : lista
 */
void ListDestroy(List *l);

/**
 * Usuwa z pamięci pojedynczy wierzchołek listy.
 * @param[in] l : wierzchołek
 */
void ListNodeDestroy(List *l);

/**
 * Usuwa wielomian z pamięci.
 * @param[in] p : wielomian
 */
void PolyDestroy(Poly *p);

/**
 * Usuwa jednomian z pamięci.
 * @param[in] m : jednomian
 */
static inline void MonoDestroy(Mono *m) {
    /* TODO */
    if(m != NULL) {
        PolyDestroy(&(m -> p));
    }
}

/**
 * Robi pełną, głęboką kopię wielomianu.
 * @param[in] p : wielomian
 * @return skopiowany wielomian
 */
Poly PolyClone(const Poly *p);

/**
 * Robi pełną, głęboką kopię jednomianu.
 * @param[in] m : jednomian
 * @return skopiowany jednomian
 */
static inline Mono MonoClone(const Mono *m) {
    /* TODO */
    return (Mono) {.p = PolyClone(&(m -> p)), .exp = m -> exp};
}

/**
 * @Zlicza, ile jednomianów ma wielomian.
 * @param[in] p : wielomian
 * @return liczba jednomianów
 */
unsigned HowManyMonos(const Poly *p);

/**
 * @Dodaje do siebie dwa jednomiany o tym samym wykładniku
 * @param[in] m : jednomian
 * @param[in] n : jednomian
 * @return jednomian będący sumą jednomianów
 */
Mono MonoAdd(const Mono *m, const Mono *n);

/**
 * Dodaje dwa wielomiany.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p + q`
 */
Poly PolyAdd(const Poly *p, const Poly *q);

/**
* Sumuje listę jednomianów i tworzy z nich wielomian.
* Przejmuje na własność zawartość tablicy @p monos.
* @param[in] count : liczba jednomianów
* @param[in] monos : tablica jednomianów
* @return wielomian będący sumą jednomianów
*/
Poly PolyAddMonos(unsigned count, const Mono monos[]);

/**
 * Sumuje listę wielomianów i tworzy z nich jeden wielomian.
 * @param[in] count : liczba wielomianów
 * @param[in] polies : tablica wielomianów
 * @return wielomian będący sumą wielomianów
 */
Poly PolyAddPolies(unsigned count, const Poly polies[]);

/**
 * Tworzy listę jednomianów z jednomianów umieszczonych w tablicy.
 * @param[in] n : wielkość tablicy
 * @param[in] monos : tablica jednomianów
 * @return lista jednomianów
 */
List *MonosToList(int n, Mono monos[]);

/**
 * Sprowadza wielomian do postaci normalnej.
 * Wielomian jest przedstawiony jako lista monomianów i wyraz wolny.
 * @param[in] head : pierwszy jednomian na liście
 * @param coeff[in] : wyraz wolny
 * @return pierwszy element zmienionej listy
 */
List *MonosNormalise(List *head, poly_coeff_t *coeff);

/**
 * Usuwa z listy jednomianów te, które są wyrazami stałymi (mają wykładnik 0, a ich współczynnik jest wielomianem stałym).
 * @param[in] head : pierwszy jednomian na liście
 * @param[in] coeff : wyraz wolny
 * @return pierwszy element zmienionej listy
 */
List *deleteConstantMonos(List *head, poly_coeff_t *coeff);

/**
 * Usuwa z listy jednomianów te, które są zerowe (ich współczynnik jest wielomianem zerowym).
 * @param head[in] : pierwszy jednomian na liście
 * @return pierwszy element zmienionej listy
 */
List *deleteZeroMonos(List *head);
/**
 * Porównuje dwa jednomiany względem ich stopnia.
 * @param[in] elem1 : jednomian Mono
 * @param[in] elem2 : jednomian Mono
 * @return 1, gdy elem1 > elem2; -1 gdy elem1 < elem2; 0 wpp
 */
int compare(const void *elem1, const void *elem2);

/**
 * Mnoży dwa wielomiany.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p * q`
 */
Poly PolyMul(const Poly *p, const Poly *q);

/**
 * Zwraca przeciwny wielomian.
 * @param[in] p : wielomian
 * @return `-p`
 */
Poly PolyNeg(const Poly *p);

/**
 * Neguje wielomian.
 * @param[in] p : wielomian
 */
void PolyNegHelp(Poly *p);

/**
 * Odejmuje wielomian od wielomianu.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p - q`
 */
Poly PolySub(const Poly *p, const Poly *q);

/**
 * Zwraca stopień wielomianu ze względu na zadaną zmienną (-1 dla wielomianu
 * tożsamościowo równego zeru).
 * Zmienne indeksowane są od 0.
 * Zmienna o indeksie 0 oznacza zmienną główną tego wielomianu.
 * Większe indeksy oznaczają zmienne wielomianów znajdujących się
 * we współczynnikach.
 * @param[in] p : wielomian
 * @param[in] var_idx : indeks zmiennej
 * @return stopień wielomianu @p p z względu na zmienną o indeksie @p var_idx
 */
poly_exp_t PolyDegBy(const Poly *p, unsigned var_idx);

/**
 * Przeszukuje jednomiany zadanego wielomianu w poszukiwaniu
 * jednomianu o najwyższym stopniu ze względu na zadaną zmienną.
 * @param[in] p : wielomian
 * @param var_idx : indeks zmiennej
 * @param i : indeks zmiennej względem której wielomian jest aktualnie
 * przeszukiwany.
 * @return stopień wielomianu @p p ze względu na zmienną o indeksie @p var_idx
 */
poly_exp_t PolyDegByHelp(const Poly* p, unsigned var_idx, unsigned i);

/**
 * Zwraca stopień wielomianu względem zmiennej o indeksie 0.
 * @param[in] p : wielomian
 * @return stopień wielomianu @p p
 */
poly_exp_t PolyDegByZero(const Poly *p);

/**
 * Zwraca stopień wielomianu (-1 dla wielomianu tożsamościowo równego zeru).
 * @param[in] p : wielomian
 * @return stopień wielomianu @p p
 */
poly_exp_t PolyDeg(const Poly *p);

/**
 * Sprawdza równość dwóch wielomianów.
 * @param[in] p : wielomian
 * @param[in] q : wielomian
 * @return `p = q`
 */
bool PolyIsEq(const Poly *p, const Poly *q);

/**
 * @brief Wylicza wartości potęg x dla danych wykładników.
 * @param[in] x
 * @param[in] count : liczba wykładników
 * @param[in] powers : potęgi
 * @param[in] exps : wykładniki
 */
void PowersFromPoly(poly_coeff_t x, unsigned count, poly_coeff_t powers[], poly_exp_t exps[]);


/**
 * Wylicza wartość wielomianu w punkcie @p x.
 * Wstawia pod pierwszą zmienną wielomianu wartość @p x.
 * W wyniku może powstać wielomian, jeśli współczynniki są wielomianem
 * i zmniejszane są indeksy zmiennych w takim wielomianie o jeden.
 * Formalnie dla wielomianu @f$p(x_0, x_1, x_2, \ldots)@f$ wynikiem jest
 * wielomian @f$p(x, x_0, x_1, \ldots)@f$.
 * @param[in] p
 * @param[in] x
 * @return @f$p(x, x_0, x_1, \ldots)@f$
 */
Poly PolyAt(const Poly *p, poly_coeff_t x);

#endif /* __POLY_H__ */
