/*
    Copyright (C) 2021 Fredrik Johansson

    This file is part of Arb.

    Arb is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include <math.h>

#include "arb.h"
#include "acb.h"
#include "arb_fpwrap.h"
#include "arb_hypgeom.h"
#include "acb_hypgeom.h"


int
arb_accurate_enough_d(const arb_t x, int flags)
{
    if (flags & FPWRAP_CORRECT_ROUNDING)
    {
        return arb_can_round_arf(x, 53, ARF_RND_NEAR);
    }

    if (arb_rel_accuracy_bits(x) >= 53 + 1)
        return 1;

    /* Rounding will give +/- 0 (we don't worry which) */
    if (mag_cmp_2exp_si(arb_radref(x), -1077) < 0 &&
            arf_cmpabs_2exp_si(arb_midref(x), -1077) < 0)
    {
        return 1;
    }

    /* Rounding will give +/- inf */
    if (arb_rel_accuracy_bits(x) > 2 &&
        arf_cmpabs_2exp_si(arb_midref(x), 1024) > 0)
    {
        return 1;
    }

    return 0;
}

int
acb_accurate_enough_d(const acb_t x, int flags)
{
    if (flags & FPWRAP_CORRECT_ROUNDING)
    {
        return arb_can_round_arf(acb_realref(x), 53, ARF_RND_NEAR) &&
               arb_can_round_arf(acb_imagref(x), 53, ARF_RND_NEAR);
    }

    if (flags & FPWRAP_ACCURATE_PARTS)
    {
        return arb_accurate_enough_d(acb_realref(x), flags) &&
               arb_accurate_enough_d(acb_imagref(x), flags);
    }

    if (acb_rel_accuracy_bits(x) >= 53 + 1)
        return 1;

    /* Rounding will give +/- 0 (we don't worry which) */
    if (mag_cmp_2exp_si(arb_radref(acb_realref(x)), -1077) < 0 &&
            arf_cmpabs_2exp_si(arb_midref(acb_realref(x)), -1077) < 0 &&
            mag_cmp_2exp_si(arb_radref(acb_imagref(x)), -1077) < 0 &&
            arf_cmpabs_2exp_si(arb_midref(acb_imagref(x)), -1077) < 0)
    {
        return 1;
    }

    /* Rounding will give +/- inf */
    if (acb_rel_accuracy_bits(x) > 2 &&
        (arf_cmpabs_2exp_si(arb_midref(acb_realref(x)), 1024) > 0 ||
         arf_cmpabs_2exp_si(arb_midref(acb_imagref(x)), 1024) > 0))
    {
        return 1;
    }

    return 0;
}

#define WP_INITIAL 64

static slong
double_wp_max(int flags)
{
    int iters;

    iters = flags / FPWRAP_WORK_LIMIT;

    if (iters <= 0)
        return 64 << 7;

    if (iters >= 25)
        return 64 << 24;

    return 64 << iters;
}


typedef void (*arb_func_1)(arb_t, const arb_t, slong prec);
typedef void (*arb_func_2)(arb_t, const arb_t, const arb_t, slong prec);
typedef void (*arb_func_3)(arb_t, const arb_t, const arb_t, const arb_t, slong prec);
typedef void (*arb_func_4)(arb_t, const arb_t, const arb_t, const arb_t, const arb_t, slong prec);

typedef void (*acb_func_1)(acb_t, const acb_t, slong prec);
typedef void (*acb_func_2)(acb_t, const acb_t, const acb_t, slong prec);
typedef void (*acb_func_3)(acb_t, const acb_t, const acb_t, const acb_t, slong prec);
typedef void (*acb_func_4)(acb_t, const acb_t, const acb_t, const acb_t, const acb_t, slong prec);

int arb_fpwrap_double_1(double * res, arb_func_1 func, double x, int flags)
{
    arb_t arb_res, arb_x;
    slong wp;
    int status;

    arb_init(arb_res);
    arb_init(arb_x);

    arb_set_d(arb_x, x);

    if (!arb_is_finite(arb_x))
    {
        *res = D_NAN;
        status = FPWRAP_UNABLE;
    }
    else
    {
        for (wp = WP_INITIAL; ; wp *= 2)
        {
            func(arb_res, arb_x, wp);

            if (arb_accurate_enough_d(arb_res, flags))
            {
                *res = arf_get_d(arb_midref(arb_res), ARF_RND_NEAR);
                status = FPWRAP_SUCCESS;
                break;
            }

            if (wp >= double_wp_max(flags))
            {
                *res = D_NAN;
                status = FPWRAP_UNABLE;
                break;
            }
        }
    }

    arb_clear(arb_x);
    arb_clear(arb_res);

    return status;
}

int arb_fpwrap_double_2(double * res, arb_func_2 func, double x1, double x2, int flags)
{
    arb_t arb_res, arb_x1, arb_x2;
    slong wp;
    int status;

    arb_init(arb_res);
    arb_init(arb_x1);
    arb_init(arb_x2);

    arb_set_d(arb_x1, x1);
    arb_set_d(arb_x2, x2);

    if (!arb_is_finite(arb_x1) || !arb_is_finite(arb_x2))
    {
        *res = D_NAN;
        status = FPWRAP_UNABLE;
    }
    else
    {
        for (wp = WP_INITIAL; ; wp *= 2)
        {
            func(arb_res, arb_x1, arb_x2, wp);

            if (arb_accurate_enough_d(arb_res, flags))
            {
                *res = arf_get_d(arb_midref(arb_res), ARF_RND_NEAR);
                status = FPWRAP_SUCCESS;
                break;
            }

            if (wp >= double_wp_max(flags))
            {
                *res = D_NAN;
                status = FPWRAP_UNABLE;
                break;
            }
        }
    }

    arb_clear(arb_x1);
    arb_clear(arb_x2);
    arb_clear(arb_res);

    return status;
}

int arb_fpwrap_double_3(double * res, arb_func_3 func, double x1, double x2, double x3, int flags)
{
    arb_t arb_res, arb_x1, arb_x2, arb_x3;
    slong wp;
    int status;

    arb_init(arb_res);
    arb_init(arb_x1);
    arb_init(arb_x2);
    arb_init(arb_x3);

    arb_set_d(arb_x1, x1);
    arb_set_d(arb_x2, x2);
    arb_set_d(arb_x3, x3);

    if (!arb_is_finite(arb_x1) || !arb_is_finite(arb_x2) || !arb_is_finite(arb_x3))
    {
        *res = D_NAN;
        status = FPWRAP_UNABLE;
    }
    else
    {
        for (wp = WP_INITIAL; ; wp *= 2)
        {
            func(arb_res, arb_x1, arb_x2, arb_x3, wp);

            if (arb_accurate_enough_d(arb_res, flags))
            {
                *res = arf_get_d(arb_midref(arb_res), ARF_RND_NEAR);
                status = FPWRAP_SUCCESS;
                break;
            }

            if (wp >= double_wp_max(flags))
            {
                *res = D_NAN;
                status = FPWRAP_UNABLE;
                break;
            }
        }
    }

    arb_clear(arb_x1);
    arb_clear(arb_x2);
    arb_clear(arb_x3);
    arb_clear(arb_res);

    return status;
}

int arb_fpwrap_double_4(double * res, arb_func_4 func, double x1, double x2, double x3, double x4, int flags)
{
    arb_t arb_res, arb_x1, arb_x2, arb_x3, arb_x4;
    slong wp;
    int status;

    arb_init(arb_res);
    arb_init(arb_x1);
    arb_init(arb_x2);
    arb_init(arb_x3);
    arb_init(arb_x4);

    arb_set_d(arb_x1, x1);
    arb_set_d(arb_x2, x2);
    arb_set_d(arb_x3, x3);
    arb_set_d(arb_x3, x4);

    if (!arb_is_finite(arb_x1) || !arb_is_finite(arb_x2) || !arb_is_finite(arb_x3) || !arb_is_finite(arb_x4))
    {
        *res = D_NAN;
        status = FPWRAP_UNABLE;
    }
    else
    {
        for (wp = WP_INITIAL; ; wp *= 2)
        {
            func(arb_res, arb_x1, arb_x2, arb_x3, arb_x4, wp);

            if (arb_accurate_enough_d(arb_res, flags))
            {
                *res = arf_get_d(arb_midref(arb_res), ARF_RND_NEAR);
                status = FPWRAP_SUCCESS;
                break;
            }

            if (wp >= double_wp_max(flags))
            {
                *res = D_NAN;
                status = FPWRAP_UNABLE;
                break;
            }
        }
    }

    arb_clear(arb_x1);
    arb_clear(arb_x2);
    arb_clear(arb_x3);
    arb_clear(arb_x4);
    arb_clear(arb_res);

    return status;
}


int arb_fpwrap_cdouble_1(complex_double * res, acb_func_1 func, complex_double x, int flags)
{
    acb_t acb_res, acb_x;
    slong wp;
    int status;

    acb_init(acb_res);
    acb_init(acb_x);

    acb_set_d_d(acb_x, x.real, x.imag);

    if (!acb_is_finite(acb_x))
    {
        res->real = D_NAN;
        res->imag = D_NAN;
        status = FPWRAP_UNABLE;
    }
    else
    {
        for (wp = WP_INITIAL; ; wp *= 2)
        {
            func(acb_res, acb_x, wp);

            if (acb_accurate_enough_d(acb_res, flags))
            {
                res->real = arf_get_d(arb_midref(acb_realref(acb_res)), ARF_RND_NEAR);
                res->imag = arf_get_d(arb_midref(acb_imagref(acb_res)), ARF_RND_NEAR);
                status = FPWRAP_SUCCESS;
                break;
            }

            if (wp >= double_wp_max(flags))
            {
                res->real = D_NAN;
                res->imag = D_NAN;
                status = FPWRAP_UNABLE;
                break;
            }
        }
    }

    acb_clear(acb_x);
    acb_clear(acb_res);

    return status;
}

int arb_fpwrap_cdouble_2(complex_double * res, acb_func_2 func, complex_double x1, complex_double x2, int flags)
{
    acb_t acb_res, acb_x1, acb_x2;
    slong wp;
    int status;

    acb_init(acb_res);
    acb_init(acb_x1);
    acb_init(acb_x2);

    acb_set_d_d(acb_x1, x1.real, x1.imag);
    acb_set_d_d(acb_x2, x2.real, x2.imag);

    if (!acb_is_finite(acb_x1) || !acb_is_finite(acb_x2))
    {
        res->real = D_NAN;
        res->imag = D_NAN;
        status = FPWRAP_UNABLE;
    }
    else
    {
        for (wp = WP_INITIAL; ; wp *= 2)
        {
            func(acb_res, acb_x1, acb_x2, wp);

            if (acb_accurate_enough_d(acb_res, flags))
            {
                res->real = arf_get_d(arb_midref(acb_realref(acb_res)), ARF_RND_NEAR);
                res->imag = arf_get_d(arb_midref(acb_imagref(acb_res)), ARF_RND_NEAR);
                status = FPWRAP_SUCCESS;
                break;
            }

            if (wp >= double_wp_max(flags))
            {
                res->real = D_NAN;
                res->imag = D_NAN;
                status = FPWRAP_UNABLE;
                break;
            }
        }
    }

    acb_clear(acb_x1);
    acb_clear(acb_x2);
    acb_clear(acb_res);

    return status;
}

int arb_fpwrap_cdouble_3(complex_double * res, acb_func_3 func, complex_double x1, complex_double x2, complex_double x3, int flags)
{
    acb_t acb_res, acb_x1, acb_x2, acb_x3;
    slong wp;
    int status;

    acb_init(acb_res);
    acb_init(acb_x1);
    acb_init(acb_x2);
    acb_init(acb_x3);

    acb_set_d_d(acb_x1, x1.real, x1.imag);
    acb_set_d_d(acb_x2, x2.real, x2.imag);
    acb_set_d_d(acb_x3, x3.real, x3.imag);

    if (!acb_is_finite(acb_x1) || !acb_is_finite(acb_x2) || !acb_is_finite(acb_x3))
    {
        res->real = D_NAN;
        res->imag = D_NAN;
        status = FPWRAP_UNABLE;
    }
    else
    {
        for (wp = WP_INITIAL; ; wp *= 2)
        {
            func(acb_res, acb_x1, acb_x2, acb_x3, wp);

            if (acb_accurate_enough_d(acb_res, flags))
            {
                res->real = arf_get_d(arb_midref(acb_realref(acb_res)), ARF_RND_NEAR);
                res->imag = arf_get_d(arb_midref(acb_imagref(acb_res)), ARF_RND_NEAR);
                status = FPWRAP_SUCCESS;
                break;
            }

            if (wp >= double_wp_max(flags))
            {
                res->real = D_NAN;
                res->imag = D_NAN;
                status = FPWRAP_UNABLE;
                break;
            }
        }
    }

    acb_clear(acb_x1);
    acb_clear(acb_x2);
    acb_clear(acb_x3);
    acb_clear(acb_res);

    return status;
}

int arb_fpwrap_cdouble_4(complex_double * res, acb_func_4 func, complex_double x1, complex_double x2, complex_double x3, complex_double x4, int flags)
{
    acb_t acb_res, acb_x1, acb_x2, acb_x3, acb_x4;
    slong wp;
    int status;

    acb_init(acb_res);
    acb_init(acb_x1);
    acb_init(acb_x2);
    acb_init(acb_x3);
    acb_init(acb_x4);

    acb_set_d_d(acb_x1, x1.real, x1.imag);
    acb_set_d_d(acb_x2, x2.real, x2.imag);
    acb_set_d_d(acb_x3, x3.real, x3.imag);
    acb_set_d_d(acb_x4, x4.real, x4.imag);

    if (!acb_is_finite(acb_x1) || !acb_is_finite(acb_x2) || !acb_is_finite(acb_x3) || !acb_is_finite(acb_x4))
    {
        res->real = D_NAN;
        res->imag = D_NAN;
        status = FPWRAP_UNABLE;
    }
    else
    {
        for (wp = WP_INITIAL; ; wp *= 2)
        {
            func(acb_res, acb_x1, acb_x2, acb_x3, acb_x4, wp);

            if (acb_accurate_enough_d(acb_res, flags))
            {
                res->real = arf_get_d(arb_midref(acb_realref(acb_res)), ARF_RND_NEAR);
                res->imag = arf_get_d(arb_midref(acb_imagref(acb_res)), ARF_RND_NEAR);
                status = FPWRAP_SUCCESS;
                break;
            }

            if (wp >= double_wp_max(flags))
            {
                res->real = D_NAN;
                res->imag = D_NAN;
                status = FPWRAP_UNABLE;
                break;
            }
        }
    }

    acb_clear(acb_x1);
    acb_clear(acb_x2);
    acb_clear(acb_x3);
    acb_clear(acb_x4);
    acb_clear(acb_res);

    return status;
}

#define DEF_DOUBLE_FUN_1(name, arb_fun) \
    int arb_fpwrap_double_ ## name(double * res, double x, int flags) \
    { \
        return arb_fpwrap_double_1(res, arb_fun, x, flags); \
    } \

#define DEF_DOUBLE_FUN_2(name, arb_fun) \
    int arb_fpwrap_double_ ## name(double * res, double x1, double x2, int flags) \
    { \
        return arb_fpwrap_double_2(res, arb_fun, x1, x2, flags); \
    } \

#define DEF_DOUBLE_FUN_3(name, arb_fun) \
    int arb_fpwrap_double_ ## name(double * res, double x1, double x2, double x3, int flags) \
    { \
        return arb_fpwrap_double_3(res, arb_fun, x1, x2, x3, flags); \
    } \

#define DEF_DOUBLE_FUN_4(name, arb_fun) \
    int arb_fpwrap_double_ ## name(double * res, double x1, double x2, double x3, double x4, int flags) \
    { \
        return arb_fpwrap_double_4(res, arb_fun, x1, x2, x3, x4, flags); \
    } \

#define DEF_CDOUBLE_FUN_1(name, acb_fun) \
    int arb_fpwrap_cdouble_ ## name(complex_double * res, complex_double x, int flags) \
    { \
        return arb_fpwrap_cdouble_1(res, acb_fun, x, flags); \
    } \

#define DEF_CDOUBLE_FUN_2(name, acb_fun) \
    int arb_fpwrap_cdouble_ ## name(complex_double * res, complex_double x1, complex_double x2, int flags) \
    { \
        return arb_fpwrap_cdouble_2(res, acb_fun, x1, x2, flags); \
    } \

#define DEF_CDOUBLE_FUN_3(name, acb_fun) \
    int arb_fpwrap_cdouble_ ## name(complex_double * res, complex_double x1, complex_double x2, complex_double x3, int flags) \
    { \
        return arb_fpwrap_cdouble_3(res, acb_fun, x1, x2, x3, flags); \
    } \

#define DEF_CDOUBLE_FUN_4(name, acb_fun) \
    int arb_fpwrap_cdouble_ ## name(complex_double * res, complex_double x1, complex_double x2, complex_double x3, complex_double x4, int flags) \
    { \
        return arb_fpwrap_cdouble_4(res, acb_fun, x1, x2, x3, x4, flags); \
    } \


DEF_DOUBLE_FUN_1(exp, arb_exp)
DEF_CDOUBLE_FUN_1(exp, acb_exp)

DEF_DOUBLE_FUN_1(expm1, arb_expm1)
DEF_CDOUBLE_FUN_1(expm1, acb_expm1)

DEF_DOUBLE_FUN_1(log, arb_log)
DEF_CDOUBLE_FUN_1(log, acb_log)

DEF_DOUBLE_FUN_1(log1p, arb_log1p)
DEF_CDOUBLE_FUN_1(log1p, acb_log1p)

DEF_DOUBLE_FUN_1(sqrt, arb_sqrt)
DEF_CDOUBLE_FUN_1(sqrt, acb_sqrt)

DEF_DOUBLE_FUN_1(rsqrt, arb_rsqrt)
DEF_CDOUBLE_FUN_1(rsqrt, acb_rsqrt)

static void _arb_cbrt(arb_t res, const arb_t x, slong prec) { arb_root_ui(res, x, 3, prec); }
static void _acb_cbrt(acb_t res, const acb_t x, slong prec) { acb_root_ui(res, x, 3, prec); }

DEF_DOUBLE_FUN_1(cbrt, _arb_cbrt)
DEF_CDOUBLE_FUN_1(cbrt, _acb_cbrt)

DEF_DOUBLE_FUN_1(sin, arb_sin)
DEF_CDOUBLE_FUN_1(sin, acb_sin)

DEF_DOUBLE_FUN_1(cos, arb_cos)
DEF_CDOUBLE_FUN_1(cos, acb_cos)

DEF_DOUBLE_FUN_1(tan, arb_tan)
DEF_CDOUBLE_FUN_1(tan, acb_tan)

DEF_DOUBLE_FUN_1(cot, arb_cot)
DEF_CDOUBLE_FUN_1(cot, acb_cot)

DEF_DOUBLE_FUN_1(sec, arb_sec)
DEF_CDOUBLE_FUN_1(sec, acb_sec)

DEF_DOUBLE_FUN_1(csc, arb_csc)
DEF_CDOUBLE_FUN_1(csc, acb_csc)

DEF_DOUBLE_FUN_1(sinc, arb_sinc)
DEF_CDOUBLE_FUN_1(sinc, acb_sinc)

DEF_DOUBLE_FUN_1(sin_pi, arb_sin_pi)
DEF_CDOUBLE_FUN_1(sin_pi, acb_sin_pi)

DEF_DOUBLE_FUN_1(cos_pi, arb_cos_pi)
DEF_CDOUBLE_FUN_1(cos_pi, acb_cos_pi)

DEF_DOUBLE_FUN_1(tan_pi, arb_tan_pi)
DEF_CDOUBLE_FUN_1(tan_pi, acb_tan_pi)

DEF_DOUBLE_FUN_1(cot_pi, arb_cot_pi)
DEF_CDOUBLE_FUN_1(cot_pi, acb_cot_pi)

DEF_DOUBLE_FUN_1(sinc_pi, arb_sinc_pi)
DEF_CDOUBLE_FUN_1(sinc_pi, acb_sinc_pi)

DEF_DOUBLE_FUN_1(asin, arb_asin)
DEF_CDOUBLE_FUN_1(asin, acb_asin)

DEF_DOUBLE_FUN_1(acos, arb_acos)
DEF_CDOUBLE_FUN_1(acos, acb_acos)

DEF_DOUBLE_FUN_1(atan, arb_atan)
DEF_CDOUBLE_FUN_1(atan, acb_atan)

DEF_DOUBLE_FUN_2(atan2, arb_atan2)

DEF_DOUBLE_FUN_1(asinh, arb_asinh)
DEF_CDOUBLE_FUN_1(asinh, acb_asinh)

DEF_DOUBLE_FUN_1(acosh, arb_acosh)
DEF_CDOUBLE_FUN_1(acosh, acb_acosh)

DEF_DOUBLE_FUN_1(atanh, arb_atanh)
DEF_CDOUBLE_FUN_1(atanh, acb_atanh)

DEF_DOUBLE_FUN_2(rising, arb_rising)
DEF_CDOUBLE_FUN_2(rising, acb_rising)

DEF_DOUBLE_FUN_1(gamma, arb_gamma)
DEF_CDOUBLE_FUN_1(gamma, acb_gamma)

DEF_DOUBLE_FUN_1(rgamma, arb_rgamma)
DEF_CDOUBLE_FUN_1(rgamma, acb_rgamma)

DEF_DOUBLE_FUN_1(lgamma, arb_lgamma)
DEF_CDOUBLE_FUN_1(lgamma, acb_lgamma)

DEF_DOUBLE_FUN_1(digamma, arb_digamma)
DEF_CDOUBLE_FUN_1(digamma, acb_digamma)

DEF_DOUBLE_FUN_1(zeta, arb_zeta)
DEF_CDOUBLE_FUN_1(zeta, acb_zeta)

DEF_DOUBLE_FUN_2(hurwitz_zeta, arb_hurwitz_zeta)
DEF_CDOUBLE_FUN_2(hurwitz_zeta, acb_hurwitz_zeta)

static void
_arb_polygamma(arb_t res, const arb_t s, const arb_t z, slong prec)
{
    acb_t t, u, v;
    acb_init(t);
    acb_init(u);
    acb_init(v);
    acb_set_arb(t, s);
    acb_set_arb(u, z);
    acb_polygamma(v, t, u, prec);
    if (acb_is_real(v))
        arb_set(res, acb_realref(v));
    else
        arb_indeterminate(res);
    acb_clear(t);
    acb_clear(u);
    acb_clear(v);
}

static void
_arb_barnes_g(arb_t res, const arb_t x, slong prec)
{
    acb_t t, u;
    acb_init(t);
    acb_init(u);
    acb_set_arb(t, x);
    acb_barnes_g(u, t, prec);
    arb_set(res, acb_realref(u));
    acb_clear(t);
    acb_clear(u);
}

static void
_arb_log_barnes_g(arb_t res, const arb_t x, slong prec)
{
    if (!arb_is_positive(x))
    {
        arb_indeterminate(res);
    }
    else
    {
        acb_t t, u;
        acb_init(t);
        acb_init(u);
        acb_set_arb(t, x);
        acb_log_barnes_g(u, t, prec);
        if (acb_is_real(u))
            arb_set(res, acb_realref(u));
        else
            arb_indeterminate(res);
        acb_clear(t);
        acb_clear(u);
    }
}

DEF_DOUBLE_FUN_1(barnes_g, _arb_barnes_g)
DEF_CDOUBLE_FUN_1(barnes_g, acb_barnes_g)

DEF_DOUBLE_FUN_1(log_barnes_g, _arb_log_barnes_g)
DEF_CDOUBLE_FUN_1(log_barnes_g, acb_log_barnes_g)

DEF_DOUBLE_FUN_2(polygamma, _arb_polygamma)
DEF_CDOUBLE_FUN_2(polygamma, acb_polygamma)

DEF_DOUBLE_FUN_2(polylog, arb_polylog)
DEF_CDOUBLE_FUN_2(polylog, acb_polylog)

DEF_DOUBLE_FUN_1(dilog, arb_hypgeom_dilog)
DEF_CDOUBLE_FUN_1(dilog, acb_hypgeom_dilog)


static void
_arb_agm1(arb_t res, const arb_t x, slong prec)
{
    arb_t t;
    arb_init(t);
    arb_one(t);
    arb_agm(res, t, x, prec);
    arb_clear(t);
}

DEF_DOUBLE_FUN_1(agm1, _arb_agm1)
DEF_CDOUBLE_FUN_1(agm1, acb_agm1)

DEF_DOUBLE_FUN_2(agm, arb_agm)
DEF_CDOUBLE_FUN_2(agm, acb_agm)


DEF_DOUBLE_FUN_1(erf, arb_hypgeom_erf)
DEF_CDOUBLE_FUN_1(erf, acb_hypgeom_erf)

DEF_DOUBLE_FUN_1(erfc, arb_hypgeom_erfc)
DEF_CDOUBLE_FUN_1(erfc, acb_hypgeom_erfc)

DEF_DOUBLE_FUN_1(erfi, arb_hypgeom_erfi)
DEF_CDOUBLE_FUN_1(erfi, acb_hypgeom_erfi)

DEF_DOUBLE_FUN_2(bessel_j, arb_hypgeom_bessel_j)
DEF_CDOUBLE_FUN_2(bessel_j, acb_hypgeom_bessel_j)

DEF_DOUBLE_FUN_2(bessel_y, arb_hypgeom_bessel_y)
DEF_CDOUBLE_FUN_2(bessel_y, acb_hypgeom_bessel_y)

DEF_DOUBLE_FUN_2(bessel_i, arb_hypgeom_bessel_i)
DEF_CDOUBLE_FUN_2(bessel_i, acb_hypgeom_bessel_i)

DEF_DOUBLE_FUN_2(bessel_k, arb_hypgeom_bessel_k)
DEF_CDOUBLE_FUN_2(bessel_k, acb_hypgeom_bessel_k)

DEF_DOUBLE_FUN_2(bessel_k_scaled, arb_hypgeom_bessel_k_scaled)
DEF_CDOUBLE_FUN_2(bessel_k_scaled, acb_hypgeom_bessel_k_scaled)

static void _arb_hypgeom_airy_ai(arb_t res, const arb_t x, slong prec) { arb_hypgeom_airy(res, NULL, NULL, NULL, x, prec); }
static void _arb_hypgeom_airy_ai_prime(arb_t res, const arb_t x, slong prec) { arb_hypgeom_airy(NULL, res, NULL, NULL, x, prec); }
static void _arb_hypgeom_airy_bi(arb_t res, const arb_t x, slong prec) { arb_hypgeom_airy(NULL, NULL, res, NULL, x, prec); }
static void _arb_hypgeom_airy_bi_prime(arb_t res, const arb_t x, slong prec) { arb_hypgeom_airy(NULL, NULL, NULL, res, x, prec); }

static void _acb_hypgeom_airy_ai(acb_t res, const acb_t x, slong prec) { acb_hypgeom_airy(res, NULL, NULL, NULL, x, prec); }
static void _acb_hypgeom_airy_ai_prime(acb_t res, const acb_t x, slong prec) { acb_hypgeom_airy(NULL, res, NULL, NULL, x, prec); }
static void _acb_hypgeom_airy_bi(acb_t res, const acb_t x, slong prec) { acb_hypgeom_airy(NULL, NULL, res, NULL, x, prec); }
static void _acb_hypgeom_airy_bi_prime(acb_t res, const acb_t x, slong prec) { acb_hypgeom_airy(NULL, NULL, NULL, res, x, prec); }

DEF_DOUBLE_FUN_1(airy_ai, _arb_hypgeom_airy_ai)
DEF_CDOUBLE_FUN_1(airy_ai, _acb_hypgeom_airy_ai)

DEF_DOUBLE_FUN_1(airy_ai_prime, _arb_hypgeom_airy_ai_prime)
DEF_CDOUBLE_FUN_1(airy_ai_prime, _acb_hypgeom_airy_ai_prime)

DEF_DOUBLE_FUN_1(airy_bi, _arb_hypgeom_airy_bi)
DEF_CDOUBLE_FUN_1(airy_bi, _acb_hypgeom_airy_bi)

DEF_DOUBLE_FUN_1(airy_bi_prime, _arb_hypgeom_airy_bi_prime)
DEF_CDOUBLE_FUN_1(airy_bi_prime, _acb_hypgeom_airy_bi_prime)

static void _arb_hypgeom_coulomb_f(arb_t res, const arb_t l, const arb_t eta, const arb_t z, slong prec) { arb_hypgeom_coulomb(res, NULL, l, eta, z, prec); }
static void _arb_hypgeom_coulomb_g(arb_t res, const arb_t l, const arb_t eta, const arb_t z, slong prec) { arb_hypgeom_coulomb(NULL, res, l, eta, z, prec); }

static void _acb_hypgeom_coulomb_f(acb_t res, const acb_t l, const acb_t eta, const acb_t z, slong prec) { acb_hypgeom_coulomb(res, NULL, NULL, NULL, l, eta, z, prec); }
static void _acb_hypgeom_coulomb_g(acb_t res, const acb_t l, const acb_t eta, const acb_t z, slong prec) { acb_hypgeom_coulomb(NULL, res, NULL, NULL, l, eta, z, prec); }
static void _acb_hypgeom_coulomb_hpos(acb_t res, const acb_t l, const acb_t eta, const acb_t z, slong prec) { acb_hypgeom_coulomb(NULL, NULL, res, NULL, l, eta, z, prec); }
static void _acb_hypgeom_coulomb_hneg(acb_t res, const acb_t l, const acb_t eta, const acb_t z, slong prec) { acb_hypgeom_coulomb(NULL, NULL, NULL, res, l, eta, z, prec); }

DEF_DOUBLE_FUN_3(coulomb_f, _arb_hypgeom_coulomb_f)
DEF_CDOUBLE_FUN_3(coulomb_f, _acb_hypgeom_coulomb_f)

DEF_DOUBLE_FUN_3(coulomb_g, _arb_hypgeom_coulomb_g)
DEF_CDOUBLE_FUN_3(coulomb_g, _acb_hypgeom_coulomb_g)

DEF_CDOUBLE_FUN_3(coulomb_hpos, _acb_hypgeom_coulomb_hpos)
DEF_CDOUBLE_FUN_3(coulomb_hneg, _acb_hypgeom_coulomb_hneg)

DEF_DOUBLE_FUN_2(chebyshev_t, arb_hypgeom_chebyshev_t)
DEF_CDOUBLE_FUN_2(chebyshev_t, acb_hypgeom_chebyshev_t)

DEF_DOUBLE_FUN_2(chebyshev_u, arb_hypgeom_chebyshev_u)
DEF_CDOUBLE_FUN_2(chebyshev_u, acb_hypgeom_chebyshev_u)

DEF_DOUBLE_FUN_4(jacobi_p, arb_hypgeom_jacobi_p)
DEF_CDOUBLE_FUN_4(jacobi_p, acb_hypgeom_jacobi_p)

DEF_DOUBLE_FUN_3(gegenbauer_c, arb_hypgeom_gegenbauer_c)
DEF_CDOUBLE_FUN_3(gegenbauer_c, acb_hypgeom_gegenbauer_c)

DEF_DOUBLE_FUN_3(laguerre_l, arb_hypgeom_laguerre_l)
DEF_CDOUBLE_FUN_3(laguerre_l, acb_hypgeom_laguerre_l)

DEF_DOUBLE_FUN_2(hermite_h, arb_hypgeom_hermite_h)
DEF_CDOUBLE_FUN_2(hermite_h, acb_hypgeom_hermite_h)



/* todo: lambertw (with branches) */
/* todo: fresnel (with flags) */
/* todo: functions with multiple outputs */
/* todo: incomplete gamma, exp integrals... */
/* todo: airy zeros */
/* legendre_p, legendre_q, +roots, spherharm */
/* todo: pfqs */

