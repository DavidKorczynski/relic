/*
 * RELIC is an Efficient LIbrary for Cryptography
 * Copyright (c) 2012 RELIC Authors
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
 * Implementation of x86-dependent routines.
 *
 * @ingroup arch
 */

#include <stdio.h>

#include "relic_types.h"
#include "relic_arch.h"
#include "relic_core.h"

#include "lzcnt.inc"

/*============================================================================*/
/* Public definitions                                                         */
/*============================================================================*/

void arch_init(void) {
	core_get()->lzcnt_ptr = (has_lzcnt_hard() ? lzcnt32_hard : lzcnt32_soft);
}

void arch_clean(void) {
	core_get()->lzcnt_ptr = NULL;
}

ull_t arch_cycles(void) {
	uint_t hi, lo;
	asm (
		"cpuid\n\t"/*serialize*/
		"rdtsc\n\t"/*read the clock*/
		"mov %%edx, %0\n\t"
		"mov %%eax, %1\n\t"
		: "=r" (hi), "=r" (lo):: "%rax", "%rbx", "%rcx", "%rdx"
	);
	return ((ull_t) lo) | (((ull_t) hi) << 32);
}

uint_t arch_lzcnt(dig_t x) {
	return core_get()->lzcnt_ptr((uint32_t)x) - (8 * sizeof(uint32_t) - WSIZE);
}
