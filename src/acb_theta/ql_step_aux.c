/*
    Copyright (C) 2023 Jean Kieffer

    This file is part of Arb.

    Arb is free software: you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.  See <http://www.gnu.org/licenses/>.
*/

#include "acb_theta.h"

void
acb_theta_ql_step_aux(acb_ptr r, acb_srcptr th, acb_srcptr th0,
    acb_srcptr roots, arb_srcptr dist, slong g, slong prec)
{
    slong n = 1 << g;
    acb_ptr res;
    ulong a;

    res = _acb_vec_init(3 * n);
    
    flint_printf("(ql_step) input:");
    _acb_vec_printd(th, 3 * n, 10);
    flint_printf("\n");

    /* Be more careful with precisions */

    /* Duplication using square roots for z + t and z + 2t */
    acb_theta_agm_mul(res + n, th0, th + n, g, prec);
    acb_theta_agm_mul(res + 2 * n, th0, th + 2 * n, g, prec);
    _acb_vec_scalar_mul_2exp_si(res + n, res + n, 2 * n, g);
    acb_theta_agm_sqrt(res + n, res + n, roots, 2 * n, prec);

    /* Duplication using divisions for z */
    acb_theta_agm_mul(res, th + n, th0 + n, g, prec);
    _acb_vec_scalar_mul_2exp_si(res, res, n, g);
    for (a = 0; a < n; a++)
    {
        acb_div(&res[a], &res[a], &res[2 * n + a], prec);
    }

    _acb_vec_set(r, res, 3 * n);
    _acb_vec_clear(res, 3 * n);
}
