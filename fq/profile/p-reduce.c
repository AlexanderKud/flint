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
#include "fq.h"
#include <stdio.h>
#include "profiler.h"

int
main(int argc, char** argv)
{
    flint_rand_t state;

    slong i, result;
    fmpz_t p;
    slong d;
    fq_ctx_t ctx;
    fq_t a,b,c;
    double dense;

    flint_randinit(state);

    fmpz_init(p);
    fmpz_set_str(p, argv[1], 10);

    d = atoi(argv[2]);
    
    fq_ctx_init(ctx,p,d,"a");

    fq_init(a, ctx);
    fq_init(b, ctx);
    fq_init(c, ctx);

    fq_randtest_not_zero(a,state,ctx);
    fq_randtest_not_zero(b,state,ctx);

    fmpz_poly_mul(c, a, b);
    


    init_clock(0);

    prof_start();
    for (i=0;  i < 1000; i++)
    {
        fmpz_poly_set(a, c);
        _fq_dense_reduce(a->coeffs, a->length, ctx);
    }
    prof_stop();
    
    dense = get_clock(0);

    init_clock(0);
    prof_start();
    for (i = 0; i < 1000; i++)
    {
        fmpz_poly_set(b, c);
        _fq_sparse_reduce(b->coeffs, b->length, ctx);
    }
    prof_stop();

    if (get_clock(0) <= dense)
    {
        result = 1;
    }
    else
    {
        result = 0;
    }
      
    
    fmpz_print(p);
    flint_printf(" %d %d %d\n", ctx->len, ctx->modulus->length, result);

    fq_clear(a, ctx);
    fq_clear(b, ctx);
    fq_clear(c, ctx);
    fq_ctx_clear(ctx);
    fmpz_clear(p);
    return 0;
}
