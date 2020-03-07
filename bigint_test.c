#include "bigint.h"

#include <stdio.h>

#define printbig(x) printf("0x%016llx%016llx\n", x.upper, x.lower);

static uint128_t fib_sequence(long long k)
{
    /* FIXME: use clz/ctz and fast algorithms to speed up */
    uint128_t f[k + 2];

    f[0] = (uint128_t){.upper = 0, .lower = 0};
    f[1] = (uint128_t){.upper = 0, .lower = 1};

    for (int i = 2; i <= k; i++) {
        add128(f + i, f[i - 1], f[i - 2]);
    }

    return f[k];
}

static uint128_t fibseq_doubling(long long k)
{
    uint128_t a = {.lower = 0, .upper = 0};
    uint128_t b = {.lower = 1, .upper = 0};
    int clz = __builtin_clzll(k);

    for (int i = (64 - clz); i > 0; i--) {
        uint128_t t1, t2, tmp;
        lsft128(&tmp, b, 1);   // tmp = 2b
        sub128(&tmp, tmp, a);  // tmp = tmp -a
        mul128(&t1, a, tmp);   // t1 = a*tmp

        mul128(&a, a, a);   // a = a^2
        mul128(&b, b, b);   // b = b^2
        add128(&t2, a, b);  // t2 = a^2 + b^2
        a = t1;
        b = t2;
        if (k & (1ull << (i - 1))) {  // current bit == 1
            add128(&t1, a, b);
            a = b;
            b = t1;
        }
    }
    return a;
}


int main(int argc, char const *argv[])
{
    uint128_t a, b, c;

    a.upper = 0;
    a.lower = 0xDEADBEEF;

    b.upper = 0;
    b.lower = 0xBEEFDEAD;

    printbig(a);
    printbig(b);

    printf("shift left 32\n");
    lsft128(&a, a, 32);
    lsft128(&b, b, 32);
    printbig(a);
    printbig(b);
    printf("shift left 16\n");
    lsft128(&a, a, 16);
    lsft128(&b, b, 16);
    printbig(a);
    printbig(b);
    printf("a + b\n");
    add128(&c, a, b);
    printbig(c);
    printf("a - b\n");
    sub128(&c, a, b);
    printbig(c);
    printf("shift right 32\n");
    rsft128(&a, a, 32);
    rsft128(&b, b, 32);
    printbig(a);
    printbig(b);
    printf("a * b\n");
    mul128(&c, a, b);
    printbig(c);
    printf("testing for upper int\n");
    a.lower = 0xF0F0F;
    a.upper = 0ull;
    b.lower = 0ull;
    b.upper = 0xF0F;
    printf("a=");
    printbig(a);
    printf("b=");
    printbig(b);
    mul128(&c, a, b);
    printf("a * b\n");
    printbig(c);

    for (size_t i = 0; i < 50000; i++) {
        printf("calculating F(180)...\n");
        c = fibseq_doubling(180);
        printbig(c);
        printf("calculating F(180) by doubling...\n");
        c = fib_sequence(180);
        printbig(c);
    }

    return 0;
}
