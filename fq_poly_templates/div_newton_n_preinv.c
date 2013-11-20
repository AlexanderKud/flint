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

    Copyright (C) 2011 William Hart
    Copyright (C) 2011 Sebastian Pancratz
    Copyright (C) 2013 Martin Lee
    Copyright (C) 2013 Mike Hansen

******************************************************************************/


#ifdef T

#include "templates.h"

void
_TEMPLATE(T, poly_div_newton_n_preinv)(TEMPLATE(T, struct) * Q, const TEMPLATE(T, struct) * A, slong lenA,
                           const TEMPLATE(T, struct) * B, slong lenB,
                           const TEMPLATE(T, struct) * Binv, slong lenBinv,
                           const TEMPLATE(T, ctx_t) ctx)
{
    const slong lenQ = lenA - lenB + 1;
    TEMPLATE(T, struct) *Arev;

    Arev = _TEMPLATE(T, vec_init)(lenQ, ctx);

    _TEMPLATE(T, poly_reverse)(Arev, A + (lenA - lenQ), lenQ, lenQ, ctx);

    _TEMPLATE(T, poly_mullow)(Q, Arev, lenQ, Binv, FLINT_MIN(lenQ, lenBinv), lenQ, ctx);

    _TEMPLATE(T, poly_reverse)(Q, Q, lenQ, lenQ, ctx);

    _TEMPLATE(T, vec_clear)(Arev, lenQ, ctx);
}

void
TEMPLATE(T, poly_div_newton_n_preinv)(TEMPLATE(T, poly_t) Q, const TEMPLATE(T, poly_t) A,
                          const TEMPLATE(T, poly_t) B, const TEMPLATE(T, poly_t) Binv,
                          const TEMPLATE(T, ctx_t) ctx)
{
    const slong lenA = A->length,
        lenB = B->length, lenQ = lenA - lenB + 1, lenBinv = Binv->length;

    TEMPLATE(T, struct) *q;

    if (lenB == 0)
    {
        flint_printf("Exception (fq_poly_div_newton). Division by zero.\n");
        abort();
    }

    if (lenA < lenB)
    {
        TEMPLATE(T, poly_zero)(Q, ctx);
        return;
    }

    if (Q == A || Q == B || Q == Binv)
    {
        q = _TEMPLATE(T, vec_init)(lenQ, ctx);
    }
    else
    {
        TEMPLATE(T, poly_fit_length)(Q, lenQ, ctx);
        q = Q->coeffs;
    }

    _TEMPLATE(T, poly_div_newton_n_preinv)(q, A->coeffs, lenA, B->coeffs, lenB,
                               Binv->coeffs, lenBinv, ctx);

    if (Q == A || Q == B || Q == Binv)
    {
        flint_free(Q->coeffs);
        Q->coeffs = q;
        Q->alloc = lenQ;
    }
    Q->length = lenQ;
}


#endif
