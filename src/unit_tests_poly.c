#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>
#include <limits.h>
#include "poly.h"
#include "cmocka.h"

#define N 18
/**
 * Atrapa funkcji main w pliku calc_poly.c
 * @return kod wyjscia
 */
extern int calc_poly_main();

int mock_fprintf(FILE* const file, const char *format, ...) CMOCKA_PRINTF_ATTRIBUTE(2, 3);
int mock_printf(const char *format, ...) CMOCKA_PRINTF_ATTRIBUTE(1, 2);

/**
 * Pomocnicze bufory, do których piszą atrapy funkcji printf i fprintf oraz
 * pozycje zapisu w tych buforach. Pozycja zapisu wskazuje bajt o wartości 0.
 */
static char fprintf_buffer[256];
static char printf_buffer[256];
static int fprintf_position = 0;
static int printf_position = 0;

/**
 * Atrapa funkcji fprintf sprawdzająca poprawność wypisywania na stderr.
 */
int mock_fprintf(FILE* const file, const char *format, ...) {
    int return_value;
    va_list args;

    assert_true(file == stderr);
    /* Poniższa asercja sprawdza też, czy fprintf_position jest nieujemne.
    W buforze musi zmieścić się kończący bajt o wartości 0. */
    assert_true((size_t)fprintf_position < sizeof(fprintf_buffer));

    va_start(args, format);
    return_value = vsnprintf(fprintf_buffer + fprintf_position,
                             sizeof(fprintf_buffer) - fprintf_position,
                             format,
                             args);
    va_end(args);

    fprintf_position += return_value;
    assert_true((size_t)fprintf_position < sizeof(fprintf_buffer));
    return return_value;
}

/**
 * Atrapa funkcji printf sprawdzająca poprawność wypisywania na stderr.
 */
int mock_printf(const char *format, ...) {
    int return_value;
    va_list args;

    /* Poniższa asercja sprawdza też, czy printf_position jest nieujemne.
    W buforze musi zmieścić się kończący bajt o wartości 0. */
    assert_true((size_t)printf_position < sizeof(printf_buffer));

    va_start(args, format);
    return_value = vsnprintf(printf_buffer + printf_position,
                             sizeof(printf_buffer) - printf_position,
                             format,
                             args);
    va_end(args);

    printf_position += return_value;
    assert_true((size_t)printf_position < sizeof(printf_buffer));
    return return_value;
}

/**
 *  Pomocniczy bufor, z którego korzystają atrapy funkcji operujących na stdin.
 */
static char input_stream_buffer[256];
static int input_stream_position = 0;
static int input_stream_end = 0;
int read_char_count;

/**
 * Atrapa funkcji getchar używana do przechwycenia czytania z stdin.
 */
int mock_getchar() {
    if (input_stream_position < input_stream_end)
        return input_stream_buffer[input_stream_position++];
    else
        return EOF;
}

/**
 * Atrapa funkcji ungetc.
 * Obsługiwane jest tylko standardowe wejście.
 */
int mock_ungetc(int c, FILE *stream) {
    assert_true(stream == stdin);
    if (input_stream_position > 0)
        return input_stream_buffer[--input_stream_position] = c;
    else
        return EOF;
}

/**
 * Funkcja inicjująca dane wejściowe dla programu korzystającego ze stdin.
 */
static void init_input_stream(const char *str) {
    memset(input_stream_buffer, 0, sizeof(input_stream_buffer));
    input_stream_position = 0;
    input_stream_end = strlen(str);
    assert_true((size_t)input_stream_end < sizeof(input_stream_buffer));
    strcpy(input_stream_buffer, str);
}

/**
 * Funkcja wołana przed każdym testem korzystającym z stdout lub stderr.
 */
static int test_setup(void **state) {
    (void)state;

    memset(fprintf_buffer, 0, sizeof(fprintf_buffer));
    memset(printf_buffer, 0, sizeof(printf_buffer));
    printf_position = 0;
    fprintf_position = 0;

    /* Zwrócenie zera oznacza sukces. */
    return 0;
}

static void test_polyzero(void **state) {
    (void)state;
    Poly p = PolyZero();
    unsigned count = 0;

    Poly exp = PolyZero();
    Poly res = PolyCompose(&p, count, NULL);
    assert_true(PolyIsEq(&res, &exp));
    PolyDestroy(&res);
    PolyDestroy(&exp);
    PolyDestroy(&p);
}


static void test_polycoeff_to_polyzero(void **state) {
    (void)state;
    Poly p = PolyZero();
    unsigned count = 1;
    Poly x = PolyFromCoeff(1);

    Poly exp = PolyZero();
    Poly res = PolyCompose(&p, count, &x);
    assert_true(PolyIsEq(&res, &exp));
    PolyDestroy(&res);
    PolyDestroy(&exp);
    PolyDestroy(&x);
}

static void test_polycoeff(void **state) {
    (void)state;
    Poly p = PolyFromCoeff(5);
    unsigned count = 0;

    Poly exp = PolyFromCoeff(5);
    Poly res = PolyCompose(&p, count, NULL);
    assert_true(PolyIsEq(&res, &exp));
    PolyDestroy(&res);
    PolyDestroy(&exp);
    PolyDestroy(&p);
}

static void test_polycoeff_to_polycoeff(void **state) {
    (void)state;
    Poly p = PolyFromCoeff(5);
    unsigned count = 1;
    Poly x = PolyFromCoeff(3);

    Poly exp = PolyFromCoeff(5);
    Poly res = PolyCompose(&p, count, &x);
    assert_true(PolyIsEq(&res, &exp));
    PolyDestroy(&res);
    PolyDestroy(&exp);
    PolyDestroy(&p);
    PolyDestroy(&x);
}

static void test_polyx0(void **state) {
    (void)state;
    Poly q = PolyFromCoeff(1);
    Mono m = MonoFromPoly(&q, 1);
    Poly p = PolyAddMonos(1, &m);
    unsigned count = 0;

    Poly exp = PolyZero();
    Poly res = PolyCompose(&p, count, NULL);
    assert_true(PolyIsEq(&res, &exp));
    PolyDestroy(&res);
    PolyDestroy(&exp);
    PolyDestroy(&p);
}

static void test_polycoeff_to_polyx0(void **state) {
    (void)state;
    Poly q = PolyFromCoeff(1);
    Mono m = MonoFromPoly(&q, 1);
    Poly p = PolyAddMonos(1, &m);
    unsigned count = 1;
    Poly x = PolyFromCoeff(5);

    Poly exp = PolyFromCoeff(5);
    Poly res = PolyCompose(&p, count, &x);
    assert_true(PolyIsEq(&res, &exp));
    PolyDestroy(&res);
    PolyDestroy(&exp);
    PolyDestroy(&p);
    PolyDestroy(&x);
}

static void test_polyx0_to_polyx0(void **state) {
    (void)state;
    Poly q = PolyFromCoeff(1);
    Mono m1 = MonoFromPoly(&q, 1);
    Poly p = PolyAddMonos(1, &m1);
    unsigned count = 1;
    Poly v = PolyFromCoeff(1);
    Mono m2 = MonoFromPoly(&v, 1);
    Poly x = PolyAddMonos(1, &m2);

    Poly u = PolyFromCoeff(1);
    Mono m3 = MonoFromPoly(&u, 1);
    Poly exp = PolyAddMonos(1, &m3);
    Poly res = PolyCompose(&p, count, &x);
    assert_true(PolyIsEq(&res, &exp));
    PolyDestroy(&res);
    PolyDestroy(&exp);
    PolyDestroy(&p);
    PolyDestroy(&x);
}

static void test_no_parameter(void **state) {
    (void)state;

    init_input_stream("COMPOSE ");
    assert_int_equal(calc_poly_main(), 0);
    assert_string_equal(printf_buffer, "");
    assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n");
}

static void test_zero_parameter(void **state) {
    (void)state;

    init_input_stream("(2,0)+(1,2)\nCOMPOSE 0");

    assert_int_equal(calc_poly_main(), 0);
    assert_string_equal(printf_buffer, "");
    assert_string_equal(fprintf_buffer, "");
}

static void test_uintmax_parameter(void **state) {
    (void)state;
    char str[N];
    snprintf(str, sizeof(str), "COMPOSE %u", UINT_MAX);
    init_input_stream(str);

    assert_int_equal(calc_poly_main(), 0);
    assert_string_equal(printf_buffer, "");
    assert_string_equal(fprintf_buffer, "ERROR 1 STACK UNDERFLOW\n");
}

static void test_negative_parameter(void **state) {
    (void)state;

    init_input_stream("COMPOSE -1");

    assert_int_equal(calc_poly_main(), 0);
    assert_string_equal(printf_buffer, "");
    assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n");
}

static void test_over_uintmax_parameter(void **state) {
    (void)state;

    init_input_stream("COMPOSE 4294967296");

    assert_int_equal(calc_poly_main(), 0);
    assert_string_equal(printf_buffer, "");
    assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n");
}

static void test_big_parameter(void **state) {
    (void)state;

    init_input_stream("COMPOSE 65536423232");

    assert_int_equal(calc_poly_main(), 0);
    assert_string_equal(printf_buffer, "");
    assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n");
}

static void test_literal_parameter(void **state) {
    (void)state;

    init_input_stream("COMPOSE dehueu");

    assert_int_equal(calc_poly_main(), 0);
    assert_string_equal(printf_buffer, "");
    assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n");
}

static void test_literal_and_numerical_parameter(void **state) {
    (void)state;

    init_input_stream("COMPOSE 3455dehueu");

    assert_int_equal(calc_poly_main(), 0);
    assert_string_equal(printf_buffer, "");
    assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n");
}

int main() {
    /* Definiujemy listę testów jednostkowych do wykonania. Każdy element
    tablicy jest wynikiem wykonania makra cmocka_unit_test,
    cmocka_unit_test_setup, cmocka_unit_test_setup_teardown,
    cmocka_unit_test_teardown lub tp. */
    const struct CMUnitTest composeTests[] = {
        cmocka_unit_test(test_polyzero),
        cmocka_unit_test(test_polycoeff_to_polyzero),
        cmocka_unit_test(test_polycoeff),
        cmocka_unit_test(test_polycoeff_to_polycoeff),
        cmocka_unit_test(test_polyx0),
        cmocka_unit_test(test_polycoeff_to_polyx0),
        cmocka_unit_test(test_polyx0_to_polyx0)
    };

    const struct CMUnitTest parseTests[] = {
        cmocka_unit_test_setup(test_no_parameter, test_setup),
        cmocka_unit_test_setup(test_zero_parameter, test_setup),
        cmocka_unit_test_setup(test_uintmax_parameter, test_setup),
        cmocka_unit_test_setup(test_negative_parameter, test_setup),
        cmocka_unit_test_setup(test_over_uintmax_parameter, test_setup),
        cmocka_unit_test_setup(test_big_parameter, test_setup),
        cmocka_unit_test_setup(test_literal_parameter, test_setup),
        cmocka_unit_test_setup(test_literal_and_numerical_parameter, test_setup)
    };

    /* Wykonujemy grupę testów wyspecyfikowanych w tablicy tests, drugi i trzeci
    parametr to wskaźniki odpowiednio na funkcję, która ma być wykonana przed
    wszystkimi testami, i funkcję, która ma być wykonana po wszystkich
    testach. */
    return cmocka_run_group_tests(composeTests, NULL, NULL) || cmocka_run_group_tests(parseTests, NULL, NULL);
}
