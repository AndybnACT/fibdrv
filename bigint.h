typedef struct uint128 {
    unsigned long long upper;
    unsigned long long lower;
} uint128_t;

static inline void add128(uint128_t *out, uint128_t op1, uint128_t op2)
{
    out->upper = op1.upper + op2.upper;
    if (op1.lower > ~op2.lower) {
        out->upper++;
    }
    out->lower = op1.lower + op2.lower;
}

static inline void sub128(uint128_t *out, uint128_t op1, uint128_t op2)
{
    out->upper = op1.upper - op2.upper;
    if (op1.lower < op2.lower) {
        out->upper--;
    }
    out->lower = op1.lower - op2.lower;
}

static inline void lsft128(uint128_t *out, uint128_t op, unsigned char shift)
{
    if (shift == 0) {
        *out = op;
        return;
    } else if (shift >= 64) {
        out->upper = op.lower << (shift - 64);
        out->lower = 0ull;
        return;
    }
    out->upper = op.upper << shift;
    out->lower = op.lower << shift;
    out->upper |= op.lower >> (64 - shift);
}

static inline void rsft128(uint128_t *out, uint128_t op, unsigned char shift)
{
    if (shift == 0) {
        *out = op;
        return;
    } else if (shift >= 64) {
        out->upper = 0ull;
        out->lower = op.upper >> (shift - 64);
        return;
    }
    out->upper = op.upper >> shift;
    // signed bit will not extend for unsigned type
    out->lower = op.lower >> shift;
    out->lower |= op.upper << (64 - shift);
}

static inline void mul128(uint128_t *out, uint128_t op1, uint128_t op2)
{
    int op1cnt =
        __builtin_popcountll(op1.lower) + __builtin_popcountll(op1.upper);
    int op2cnt =
        __builtin_popcountll(op2.lower) + __builtin_popcountll(op2.upper);
    uint128_t lo, hi;
    if (op1cnt > op2cnt) {
        lo = op2;
        hi = op1;
    } else {
        lo = op1;
        hi = op2;
    }
    out->lower = 0ull;
    out->upper = 0ull;
    while (lo.lower) {
        uint128_t prod;
        unsigned char ctz = __builtin_ctzll(lo.lower);
        lsft128(&prod, hi, ctz);
        add128(out, prod, *out);
        lo.lower &= ~(1ull << ctz);
        // printf("lower: %llx, ctz=%d\n", lo.lower, ctz);
    }
    while (lo.upper) {
        uint128_t prod;
        unsigned char ctz = __builtin_ctzll(lo.upper);
        lsft128(&prod, hi, ctz + 64);
        add128(out, prod, *out);
        lo.upper &= ~(1ull << ctz);
        // printf("upper: %llx, ctz=%d\n", lo.upper, ctz);
    }
}
