#include "params.h"
#include "poly.h"
#include "polyvec.h"
#ifdef MLKEM_PIE
#include "reduce.h"
#include "pie_vec.h"
#endif
#include <stdint.h>

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_polyvec_compress
*
* Description: Compress and serialize vector of polynomials
*
* Arguments:   - uint8_t *r: pointer to output byte array
*                            (needs space for KYBER_POLYVECCOMPRESSEDBYTES)
*              - const polyvec *a: pointer to input vector of polynomials
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_polyvec_compress(uint8_t r[KYBER_POLYVECCOMPRESSEDBYTES], const polyvec *a) {
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

    const uint32_t recip = 1290167u;
    const uint32_t c0_u = recip & 0xFFFFu;
    const int16_t c0_s = (int16_t)c0_u;
    const int16_t c1 = (int16_t)(recip >> 16);

    // Reconstruct (acc * recip) >> 32 using 16x16 partial products.

    for (k = 0; k < 8; k++) {
        c0_vec[k] = c0_s;
        c1_vec[k] = c1;
    }

    for (i = 0; i < KYBER_K; i++) {
        for (j = 0; j < KYBER_N / 4; j += 2) {
            for (k = 0; k < 8; k++) {
                int16_t coeff = a->vec[i].coeffs[4 * j + k];
                uint32_t acc;

                coeff += ((int16_t)coeff >> 15) & KYBER_Q;
                acc = ((uint32_t)(uint16_t)coeff << 10) + 1665u;

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

                t8[k] = (uint16_t)(hi & 0x3FFu);
            }

            r[0] = (uint8_t)(t8[0] >> 0);
            r[1] = (uint8_t)((t8[0] >> 8) | (t8[1] << 2));
            r[2] = (uint8_t)((t8[1] >> 6) | (t8[2] << 4));
            r[3] = (uint8_t)((t8[2] >> 4) | (t8[3] << 6));
            r[4] = (uint8_t)(t8[3] >> 2);
            r += 5;

            r[0] = (uint8_t)(t8[4] >> 0);
            r[1] = (uint8_t)((t8[4] >> 8) | (t8[5] << 2));
            r[2] = (uint8_t)((t8[5] >> 6) | (t8[6] << 4));
            r[3] = (uint8_t)((t8[6] >> 4) | (t8[7] << 6));
            r[4] = (uint8_t)(t8[7] >> 2);
            r += 5;
        }
    }
#else
    uint64_t d0;
    uint16_t t[4];

    for (i = 0; i < KYBER_K; i++) {
        for (j = 0; j < KYBER_N / 4; j++) {
            for (k = 0; k < 4; k++) {
                t[k]  = a->vec[i].coeffs[4 * j + k];
                t[k] += ((int16_t)t[k] >> 15) & KYBER_Q;
                /*      t[k]  = ((((uint32_t)t[k] << 10) + KYBER_Q/2)/ KYBER_Q) & 0x3ff; */
                d0 = t[k];
                d0 <<= 10;
                d0 += 1665;
                d0 *= 1290167;
                d0 >>= 32;
                t[k] = d0 & 0x3ff;
            }

            r[0] = (uint8_t)(t[0] >> 0);
            r[1] = (uint8_t)((t[0] >> 8) | (t[1] << 2));
            r[2] = (uint8_t)((t[1] >> 6) | (t[2] << 4));
            r[3] = (uint8_t)((t[2] >> 4) | (t[3] << 6));
            r[4] = (uint8_t)(t[3] >> 2);
            r += 5;
        }
    }
#endif
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_polyvec_decompress
*
* Description: De-serialize and decompress vector of polynomials;
*              approximate inverse of PQCLEAN_MLKEM512_CLEAN_polyvec_compress
*
* Arguments:   - polyvec *r:       pointer to output vector of polynomials
*              - const uint8_t *a: pointer to input byte array
*                                  (of length KYBER_POLYVECCOMPRESSEDBYTES)
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_polyvec_decompress(polyvec *r, const uint8_t a[KYBER_POLYVECCOMPRESSEDBYTES]) {
    unsigned int i, j, k;

    uint16_t t[4];
#ifdef MLKEM_PIE
    int16_t qvec[8] __attribute__((aligned(16)));
    int16_t tv[8] __attribute__((aligned(16)));
    int16_t prod_lo[8] __attribute__((aligned(16)));
    int16_t prod_hi[8] __attribute__((aligned(16)));

    for (k = 0; k < 8; k++) {
        qvec[k] = (int16_t)KYBER_Q;
    }

    for (i = 0; i < KYBER_K; i++) {
        for (j = 0; j < KYBER_N / 4; j += 2) {
            uint16_t t1[4];

            t[0] = (a[0] >> 0) | ((uint16_t)a[1] << 8);
            t[1] = (a[1] >> 2) | ((uint16_t)a[2] << 6);
            t[2] = (a[2] >> 4) | ((uint16_t)a[3] << 4);
            t[3] = (a[3] >> 6) | ((uint16_t)a[4] << 2);
            a += 5;

            t1[0] = (a[0] >> 0) | ((uint16_t)a[1] << 8);
            t1[1] = (a[1] >> 2) | ((uint16_t)a[2] << 6);
            t1[2] = (a[2] >> 4) | ((uint16_t)a[3] << 4);
            t1[3] = (a[3] >> 6) | ((uint16_t)a[4] << 2);
            a += 5;

            tv[0] = (int16_t)(t[0] & 0x3FF);
            tv[1] = (int16_t)(t[1] & 0x3FF);
            tv[2] = (int16_t)(t[2] & 0x3FF);
            tv[3] = (int16_t)(t[3] & 0x3FF);
            tv[4] = (int16_t)(t1[0] & 0x3FF);
            tv[5] = (int16_t)(t1[1] & 0x3FF);
            tv[6] = (int16_t)(t1[2] & 0x3FF);
            tv[7] = (int16_t)(t1[3] & 0x3FF);

            mlkem_pie_mul_lohi_8(tv, qvec, prod_lo, prod_hi);

            for (k = 0; k < 8; k++) {
                uint32_t prod = ((uint32_t)(uint16_t)prod_hi[k] << 16) | (uint16_t)prod_lo[k];
                uint16_t val = (uint16_t)((prod + 512) >> 10);
                if (k < 4) {
                    r->vec[i].coeffs[4 * j + k] = (int16_t)val;
                } else {
                    r->vec[i].coeffs[4 * (j + 1) + (k - 4)] = (int16_t)val;
                }
            }
        }
    }
#else
    for (i = 0; i < KYBER_K; i++) {
        for (j = 0; j < KYBER_N / 4; j++) {
            t[0] = (a[0] >> 0) | ((uint16_t)a[1] << 8);
            t[1] = (a[1] >> 2) | ((uint16_t)a[2] << 6);
            t[2] = (a[2] >> 4) | ((uint16_t)a[3] << 4);
            t[3] = (a[3] >> 6) | ((uint16_t)a[4] << 2);
            a += 5;

            for (k = 0; k < 4; k++) {
                r->vec[i].coeffs[4 * j + k] = ((uint32_t)(t[k] & 0x3FF) * KYBER_Q + 512) >> 10;
            }
        }
    }
#endif
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_polyvec_tobytes
*
* Description: Serialize vector of polynomials
*
* Arguments:   - uint8_t *r: pointer to output byte array
*                            (needs space for KYBER_POLYVECBYTES)
*              - const polyvec *a: pointer to input vector of polynomials
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_polyvec_tobytes(uint8_t r[KYBER_POLYVECBYTES], const polyvec *a) {
    unsigned int i;
    for (i = 0; i < KYBER_K; i++) {
        PQCLEAN_MLKEM512_CLEAN_poly_tobytes(r + i * KYBER_POLYBYTES, &a->vec[i]);
    }
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_polyvec_frombytes
*
* Description: De-serialize vector of polynomials;
*              inverse of PQCLEAN_MLKEM512_CLEAN_polyvec_tobytes
*
* Arguments:   - uint8_t *r:       pointer to output byte array
*              - const polyvec *a: pointer to input vector of polynomials
*                                  (of length KYBER_POLYVECBYTES)
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_polyvec_frombytes(polyvec *r, const uint8_t a[KYBER_POLYVECBYTES]) {
    unsigned int i;
    for (i = 0; i < KYBER_K; i++) {
        PQCLEAN_MLKEM512_CLEAN_poly_frombytes(&r->vec[i], a + i * KYBER_POLYBYTES);
    }
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_polyvec_ntt
*
* Description: Apply forward NTT to all elements of a vector of polynomials
*
* Arguments:   - polyvec *r: pointer to in/output vector of polynomials
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_polyvec_ntt(polyvec *r) {
    unsigned int i;
    for (i = 0; i < KYBER_K; i++) {
        PQCLEAN_MLKEM512_CLEAN_poly_ntt(&r->vec[i]);
    }
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_polyvec_invntt_tomont
*
* Description: Apply inverse NTT to all elements of a vector of polynomials
*              and multiply by Montgomery factor 2^16
*
* Arguments:   - polyvec *r: pointer to in/output vector of polynomials
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_polyvec_invntt_tomont(polyvec *r) {
    unsigned int i;
    for (i = 0; i < KYBER_K; i++) {
        PQCLEAN_MLKEM512_CLEAN_poly_invntt_tomont(&r->vec[i]);
    }
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_polyvec_basemul_acc_montgomery
*
* Description: Multiply elements of a and b in NTT domain, accumulate into r,
*              and multiply by 2^-16.
*
* Arguments: - poly *r: pointer to output polynomial
*            - const polyvec *a: pointer to first input vector of polynomials
*            - const polyvec *b: pointer to second input vector of polynomials
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_polyvec_basemul_acc_montgomery(poly *r, const polyvec *a, const polyvec *b) {
    unsigned int i;
    poly t;

    PQCLEAN_MLKEM512_CLEAN_poly_basemul_montgomery(r, &a->vec[0], &b->vec[0]);
    for (i = 1; i < KYBER_K; i++) {
        PQCLEAN_MLKEM512_CLEAN_poly_basemul_montgomery(&t, &a->vec[i], &b->vec[i]);
        PQCLEAN_MLKEM512_CLEAN_poly_add(r, r, &t);
    }

    PQCLEAN_MLKEM512_CLEAN_poly_reduce(r);
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_polyvec_reduce
*
* Description: Applies Barrett reduction to each coefficient
*              of each element of a vector of polynomials;
*              for details of the Barrett reduction see comments in reduce.c
*
* Arguments:   - polyvec *r: pointer to input/output polynomial
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_polyvec_reduce(polyvec *r) {
    unsigned int i;
    for (i = 0; i < KYBER_K; i++) {
        PQCLEAN_MLKEM512_CLEAN_poly_reduce(&r->vec[i]);
    }
}

/*************************************************
* Name:        PQCLEAN_MLKEM512_CLEAN_polyvec_add
*
* Description: Add vectors of polynomials
*
* Arguments: - polyvec *r: pointer to output vector of polynomials
*            - const polyvec *a: pointer to first input vector of polynomials
*            - const polyvec *b: pointer to second input vector of polynomials
**************************************************/
void PQCLEAN_MLKEM512_CLEAN_polyvec_add(polyvec *r, const polyvec *a, const polyvec *b) {
    unsigned int i;
    for (i = 0; i < KYBER_K; i++) {
        PQCLEAN_MLKEM512_CLEAN_poly_add(&r->vec[i], &a->vec[i], &b->vec[i]);
    }
}
