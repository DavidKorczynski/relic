/*
 * RELIC is an Efficient LIbrary for Cryptography
 * Copyright (c) 2009 RELIC Authors
 *
 * This file is part of RELIC. RELIC is legal property of its developers,
 * whose names are not listed here. Please refer to the COPYRIGHT file
 * for contact information.
 *
 * RELIC is free software; you can redistribute it and/or modify it under the
 * terms of the version 2.1 (or later) of the GNU Lesser General Public License
 * as published by the Free Software Foundation; or version 2.0 of the Apache
 * License as published by the Apache Software Foundation. See the LICENSE files
 * for more details.
 *
 * RELIC is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the LICENSE files for more details.
 *
 * You should have received a copy of the GNU Lesser General Public or the
 * Apache License along with RELIC. If not, see <https://www.gnu.org/licenses/>
 * or <https://www.apache.org/licenses/>.
 */

/**
 * @file
 *
 * Implementation of the multiple precision integer arithmetic multiplication
 * functions.
 *
 * @ingroup bn
 */

#include "relic_bn.h"
#include "relic_bn_low.h"

/*============================================================================*/
/* Private definitions                                                        */
/*============================================================================*/

/**
 * Computes the step of a Comba squaring.
 *
 * @param[in,out] R2		- most significant word of the triple register.
 * @param[in,out] R1		- middle word of the triple register.
 * @param[in,out] R0		- lowest significant word of the triple register.
 * @param[in] A				- the first digit to multiply.
 * @param[in] B				- the second digit to multiply.
 */
#define COMBA_STEP_SQR(R2, R1, R0, A, B)									\
	dig_t _r0, _r1;															\
	RLC_MUL_DIG(_r1, _r0, A, B);											\
	dig_t _s0 = _r0 + _r0;													\
	dig_t _s1 = _r1 + _r1 + (_s0 < _r0);									\
	dig_t _r = (R1);														\
	(R0) += _s0;															\
	(R1) += (R0) < _s0;														\
	(R2) += (R1) < _r;														\
	(R1) += _s1;															\
	(R2) += (R1) < _s1;														\
	(R2) += (_s1 < _r1);													\

/**
 * Computes the step of a Comba squaring when the loop length is odd.
 *
 * @param[in,out] R2		- most significant word of the triple register.
 * @param[in,out] R1		- middle word of the triple register.
 * @param[in,out] R0		- lowest significant word of the triple register.
 * @param[in] A				- the first digit to multiply.
 */
#define COMBA_FINAL(R2, R1, R0, A)											\
	dig_t _r0, _r1;															\
	RLC_MUL_DIG(_r1, _r0, A, A);											\
	dig_t _r = (R1);														\
	(R0) += _r0;															\
	(R1) += (R0) < _r0;														\
	(R2) += (R1) < _r;														\
	(R1) += _r1;															\
	(R2) += (R1) < _r1;														\

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

#include <assert.h>
dig_t bn_sqra_low(dig_t *c, const dig_t *a, int size) {
	int i;
	dig_t t, c0, c1, _r0, _r1, r0, r1, s0, s1, t0, t1;

	/* Accumulate this column with the square of a->dp[i]. */
	t = a[0];
	RLC_MUL_DIG(_r1, _r0, t, t);
	r0 = _r0 + c[0];
	r1 = _r1 + (r0 < _r0);
	c[0] = r0;

	/* Update the carry. */
	c0 = r1;
	c1 = 0;

	for (i = 1; i < size; i++) {
		RLC_MUL_DIG(_r1, _r0, t, a[i]);
		r0 = _r0 + _r0;
		r1 = _r1 + _r1 + (r0 < _r0);

		s0 = r0 + c0;
		s1 = r1 + (s0 < r0);

		t0 = s0 + c[i];
		t1 = s1 + (t0 < s0);
		c[i] = t0;

		/* Accumulate the old delayed carry. */
		c0 = t1 + c1;
		/* Compute the new delayed carry. */
		c1 = (t1 < s1) || (s1 < r1) || (r1 < _r1) || (c0 < c1);
	}
	c[size] += c0;
	c1 += (c[size] < c0);
	return c1;
}

void bn_sqrn_low(dig_t *c, const dig_t *a, int size) {
	int i, j;
	const dig_t *tmpa, *tmpb;
	dig_t r0, r1, r2;

	/* Zero the accumulator. */
	r0 = r1 = r2 = 0;

	/* Comba squaring produces one column of the result per iteration. */
	for (i = 0; i < size; i++, c++) {
		tmpa = a;
		tmpb = a + i;

		/* Compute the number of additions in this column. */
		j = (i + 1);
		for (j = 0; j < (i + 1) / 2; j++, tmpa++, tmpb--) {
			COMBA_STEP_SQR(r2, r1, r0, *tmpa, *tmpb);
		}
		if (!(i & 0x01)) {
			COMBA_FINAL(r2, r1, r0, *tmpa);
		}
		*c = r0;
		r0 = r1;
		r1 = r2;
		r2 = 0;
	}
	for (i = 0; i < size; i++, c++) {
		tmpa = a + (i + 1);
		tmpb = a + (size - 1);

		/* Compute the number of additions in this column. */
		for (j = 0; j < (size - 1 - i) / 2; j++, tmpa++, tmpb--) {
			COMBA_STEP_SQR(r2, r1, r0, *tmpa, *tmpb);
		}
		if (!((size - i) & 0x01)) {
			COMBA_FINAL(r2, r1, r0, *tmpa);
		}
		*c = r0;
		r0 = r1;
		r1 = r2;
		r2 = 0;
	}
}
