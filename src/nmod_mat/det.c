/*
    Copyright (C) 2010 Fredrik Johansson

    This file is part of FLINT.

    FLINT is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.  See <https://www.gnu.org/licenses/>.
*/

#include "nmod.h"
#include "nmod_mat.h"
#include "nmod_poly.h"
#include "perm.h"

static ulong
_nmod_mat_det_2x2(ulong a, ulong b, ulong c, ulong d, nmod_t mod)
{
    b = nmod_neg(b, mod);
    return nmod_addmul(nmod_mul(a, d, mod), b, c, mod);
}

static ulong
_nmod_mat_det_3x3(ulong a, ulong b, ulong c,
                  ulong d, ulong e, ulong f,
                  ulong g, ulong h, ulong i, nmod_t mod)
{
    ulong s, t, u;

    s = _nmod_mat_det_2x2(e, f, h, i, mod);
    t = _nmod_mat_det_2x2(g, i, d, f, mod);
    u = _nmod_mat_det_2x2(d, e, g, h, mod);

    s = nmod_mul(a, s, mod);
    s = nmod_addmul(s, b, t, mod);
    s = nmod_addmul(s, c, u, mod);

    return s;
}

static ulong
_nmod_mat_det_4x4(const ulong * mm, slong stride, nmod_t mod)
{
    ulong s, t, u, v;

#define mat(ii,jj) mm[(ii) * stride + (jj)]

    s = _nmod_mat_det_3x3(mat(1,1), mat(1,2), mat(1,3),
                          mat(2,1), mat(2,2), mat(2,3),
                          mat(3,1), mat(3,2), mat(3,3), mod);

    t = _nmod_mat_det_3x3(mat(1,0), mat(1,2), mat(1,3),
                          mat(2,0), mat(2,2), mat(2,3),
                          mat(3,0), mat(3,2), mat(3,3), mod);

    u = _nmod_mat_det_3x3(mat(1,0), mat(1,1), mat(1,3),
                          mat(2,0), mat(2,1), mat(2,3),
                          mat(3,0), mat(3,1), mat(3,3), mod);

    v = _nmod_mat_det_3x3(mat(1,0), mat(1,1), mat(1,2),
                          mat(2,0), mat(2,1), mat(2,2),
                          mat(3,0), mat(3,1), mat(3,2), mod);

    t = nmod_neg(t, mod);
    v = nmod_neg(v, mod);

    s = nmod_mul(mat(0,0), s, mod);
    s = nmod_addmul(s, mat(0,1), t, mod);
    s = nmod_addmul(s, mat(0,2), u, mod);
    s = nmod_addmul(s, mat(0,3), v, mod);

#undef mat

    return s;
}

ulong
_nmod_mat_det(nmod_mat_t A)
{
    ulong det;
    slong * P;

    slong m = A->r;
    slong rank;
    slong i;

    P = flint_malloc(sizeof(slong) * m);
    rank = nmod_mat_lu(P, A, 1);

    det = UWORD(0);

    if (rank == m)
    {
        det = UWORD(1);
        for (i = 0; i < m; i++)
            det = n_mulmod2_preinv(det, nmod_mat_entry(A, i, i),
                A->mod.n, A->mod.ninv);
    }

    if (_perm_parity(P, m) == 1)
        det = nmod_neg(det, A->mod);

    flint_free(P);
    return det;
}

ulong
nmod_mat_det(const nmod_mat_t A)
{
    nmod_mat_t tmp;
    ulong det;
    slong dim = A->r;

    if (dim != A->c)
    {
        flint_throw(FLINT_ERROR, "Exception (nmod_mat_det). Non-square matrix.\n");
    }

    if (dim == 0) return A->mod.n != 1;

    if (dim == 1) return nmod_mat_entry(A, 0, 0);

    if (dim == 2) return _nmod_mat_det_2x2(
        nmod_mat_entry(A, 0, 0), nmod_mat_entry(A, 0, 1),
        nmod_mat_entry(A, 1, 0), nmod_mat_entry(A, 1, 1), A->mod);

    if (dim == 3) return _nmod_mat_det_3x3(
        nmod_mat_entry(A, 0, 0), nmod_mat_entry(A, 0, 1), nmod_mat_entry(A, 0, 2),
        nmod_mat_entry(A, 1, 0), nmod_mat_entry(A, 1, 1), nmod_mat_entry(A, 1, 2),
        nmod_mat_entry(A, 2, 0), nmod_mat_entry(A, 2, 1), nmod_mat_entry(A, 2, 2), A->mod);

    if (dim == 4) return _nmod_mat_det_4x4(A->entries, A->stride, A->mod);

    if (dim <= 8)
    {
        ulong cp[9];
        _nmod_mat_charpoly_berkowitz(cp, A, A->mod);
        if (dim % 2)
            return nmod_neg(cp[0], A->mod);
        else
            return cp[0];
    }

    nmod_mat_init_set(tmp, A);
    if (n_is_prime(A->mod.n))
       det = _nmod_mat_det(tmp);
    else
       det = _nmod_mat_det_howell(tmp);
    nmod_mat_clear(tmp);

    return det;
}
