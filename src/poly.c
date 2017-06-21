/** @file
   Implementacja klasy wielomianów

   @author Anna Kramarska <ak385833@students.mimuw.edu.pl>
   @copyright Uniwersytet Warszawski
   @date 2017-05-13
*/


#include <math.h>
#include <stdio.h>
#include "poly.h"

void PolyDestroy(Poly *p) {
    if(p != NULL) {
        if(p -> monos != NULL) {
            ListDestroy(p -> monos);
        }
    }
}

void ListDestroy(List *l) {
    if(l != NULL) {
        if(l -> next != NULL) {
            ListDestroy(l -> next);
        }
        l -> next = NULL;
        ListNodeDestroy(l);
    }
}

void ListNodeDestroy(List *l) {
    if(l != NULL) {
        MonoDestroy(l -> m);
        free(l -> m);
        l -> m = NULL;
        free(l);
        l = NULL;
    }
}

Poly PolyClone(const Poly *p) {
    if(p -> monos != NULL) {
        List *monos = (List *)malloc(sizeof(List));
        assert(monos != NULL);
        monos -> m = (Mono *)malloc(sizeof(Mono));
        assert(monos -> m != NULL);
        *(monos -> m) = MonoClone(p -> monos -> m);
        monos -> next = NULL;
        List *temp = monos;
        for(List *count = p -> monos -> next; count != NULL; count = count -> next) {
            temp -> next = (List *)malloc(sizeof(List));
            assert(temp -> next != NULL);
            temp = temp -> next;
            temp -> m = (Mono *)malloc(sizeof(Mono));
            assert(temp -> m != NULL);
            *(temp -> m) = MonoClone(count -> m);
            temp -> next = NULL;
        }
        return (Poly) {.coeff = p -> coeff, .monos = monos};
    } else {
        return (Poly) {.coeff = p -> coeff, .monos = NULL};
    }
}

unsigned HowManyMonos(const Poly *p) {
    unsigned i = 0;
    for(List *l = p -> monos; l != NULL; i++, l = l -> next);
    return i;
}

Poly PolyAdd(const Poly *p, const Poly *q) {
    if(PolyIsZero(p) && PolyIsZero(q)) {
        return PolyZero();
    } else if(PolyIsZero(p)) {
        return PolyClone(q);
    } else if(PolyIsZero(q)) {
        return PolyClone(p);
    } else if(PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return PolyFromCoeff(p -> coeff + q -> coeff);
    } else if(PolyIsCoeff(p)) {
        Poly k = PolyClone(q);
        k.coeff += p -> coeff;
        return k;
    } else if(PolyIsCoeff(q)) {
        Poly k = PolyClone(p);
        k.coeff += q -> coeff;
        return k;
    } else {
        Poly k;
        k.coeff = p -> coeff + q -> coeff;
        List *head = (List *)malloc(sizeof(List));
        assert(head != NULL);
        head -> next = NULL;
        head -> m = NULL;
        List *l_p = p -> monos;
        List *l_q = q -> monos;
        List *tmp = head;
        while(l_p != NULL && l_q != NULL) {
            if(l_p -> m -> exp > l_q -> m -> exp) {
                Mono *m = (Mono *)malloc(sizeof(Mono));
                assert(m != NULL);
                *m = MonoClone(l_q -> m);
                l_q = l_q -> next;
                tmp -> next = (List *)malloc(sizeof(List));
                assert(tmp -> next != NULL);
                tmp = tmp -> next;
                tmp -> m = m;
                tmp -> next = NULL;
            } else if(l_p -> m -> exp < l_q -> m -> exp) {
                Mono *m = (Mono *)malloc(sizeof(Mono));
                assert(m != NULL);
                *m = MonoClone(l_p -> m);
                l_p = l_p -> next;
                tmp -> next = (List *)malloc(sizeof(List));
                assert(tmp -> next != NULL);
                tmp = tmp -> next;
                tmp -> m = m;
                tmp -> next = NULL;
            } else {
                Poly l = PolyAdd(&(l_p -> m -> p), &(l_q -> m -> p));
                if(PolyIsZero(&l)) {
                    PolyDestroy(&l);
                } else if(l_p -> m -> exp == 0 && PolyIsCoeff(&l)) {
                    k.coeff += l.coeff;
                    PolyDestroy(&l);
                } else {
                    Mono *m = (Mono *)malloc(sizeof(Mono));
                    assert(m != NULL);
                    m -> p = l;
                    m -> exp = l_p -> m -> exp;
                    tmp -> next = (List *)malloc(sizeof(List));
                    assert(tmp -> next != NULL);
                    tmp = tmp -> next;
                    tmp -> m = m;
                    tmp -> next = NULL;
                }
                l_p = l_p -> next;
                l_q = l_q -> next;
            }
        }
        while(l_p != NULL) {
            Mono *m = (Mono *)malloc(sizeof(Mono));
            assert(m != NULL);
            *m = MonoClone(l_p -> m);
            tmp -> next = (List *)malloc(sizeof(List));
            assert(tmp -> next != NULL);
            tmp = tmp -> next;
            tmp -> m = m;
            tmp -> next = NULL;
            l_p = l_p -> next;
        }
        while(l_q != NULL) {
            Mono *m = (Mono *)malloc(sizeof(Mono));
            assert(m != NULL);
            *m = MonoClone(l_q -> m);
            tmp -> next = (List *)malloc(sizeof(List));
            assert(tmp -> next != NULL);
            tmp = tmp -> next;
            tmp -> m = m;
            tmp -> next = NULL;
            l_q = l_q -> next;
        }
        k.monos = head -> next;
        free(head);
        return k;
    }
}

int compare(const void *elem1, const void *elem2) {
    Mono m1 = *((Mono *)elem1);
    Mono m2 = *((Mono *)elem2);
    if(m1.exp > m2.exp) {
        return 1;
    } else if (m1.exp < m2.exp) {
        return -1;
    } else {
        return 0;
    }
}

List *MonosToList(int n, Mono monos[]) {
    List *head = (List *)malloc(sizeof(List));
    assert(head != NULL);
    head -> m = (Mono *)malloc(sizeof(Mono));
    assert(head -> m != NULL);
    *(head -> m) = monos[0];
    head -> next = NULL;
    List *temp = head;
    for(int i = 1; i < n; i++) {
        temp -> next = (List *)malloc(sizeof(List));
        assert(temp -> next != NULL);
        temp = temp -> next;
        temp -> next = NULL;
        temp -> m = (Mono *)malloc(sizeof(Mono));
        assert(temp -> m != NULL);
        *(temp -> m) = monos[i];
    }
    return head;
}

List *deleteMonosWithEqualExp(List *head) {
    for(List *count = head; count != NULL && count -> next != NULL;) {
        if(count -> m -> exp == count -> next -> m -> exp) {
            List *temp = count -> next;
            Poly tmp = count -> m -> p;
            count -> m -> p = PolyAdd(&(count -> m -> p), &(count -> next -> m -> p));
            PolyDestroy(&tmp);
            count -> next = count -> next -> next;
            ListNodeDestroy(temp);
        } else {
            count = count -> next;
        }
    }
    return head;
}

List *deleteConstantMonos(List *head, poly_coeff_t *coeff) {
    for(List *count = head; count != NULL && count -> next != NULL; count = count -> next) {
        if(PolyIsZero(&(count -> next -> m -> p))) {
            List *temp = count -> next;
            count -> next = count -> next -> next;
            ListNodeDestroy(temp);
        }
        if(count -> next -> m -> exp == 0 && PolyIsCoeff(&(count -> next -> m -> p))) {
            (*coeff) += count -> next -> m -> p.coeff;
            List *temp = count -> next;
            count -> next = count -> next -> next;
            ListNodeDestroy(temp);
        } else if(count -> next -> m -> exp == 0){
            (*coeff) += count -> next -> m -> p.coeff;
            count -> next -> m -> p.coeff = 0;
        }
    }
    if(head != NULL && PolyIsZero(&(head -> m -> p))) {
        List *temp = head;
        head = head -> next;
        ListNodeDestroy(temp);
    }
    if(head != NULL && head -> m -> exp == 0 && PolyIsCoeff(&(head -> m -> p))) {
        (*coeff) += head -> m -> p.coeff;
        List *temp = head;
        head = head -> next;
        ListNodeDestroy(temp);
    } else if(head != NULL && head -> m -> exp == 0) {
        (*coeff) += head -> m -> p.coeff;
        head -> m -> p.coeff = 0;
    }
    return head;
}

List *MonosNormalise(List *head, poly_coeff_t *coeff) {
    head = deleteMonosWithEqualExp(head);
    head = deleteConstantMonos(head, coeff);
    return head;
}

Poly PolyAddMonos(unsigned count, const Mono monos[]) {
    poly_coeff_t coeff = 0;
    Mono *copy = (Mono *) monos;
    qsort(copy, count, sizeof(Mono), compare);
    List *list_of_monos = MonosToList(count, copy);
    list_of_monos = MonosNormalise(list_of_monos, &coeff);
    return (Poly) {.monos = list_of_monos, .coeff = coeff};
}

Poly PolyMul(const Poly *p, const Poly *q) {
    if(PolyIsCoeff(p) && PolyIsCoeff(q)) {
            return PolyFromCoeff(p -> coeff * q -> coeff);
    } else if(PolyIsZero(p) || PolyIsZero(q)) {
            return PolyZero();
    } else if(PolyIsCoeff(p)) {
        poly_exp_t n = HowManyMonos(q);
        Mono *monos = (Mono *)calloc(n, sizeof(Mono));
        assert(monos != NULL);
        Poly coeff_p = PolyFromCoeff(p -> coeff);
        int i = 0;
        for(List *l_q = q -> monos; l_q != NULL; l_q = l_q -> next) {
                monos[i].p = PolyMul(&(l_q -> m -> p), &coeff_p);
                monos[i].exp = l_q -> m -> exp;
                i++;
        }
        PolyDestroy(&coeff_p);
        Poly k = PolyAddMonos(n, monos);
        k.coeff += p -> coeff * q -> coeff;
        free(monos);
        return k;
    } else if(PolyIsCoeff(q)) {
        poly_exp_t n = HowManyMonos(p);
        Mono *monos = (Mono *)calloc(n, sizeof(Mono));
        assert(monos != NULL);
        Poly coeff_q = PolyFromCoeff(q -> coeff);
        int i = 0;
        for(List *l_p = p -> monos; l_p != NULL; l_p = l_p -> next) {
            monos[i].p = PolyMul(&(l_p -> m -> p), &coeff_q);
            monos[i].exp = l_p -> m -> exp;
            i++;
        }
        PolyDestroy(&coeff_q);
        Poly k = PolyAddMonos(n, monos);
        k.coeff += p -> coeff * q -> coeff;
        free(monos);
        return k;
    } else {
        poly_exp_t n = (HowManyMonos(p) + 1) * (HowManyMonos(q) + 1) - 1;
        Mono *monos = (Mono *)calloc(n, sizeof(Mono));
        assert(monos != NULL);
        Poly coeff_q = PolyFromCoeff(q -> coeff);
        Poly coeff_p = PolyFromCoeff(p -> coeff);
        int i = 0;
        bool iscoeffmul = false;
        for(List *l_p = p -> monos; l_p != NULL; l_p = l_p -> next) {
            for(List *l_q = q -> monos; l_q != NULL; l_q = l_q -> next) {
                if(!iscoeffmul) {
                    monos[i].p = PolyMul(&(l_q -> m -> p), &coeff_p);
                    monos[i].exp = l_q -> m -> exp;
                    i++;
                }
                monos[i].p = PolyMul(&(l_p -> m -> p), &(l_q -> m -> p));
                monos[i].exp = l_p -> m -> exp + l_q -> m -> exp;
                i++;
            }
            iscoeffmul = true;
            monos[i].p = PolyMul(&(l_p -> m -> p), &coeff_q);
            monos[i].exp = l_p -> m -> exp;
            i++;
        }
        PolyDestroy(&coeff_p);
        PolyDestroy(&coeff_q);
        Poly k = PolyAddMonos(n, monos);
        k.coeff += p -> coeff * q -> coeff;
        free(monos);
        return k;
    }
}


Poly PolyNeg(const Poly *p) {
    Poly neg_p = PolyClone(p);
    PolyNegHelp(&neg_p);
    return neg_p;
}

void PolyNegHelp(Poly *p) {
    if(!PolyIsZero(p)) {
        if(!PolyIsCoeff(p)) {
            for(List *count = p -> monos; count != NULL; count = count -> next) {
                PolyNegHelp(&(count -> m -> p));
            }
        }
        p -> coeff *= -1;
    }
}

Poly PolySub(const Poly *p, const Poly *q) {
    if(PolyIsZero(p) && PolyIsZero(q)) {
        return PolyZero();
    } else if(PolyIsZero(p)) {
        return PolyNeg(q);
    } else if(PolyIsZero(q)) {
        return PolyClone(p);
    } else if(PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return PolyFromCoeff(p -> coeff - q -> coeff);
    } else if(PolyIsCoeff(q)) {
        Poly k = PolyClone(p);
        k.coeff -= q -> coeff;
        return k;
    } else if(PolyIsCoeff(p)) {
        Poly k = PolyNeg(q);
        k.coeff += p -> coeff;
        return k;
    }
    Poly neg_q = PolyNeg(q);
    Poly result = PolyAdd(p, &neg_q);
    PolyDestroy(&neg_q);
    return result;
}


poly_exp_t PolyDegBy(const Poly *p, unsigned var_idx) {
    if(PolyIsZero(p)) {
        return -1;
    } else if(PolyIsCoeff(p) && var_idx == 0) {
        return 0;
    } else if(PolyIsCoeff(p)) {
        return -1;
    }
    return PolyDegByHelp(p, var_idx, 0);
}

poly_exp_t PolyDegByZero(const Poly *p) {
    List *l;
    for(l = p -> monos; l -> next != NULL; l = l -> next);
    return l -> m -> exp;
}

poly_exp_t PolyDegByHelp(const Poly *p, unsigned var_idx, unsigned i) {
    if(i == var_idx) {
        if(PolyIsCoeff(p)) {
            return 0;
        } else if(PolyIsZero(p)) {
            return -1;
        } else {
            return PolyDegByZero(p);
        }
    } else {
        poly_exp_t max = 0;
        for(List *count = p -> monos; count != NULL; count = count -> next) {
            poly_exp_t n = PolyDegByHelp(&(count -> m -> p), var_idx, i + 1);
            if(max < n) {
                max = n;
            }
        }
        return max;
    }
}

poly_exp_t PolyDeg(const Poly *p) {
    if(PolyIsZero(p)) {
        return -1;
    } else if(PolyIsCoeff(p)) {
        return 0;
    } else {
        poly_exp_t max = -1;
        for(List *l = p -> monos; l != NULL; l = l -> next) {
            poly_exp_t n = PolyDeg(&(l -> m -> p));
            n += l -> m -> exp;
            if(max < n) {
                max = n;
            }
        }
        return max;
    }
}

bool PolyIsEq(const Poly *p, const Poly *q) {
    if(PolyIsZero(p) && PolyIsZero(q)) {
        return true;
    } else if(PolyIsZero(p) || PolyIsZero(q)) {
        return false;
    } else if(PolyIsCoeff(p) && PolyIsCoeff(q) && p -> coeff == q -> coeff) {
        return true;
    } else if(PolyIsCoeff(p) || PolyIsCoeff(q)) {
        return false;
    } else {
        if(p -> coeff != q -> coeff) {
            return false;
        }
        bool is_eq = true;
        List *count_p = p -> monos;
        List *count_q = q -> monos;
        for(; count_p != NULL && count_q != NULL; count_p = count_p -> next, count_q = count_q -> next) {
            if(count_p -> m -> exp != count_q -> m -> exp) {
                return false;
            }
            is_eq = PolyIsEq(&(count_p -> m -> p), &(count_q -> m -> p));
            if(is_eq == false) {
                return false;
            }
        }
        if(count_p != NULL || count_q != NULL) {
            return false;
        }
        return true;
    }
}

void PowersFromPoly(poly_coeff_t x, unsigned count, poly_coeff_t powers[], poly_exp_t exps[], Poly coeffs[]) {
    powers[0] = 1;
    for(int i = 0; i < exps[0]; i++) {
        powers[0] *= x;
    }
    coeffs[0] = PolyFromCoeff(powers[0]);
    for(unsigned i = 1; i < count; i++) {
        powers[i] = powers[i - 1];
        for(int j = 0; j < exps[i] - exps[i- 1]; j++) {
            powers[i] *= x;
        }
        coeffs[i] = PolyFromCoeff(powers[i]);
    }
}

Poly PolyAt(const Poly *p, poly_coeff_t x) {
    if(x == 0 || PolyIsZero(p)) {
        return PolyZero();
    }
    if(PolyIsCoeff(p)) {
        return PolyClone(p);
    }
    unsigned n = HowManyMonos(p);
    unsigned i = 0;
    unsigned monos_count = 0;
    poly_coeff_t coeff = 0;
    Poly *polies = (Poly *)calloc(n, sizeof(Poly));
    assert(polies != NULL);
    Poly *new_polies = (Poly *)calloc(n, sizeof(Poly));
    assert(new_polies != NULL);
    poly_exp_t exps[n];
    for(List *count = p -> monos; count != NULL; i++, count = count -> next) {
        polies[i] = count -> m -> p;
        monos_count += HowManyMonos(&(count -> m -> p));
        exps[i] = count -> m -> exp;
    }
    Poly *coeffs = (Poly *)calloc(n, sizeof(Poly));
    poly_coeff_t powers[n];
    PowersFromPoly(x, n, powers, exps, coeffs);
    for(i = 0; i < n; i++) {
        new_polies[i] = PolyMul(&(polies[i]), &(coeffs[i]));
        coeff += new_polies[i].coeff;
    }
    for(unsigned a = 0; a < n; a++) {
        PolyDestroy(&coeffs[a]);
    }
    free(coeffs);
    free(polies);
    Poly k = PolyZero();
    for(unsigned a = 0; a < n; a++) {
        List *tmp = k.monos;
        k = PolyAdd(&k, &(new_polies[a]));
        ListDestroy(tmp);
    }
    for(unsigned a = 0; a < n; a++) {
        PolyDestroy(&new_polies[a]);
    }
    k.coeff += p -> coeff;
    free(new_polies);
    return k;
}

Poly PolyCompose(const Poly *p, unsigned count, const Poly x[]) {
    if(count == 0) {
        return PolyFromCoeff(p -> coeff);
    }
    return PolyComposeHelp(p, count, x, 0);
}

Poly PolyExp(const Poly *p, unsigned n) {
    if(n == 0) {
        return PolyFromCoeff(1);
    }
    if(n % 2 == 1) {
        Poly tmp = PolyExp(p, (n - 1) / 2);
        Poly mul = PolyMul(&tmp, &tmp);
        Poly res = PolyMul(p, &mul);
        PolyDestroy(&tmp);
        PolyDestroy(&mul);
        return res;
    }
    Poly tmp = PolyExp(p, n / 2);
    Poly mul = PolyMul(&tmp, &tmp);
    PolyDestroy(&tmp);
    return mul;
}

Poly PolyComposeHelp(const Poly *p, unsigned count, const Poly x[], unsigned idx) {
    if(PolyIsCoeff(p)) {
        return PolyFromCoeff(p -> coeff);
    }
    if(idx == count) {
        return PolyFromCoeff(p -> coeff);
    }
    Poly all = PolyZero();
    for(List *mono = p -> monos; mono != NULL; mono = mono -> next) {
        Poly q = PolyComposeHelp(&(mono -> m -> p), count, x, idx + 1);
        Poly coeff = x[idx];
        Poly v = PolyExp(&coeff, mono -> m -> exp);
        Poly mul = PolyMul(&q, &v);
        Poly tmp = all;
        all = PolyAdd(&mul, &all);
        PolyDestroy(&tmp);
        PolyDestroy(&v);
        PolyDestroy(&q);
        PolyDestroy(&mul);
    }
    all.coeff += p -> coeff;
    return all;
}

