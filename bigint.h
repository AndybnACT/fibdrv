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

static inline void sft128(uint128_t *out, uint128_t op, unsigned char shift) {}

static inline void mul128(uint128_t *out, uint128_t op1, uint128_t op2) {}
