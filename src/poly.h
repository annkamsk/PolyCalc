/** @file
   Interfejs klasy wielomianów

   @author Anna Kramarska <ak385833@students.mimuw.edu.pl>
   @copyright Uniwersytet Warszawski
   @date 2017-05-13
*/

#ifndef __POLY_H__
#define __POLY_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

/** Funkcja zajmujaca sie nieuzywanymi zmiennymi */
#define UNUSED(x) (void)(x)

/** Typ współczynników wielomianu */
typedef long poly_coeff_t;

/** Typ wykładników wielomianu */
typedef int poly_exp_t;

/**
 * Struktura przechowująca wielomian
 */
typedef struct Poly
{
    struct List *monos; ///< lista jednomianow
    poly_coeff_t coeff; ///< wyraz wolny
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
} Mono;

/**
 * Struktura będąca listą
 * Struktura przechowuje jednomiany należące do jednego wielomianu.
 */
typedef struct List
{
    struct Mono *m; ///< jednomian
    struct List *next; ///< nastepny element listy jednomianow
} List;

/**
 * Tworzy wielomian, który jest współczynnikiem.
 * @param[in] c : wartość współczynnika
 * @return wielomian
 */
static inline Poly PolyFromCoeff(poly_coeff_t c) {
    return (Poly) {.monos = NULL, .coeff = c};
}

/**
 * Tworzy wielomian tożsamościowo równy zeru.
 * @return wielomian
 */
static inline Poly PolyZero() {
    return (Poly) {.monos = NULL, .coeff = 0};
}

/**
 * Tworzy jednomian `p * x^e`.
 * Tworzony jednomian przejmuje na własność (kopiuje) wielomian @p p.
 * @param[in] p : wielomian - współczynnik jednomianu
 * @param[in] e : wykładnik
 * @return jednomian `p * x^e`
 */
static inline Mono MonoFromPoly(const Poly *p, poly_exp_t e) {
    return (Mono) {.p = *p, .exp = e};
}

/**
 * Sprawdza, czy wielomian jest współczynnikiem.
 * @param[in] p : wielomian
 * @return Czy wielomian jest współczynnikiem?
 */
static inline bool PolyIsCoeff(const Poly *p) {
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
    if(p -> monos == NULL && p -> coeff == 0) {
        return true;
    } else {
        return false;
    }
}

/**
 * Usuwa wszystkie wierzchołki listy z pamięci razem
 * z jednomianami, do których trzyma wskaźniki.
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
    return (Mono) {.p = PolyClone(&(m -> p)), .exp = m -> exp};
}

/**
 * Zlicza, ile jednomianów ma wielomian @p p.
 * @param[in] p : wielomian
 * @return liczba jednomianów
 */
unsigned HowManyMonos(const Poly *p);

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
* @param count : liczba jednomianów
* @param monos : tablica jednomianów
* @return wielomian będący sumą jednomianów
*/
Poly PolyAddMonos(unsigned count, const Mono monos[]);

/**
 * Tworzy listę jednomianów z jednomianów umieszczonych w tablicy.
 * @param n : wielkość tablicy
 * @param monos : tablica jednomianów
 * @return lista jednomianów
 */
List *MonosToList(int n, Mono monos[]);

/**
 * Sprowadza wielomian do postaci normalnej.
 * Wielomian jest przedstawiony jako lista monomianów i wyraz wolny.
 * @param head : pierwszy element listy
 * @param coeff : wyraz wolny
 * @return pierwszy element zmienionej listy
 */
List *MonosNormalise(List *head, poly_coeff_t *coeff);

/**
 * Dodaje do siebie te jednomiany z listy, ktore maja rowne wykladniki.
 * @param head : pierwszy elementy listy
 * @return pierwszy element zmienionej listy
 */
List *deleteMonosWithEqualExp(List *head);

/**
 * Usuwa z listy jednomianów te, które są wyrazami stałymi
 * (mają wykładnik 0, a ich współczynnik jest wielomianem stałym).
 * @param head : pierwszy element listy
 * @param coeff : wyraz wolny
 * @return pierwszy element zmienionej listy
 */
List *deleteConstantMonos(List *head, poly_coeff_t *coeff);

/**
 * Porównuje dwa jednomiany względem ich stopnia.
 * @param elem1 : jednomian Mono
 * @param elem2 : jednomian Mono
 * @return 1, gdy elem1 > elem2; -1 gdy elem1 < elem2; 0 wpp
 */
int compare(const void *elem1, const void *elem2);

/**
 * Mnoży dwa wielomiany.
 * @param p : wielomian
 * @param q : wielomian
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
 * Zmienia wielomian na przeciwny.
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
 * Wylicza wartości potęg x dla danych wykładników
 * oraz zamienia je w wielomiany stałe.
 * @param[in] x
 * @param[in] count : liczba wykładników
 * @param[in] powers : x podniesione do danej potęgi
 * @param[in] exps : wykładniki
 * @param[in] coeffs : wielomiany stałe
 */
void PowersFromPoly(poly_coeff_t x, unsigned count, poly_coeff_t powers[], poly_exp_t exps[], Poly coeffs[]);


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

/**
 * W wielomianie @p podstawia pod zmienna o indeksie i wielomian x[i].
 * @param p : wielomian
 * @param count : liczba wielomianow do podstawienia
 * @param x : tablica wielomianow do podstawienia
 * @return utworzony w ten sposob wielomian
 */
Poly PolyCompose(const Poly *p, unsigned count, const Poly x[]);

/**
 * Rekurencyjnie znajduje wielomian wynikowy dla funkcji PolyCompose.
 * @param p : wielomian
 * @param count : liczba wielomianow do podstawienia
 * @param x : tablica wielomianow do podstawienia
 * @param idx : indeks zmiennej aktualnie obliczanej
 * @return wielomian wynikowy
 */
Poly PolyComposeHelp(const Poly *p, unsigned count, const Poly x[], unsigned idx);

/**
 * Podnosi wielomian do potęgi n.
 * @param p : wielomian
 * @param n : potęga
 * @return wynik
 */
Poly PolyExp(const Poly *p, unsigned n);

#endif /* __POLY_H__ */
