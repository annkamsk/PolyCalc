#include <math.h>
#include <stdio.h>
#include "poly.h"


void PolyToStringHelp(Poly *p, int ind) {
    if(p -> coeff != 0) {
        printf("%li", p -> coeff);
        if(p -> monos != NULL) {
            printf("+");
        }
    }
    if(p -> monos != NULL) {
        for(List *l = p -> monos; l != NULL; l = l -> next) {
            putchar('(');
            PolyToStringHelp(&(l -> m -> p), ind + 1);
            putchar(')');
            printf("*x%d^%d", ind, l -> m -> exp);
            if(l -> next != NULL) {
                printf("+");
            }
        }
    }
}

void PolyToString(Poly *p) {
    if(PolyIsZero(p)) {
        printf("0");
    } else {
        PolyToStringHelp(p, 0);
    }
    printf("\n");
}

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
        monos -> m = (Mono *)malloc(sizeof(Mono));
        *(monos -> m) = MonoClone(p -> monos -> m);
        monos -> next = NULL;
        List *temp = monos;
        for(List *count = p -> monos -> next; count != NULL; count = count -> next) {
            temp -> next = (List *)malloc(sizeof(List));
            temp = temp -> next;
            temp -> m = (Mono *)malloc(sizeof(Mono));
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
        Poly k = PolyZero();
        unsigned n = HowManyMonos(p) + HowManyMonos(q);
        if(n > 0) {
            int i = 0;
            Mono *monos = calloc(n, sizeof(Mono));
            for(List *count = p -> monos; count != NULL; i++, count = count -> next) {
                monos[i] = MonoClone(count -> m);
            }
            for(List *count = q -> monos; count != NULL; i++, count = count -> next) {
                monos[i] = MonoClone(count -> m);
            }
            k = PolyAddMonos(n, monos);
            free(monos);
            monos = NULL;
        }
        k.coeff = k.coeff + p -> coeff + q -> coeff;
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

void copyAndSortArray(int n, const Mono monos[], Mono copy[]) {
    for(int i = 0; i < n; i++) {
        copy[i] = monos[i];
    }
    qsort(copy, n, sizeof(Mono), compare);
}

List *MonosToList(int n, Mono monos[]) {
    List *head = (List *)malloc(sizeof(List));
    head -> m = (Mono *)malloc(sizeof(Mono));
    *(head -> m) = monos[0];
    head -> next = NULL;
    List *temp = head;
    for(int i = 1; i < n; i++) {
        temp -> next = (List *)malloc(sizeof(List));
        temp = temp -> next;
        temp -> next = NULL;
        temp -> m = (Mono *)malloc(sizeof(Mono));
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
        }
    }
    if(head != NULL && PolyIsZero(&(head -> m -> p))) {
        List *temp = head;
        head = head -> next;
        ListNodeDestroy(temp);
    }
    if(head != NULL && head -> m -> exp == 0 && PolyIsCoeff(&(head -> m -> p))) {
        (*coeff) = (*coeff) + head -> m -> p.coeff;
        List *temp = head;
        head = head -> next;
        ListNodeDestroy(temp);
    }
    return head;
}

List *MonosNormalise(List *head, poly_coeff_t *coeff) {
    head = deleteMonosWithEqualExp(head);
    head = deleteConstantMonos(head, coeff);
    return head;
}

Poly PolyAddMonos(unsigned count, const Mono monos[]) {
    //Mono *copy = (Mono *)calloc(count, sizeof(Mono));
    poly_coeff_t coeff = 0;
    //copyAndSortArray(count, monos, copy);
    Mono *copy = (Mono *) monos;
    qsort(copy, count, sizeof(Mono), compare);
    List *list_of_monos = MonosToList(count, copy);
    list_of_monos = MonosNormalise(list_of_monos, &coeff);
    //free(copy);
    return (Poly) {.monos = list_of_monos, .coeff = coeff};
}

Poly PolyMul(const Poly *p, const Poly *q) {
    if(PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return PolyFromCoeff(p -> coeff * q -> coeff);
    } else if(PolyIsZero(p) || PolyIsZero(q)) {
        return PolyZero();
    } else if(PolyIsCoeff(p)) {
        poly_exp_t n = HowManyMonos(q) + 1;
        Mono *monos = (Mono *)calloc(n, sizeof(Mono));
        Poly coeff_p = PolyFromCoeff(p -> coeff);
        int i = 0;
        for(List *l_q = q -> monos; l_q != NULL; l_q = l_q -> next) {
              monos[i].p = PolyMul(&(l_q -> m -> p), &coeff_p);
              monos[i].exp = l_q -> m -> exp;
              i++;
        }
        monos[i].p = PolyFromCoeff(q -> coeff * p -> coeff);
        monos[i].exp = 0;
        PolyDestroy(&coeff_p);
        Poly k = PolyAddMonos(n, monos);
        free(monos);
        return k;
    } else if(PolyIsCoeff(q)) {
        poly_exp_t n = HowManyMonos(p) + 1;
        Mono *monos = (Mono *)calloc(n, sizeof(Mono));
        Poly coeff_q = PolyFromCoeff(q -> coeff);
        int i = 0;
        for(List *l_p = p -> monos; l_p != NULL; l_p = l_p -> next) {
              monos[i].p = PolyMul(&(l_p -> m -> p), &coeff_q);
              monos[i].exp = l_p -> m -> exp;
              i++;
        }
        monos[i].p = PolyFromCoeff(q -> coeff * p -> coeff);
        monos[i].exp = 0;
        PolyDestroy(&coeff_q);
        Poly k = PolyAddMonos(n, monos);
        free(monos);
        return k;
    } else {
        poly_exp_t n = (HowManyMonos(p) + 1) * (HowManyMonos(q) + 1);
        Mono *monos = (Mono *)calloc(n, sizeof(Mono));
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
        monos[i].p = PolyFromCoeff(q -> coeff * p -> coeff);
        monos[i].exp = 0;
        PolyDestroy(&coeff_p);
        PolyDestroy(&coeff_q);
        Poly k = PolyAddMonos(n, monos);
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
    Poly neg_q = PolyNeg(q);
    Poly result = PolyAdd(p, &neg_q);
    PolyDestroy(&neg_q);
    return result;
}


poly_exp_t PolyDegBy(const Poly *p, unsigned var_idx) {
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
        int max = -1;
        for(List *count = p -> monos; count != NULL; count = count -> next) {
            int n = PolyDegByHelp(&(count -> m -> p), var_idx, i + 1);
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
            int n = 0;
            int m = 0;
            for(int i = 0; n != -1; i++) {
                n = PolyDegBy(&(p -> monos -> m -> p), i);
                m =+ n;
            }
            m = m + l -> m -> exp + 1; // +1 bo odjęliśmy -1 w ostatnim przejściu pętli for
            if(max < m) {
                max = m;
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
        bool is_eq = true;
        List *count_p = p -> monos;
        List *count_q = q -> monos;
        for(; count_p != NULL && count_q != NULL; count_p = count_p -> next, count_q = count_q -> next) {
            is_eq = PolyIsEq(&(count_p -> m -> p), &(count_q -> m -> p));
            if(is_eq == false) {
                return is_eq;
            }
        }
        if(count_p != NULL || count_q != NULL) {
            return false;
        } else {
            return is_eq;
        }
    }
}

void PowersFromPoly(poly_coeff_t x, unsigned count, poly_coeff_t powers[], poly_exp_t exps[]) {
    powers[0] = x;
    for(int i = 0; i < exps[0] - 1; i++) {
        powers[0] *= x;
    }
    for(unsigned i = 1; i < count; i++) {
        powers[i] = powers[i + 1];
        for(int j = 0; j < exps[i] - exps[i+1]; j++) {
            powers[i] *= x;
        }
    }
}


Poly PolyAt(const Poly *p, poly_coeff_t x) {
    unsigned n = HowManyMonos(p);
    unsigned i = 0;
    poly_coeff_t coeff = 0;
    Poly *polies = (Poly *)calloc(n, sizeof(Poly));
    poly_exp_t exps[n];
    for(List *count = p -> monos; count != NULL; i++, count = count -> next) {
        polies[i] = count -> m -> p;
        exps[i] = count -> m -> exp;
    }
    Poly *coeffs = (Poly *)calloc(n, sizeof(Poly));
    poly_coeff_t powers[n];
    PowersFromPoly(x, n, powers, exps);
    for(i = 0; i < n; i++) {
        coeffs[i] = PolyFromCoeff(powers[i]);
    }
    for(i = 0; i < n; i++) {
        polies[i] = PolyMul(&(polies[i]), &(coeffs[i]));
        coeff += polies[i].coeff;
    }
    free(coeffs);
    Poly k = PolyAddPolies(n, polies);
    k.coeff += coeff;
    for(unsigned j = 0; j < n; j++) {
        PolyDestroy(&polies[j]);
    }
    free(polies);
    return k;
}

Poly PolyAddPolies(unsigned count, const Poly polies[]) {
    unsigned n = 0;
    for(unsigned i = 0; i < count; i++) { //zliczam jednomiany we wszystkich wielomianach
        for(List *l = polies[i].monos; l != NULL; n++, l = l -> next);
    }
    Mono *monos = (Mono *)calloc(n, sizeof(Mono));
    for(unsigned i = 0; i < count;) {
        for(List *l = polies[i].monos; l != NULL; i++, l = l -> next) {
            monos[i] = MonoClone(l -> m);
        }
    }
    List *m = MonosToList(n, monos);
    Poly p = {.monos = m, .coeff = 0};
    free(monos);
    return p;
}

