#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/ansi_fp.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/ctype.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/math.h"

static int __count_trailing_zerol(unsigned int arg0) {
    int var_r4;
    int var_r5;
    int var_r6;
    int var_r7;
    unsigned int var_r8;

    var_r5 = 0x20;
    var_r8 = 0xFFFF;
    var_r6 = 0x10;
    var_r4 = 0;
    var_r7 = 0x10;
    while (var_r5 != 0) {
        if (!(arg0 & var_r8)) {
            var_r4 += var_r7;
            arg0 >>= var_r7;
            var_r5 -= var_r7;
        } else if (var_r8 == 1) {
            break;
        }
        if (var_r6 > 1) {
            var_r6 /= 2;
        }
        if (var_r8 > 1) {
            var_r8 >>= var_r6;
            var_r7 -= var_r6;
        }
    }
    return var_r4;
}

static inline int __count_trailing_zero(double n){
    if (__LO(n) != 0U) {
        return __count_trailing_zerol(__LO(n));
    } else {
        return 32 + __count_trailing_zerol(__HI(n) | 0x100000);
    }
}

static void __ull2dec(decimal* result, unsigned long long val) {
    result->sign = 0;

    if (val == 0) {
        result->exp = 0;
        result->sig.length = 1;
        result->sig.text[0] = 0;
        return;
    }

    result->sig.length = 0;

    for(; val != 0; val /= 10) {
        result->sig.text[result->sig.length++] = val % 10;
    }

    {
        unsigned char* i = result->sig.text;
        unsigned char* j = result->sig.text + result->sig.length;

        for (; i < --j; ++i) {
            unsigned char t = *i;
            *i = *j;
            *j = t;
        }
    }

    result->exp = result->sig.length - 1;
}

static inline void __dorounddecup(decimal* d, int digits){
    unsigned char* b = d->sig.text;
    unsigned char* i = b + digits - 1;

    while(1){
        if (*i < 9) {
            *i += 1;
            break;
        }
        if (i == b) {
            *i = 1;
            d->exp++;
            break;
        }
        *i-- = 0;
    }
}

static void __timesdec(decimal* result, const decimal* x, const decimal* y) {
    unsigned int accumulator = 0;
    unsigned char mantissa[SIGDIGLEN * 2];
    int i = x->sig.length + y->sig.length - 1;
    unsigned char* pDigit;
    unsigned char* ip = mantissa + i + 1;
    unsigned char* ep = ip;

    result->sign = 0;

    for(; i > 0; i--) {
        int k = y->sig.length - 1;
        int j = i - k - 1;
        int l;
        int t;
        const unsigned char* jp;
        const unsigned char* kp;

        if (j < 0) {
            j = 0;
            k = i - 1;
        }

        jp = x->sig.text + j;
        kp = y->sig.text + k;
        l = k + 1;
        t = x->sig.length - j;

        if (l > t) l = t;

        for (; l > 0; l--, jp++, kp--) {
            accumulator += *jp * *kp;
        }

        *--ip = accumulator % 10;
        accumulator /= 10;
    }

    result->exp = (short)(x->exp + y->exp);

    if (accumulator) {
        *--ip = accumulator;
        result->exp++;
    }

    for (i = 0; i < SIGDIGLEN && ip < ep; i++, ip++) {
        result->sig.text[i] = *ip;
    }
    result->sig.length = i;

    if (ip < ep && *ip >= 5){
        if (*ip == 5){
            unsigned char* jp = ip + 1;
            for (; jp < ep; jp++) {
                if (*jp != 0) goto round;
            }
            if ((ip[-1] & 1) == 0) return;
        }
    round:
        __dorounddecup(result, result->sig.length);
    }
}

static void __str2dec(decimal* d, const char* s, short exp) {
    int i;

    d->exp = exp;
    d->sign = 0;

    for (i = 0; i < SIGDIGLEN && *s; ) {
        d->sig.text[i++] = *s++ - '0';
    }
    d->sig.length = i;

    if (*s != 0 && *s >= 5) {
        const char* p = s + 1;

        for (; *p != 0; p++) {
            if (*p != '0') goto round;
        }

        if ((d->sig.text[i - 1] & 1) == 0) return;
    round:
        __dorounddecup(d, d->sig.length);
    }
}

static void __two_exp(decimal* result, short exp) {
    decimal sp8C;
    decimal sp60;
    decimal sp34;
    decimal sp8;

    switch (exp) {
        case -64:
            __str2dec(result, "542101086242752217003726400434970855712890625", -20);
            break;
        case -53:
            __str2dec(result, "11102230246251565404236316680908203125", -16);
            break;
        case -32:
            __str2dec(result, "23283064365386962890625", -10);
            break;
        case -16:
            __str2dec(result, "152587890625", -5);
            break;
        case -8:
            __str2dec(result, "390625", -3);
            break;
        case -7:
            __str2dec(result, "78125", -3);
            break;
        case -6:
            __str2dec(result, "15625", -2);
            break;
        case -5:
            __str2dec(result, "3125", -2);
            break;
        case -4:
            __str2dec(result, "625", -2);
            break;
        case -3:
            __str2dec(result, "125", -1);
            break;
        case -2:
            __str2dec(result, "25", -1);
            break;
        case -1:
            __str2dec(result, "5", -1);
            break;
        case 0:
            __str2dec(result, "1", 0);
            break;
        case 1:
            __str2dec(result, "2", 0);
            break;
        case 2:
            __str2dec(result, "4", 0);
            break;
        case 3:
            __str2dec(result, "8", 0);
            break;
        case 4:
            __str2dec(result, "16", 1);
            break;
        case 5:
            __str2dec(result, "32", 1);
            break;
        case 6:
            __str2dec(result, "64", 1);
            break;
        case 7:
            __str2dec(result, "128", 2);
            break;
        case 8:
            __str2dec(result, "256", 2);
            break;
        default:
            __two_exp(&sp8C, exp / 2);
            __timesdec(result, &sp8C, &sp8C);
            if (exp & 1) {
                sp60 = *result;
                if (exp > 0) {
                    __str2dec(&sp34, "2", 0);
                    __timesdec(result, &sp60, &sp34);
                } else {
                    __str2dec(&sp8, "5", -1);
                    __timesdec(result, &sp60, &sp8);
                }
            }
            break;
    }
}

static int __equals_dec(const decimal* x, const decimal* y) {
    if (x->sig.text[0] == 0) {
        if (y->sig.text[0] == 0)
            return 1;
        return 0;
    }
    if (y->sig.text[0] == 0) {
        if (x->sig.text[0] == 0)
            return 1;
        return 0;
    }
    if (x->exp == y->exp) {
        int i;
        int l = x->sig.length;
        if (l > y->sig.length)
            l = y->sig.length;
        for (i = 0; i < l; ++i)
            if (x->sig.text[i] != y->sig.text[i])
                return 0;
        if (l == x->sig.length) {
            for (; i < y->sig.length; ++i)
                if (y->sig.text[i] != 0)
                    return 0;
        } else {
            for (; i < x->sig.length; ++i)
                if (x->sig.text[i] != 0)
                    return 0;
        }
        return 1;
    }
    return 0;
}

static int __less_dec(const decimal* x, const decimal* y) {
    if (x->sig.text[0] == 0) {
        if (y->sig.text[0] != 0)
            return 1;
        return 0;
    }
    if (y->sig.text[0] == 0)
        return 0;
    if (x->exp == y->exp) {
        int i;
        int l = x->sig.length;
        if (l > y->sig.length)
            l = y->sig.length;
        for (i = 0; i < l; ++i) {
            if (x->sig.text[i] < y->sig.text[i])
                return 1;
            if (y->sig.text[i] < x->sig.text[i])
                return 0;
        }
        if (l == x->sig.length) {
            for (; i < y->sig.length; ++i)
                if (y->sig.text[i] != 0)
                    return 1;
        }
        return 0;
    }
    return x->exp < y->exp;
}

static void __minus_dec(decimal* z, const decimal* x, const decimal* y) {
    int zlen, dexp;
    unsigned char *ib, *i, *ie;
    unsigned char const *jb, *j, *jn;
    *z = *x;
    if (y->sig.text[0] == 0)
        return;
    zlen = z->sig.length;
    if (zlen < y->sig.length)
        zlen = y->sig.length;
    dexp = z->exp - y->exp;
    zlen += dexp;
    if (zlen > SIGDIGLEN)
        zlen = SIGDIGLEN;
    while (z->sig.length < zlen)
        z->sig.text[z->sig.length++] = 0;
    ib = z->sig.text;
    i = ib + zlen;
    if (y->sig.length + dexp < zlen)
        i = ib + (y->sig.length + dexp);
    jb = y->sig.text;
    j = jb + (i - ib - dexp);
    jn = j;
    while (i > ib && j > jb) {
        --i;
        --j;
        if (*i < *j) {
            unsigned char* k = i - 1;
            while (*k == 0)
                --k;
            while (k != i) {
                --*k;
                *++k += 10;
            }
        }
        *i -= *j;
    }
    if (jn - jb < y->sig.length) {
        int round_down = 0;
        if (*jn < 5)
            round_down = 1;
        else if (*jn == 5) {
            unsigned char const* je = y->sig.text + y->sig.length;
            for (j = jn + 1; j < je; ++j)
                if (*j != 0)
                    goto done;
            i = ib + (jn - jb) + dexp - 1;
            if (*i & 1)
                round_down = 1;
        }
        if (round_down) {
            if (*i < 1) {
                unsigned char* k = i - 1;
                while (*k == 0)
                    --k;
                while (k != i) {
                    --*k;
                    *++k += 10;
                }
            }
            *i -= 1;
        }
    }
done:
    for (i = ib; *i == 0; ++i) {
    }
    if (i > ib) {
        unsigned char dl = (unsigned char)(i - ib);
        z->exp -= dl;
        ie = ib + z->sig.length;
        for (; i < ie; ++i, ++ib)
            *ib = *i;
        z->sig.length -= dl;
    }
    ib = z->sig.text;
    for (i = ib + z->sig.length; i > ib;) {
        --i;
        if (*i != 0)
            break;
    }
    z->sig.length = (unsigned char)(i - ib + 1);
}

static void __num2dec_internal(decimal* d, double x) {
    signed char sign = (signbit(x) != 0);

    if (x == 0) {
        d->sign = sign;
        d->exp = 0;
        d->sig.length = 1;
        d->sig.text[0] = 0;
        return;
    }

    if (!isfinite(x)) {
        d->sign = sign;
        d->exp = 0;
        d->sig.length = 1;
        d->sig.text[0] = (fpclassify(x) == 1) ? 'N' : 'I';
        return;
    }

    if (sign != 0) {
        x = -x;
    }

    {
        int exp;
        double frac = frexp(x, &exp);
        short num_bits_extract = DBL_MANT_DIG - __count_trailing_zero(frac);
        decimal int_d, pow2_d;
        double sp30;

        __two_exp(&pow2_d, exp - num_bits_extract);
        modf(ldexp(frac, num_bits_extract), &sp30);
        __ull2dec(&int_d, sp30);
        __timesdec(d, &int_d, &pow2_d);
        d->sign = sign;
    }
}

static inline int __must_round(const decimal* d, int digits){
    unsigned char const* i = d->sig.text + digits;

    if (*i > 5) {
        return 1;
    }

    if (*i < 5) {
        return -1;
    }

    {
        unsigned char const* e = d->sig.text + d->sig.length;

        for(i++; i < e; i++){
            if (*i != 0) {
                return 1;
            }
        }
    }

    if (d->sig.text[digits - 1] & 1) {
        return 1;
    }

    return -1;
}

static inline void __rounddec(decimal* d, int digits){
    if (digits > 0 && digits < d->sig.length) {
        int unkBool = __must_round(d,digits);
        d->sig.length = digits;

        if (unkBool >= 0) {
            __dorounddecup(d, digits);
        }
    }
}

void __num2dec(const decform* form, double x, decimal* d) {
    short digits = form->digits;
    int i;
    __num2dec_internal(d, x);

    if (d->sig.text[0] > 9) {
        return;
    }

    if (digits > SIGDIGLEN) {
        digits = SIGDIGLEN;
    }

    __rounddec(d, digits);

    while(d->sig.length < digits){
        d->sig.text[d->sig.length++] = 0;
    }

    d->exp -= d->sig.length - 1;

    for(i = 0; i < d->sig.length; i++) {
        d->sig.text[i] += '0';
    }
}

double __dec2num(const decimal* d) {
    if (d->sig.length <= 0)
        return copysign(0.0, d->sign == 0 ? 1.0 : -1.0);

    switch (d->sig.text[0]) {
    case '0':
        return copysign(0.0, d->sign == 0 ? 1.0 : -1.0);
    case 'I':
        return copysign((double)INFINITY, d->sign == 0 ? 1.0 : -1.0);
    case 'N': {
        double result;
        unsigned long long* ll = (unsigned long long*)&result;
        *ll = 0x7FF0000000000000;
        if (d->sign)
            *ll |= 0x8000000000000000;
        if (d->sig.length == 1)
            *ll |= 0x0008000000000000;
        else {
            unsigned char* p = (unsigned char*)&result + 1;
            int placed_non_zero = 0;
            int low = 1;
            int i;
            int e = d->sig.length;
            if (e > 14)
                e = 14;
            for (i = 1; i < e; ++i) {
                unsigned char c = d->sig.text[i];
                if (isdigit(c))
                    c -= '0';
                else
                    c = (unsigned char)(tolower(c) - 'a' + 10);
                if (c != 0)
                    placed_non_zero = 1;
                if (low)
                    *p++ |= c;
                else
                    *p = (unsigned char)(c << 4);
                low = !low;
            }
            if (!placed_non_zero)
                *ll |= 0x0008000000000000;
        }
        return result;
    }
    }

    {
        static double pow_10[8] = { 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8 };
        decimal dec = *d;
        unsigned char* i = dec.sig.text;
        unsigned char* e = i + dec.sig.length;
        double first_guess;
        int exponent;

        for (; i < e; ++i)
            *i -= '0';
        dec.exp += dec.sig.length - 1;
        exponent = dec.exp;
        i = dec.sig.text;
        first_guess = *i++;
        while (i < e) {
            unsigned long ival = 0;
            int j;
            double temp1, temp2;
            int ndig = (e - i) % 8;
            if (ndig == 0)
                ndig = 8;
            for (j = 0; j < ndig; ++j, ++i)
                ival = ival * 10 + *i;
            temp1 = first_guess * pow_10[ndig - 1];
            temp2 = temp1 + ival;
            if (ival != 0 && temp1 == temp2)
                break;
            first_guess = temp2;
            exponent -= ndig;
        }
        if (exponent < 0)
            first_guess /= pow(5.0, -exponent);
        else
            first_guess *= pow(5.0, exponent);
        first_guess = ldexp(first_guess, exponent);
        if (isinf(first_guess)) {
            decimal max;
            __str2dec(&max, "179769313486231580793729011405303420", 308);
            if (__less_dec(&max, &dec))
                goto done;
            first_guess = DBL_MAX;
        }
        {
            decimal feedback1;
            __num2dec_internal(&feedback1, first_guess);
            if (__equals_dec(&feedback1, &dec))
                goto done;
            if (__less_dec(&feedback1, &dec)) {
                decimal feedback2, difflow, diffhigh;
                double next_guess = nextafter(first_guess, INFINITY);
                if (isinf(next_guess)) {
                    first_guess = next_guess;
                    goto done;
                }
                __num2dec_internal(&feedback2, next_guess);
                while (__less_dec(&feedback2, &dec)) {
                    feedback1 = feedback2;
                    first_guess = next_guess;
                    next_guess = nextafter(next_guess, INFINITY);
                    if (isinf(next_guess)) {
                        first_guess = next_guess;
                        goto done;
                    }
                    __num2dec_internal(&feedback2, next_guess);
                }
                __minus_dec(&difflow, &dec, &feedback1);
                __minus_dec(&diffhigh, &feedback2, &dec);
                if (__equals_dec(&difflow, &diffhigh)) {
                    if (*(unsigned long long*)&first_guess & 1)
                        first_guess = next_guess;
                } else if (!__less_dec(&difflow, &diffhigh))
                    first_guess = next_guess;
            } else {
                decimal feedback2, difflow, diffhigh;
                double next_guess = nextafter(first_guess, -INFINITY);
                __num2dec_internal(&feedback2, next_guess);
                while (__less_dec(&dec, &feedback2)) {
                    feedback1 = feedback2;
                    first_guess = next_guess;
                    next_guess = nextafter(next_guess, -INFINITY);
                    __num2dec_internal(&feedback2, next_guess);
                }
                __minus_dec(&difflow, &dec, &feedback2);
                __minus_dec(&diffhigh, &feedback1, &dec);
                if (__equals_dec(&difflow, &diffhigh)) {
                    if (*(unsigned long long*)&first_guess & 1)
                        first_guess = next_guess;
                } else if (__less_dec(&difflow, &diffhigh))
                    first_guess = next_guess;
            }
        }
    done:
        if (dec.sign)
            first_guess = -first_guess;
        return first_guess;
    }
}
