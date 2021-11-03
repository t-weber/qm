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

using t_real = double;
using t_cplx = std::complex<t_real>;
using t_vec = m::vec<t_cplx, std::vector>;
using t_mat = m::mat<t_cplx, std::vector>;

#endif
