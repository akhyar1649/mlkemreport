#ifndef MLKEM_PIE_VEC_H
#define MLKEM_PIE_VEC_H

#include <stdint.h>

// PIE vector add/sub helper for 8x int16 lanes.
// VADDS/VSUBS saturate, but ML-KEM NTT ranges stay within int16 limits.
// Requires 16-byte aligned pointers.
static inline void mlkem_pie_addsub_8(const int16_t *a,
                                      const int16_t *b,
                                      int16_t *out_add,
                                      int16_t *out_sub) {
    const int16_t *pa = a;
    const int16_t *pb = b;
    int16_t *poa = out_add;
    int16_t *pos = out_sub;

    __asm__ volatile(
        "EE.VLD.128.IP q0, %0, 16\n"
        "EE.VLD.128.IP q1, %1, 16\n"
        "EE.VADDS.S16 q2, q0, q1\n"
        "EE.VSUBS.S16 q3, q0, q1\n"
        "EE.VST.128.IP q2, %2, 16\n"
        "EE.VST.128.IP q3, %3, 16\n"
        : "+r"(pa), "+r"(pb), "+r"(poa), "+r"(pos)
        :
        : "memory");
}

// PIE vector add helper for 8x int16 lanes.
// Requires 16-byte aligned pointers.
static inline void mlkem_pie_add_8(const int16_t *a, const int16_t *b, int16_t *out) {
    const int16_t *pa = a;
    const int16_t *pb = b;
    int16_t *po = out;

    __asm__ volatile(
        "EE.VLD.128.IP q0, %0, 16\n"
        "EE.VLD.128.IP q1, %1, 16\n"
        "EE.VADDS.S16 q2, q0, q1\n"
        "EE.VST.128.IP q2, %2, 16\n"
        : "+r"(pa), "+r"(pb), "+r"(po)
        :
        : "memory");
}

// PIE vector sub helper for 8x int16 lanes.
// Requires 16-byte aligned pointers.
static inline void mlkem_pie_sub_8(const int16_t *a, const int16_t *b, int16_t *out) {
    const int16_t *pa = a;
    const int16_t *pb = b;
    int16_t *po = out;

    __asm__ volatile(
        "EE.VLD.128.IP q0, %0, 16\n"
        "EE.VLD.128.IP q1, %1, 16\n"
        "EE.VSUBS.S16 q2, q0, q1\n"
        "EE.VST.128.IP q2, %2, 16\n"
        : "+r"(pa), "+r"(pb), "+r"(po)
        :
        : "memory");
}

// PIE vector multiply helper for 8x int16 lanes (low 16-bit result only).
// Requires 16-byte aligned pointers.
static inline void mlkem_pie_mul_lo_8(const int16_t *a,
                                      const int16_t *b,
                                      int16_t *out_lo) {
    const int16_t *pa = a;
    const int16_t *pb = b;
    int16_t *plo = out_lo;
    unsigned int sar_old;

    __asm__ volatile("rsr.sar %0" : "=a"(sar_old));

    __asm__ volatile("wsr.sar %0\n"
                     "rsync\n"
                     :
                     : "a"(0));

    __asm__ volatile(
        "EE.VLD.128.IP q0, %0, 16\n"
        "EE.VLD.128.IP q1, %1, 16\n"
        "EE.VMUL.S16 q2, q0, q1\n"
        "EE.VST.128.IP q2, %2, 16\n"
        : "+r"(pa), "+r"(pb), "+r"(plo)
        :
        : "memory");

    __asm__ volatile("wsr.sar %0\n"
                     "rsync\n"
                     :
                     : "a"(sar_old));
}

// PIE vector multiply helper for 2x 8-lane vectors (low 16-bit result only).
// Requires 16-byte aligned pointers.
static inline void mlkem_pie_mul_lo_8x2(const int16_t *a0,
                                        const int16_t *b0,
                                        int16_t *out0,
                                        const int16_t *a1,
                                        const int16_t *b1,
                                        int16_t *out1) {
#if defined(MLKEM_PIE_AGGRESSIVE)
    const int16_t *pa0 = a0;
    const int16_t *pb0 = b0;
    int16_t *po0 = out0;
    const int16_t *pa1 = a1;
    const int16_t *pb1 = b1;
    int16_t *po1 = out1;
    unsigned int sar_old;

    __asm__ volatile("rsr.sar %0" : "=a"(sar_old));

    __asm__ volatile("wsr.sar %0\n"
                     "rsync\n"
                     :
                     : "a"(0));

    __asm__ volatile(
        "EE.VLD.128.IP q0, %0, 16\n"
        "EE.VLD.128.IP q1, %1, 16\n"
        "EE.VMUL.S16 q2, q0, q1\n"
        "EE.VST.128.IP q2, %2, 16\n"
        "EE.VLD.128.IP q0, %3, 16\n"
        "EE.VLD.128.IP q1, %4, 16\n"
        "EE.VMUL.S16 q2, q0, q1\n"
        "EE.VST.128.IP q2, %5, 16\n"
        : "+r"(pa0), "+r"(pb0), "+r"(po0), "+r"(pa1), "+r"(pb1), "+r"(po1)
        :
        : "memory");

    __asm__ volatile("wsr.sar %0\n"
                     "rsync\n"
                     :
                     : "a"(sar_old));
#else
    mlkem_pie_mul_lo_8(a0, b0, out0);
    mlkem_pie_mul_lo_8(a1, b1, out1);
#endif
}

// PIE vector multiply helper for 8x int16 lanes.
// Uses SAR=0/16 to capture low/high 16-bit halves of 32-bit products.
// Requires 16-byte aligned pointers.
static inline void mlkem_pie_mul_lohi_8(const int16_t *a,
                                        const int16_t *b,
                                        int16_t *out_lo,
                                        int16_t *out_hi) {
    const int16_t *pa = a;
    const int16_t *pb = b;
    int16_t *plo = out_lo;
    int16_t *phi = out_hi;
    unsigned int sar_old;

    __asm__ volatile("rsr.sar %0" : "=a"(sar_old));

    __asm__ volatile("wsr.sar %0\n"
                     "rsync\n"
                     :
                     : "a"(0));

    __asm__ volatile(
        "EE.VLD.128.IP q0, %0, 16\n"
        "EE.VLD.128.IP q1, %1, 16\n"
        "EE.VMUL.S16 q2, q0, q1\n"
        "EE.VST.128.IP q2, %2, 16\n"
        : "+r"(pa), "+r"(pb), "+r"(plo)
        :
        : "memory");

    pa = a;
    pb = b;

    __asm__ volatile("wsr.sar %0\n"
                     "rsync\n"
                     :
                     : "a"(16));

    __asm__ volatile(
        "EE.VLD.128.IP q0, %0, 16\n"
        "EE.VLD.128.IP q1, %1, 16\n"
        "EE.VMUL.S16 q2, q0, q1\n"
        "EE.VST.128.IP q2, %2, 16\n"
        : "+r"(pa), "+r"(pb), "+r"(phi)
        :
        : "memory");

    __asm__ volatile("wsr.sar %0\n"
                     "rsync\n"
                     :
                     : "a"(sar_old));
}

// PIE vector multiply helper for 2x 8-lane vectors (low/high 16-bit halves).
// Requires 16-byte aligned pointers.
static inline void mlkem_pie_mul_lohi_8x2(const int16_t *a0,
                                          const int16_t *b0,
                                          int16_t *lo0,
                                          int16_t *hi0,
                                          const int16_t *a1,
                                          const int16_t *b1,
                                          int16_t *lo1,
                                          int16_t *hi1) {
#if defined(MLKEM_PIE_AGGRESSIVE)
    const int16_t *pa0 = a0;
    const int16_t *pb0 = b0;
    int16_t *plo0 = lo0;
    const int16_t *pa1 = a1;
    const int16_t *pb1 = b1;
    int16_t *plo1 = lo1;
    unsigned int sar_old;

    const int16_t *pa0_base = a0;
    const int16_t *pb0_base = b0;
    const int16_t *pa1_base = a1;
    const int16_t *pb1_base = b1;

    __asm__ volatile("rsr.sar %0" : "=a"(sar_old));

    __asm__ volatile("wsr.sar %0\n"
                     "rsync\n"
                     :
                     : "a"(0));

    __asm__ volatile(
        "EE.VLD.128.IP q0, %0, 16\n"
        "EE.VLD.128.IP q1, %1, 16\n"
        "EE.VMUL.S16 q2, q0, q1\n"
        "EE.VST.128.IP q2, %2, 16\n"
        "EE.VLD.128.IP q0, %3, 16\n"
        "EE.VLD.128.IP q1, %4, 16\n"
        "EE.VMUL.S16 q2, q0, q1\n"
        "EE.VST.128.IP q2, %5, 16\n"
        : "+r"(pa0), "+r"(pb0), "+r"(plo0), "+r"(pa1), "+r"(pb1), "+r"(plo1)
        :
        : "memory");

    pa0 = pa0_base;
    pb0 = pb0_base;
    pa1 = pa1_base;
    pb1 = pb1_base;

    __asm__ volatile("wsr.sar %0\n"
                     "rsync\n"
                     :
                     : "a"(16));

    __asm__ volatile(
        "EE.VLD.128.IP q0, %0, 16\n"
        "EE.VLD.128.IP q1, %1, 16\n"
        "EE.VMUL.S16 q2, q0, q1\n"
        "EE.VST.128.IP q2, %2, 16\n"
        "EE.VLD.128.IP q0, %3, 16\n"
        "EE.VLD.128.IP q1, %4, 16\n"
        "EE.VMUL.S16 q2, q0, q1\n"
        "EE.VST.128.IP q2, %5, 16\n"
        : "+r"(pa0), "+r"(pb0), "+r"(hi0), "+r"(pa1), "+r"(pb1), "+r"(hi1)
        :
        : "memory");

    __asm__ volatile("wsr.sar %0\n"
                     "rsync\n"
                     :
                     : "a"(sar_old));
#else
    mlkem_pie_mul_lohi_8(a0, b0, lo0, hi0);
    mlkem_pie_mul_lohi_8(a1, b1, lo1, hi1);
#endif
}

// PIE Montgomery reduce helper for 8x lanes.
// Requires QINV and KYBER_Q macros from reduce/params headers.
// Inputs are low/high 16-bit halves of signed 32-bit products.
// Requires 16-byte aligned pointers.
static inline void mlkem_pie_montgomery_reduce_vec8(const int16_t *prod_lo,
                                                    const int16_t *prod_hi,
                                                    int16_t *out) {
    const int16_t *qinvp;
    const int16_t *qp;
#if defined(MLKEM_PIE_AGGRESSIVE)
    static const int16_t qinv_vec[8] __attribute__((aligned(16))) = {
        (int16_t)QINV, (int16_t)QINV, (int16_t)QINV, (int16_t)QINV,
        (int16_t)QINV, (int16_t)QINV, (int16_t)QINV, (int16_t)QINV
    };
    static const int16_t q_vec[8] __attribute__((aligned(16))) = {
        (int16_t)KYBER_Q, (int16_t)KYBER_Q, (int16_t)KYBER_Q, (int16_t)KYBER_Q,
        (int16_t)KYBER_Q, (int16_t)KYBER_Q, (int16_t)KYBER_Q, (int16_t)KYBER_Q
    };
#else
    int16_t qinv_vec[8] __attribute__((aligned(16)));
    int16_t q_vec[8] __attribute__((aligned(16)));
#endif
    int16_t tvec[8] __attribute__((aligned(16)));
    int16_t tq_lo[8] __attribute__((aligned(16)));
    int16_t tq_hi[8] __attribute__((aligned(16)));
    unsigned int i;

#if defined(MLKEM_PIE_AGGRESSIVE)
    qinvp = qinv_vec;
    qp = q_vec;
#else
    for (i = 0; i < 8; i++) {
        qinv_vec[i] = (int16_t)QINV;
        q_vec[i] = (int16_t)KYBER_Q;
    }

    qinvp = qinv_vec;
    qp = q_vec;
#endif

    mlkem_pie_mul_lo_8(prod_lo, qinvp, tvec);
    mlkem_pie_mul_lohi_8(tvec, qp, tq_lo, tq_hi);

    for (i = 0; i < 8; i++) {
        int32_t a = ((int32_t)prod_hi[i] << 16) | (uint16_t)prod_lo[i];
        int32_t tq = ((int32_t)tq_hi[i] << 16) | (uint16_t)tq_lo[i];
        out[i] = (int16_t)((a - tq) >> 16);
    }
}

// PIE Montgomery reduce helper for 2x 8-lane vectors.
// Requires 16-byte aligned pointers.
static inline void mlkem_pie_montgomery_reduce_vec8x2(const int16_t *prod_lo0,
                                                      const int16_t *prod_hi0,
                                                      int16_t *out0,
                                                      const int16_t *prod_lo1,
                                                      const int16_t *prod_hi1,
                                                      int16_t *out1) {
    const int16_t *qinvp;
    const int16_t *qp;
#if defined(MLKEM_PIE_AGGRESSIVE)
    static const int16_t qinv_vec[8] __attribute__((aligned(16))) = {
        (int16_t)QINV, (int16_t)QINV, (int16_t)QINV, (int16_t)QINV,
        (int16_t)QINV, (int16_t)QINV, (int16_t)QINV, (int16_t)QINV
    };
    static const int16_t q_vec[8] __attribute__((aligned(16))) = {
        (int16_t)KYBER_Q, (int16_t)KYBER_Q, (int16_t)KYBER_Q, (int16_t)KYBER_Q,
        (int16_t)KYBER_Q, (int16_t)KYBER_Q, (int16_t)KYBER_Q, (int16_t)KYBER_Q
    };
#else
    int16_t qinv_vec[8] __attribute__((aligned(16)));
    int16_t q_vec[8] __attribute__((aligned(16)));
#endif
    int16_t tvec0[8] __attribute__((aligned(16)));
    int16_t tvec1[8] __attribute__((aligned(16)));
    int16_t tq_lo0[8] __attribute__((aligned(16)));
    int16_t tq_hi0[8] __attribute__((aligned(16)));
    int16_t tq_lo1[8] __attribute__((aligned(16)));
    int16_t tq_hi1[8] __attribute__((aligned(16)));
    unsigned int i;

#if defined(MLKEM_PIE_AGGRESSIVE)
    qinvp = qinv_vec;
    qp = q_vec;
#else
    for (i = 0; i < 8; i++) {
        qinv_vec[i] = (int16_t)QINV;
        q_vec[i] = (int16_t)KYBER_Q;
    }

    qinvp = qinv_vec;
    qp = q_vec;
#endif

    mlkem_pie_mul_lo_8x2(prod_lo0, qinvp, tvec0, prod_lo1, qinvp, tvec1);
    mlkem_pie_mul_lohi_8x2(tvec0, qp, tq_lo0, tq_hi0, tvec1, qp, tq_lo1, tq_hi1);

    for (i = 0; i < 8; i++) {
        int32_t a0 = ((int32_t)prod_hi0[i] << 16) | (uint16_t)prod_lo0[i];
        int32_t tq0 = ((int32_t)tq_hi0[i] << 16) | (uint16_t)tq_lo0[i];
        out0[i] = (int16_t)((a0 - tq0) >> 16);

        int32_t a1 = ((int32_t)prod_hi1[i] << 16) | (uint16_t)prod_lo1[i];
        int32_t tq1 = ((int32_t)tq_hi1[i] << 16) | (uint16_t)tq_lo1[i];
        out1[i] = (int16_t)((a1 - tq1) >> 16);
    }
}

#endif
