#include "params.h"
#include "poly.h"
#include "polyvec.h"
#ifdef MLKEM_PIE
#include "reduce.h"
#include "pie_vec.h"
#endif
#include <stdint.h>

/*************************************************
* Name:        PQCLEAN_MLKEM1024_CLEAN_polyvec_compress
*
* Description: Compress and serialize vector of polynomials
*
* Arguments:   - uint8_t *r: pointer to output byte array
*                            (needs space for KYBER_POLYVECCOMPRESSEDBYTES)
*              - const polyvec *a: pointer to input vector of polynomials
**************************************************/
void PQCLEAN_MLKEM1024_CLEAN_polyvec_compress(uint8_t r[KYBER_POLYVECCOMPRESSEDBYTES], const polyvec *a) {
    unsigned int i, j, k;
#ifdef MLKEM_PIE
    uint16_t t8[8];
    uint16_t a0_u[8];
    uint16_t a0_carry[8];
    int16_t a0_s[8] __attribute__((aligned(16)));
    int16_t a1_vec[8] __attribute__((aligned(16)));
    int16_t c0_vec[8] __attribute__((aligned(16)));
    int16_t c1_vec[8] __attribute__((aligned(16)));
    int16_t p0_lo[8] __attribute__((aligned(16)));
    int16_t p0_hi[8] __attribute__((aligned(16)));
    int16_t p1_lo[8] __attribute__((aligned(16)));
    int16_t p1_hi[8] __attribute__((aligned(16)));
    int16_t p2_lo[8] __attribute__((aligned(16)));
    int16_t p2_hi[8] __attribute__((aligned(16)));
    int16_t p3_lo[8] __attribute__((aligned(16)));
    int16_t p3_hi[8] __attribute__((aligned(16)));

    const uint32_t recip = 1290168u;
    const uint32_t c0_u = recip & 0xFFFFu;
    const int16_t c0_s = (int16_t)c0_u;
    const int16_t c1 = (int16_t)(recip >> 16);

    // Reconstruct (acc * recip) >> 32 using 16x16 partial products.

    for (k = 0; k < 8; k++) {
        c0_vec[k] = c0_s;
        c1_vec[k] = c1;
    }

    for (i = 0; i < KYBER_K; i++) {
        for (j = 0; j < KYBER_N / 8; j++) {
            for (k = 0; k < 8; k++) {
                int16_t coeff = a->vec[i].coeffs[8 * j + k];
                uint32_t acc;

                coeff += ((int16_t)coeff >> 15) & KYBER_Q;
                acc = ((uint32_t)(uint16_t)coeff << 11) + 1664u;

                a0_u[k] = (uint16_t)acc;
                a0_carry[k] = a0_u[k] >> 15;
                a0_s[k] = (int16_t)a0_u[k];
                a1_vec[k] = (int16_t)(acc >> 16);
            }

            mlkem_pie_mul_lohi_8x2(a0_s, c0_vec, p0_lo, p0_hi, a0_s, c1_vec, p1_lo, p1_hi);
            mlkem_pie_mul_lohi_8x2(a1_vec, c0_vec, p2_lo, p2_hi, a1_vec, c1_vec, p3_lo, p3_hi);

            for (k = 0; k < 8; k++) {
                uint32_t p0_s = ((uint32_t)(uint16_t)p0_lo[k]) | ((uint32_t)(uint16_t)p0_hi[k] << 16);
                uint32_t p1_s = ((uint32_t)(uint16_t)p1_lo[k]) | ((uint32_t)(uint16_t)p1_hi[k] << 16);
                uint32_t p2_s = ((uint32_t)(uint16_t)p2_lo[k]) | ((uint32_t)(uint16_t)p2_hi[k] << 16);
                uint32_t p3_s = ((uint32_t)(uint16_t)p3_lo[k]) | ((uint32_t)(uint16_t)p3_hi[k] << 16);

                uint32_t p0 = p0_s;
                uint32_t p1;
                uint32_t p2;
                uint32_t p3;
                uint32_t s;
                uint32_t low;
                uint32_t carry;
                uint32_t hi;

                p0 += ((uint32_t)a0_u[k]) << 16;
                p0 += (uint32_t)a0_carry[k] * (c0_u << 16);

                p1 = p1_s + ((uint32_t)a0_carry[k] * ((uint32_t)c1 << 16));
                p2 = p2_s + ((uint32_t)(uint16_t)a1_vec[k] << 16);
                p3 = p3_s;

                s = p1 + p2;
                low = p0 + ((s & 0xFFFFu) << 16);
                carry = (low < p0);
                hi = p3 + (s >> 16) + carry;

                t8[k] = (uint16_t)(hi & 0x7FFu);
            }

            r[ 0] = (uint8_t)(t8[0] >>  0);
            r[ 1] = (uint8_t)((t8[0] >>  8) | (t8[1] << 3));
            r[ 2] = (uint8_t)((t8[1] >>  5) | (t8[2] << 6));
            r[ 3] = (uint8_t)(t8[2] >>  2);
            r[ 4] = (uint8_t)((t8[2] >> 10) | (t8[3] << 1));
            r[ 5] = (uint8_t)((t8[3] >>  7) | (t8[4] << 4));
            r[ 6] = (uint8_t)((t8[4] >>  4) | (t8[5] << 7));
            r[ 7] = (uint8_t)(t8[5] >>  1);
            r[ 8] = (uint8_t)((t8[5] >>  9) | (t8[6] << 2));
            r[ 9] = (uint8_t)((t8[6] >>  6) | (t8[7] << 5));
            r[10] = (uint8_t)(t8[7] >>  3);
            r += 11;
        }
    }
#else
    uint64_t d0;
    uint16_t t[8];

    for (i = 0; i < KYBER_K; i++) {
        for (j = 0; j < KYBER_N / 8; j++) {
            for (k = 0; k < 8; k++) {
                t[k]  = a->vec[i].coeffs[8 * j + k];
                t[k] += ((int16_t)t[k] >> 15) & KYBER_Q;
                /*      t[k]  = ((((uint32_t)t[k] << 11) + KYBER_Q/2)/KYBER_Q) & 0x7ff; */
                d0 = t[k];
                d0 <<= 11;
                d0 += 1664;
                d0 *= 645084;
                d0 >>= 31;
                t[k] = d0 & 0x7ff;
            }

            r[ 0] = (uint8_t)(t[0] >>  0);
            r[ 1] = (uint8_t)((t[0] >>  8) | (t[1] << 3));
            r[ 2] = (uint8_t)((t[1] >>  5) | (t[2] << 6));
            r[ 3] = (uint8_t)(t[2] >>  2);
            r[ 4] = (uint8_t)((t[2] >> 10) | (t[3] << 1));
            r[ 5] = (uint8_t)((t[3] >>  7) | (t[4] << 4));
            r[ 6] = (uint8_t)((t[4] >>  4) | (t[5] << 7));
            r[ 7] = (uint8_t)(t[5] >>  1);
            r[ 8] = (uint8_t)((t[5] >>  9) | (t[6] << 2));
            r[ 9] = (uint8_t)((t[6] >>  6) | (t[7] << 5));
            r[10] = (uint8_t)(t[7] >>  3);
            r += 11;
        }
    }
#endif
}

/*************************************************
* Name:        PQCLEAN_MLKEM1024_CLEAN_polyvec_decompress
*
* Description: De-serialize and decompress vector of polynomials;
*              approximate inverse of PQCLEAN_MLKEM1024_CLEAN_polyvec_compress
*
* Arguments:   - polyvec *r:       pointer to output vector of polynomials
*              - const uint8_t *a: pointer to input byte array
*                                  (of length KYBER_POLYVECCOMPRESSEDBYTES)
**************************************************/
void PQCLEAN_MLKEM1024_CLEAN_polyvec_decompress(polyvec *r, const uint8_t a[KYBER_POLYVECCOMPRESSEDBYTES]) {
    unsigned int i, j, k;

    uint16_t t[8];
#ifdef MLKEM_PIE
    int16_t qvec[8] __attribute__((aligned(16)));
    int16_t tv0[8] __attribute__((aligned(16)));
    int16_t tv1[8] __attribute__((aligned(16)));
    int16_t prod_lo0[8] __attribute__((aligned(16)));
    int16_t prod_hi0[8] __attribute__((aligned(16)));
    int16_t prod_lo1[8] __attribute__((aligned(16)));
    int16_t prod_hi1[8] __attribute__((aligned(16)));

    for (k = 0; k < 8; k++) {
        qvec[k] = (int16_t)KYBER_Q;
    }

    for (i = 0; i < KYBER_K; i++) {
        for (j = 0; j < KYBER_N / 8; j += 2) {
            t[0] = (a[0] >> 0) | ((uint16_t)a[1] << 8);
            t[1] = (a[1] >> 3) | ((uint16_t)a[2] << 5);
            t[2] = (a[2] >> 6) | ((uint16_t)a[3] << 2) | ((uint16_t)a[4] << 10);
            t[3] = (a[4] >> 1) | ((uint16_t)a[5] << 7);
            t[4] = (a[5] >> 4) | ((uint16_t)a[6] << 4);
            t[5] = (a[6] >> 7) | ((uint16_t)a[7] << 1) | ((uint16_t)a[8] << 9);
            t[6] = (a[8] >> 2) | ((uint16_t)a[9] << 6);
            t[7] = (a[9] >> 5) | ((uint16_t)a[10] << 3);
            a += 11;

            tv0[0] = (int16_t)(t[0] & 0x7FF);
            tv0[1] = (int16_t)(t[1] & 0x7FF);
            tv0[2] = (int16_t)(t[2] & 0x7FF);
            tv0[3] = (int16_t)(t[3] & 0x7FF);
            tv0[4] = (int16_t)(t[4] & 0x7FF);
            tv0[5] = (int16_t)(t[5] & 0x7FF);
            tv0[6] = (int16_t)(t[6] & 0x7FF);
            tv0[7] = (int16_t)(t[7] & 0x7FF);

            t[0] = (a[0] >> 0) | ((uint16_t)a[1] << 8);
            t[1] = (a[1] >> 3) | ((uint16_t)a[2] << 5);
            t[2] = (a[2] >> 6) | ((uint16_t)a[3] << 2) | ((uint16_t)a[4] << 10);
            t[3] = (a[4] >> 1) | ((uint16_t)a[5] << 7);
            t[4] = (a[5] >> 4) | ((uint16_t)a[6] << 4);
            t[5] = (a[6] >> 7) | ((uint16_t)a[7] << 1) | ((uint16_t)a[8] << 9);
            t[6] = (a[8] >> 2) | ((uint16_t)a[9] << 6);
            t[7] = (a[9] >> 5) | ((uint16_t)a[10] << 3);
            a += 11;

            tv1[0] = (int16_t)(t[0] & 0x7FF);
            tv1[1] = (int16_t)(t[1] & 0x7FF);
            tv1[2] = (int16_t)(t[2] & 0x7FF);
            tv1[3] = (int16_t)(t[3] & 0x7FF);
            tv1[4] = (int16_t)(t[4] & 0x7FF);
            tv1[5] = (int16_t)(t[5] & 0x7FF);
            tv1[6] = (int16_t)(t[6] & 0x7FF);
            tv1[7] = (int16_t)(t[7] & 0x7FF);

            mlkem_pie_mul_lohi_8x2(tv0, qvec, prod_lo0, prod_hi0, tv1, qvec, prod_lo1, prod_hi1);

            for (k = 0; k < 8; k++) {
                uint32_t prod = ((uint32_t)(uint16_t)prod_hi0[k] << 16) | (uint16_t)prod_lo0[k];
                uint16_t val = (uint16_t)((prod + 1024) >> 11);
                r->vec[i].coeffs[8 * j + k] = (int16_t)val;
            }

            for (k = 0; k < 8; k++) {
                uint32_t prod = ((uint32_t)(uint16_t)prod_hi1[k] << 16) | (uint16_t)prod_lo1[k];
                uint16_t val = (uint16_t)((prod + 1024) >> 11);
                r->vec[i].coeffs[8 * (j + 1) + k] = (int16_t)val;
            }
        }
    }
#else
    for (i = 0; i < KYBER_K; i++) {
        for (j = 0; j < KYBER_N / 8; j++) {
            t[0] = (a[0] >> 0) | ((uint16_t)a[ 1] << 8);
            t[1] = (a[1] >> 3) | ((uint16_t)a[ 2] << 5);
            t[2] = (a[2] >> 6) | ((uint16_t)a[ 3] << 2) | ((uint16_t)a[4] << 10);
            t[3] = (a[4] >> 1) | ((uint16_t)a[ 5] << 7);
            t[4] = (a[5] >> 4) | ((uint16_t)a[ 6] << 4);
            t[5] = (a[6] >> 7) | ((uint16_t)a[ 7] << 1) | ((uint16_t)a[8] << 9);
            t[6] = (a[8] >> 2) | ((uint16_t)a[ 9] << 6);
            t[7] = (a[9] >> 5) | ((uint16_t)a[10] << 3);
            a += 11;

            for (k = 0; k < 8; k++) {
                r->vec[i].coeffs[8 * j + k] = ((uint32_t)(t[k] & 0x7FF) * KYBER_Q + 1024) >> 11;
            }
        }
    }
#endif
}

/*************************************************
* Name:        PQCLEAN_MLKEM1024_CLEAN_polyvec_tobytes
*
* Description: Serialize vector of polynomials
*
* Arguments:   - uint8_t *r: pointer to output byte array
*                            (needs space for KYBER_POLYVECBYTES)
*              - const polyvec *a: pointer to input vector of polynomials
**************************************************/
void PQCLEAN_MLKEM1024_CLEAN_polyvec_tobytes(uint8_t r[KYBER_POLYVECBYTES], const polyvec *a) {
    unsigned int i;
    for (i = 0; i < KYBER_K; i++) {
        PQCLEAN_MLKEM1024_CLEAN_poly_tobytes(r + i * KYBER_POLYBYTES, &a->vec[i]);
    }
}

/*************************************************
* Name:        PQCLEAN_MLKEM1024_CLEAN_polyvec_frombytes
*
* Description: De-serialize vector of polynomials;
*              inverse of PQCLEAN_MLKEM1024_CLEAN_polyvec_tobytes
*
* Arguments:   - uint8_t *r:       pointer to output byte array
*              - const polyvec *a: pointer to input vector of polynomials
*                                  (of length KYBER_POLYVECBYTES)
**************************************************/
void PQCLEAN_MLKEM1024_CLEAN_polyvec_frombytes(polyvec *r, const uint8_t a[KYBER_POLYVECBYTES]) {
    unsigned int i;
    for (i = 0; i < KYBER_K; i++) {
        PQCLEAN_MLKEM1024_CLEAN_poly_frombytes(&r->vec[i], a + i * KYBER_POLYBYTES);
    }
}

/*************************************************
* Name:        PQCLEAN_MLKEM1024_CLEAN_polyvec_ntt
*
* Description: Apply forward NTT to all elements of a vector of polynomials
*
* Arguments:   - polyvec *r: pointer to in/output vector of polynomials
**************************************************/
void PQCLEAN_MLKEM1024_CLEAN_polyvec_ntt(polyvec *r) {
    unsigned int i;
    for (i = 0; i < KYBER_K; i++) {
        PQCLEAN_MLKEM1024_CLEAN_poly_ntt(&r->vec[i]);
    }
}

/*************************************************
* Name:        PQCLEAN_MLKEM1024_CLEAN_polyvec_invntt_tomont
*
* Description: Apply inverse NTT to all elements of a vector of polynomials
*              and multiply by Montgomery factor 2^16
*
* Arguments:   - polyvec *r: pointer to in/output vector of polynomials
**************************************************/
void PQCLEAN_MLKEM1024_CLEAN_polyvec_invntt_tomont(polyvec *r) {
    unsigned int i;
    for (i = 0; i < KYBER_K; i++) {
        PQCLEAN_MLKEM1024_CLEAN_poly_invntt_tomont(&r->vec[i]);
    }
}

/*************************************************
* Name:        PQCLEAN_MLKEM1024_CLEAN_polyvec_basemul_acc_montgomery
*
* Description: Multiply elements of a and b in NTT domain, accumulate into r,
*              and multiply by 2^-16.
*
* Arguments: - poly *r: pointer to output polynomial
*            - const polyvec *a: pointer to first input vector of polynomials
*            - const polyvec *b: pointer to second input vector of polynomials
**************************************************/
void PQCLEAN_MLKEM1024_CLEAN_polyvec_basemul_acc_montgomery(poly *r, const polyvec *a, const polyvec *b) {
    unsigned int i;
    poly t;

    PQCLEAN_MLKEM1024_CLEAN_poly_basemul_montgomery(r, &a->vec[0], &b->vec[0]);
    for (i = 1; i < KYBER_K; i++) {
        PQCLEAN_MLKEM1024_CLEAN_poly_basemul_montgomery(&t, &a->vec[i], &b->vec[i]);
        PQCLEAN_MLKEM1024_CLEAN_poly_add(r, r, &t);
    }

    PQCLEAN_MLKEM1024_CLEAN_poly_reduce(r);
}

/*************************************************
* Name:        PQCLEAN_MLKEM1024_CLEAN_polyvec_reduce
*
* Description: Applies Barrett reduction to each coefficient
*              of each element of a vector of polynomials;
*              for details of the Barrett reduction see comments in reduce.c
*
* Arguments:   - polyvec *r: pointer to input/output polynomial
**************************************************/
void PQCLEAN_MLKEM1024_CLEAN_polyvec_reduce(polyvec *r) {
    unsigned int i;
    for (i = 0; i < KYBER_K; i++) {
        PQCLEAN_MLKEM1024_CLEAN_poly_reduce(&r->vec[i]);
    }
}

/*************************************************
* Name:        PQCLEAN_MLKEM1024_CLEAN_polyvec_add
*
* Description: Add vectors of polynomials
*
* Arguments: - polyvec *r: pointer to output vector of polynomials
*            - const polyvec *a: pointer to first input vector of polynomials
*            - const polyvec *b: pointer to second input vector of polynomials
**************************************************/
void PQCLEAN_MLKEM1024_CLEAN_polyvec_add(polyvec *r, const polyvec *a, const polyvec *b) {
    unsigned int i;
    for (i = 0; i < KYBER_K; i++) {
        PQCLEAN_MLKEM1024_CLEAN_poly_add(&r->vec[i], &a->vec[i], &b->vec[i]);
    }
}
