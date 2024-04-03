#include "fpmath.h"

/* Fixed point multiplication */
fx16_t fx_mul(fx16_t multiplier, fx16_t multiplicand)
{
    long long temp = (long long) multiplier * (long long) multiplicand;
    return (fx16_t) (temp >> FRAC_BITS);
}

/* Fixed point division */
fx16_t fx_div(fx16_t dividend, fx16_t divisor)
{
    long long temp = ((long long) dividend << FRAC_BITS) / divisor;
    return (fx16_t) temp;
}

/* Fixed point log_2 */
fx16_t fx_log(fx16_t v)
{
    unsigned int x = (uint32_t) v;
    unsigned int t;
    fx16_t y;

    y = 0xa65af;
    if (x < 0x00008000)
        x <<= 16, y -= 0xb1721;
    if (x < 0x00800000)
        x <<= 8, y -= 0x58b91;
    if (x < 0x08000000)
        x <<= 4, y -= 0x2c5c8;
    if (x < 0x20000000)
        x <<= 2, y -= 0x162e4;
    if (x < 0x40000000)
        x <<= 1, y -= 0x0b172;
    t = x + (x >> 1);
    if ((t & 0x80000000) == 0)
        x = t, y -= 0x067cd;
    t = x + (x >> 2);
    if ((t & 0x80000000) == 0)
        x = t, y -= 0x03920;
    t = x + (x >> 3);
    if ((t & 0x80000000) == 0)
        x = t, y -= 0x01e27;
    t = x + (x >> 4);
    if ((t & 0x80000000) == 0)
        x = t, y -= 0x00f85;
    t = x + (x >> 5);
    if ((t & 0x80000000) == 0)
        x = t, y -= 0x007e1;
    t = x + (x >> 6);
    if ((t & 0x80000000) == 0)
        x = t, y -= 0x003f8;
    t = x + (x >> 7);
    if ((t & 0x80000000) == 0)
        x = t, y -= 0x001fe;
    x = 0x80000000 - x;
    y -= x >> 15;
    return y;
}

/* Fixed point sqrt */
fx16_t fx_sqrt(fx16_t v)
{
    uint32_t t, q, b, r;
    r = (int32_t) v;
    q = 0;
    b = 0x40000000UL;
    if (r < 0x40000200) {
        while (b != 0x40) {
            t = q + b;
            if (r >= t) {
                r -= t;
                q = t + b; /* equivalent to q += 2*b */
            }
            r <<= 1;
            b >>= 1;
        }
        q >>= 8;
        return q;
    }
    while (b > 0x40) {
        t = q + b;
        if (r >= t) {
            r -= t;
            q = t + b; /* equivalent to q += 2*b */
        }
        if ((r & 0x80000000) != 0) {
            q >>= 1;
            b >>= 1;
            r >>= 1;
            while (b > 0x20) {
                t = q + b;
                if (r >= t) {
                    r -= t;
                    q = t + b;
                }
                r <<= 1;
                b >>= 1;
            }
            q >>= 7;
            return q;
        }
        r <<= 1;
        b >>= 1;
    }
    q >>= 8;
    return q;
}