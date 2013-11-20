/*=============================================================================

    This file is part of FLINT.

    FLINT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    FLINT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FLINT; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

=============================================================================*/
/******************************************************************************

    Copyright (C) 2013 Mike Hansen

******************************************************************************/


#ifdef T

#include "templates.h"


int
main(void)
{
    int i, result;
    flint_rand_t state;
    flint_randinit(state);

    flint_printf("powmod_x_fmpz_preinv...");
    fflush(stdout);

    /* Aliasing of res and f */
    for (i = 0; i < 25; i++)
    {
        TEMPLATE(T, ctx_t) ctx;
        TEMPLATE(T, poly_t) res, t, f, finv;
        ulong exp;
        fmpz_t expz;

        TEMPLATE(T, ctx_randtest)(ctx, state);

        exp = n_randint(state, 50);
        fmpz_init_set_ui(expz, exp);

        TEMPLATE(T, poly_init)(f, ctx);
        TEMPLATE(T, poly_init)(finv, ctx);
        TEMPLATE(T, poly_init)(res, ctx);
        TEMPLATE(T, poly_init)(t, ctx);

        TEMPLATE(T, poly_randtest_not_zero)(f, state, n_randint(state, 50) + 1, ctx);

        TEMPLATE(T, poly_reverse)(finv, f, f->length, ctx);
        TEMPLATE(T, poly_inv_series_newton)(finv, finv, f->length, ctx);

        TEMPLATE(T, poly_powmod_x_fmpz_preinv)(res, expz, f, finv, ctx);
        TEMPLATE(T, poly_powmod_x_fmpz_preinv)(f, expz, f, finv, ctx);

        result = (TEMPLATE(T, poly_equal)(res, f, ctx));
        if (!result)
        {
            flint_printf("FAIL:\n");
            flint_printf("f:\n"); TEMPLATE(T, poly_print)(f, ctx), flint_printf("\n\n");
            flint_printf("res:\n"); TEMPLATE(T, poly_print)(res, ctx), flint_printf("\n\n");
            abort();
        }

        TEMPLATE(T, poly_clear)(f, ctx);
        TEMPLATE(T, poly_clear)(finv, ctx);
        TEMPLATE(T, poly_clear)(res, ctx);
        TEMPLATE(T, poly_clear)(t, ctx);
        fmpz_clear(expz);

        TEMPLATE(T, ctx_clear)(ctx);
    }

    /* No aliasing -- compare with binexp */
    for (i = 0; i < 100; i++)
    {
        TEMPLATE(T, ctx_t) ctx;
        TEMPLATE(T, poly_t) a, res1, res2, t, f, finv;
        ulong exp;
        fmpz_t expz;

        TEMPLATE(T, ctx_randtest)(ctx, state);

        exp = n_randint(state, 50);

        TEMPLATE(T, poly_init)(a, ctx);
        TEMPLATE(T, poly_init)(f, ctx);
        TEMPLATE(T, poly_init)(finv, ctx);
        TEMPLATE(T, poly_init)(res1, ctx);
        TEMPLATE(T, poly_init)(res2, ctx);
        TEMPLATE(T, poly_init)(t, ctx);

        TEMPLATE(T, poly_gen)(a, ctx);
        TEMPLATE(T, poly_randtest_not_zero)(f, state, n_randint(state, 50) + 1, ctx);
        fmpz_init_set_ui(expz, exp);

        TEMPLATE(T, poly_reverse)(finv, f, f->length, ctx);
        TEMPLATE(T, poly_inv_series_newton)(finv, finv, f->length, ctx);

        TEMPLATE(T, poly_powmod_fmpz_binexp_preinv)(res1, a, expz, f, finv, ctx);
        TEMPLATE(T, poly_powmod_x_fmpz_preinv)(res2, expz, f, finv, ctx);

        result = (TEMPLATE(T, poly_equal)(res1, res2, ctx));
        if (!result)
        {
            flint_printf("FAIL:\n");
            flint_printf("a:\n"); TEMPLATE(T, poly_print)(a, ctx), flint_printf("\n\n");
            flint_printf("f:\n"); TEMPLATE(T, poly_print)(f, ctx), flint_printf("\n\n");
            flint_printf("res1:\n"); TEMPLATE(T, poly_print)(res1, ctx), flint_printf("\n\n");
            flint_printf("res2:\n"); TEMPLATE(T, poly_print)(res2, ctx), flint_printf("\n\n");
            abort();
        }

        TEMPLATE(T, poly_clear)(a, ctx);
        TEMPLATE(T, poly_clear)(f, ctx);
        TEMPLATE(T, poly_clear)(finv, ctx);
        TEMPLATE(T, poly_clear)(res1, ctx);
        TEMPLATE(T, poly_clear)(res2, ctx);
        TEMPLATE(T, poly_clear)(t, ctx);
        fmpz_clear(expz);

        TEMPLATE(T, ctx_clear)(ctx);
    }

    flint_randclear(state);
    flint_cleanup();
    flint_printf("PASS\n");
    return 0;
}

#endif
