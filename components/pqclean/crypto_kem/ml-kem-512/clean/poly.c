#include "cbd.h"
#include "ntt.h"
#include "params.h"
#include "poly.h"
#include "reduce.h"
#include "symmetric.h"
#include "verify.h"
#include <stdint.h>
#ifdef MLKEM_PIE
#include "pie_vec.h"
#endif

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_poly_compress
*
* Description: Compression and subsequent serialization of a polynomial
*
* Arguments:   - uint8_t *r: pointer to output byte array
*                            (of length KYBER_POLYCOMPRESSEDBYTES)
*              - const poly *a: pointer to input polynomial
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_poly_compress(uint8_t r[KYBER_POLYCOMPRESSEDBYTES], const poly *a) {
    unsigned int i, j;
    int16_t u;
    uint32_t d0;
    uint8_t t[8];


    for (i = 0; i < KYBER_N / 8; i++) {
        for (j = 0; j < 8; j++) {
            // map to positive standard representatives
            u  = a->coeffs[8 * i + j];
            u += (u >> 15) & KYBER_Q;
            /*    t[j] = ((((uint16_t)u << 4) + KYBER_Q/2)/KYBER_Q) & 15; */
            d0 = u << 4;
            d0 += 1665;
            d0 *= 80635;
            d0 >>= 28;
            t[j] = d0 & 0xf;
        }

        r[0] = t[0] | (t[1] << 4);
        r[1] = t[2] | (t[3] << 4);
        r[2] = t[4] | (t[5] << 4);
        r[3] = t[6] | (t[7] << 4);
        r += 4;
    }
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_poly_decompress
*
* Description: De-serialization and subsequent decompression of a polynomial;
*              approximate inverse of PQCLEAN_MLKEM512_CLEAN_poly_compress
*
* Arguments:   - poly *r: pointer to output polynomial
*              - const uint8_t *a: pointer to input byte array
*                                  (of length KYBER_POLYCOMPRESSEDBYTES bytes)
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_poly_decompress(poly *r, const uint8_t a[KYBER_POLYCOMPRESSEDBYTES]) {
    size_t i;

    for (i = 0; i < KYBER_N / 2; i++) {
        r->coeffs[2 * i + 0] = (((uint16_t)(a[0] & 15) * KYBER_Q) + 8) >> 4;
        r->coeffs[2 * i + 1] = (((uint16_t)(a[0] >> 4) * KYBER_Q) + 8) >> 4;
        a += 1;
    }
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_poly_tobytes
*
* Description: Serialization of a polynomial
*
* Arguments:   - uint8_t *r: pointer to output byte array
*                            (needs space for KYBER_POLYBYTES bytes)
*              - const poly *a: pointer to input polynomial
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_poly_tobytes(uint8_t r[KYBER_POLYBYTES], const poly *a) {
    size_t i;
    uint16_t t0, t1;

    for (i = 0; i < KYBER_N / 2; i++) {
        // map to positive standard representatives
        t0  = a->coeffs[2 * i];
        t0 += ((int16_t)t0 >> 15) & KYBER_Q;
        t1 = a->coeffs[2 * i + 1];
        t1 += ((int16_t)t1 >> 15) & KYBER_Q;
        r[3 * i + 0] = (uint8_t)(t0 >> 0);
        r[3 * i + 1] = (uint8_t)((t0 >> 8) | (t1 << 4));
        r[3 * i + 2] = (uint8_t)(t1 >> 4);
    }
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_poly_frombytes
*
* Description: De-serialization of a polynomial;
*              inverse of PQCLEAN_MLKEM512_CLEAN_poly_tobytes
*
* Arguments:   - poly *r: pointer to output polynomial
*              - const uint8_t *a: pointer to input byte array
*                                  (of KYBER_POLYBYTES bytes)
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_poly_frombytes(poly *r, const uint8_t a[KYBER_POLYBYTES]) {
    size_t i;
    for (i = 0; i < KYBER_N / 2; i++) {
        r->coeffs[2 * i]   = ((a[3 * i + 0] >> 0) | ((uint16_t)a[3 * i + 1] << 8)) & 0xFFF;
        r->coeffs[2 * i + 1] = ((a[3 * i + 1] >> 4) | ((uint16_t)a[3 * i + 2] << 4)) & 0xFFF;
    }
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_poly_frommsg
*
* Description: Convert 32-byte message to polynomial
*
* Arguments:   - poly *r: pointer to output polynomial
*              - const uint8_t *msg: pointer to input message
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_poly_frommsg(poly *r, const uint8_t msg[KYBER_INDCPA_MSGBYTES]) {
    size_t i, j;

    for (i = 0; i < KYBER_N / 8; i++) {
        for (j = 0; j < 8; j++) {
            r->coeffs[8 * i + j] = 0;
            PQCLEAN_MLKEM512_CLEAN_cmov_int16(r->coeffs + 8 * i + j, ((KYBER_Q + 1) / 2), (msg[i] >> j) & 1);
        }
    }
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_poly_tomsg
*
* Description: Convert polynomial to 32-byte message
*
* Arguments:   - uint8_t *msg: pointer to output message
*              - const poly *a: pointer to input polynomial
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_poly_tomsg(uint8_t msg[KYBER_INDCPA_MSGBYTES], const poly *a) {
    unsigned int i, j;
    uint32_t t;

    for (i = 0; i < KYBER_N / 8; i++) {
        msg[i] = 0;
        for (j = 0; j < 8; j++) {
            t  = a->coeffs[8 * i + j];
            // t += ((int16_t)t >> 15) & KYBER_Q;
            // t  = (((t << 1) + KYBER_Q/2)/KYBER_Q) & 1;
            t <<= 1;
            t += 1665;
            t *= 80635;
            t >>= 28;
            t &= 1;
            msg[i] |= t << j;
        }
    }
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_poly_getnoise_eta1
*
* Description: Sample a polynomial deterministically from a seed and a nonce,
*              with output polynomial close to centered binomial distribution
*              with parameter KYBER_ETA1
*
* Arguments:   - poly *r: pointer to output polynomial
*              - const uint8_t *seed: pointer to input seed
*                                     (of length KYBER_SYMBYTES bytes)
*              - uint8_t nonce: one-byte input nonce
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_poly_getnoise_eta1(poly *r, const uint8_t seed[KYBER_SYMBYTES], uint8_t nonce) {
    uint8_t buf[KYBER_ETA1 * KYBER_N / 4];
    prf(buf, sizeof(buf), seed, nonce);
    PQCLEAN_MLKEM512_CLEAN_poly_cbd_eta1(r, buf);
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_poly_getnoise_eta2
*
* Description: Sample a polynomial deterministically from a seed and a nonce,
*              with output polynomial close to centered binomial distribution
*              with parameter KYBER_ETA2
*
* Arguments:   - poly *r: pointer to output polynomial
*              - const uint8_t *seed: pointer to input seed
*                                     (of length KYBER_SYMBYTES bytes)
*              - uint8_t nonce: one-byte input nonce
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_poly_getnoise_eta2(poly *r, const uint8_t seed[KYBER_SYMBYTES], uint8_t nonce) {
    uint8_t buf[KYBER_ETA2 * KYBER_N / 4];
    prf(buf, sizeof(buf), seed, nonce);
    PQCLEAN_MLKEM512_CLEAN_poly_cbd_eta2(r, buf);
}


/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_poly_ntt
*
* Description: Computes negacyclic number-theoretic transform (NTT) of
*              a polynomial in place;
*              inputs assumed to be in normal order, output in bitreversed order
*
* Arguments:   - uint16_t *r: pointer to in/output polynomial
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_poly_ntt(poly *r) {
    PQCLEAN_MLKEM512_CLEAN_ntt(r->coeffs);
    PQCLEAN_MLKEM512_CLEAN_poly_reduce(r);
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_poly_invntt_tomont
*
* Description: Computes inverse of negacyclic number-theoretic transform (NTT)
*              of a polynomial in place;
*              inputs assumed to be in bitreversed order, output in normal order
*
* Arguments:   - uint16_t *a: pointer to in/output polynomial
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_poly_invntt_tomont(poly *r) {
    PQCLEAN_MLKEM512_CLEAN_invntt(r->coeffs);
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_poly_basemul_montgomery
*
* Description: Multiplication of two polynomials in NTT domain
*
* Arguments:   - poly *r: pointer to output polynomial
*              - const poly *a: pointer to first input polynomial
*              - const poly *b: pointer to second input polynomial
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_poly_basemul_montgomery(poly *r, const poly *a, const poly *b) {
    size_t i;
#ifdef MLKEM_PIE
    const size_t nblocks = KYBER_N / 4;
    int16_t avec0[8] __attribute__((aligned(16)));
    int16_t bvec0[8] __attribute__((aligned(16)));
    int16_t avec1[8] __attribute__((aligned(16)));
    int16_t bvec1[8] __attribute__((aligned(16)));
    int16_t prod_lo0[8] __attribute__((aligned(16)));
    int16_t prod_hi0[8] __attribute__((aligned(16)));
    int16_t prod_lo1[8] __attribute__((aligned(16)));
    int16_t prod_hi1[8] __attribute__((aligned(16)));
    int16_t mul0[8];
    int16_t mul1[8];

    for (i = 0; i + 1 < nblocks; i += 2) {
        const int16_t *ap0 = &a->coeffs[4 * i];
        const int16_t *bp0 = &b->coeffs[4 * i];
        const int16_t *ap1 = &a->coeffs[4 * (i + 1)];
        const int16_t *bp1 = &b->coeffs[4 * (i + 1)];
        int16_t zeta0 = PQCLEAN_MLKEM512_CLEAN_zetas[64 + i];
        int16_t zeta1 = PQCLEAN_MLKEM512_CLEAN_zetas[64 + i + 1];
        int16_t zeta0_neg = (int16_t)-zeta0;
        int16_t zeta1_neg = (int16_t)-zeta1;
        int16_t *rp0 = &r->coeffs[4 * i];
        int16_t *rp1 = &r->coeffs[4 * (i + 1)];

        // Lane layout: [a1*b1, a1*b0, a0*b1, a0*b0, a3*b3, a3*b2, a2*b3, a2*b2]
        avec0[0] = ap0[1];
        bvec0[0] = bp0[1];
        avec0[1] = ap0[1];
        bvec0[1] = bp0[0];
        avec0[2] = ap0[0];
        bvec0[2] = bp0[1];
        avec0[3] = ap0[0];
        bvec0[3] = bp0[0];
        avec0[4] = ap0[3];
        bvec0[4] = bp0[3];
        avec0[5] = ap0[3];
        bvec0[5] = bp0[2];
        avec0[6] = ap0[2];
        bvec0[6] = bp0[3];
        avec0[7] = ap0[2];
        bvec0[7] = bp0[2];

        avec1[0] = ap1[1];
        bvec1[0] = bp1[1];
        avec1[1] = ap1[1];
        bvec1[1] = bp1[0];
        avec1[2] = ap1[0];
        bvec1[2] = bp1[1];
        avec1[3] = ap1[0];
        bvec1[3] = bp1[0];
        avec1[4] = ap1[3];
        bvec1[4] = bp1[3];
        avec1[5] = ap1[3];
        bvec1[5] = bp1[2];
        avec1[6] = ap1[2];
        bvec1[6] = bp1[3];
        avec1[7] = ap1[2];
        bvec1[7] = bp1[2];

        mlkem_pie_mul_lohi_8x2(avec0, bvec0, prod_lo0, prod_hi0, avec1, bvec1, prod_lo1, prod_hi1);
        mlkem_pie_montgomery_reduce_vec8x2(prod_lo0, prod_hi0, mul0, prod_lo1, prod_hi1, mul1);

        rp0[0] = PQCLEAN_MLKEM512_CLEAN_montgomery_reduce((int32_t)mul0[0] * zeta0) + mul0[3];
        rp0[1] = mul0[2] + mul0[1];
        rp0[2] = PQCLEAN_MLKEM512_CLEAN_montgomery_reduce((int32_t)mul0[4] * zeta0_neg) + mul0[7];
        rp0[3] = mul0[6] + mul0[5];

        rp1[0] = PQCLEAN_MLKEM512_CLEAN_montgomery_reduce((int32_t)mul1[0] * zeta1) + mul1[3];
        rp1[1] = mul1[2] + mul1[1];
        rp1[2] = PQCLEAN_MLKEM512_CLEAN_montgomery_reduce((int32_t)mul1[4] * zeta1_neg) + mul1[7];
        rp1[3] = mul1[6] + mul1[5];
    }

    if (i < nblocks) {
        const int16_t *ap = &a->coeffs[4 * i];
        const int16_t *bp = &b->coeffs[4 * i];
        int16_t zeta = PQCLEAN_MLKEM512_CLEAN_zetas[64 + i];
        int16_t zeta_neg = (int16_t)-zeta;
        int16_t prod_lo[8] __attribute__((aligned(16)));
        int16_t prod_hi[8] __attribute__((aligned(16)));
        int16_t mul[8];

        avec0[0] = ap[1];
        bvec0[0] = bp[1];
        avec0[1] = ap[1];
        bvec0[1] = bp[0];
        avec0[2] = ap[0];
        bvec0[2] = bp[1];
        avec0[3] = ap[0];
        bvec0[3] = bp[0];
        avec0[4] = ap[3];
        bvec0[4] = bp[3];
        avec0[5] = ap[3];
        bvec0[5] = bp[2];
        avec0[6] = ap[2];
        bvec0[6] = bp[3];
        avec0[7] = ap[2];
        bvec0[7] = bp[2];

        mlkem_pie_mul_lohi_8(avec0, bvec0, prod_lo, prod_hi);
        mlkem_pie_montgomery_reduce_vec8(prod_lo, prod_hi, mul);

        r->coeffs[4 * i + 0] = PQCLEAN_MLKEM512_CLEAN_montgomery_reduce((int32_t)mul[0] * zeta) + mul[3];
        r->coeffs[4 * i + 1] = mul[2] + mul[1];
        r->coeffs[4 * i + 2] = PQCLEAN_MLKEM512_CLEAN_montgomery_reduce((int32_t)mul[4] * zeta_neg) + mul[7];
        r->coeffs[4 * i + 3] = mul[6] + mul[5];
    }
#else
    for (i = 0; i < KYBER_N / 4; i++) {
        PQCLEAN_MLKEM512_CLEAN_basemul(&r->coeffs[4 * i], &a->coeffs[4 * i], &b->coeffs[4 * i], PQCLEAN_MLKEM512_CLEAN_zetas[64 + i]);
        PQCLEAN_MLKEM512_CLEAN_basemul(&r->coeffs[4 * i + 2], &a->coeffs[4 * i + 2], &b->coeffs[4 * i + 2], -PQCLEAN_MLKEM512_CLEAN_zetas[64 + i]);
    }
#endif
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_poly_tomont
*
* Description: Inplace conversion of all coefficients of a polynomial
*              from normal domain to Montgomery domain
*
* Arguments:   - poly *r: pointer to input/output polynomial
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_poly_tomont(poly *r) {
    size_t i;
    const int16_t f = (1ULL << 32) % KYBER_Q;
#ifdef MLKEM_PIE
    int16_t fvec[8] __attribute__((aligned(16)));
    int16_t prod_lo0[8] __attribute__((aligned(16)));
    int16_t prod_hi0[8] __attribute__((aligned(16)));
    int16_t prod_lo1[8] __attribute__((aligned(16)));
    int16_t prod_hi1[8] __attribute__((aligned(16)));
    int16_t out0[8] __attribute__((aligned(16)));
    int16_t out1[8] __attribute__((aligned(16)));

    for (i = 0; i < 8; i++) {
        fvec[i] = f;
    }

    for (i = 0; i < KYBER_N; i += 16) {
        mlkem_pie_mul_lohi_8x2(fvec, &r->coeffs[i], prod_lo0, prod_hi0,
                               fvec, &r->coeffs[i + 8], prod_lo1, prod_hi1);
        mlkem_pie_montgomery_reduce_vec8x2(prod_lo0, prod_hi0, out0, prod_lo1, prod_hi1, out1);
        for (size_t k = 0; k < 8; k++) {
            r->coeffs[i + k] = out0[k];
            r->coeffs[i + 8 + k] = out1[k];
        }
    }
#else
    for (i = 0; i < KYBER_N; i++) {
        r->coeffs[i] = PQCLEAN_MLKEM512_CLEAN_montgomery_reduce((int32_t)r->coeffs[i] * f);
    }
#endif
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_poly_reduce
*
* Description: Applies Barrett reduction to all coefficients of a polynomial
*              for details of the Barrett reduction see comments in reduce.c
*
* Arguments:   - poly *r: pointer to input/output polynomial
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_poly_reduce(poly *r) {
    size_t i;
#ifdef MLKEM_PIE
    const int16_t v = ((1 << 26) + KYBER_Q / 2) / KYBER_Q;
    int16_t vvec[8] __attribute__((aligned(16)));
    int16_t prod_lo0[8] __attribute__((aligned(16)));
    int16_t prod_hi0[8] __attribute__((aligned(16)));
    int16_t prod_lo1[8] __attribute__((aligned(16)));
    int16_t prod_hi1[8] __attribute__((aligned(16)));

    for (i = 0; i < 8; i++) {
        vvec[i] = v;
    }

    for (i = 0; i < KYBER_N; i += 16) {
        mlkem_pie_mul_lohi_8x2(vvec, &r->coeffs[i], prod_lo0, prod_hi0,
                               vvec, &r->coeffs[i + 8], prod_lo1, prod_hi1);
        for (size_t k = 0; k < 8; k++) {
            int32_t prod = ((int32_t)prod_hi0[k] << 16) | (uint16_t)prod_lo0[k];
            int32_t t = (prod + (1 << 25)) >> 26;
            r->coeffs[i + k] = (int16_t)(r->coeffs[i + k] - (int16_t)(t * KYBER_Q));
        }
        for (size_t k = 0; k < 8; k++) {
            int32_t prod = ((int32_t)prod_hi1[k] << 16) | (uint16_t)prod_lo1[k];
            int32_t t = (prod + (1 << 25)) >> 26;
            r->coeffs[i + 8 + k] = (int16_t)(r->coeffs[i + 8 + k] - (int16_t)(t * KYBER_Q));
        }
    }
#else
    for (i = 0; i < KYBER_N; i++) {
        r->coeffs[i] = PQCLEAN_MLKEM512_CLEAN_barrett_reduce(r->coeffs[i]);
    }
#endif
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_poly_add
*
* Description: Add two polynomials; no modular reduction is performed
*
* Arguments: - poly *r: pointer to output polynomial
*            - const poly *a: pointer to first input polynomial
*            - const poly *b: pointer to second input polynomial
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_poly_add(poly *r, const poly *a, const poly *b) {
    size_t i;
#ifdef MLKEM_PIE
    for (i = 0; i < KYBER_N; i += 8) {
        mlkem_pie_add_8(&a->coeffs[i], &b->coeffs[i], &r->coeffs[i]);
    }
#else
    for (i = 0; i < KYBER_N; i++) {
        r->coeffs[i] = a->coeffs[i] + b->coeffs[i];
    }
#endif
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_poly_sub
*
* Description: Subtract two polynomials; no modular reduction is performed
*
* Arguments: - poly *r:       pointer to output polynomial
*            - const poly *a: pointer to first input polynomial
*            - const poly *b: pointer to second input polynomial
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_poly_sub(poly *r, const poly *a, const poly *b) {
    size_t i;
#ifdef MLKEM_PIE
    for (i = 0; i < KYBER_N; i += 8) {
        mlkem_pie_sub_8(&a->coeffs[i], &b->coeffs[i], &r->coeffs[i]);
    }
#else
    for (i = 0; i < KYBER_N; i++) {
        r->coeffs[i] = a->coeffs[i] - b->coeffs[i];
    }
#endif
}
