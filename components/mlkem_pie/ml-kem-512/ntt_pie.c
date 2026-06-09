#include "ntt.h"
#include "params.h"
#include "reduce.h"
#include "pie_vec.h"
#include <stdint.h>

// PIE-ready version: aligned zetas for vector loads.

const int16_t __attribute__((aligned(16))) PQCLEAN_MLKEM512_CLEAN_zetas[128] = {
    -1044,  -758,  -359, -1517,  1493,  1422,   287,   202,
    -171,   622,  1577,   182,   962, -1202, -1474,  1468,
    573, -1325,   264,   383,  -829,  1458, -1602,  -130,
    -681,  1017,   732,   608, -1542,   411,  -205, -1571,
    1223,   652,  -552,  1015, -1293,  1491,  -282, -1544,
    516,    -8,  -320,  -666, -1618, -1162,   126,  1469,
    -853,   -90,  -271,   830,   107, -1421,  -247,  -951,
    -398,   961, -1508,  -725,   448, -1065,   677, -1275,
    -1103,   430,   555,   843, -1251,   871,  1550,   105,
    422,   587,   177,  -235,  -291,  -460,  1574,  1653,
    -246,   778,  1159,  -147,  -777,  1483,  -602,  1119,
    -1590,   644,  -872,   349,   418,   329,  -156,   -75,
    817,  1097,   603,   610,  1322, -1285, -1465,   384,
    -1215,  -136,  1218, -1335,  -874,   220, -1187, -1659,
    -1185, -1530, -1278,   794, -1510,  -854,  -870,   478,
    -108,  -308,   996,   991,   958, -1460,  1522,  1628
};

static int16_t fqmul(int16_t a, int16_t b) {
    return PQCLEAN_MLKEM512_CLEAN_montgomery_reduce((int32_t)a * b);
}

static void fqmul_vec8_zvec(int16_t *out, const int16_t zvec[8], const int16_t *in) {
    int16_t prod_lo[8] __attribute__((aligned(16)));
    int16_t prod_hi[8] __attribute__((aligned(16)));

    mlkem_pie_mul_lohi_8(zvec, in, prod_lo, prod_hi);
    mlkem_pie_montgomery_reduce_vec8(prod_lo, prod_hi, out);
}

static void fqmul_vec8_zvec_batch2(int16_t *out0,
                                   int16_t *out1,
                                   const int16_t zvec[8],
                                   const int16_t *in0,
                                   const int16_t *in1) {
    int16_t prod_lo0[8] __attribute__((aligned(16)));
    int16_t prod_hi0[8] __attribute__((aligned(16)));
    int16_t prod_lo1[8] __attribute__((aligned(16)));
    int16_t prod_hi1[8] __attribute__((aligned(16)));

    mlkem_pie_mul_lohi_8x2(zvec, in0, prod_lo0, prod_hi0, zvec, in1, prod_lo1, prod_hi1);
    mlkem_pie_montgomery_reduce_vec8x2(prod_lo0, prod_hi0, out0, prod_lo1, prod_hi1, out1);
}

void PQCLEAN_MLKEM512_CLEAN_ntt(int16_t r[256]) {
    unsigned int len, start, j, k, i;
    int16_t t, zeta;
    int16_t tvec0[8] __attribute__((aligned(16)));
    int16_t tvec1[8] __attribute__((aligned(16)));

    k = 1;
    for (len = 128; len >= 2; len >>= 1) {
        for (start = 0; start < 256; start = j + len) {
            zeta = PQCLEAN_MLKEM512_CLEAN_zetas[k++];
            if (len >= 8) {
                int16_t zvec[8] __attribute__((aligned(16)));
                for (i = 0; i < 8; i++) {
                    zvec[i] = zeta;
                }
                for (j = start; j + 15 < start + len; j += 16) {
                    fqmul_vec8_zvec_batch2(tvec0, tvec1, zvec, &r[j + len], &r[j + len + 8]);
                    mlkem_pie_addsub_8(&r[j], tvec0, &r[j], &r[j + len]);
                    mlkem_pie_addsub_8(&r[j + 8], tvec1, &r[j + 8], &r[j + len + 8]);
                }
                for (; j < start + len; j += 8) {
                    fqmul_vec8_zvec(tvec0, zvec, &r[j + len]);
                    mlkem_pie_addsub_8(&r[j], tvec0, &r[j], &r[j + len]);
                }
            } else {
                for (j = start; j < start + len; j++) {
                    t = fqmul(zeta, r[j + len]);
                    r[j + len] = r[j] - t;
                    r[j] = r[j] + t;
                }
            }
        }
    }
}

void PQCLEAN_MLKEM512_CLEAN_invntt(int16_t r[256]) {
    unsigned int start, len, j, k, i;
    int16_t t, zeta;
    const int16_t f = 1441; // mont^2/128
    int16_t sumvec0[8] __attribute__((aligned(16)));
    int16_t diffvec0[8] __attribute__((aligned(16)));
    int16_t sumvec1[8] __attribute__((aligned(16)));
    int16_t diffvec1[8] __attribute__((aligned(16)));

    k = 127;
    for (len = 2; len <= 128; len <<= 1) {
        for (start = 0; start < 256; start = j + len) {
            zeta = PQCLEAN_MLKEM512_CLEAN_zetas[k--];
            if (len >= 8) {
                int16_t zvec[8] __attribute__((aligned(16)));
                for (i = 0; i < 8; i++) {
                    zvec[i] = zeta;
                }
                for (j = start; j + 15 < start + len; j += 16) {
                    mlkem_pie_addsub_8(&r[j + len], &r[j], sumvec0, diffvec0);
                    mlkem_pie_addsub_8(&r[j + len + 8], &r[j + 8], sumvec1, diffvec1);
                    for (i = 0; i < 8; i++) {
                        r[j + i] = PQCLEAN_MLKEM512_CLEAN_barrett_reduce(sumvec0[i]);
                        r[j + 8 + i] = PQCLEAN_MLKEM512_CLEAN_barrett_reduce(sumvec1[i]);
                    }
                    fqmul_vec8_zvec_batch2(&r[j + len], &r[j + len + 8], zvec, diffvec0, diffvec1);
                }
                for (; j < start + len; j += 8) {
                    mlkem_pie_addsub_8(&r[j + len], &r[j], sumvec0, diffvec0);
                    for (i = 0; i < 8; i++) {
                        r[j + i] = PQCLEAN_MLKEM512_CLEAN_barrett_reduce(sumvec0[i]);
                    }
                    fqmul_vec8_zvec(&r[j + len], zvec, diffvec0);
                }
            } else {
                for (j = start; j < start + len; j++) {
                    t = r[j];
                    r[j] = PQCLEAN_MLKEM512_CLEAN_barrett_reduce(t + r[j + len]);
                    r[j + len] = r[j + len] - t;
                    r[j + len] = fqmul(zeta, r[j + len]);
                }
            }
        }
    }

    {
        int16_t fvec[8] __attribute__((aligned(16)));
        for (i = 0; i < 8; i++) {
            fvec[i] = f;
        }
        for (j = 0; j < 256; j += 16) {
            fqmul_vec8_zvec_batch2(&r[j], &r[j + 8], fvec, &r[j], &r[j + 8]);
        }
    }
}

void PQCLEAN_MLKEM512_CLEAN_basemul(int16_t r[2], const int16_t a[2], const int16_t b[2], int16_t zeta) {
    r[0] = fqmul(a[1], b[1]);
    r[0] = fqmul(r[0], zeta);
    r[0] += fqmul(a[0], b[0]);
    r[1] = fqmul(a[0], b[1]);
    r[1] += fqmul(a[1], b[0]);
}
