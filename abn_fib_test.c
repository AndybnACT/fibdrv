#include <stdint.h>
#include <stdio.h>

#include "abn.h"

bn bn_fib_doubling(uint64_t k)
{
    bn a, b;
    bn t1, t2, bb, tmp, bsquare, asquare;
    int clz;
    int f_i = 0;

    bn_init(&a);
    bn_init(&b);
    bn_init(&t1);
    bn_init(&t2);
    bn_init(&bb);
    bn_init(&tmp);
    bn_init(&bsquare);
    bn_init(&asquare);

    bn_set(&a, 0);
    bn_set(&b, 1);
    if (k <= 1) {
        return k == 0 ? a : b;
    }

    clz = __builtin_clzll(k);
    for (int i = (64 - clz); i > 0; i--) {
        bn_assign(&bb, &b);
        __bn_shld(&bb, 1);
        bn_sub(&tmp, &bb, &a);

        bn_mul_comba(&t1, &a, &tmp);
        bn_mul_comba(&asquare, &a, &a);
        bn_mul_comba(&bsquare, &b, &b);
        bn_add(&t2, &asquare, &bsquare);

        bn_assign(&a, &t1);
        bn_assign(&b, &t2);
        if (k & (1ull << (i - 1))) {  // current bit == 1
            bn_add(&t1, &a, &b);
            bn_assign(&a, &b);
            bn_assign(&b, &t1);
            f_i = (f_i * 2) + 1;
        } else {
            f_i = f_i * 2;
        }
    }

    bn_free(&b);
    bn_free(&t1);
    bn_free(&t2);
    bn_free(&bb);
    bn_free(&tmp);
    bn_free(&bsquare);
    bn_free(&asquare);
    return a;
}

int main(int argc, char const *argv[])
{
    bn f30000;
    f30000 = bn_fib_doubling(30000);
    bn_print(&f30000);
    return 0;
}