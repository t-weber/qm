/**
 * basic data types
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_TYPES_H__
#define __QM_TYPES_H__

#include "lib/math_algos.h"
#include "lib/math_conts.h"

// primitive types
using t_int = int;
using t_uint = unsigned int;
using t_real = double;

// complex type
using t_cplx = std::complex<t_real>;

// vector and matrix type
using t_vec = m::vec<t_cplx, std::vector>;
using t_mat = m::mat<t_cplx, std::vector>;

#endif
