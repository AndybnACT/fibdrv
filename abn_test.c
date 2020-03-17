#include "abn.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define FIB_TEST 500

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
        // printf("============= b, 2b\n" );
        // bn_print(&b);
        // bn_print(&bb);
        // printf("============= b, 2b\n" );
        bn_sub(&tmp, &bb, &a);
        // printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- 2b, a, 2b-a\n" );
        // bn_print(&bb);
        // bn_print(&a);
        // bn_print(&tmp);
        // printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- 2b, a, 2b-a\n" );

        bn_mul_comba(&t1, &a, &tmp);
        // printf("======= a, tmp, a*tmp (t1)\n");
        // bn_print(&a);
        // bn_print(&tmp);
        // bn_print(&t1);
        // printf("======================\n");
        bn_mul_comba(&asquare, &a, &a);
        // printf("======= a, a, a*a\n");
        // bn_print(&a);
        // bn_print(&a);
        // bn_print(&asquare);
        // printf("======================\n");
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
        // printf("-=x-=x-x=-x=-x=x-=x-x=-x=-x=-x-=x=-x f[%d] = ", f_i);
        // bn_print(&a);
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
    bn x, y, z;
    bn aa, bb, cc;
    bn fibnum;
    uint64_t a, b;
    struct prod c;

    a = 0xDEADBEAF;
    b = 0xFFFFFFFFFFFFFFFFull;
    c = __mul_ll(a, b);
    printf("==> 0x%lx%016lx\n", c.hi, c.lo);

    bn_init(&x);
    bn_init(&y);
    bn_init(&z);
    bn_init(&aa);
    bn_init(&bb);
    bn_init(&cc);

    if (STACKALLOC >= 5) {
        bn_set(&x, 0xFFFFFFFFFFFFFFFFull);
        bn_set(&y, 0x1);
        bn_set(&z, 0);

        x.stack[1] = 0xFFFFFFFFFFFFFFFFull;
        x.stack[2] = 0;
        x.stack[3] = 0x1;
        x.cnt = 4;
        // y.stack[1] = 0xFFFFFFFFFFFFFFFFull;
        y.stack[3] = 0x2;
        y.cnt = 4;
        printf("x = ");
        bn_print(&x);
        printf("y = ");
        bn_print(&y);
        bn_add(&z, &x, &y);
        printf("z = x+ y = ");
        bn_print(&z);

        bn_sub(&aa, &z, &y);
        printf("aa = z - y =");
        bn_print(&aa);
    }

    printf("x = %p\n", &x);
    printf("y = %p\n", &y);
    printf("z = %p\n", &z);

    printf(">>>>>>>>>>>>fib test<<<<<<<<<<<<\n");
    bn_init(&x);
    bn_init(&y);
    bn_init(&z);
    bn_set(&x, 0);
    bn_set(&y, 1);
    bn_set(&z, 1);
    for (size_t i = 0; i < FIB_TEST - 1; i++) {
        bn_add(&z, &x, &y);
        bn_assign(&x, &y);
        bn_assign(&y, &z);
        printf("=========> Fib[%zu] = ", i + 2);
        bn_print(&z);
    }
    fibnum = bn_fib_doubling(FIB_TEST);
    printf("fast doubling ====>");
    bn_print(&fibnum);
    printf(">>>>>>>>>>>>shift test<<<<<<<<<<<<\n");

    bn_shl(&z, 21);
    bn_print(&z);

    __bn_shrd(&z, 21);
    bn_print(&z);

    printf(">>>>>>>>>>>> mul test <<<<<<<<<<<<\n");
    printf("y = ");
    bn_print(&y);
    printf("z = ");
    bn_print(&z);

    bn_mul_comba(&cc, &z, &y);
    printf("cc = y * z =");
    bn_print(&cc);


    bn_free(&z);
    bn_free(&x);
    bn_free(&y);
    bn_free(&aa);
    bn_free(&bb);
    bn_free(&cc);
    bn_free(&fibnum);

    return 0;
}