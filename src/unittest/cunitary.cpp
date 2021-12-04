/**
 * controlled unitary gate test
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date 4-dec-2021
 * @license see 'LICENSE' file
 *
 * References:
 *  * https://www.boost.org/doc/libs/1_76_0/libs/test/doc/html/index.html
 *
 * g++ -std=c++20 -I.. -Wall -Wextra -Weffc++ -o cunitary cunitary.cpp
 */

#define BOOST_TEST_MODULE test_cunitary

#include <complex>
#include <vector>
#include <tuple>

#include <boost/test/included/unit_test.hpp>
#include <boost/type_index.hpp>
namespace test = boost::unit_test;
namespace ty = boost::typeindex;

#include "lib/math_algos.h"
#include "lib/math_conts.h"
#include "lib/qm_algos.h"

template<class t_real> using t_cplx = std::complex<t_real>;
template<class t_real> using t_vec = m::vec<t_cplx<t_real>, std::vector>;
template<class t_real> using t_mat = m::mat<t_cplx<t_real>, std::vector>;


BOOST_AUTO_TEST_CASE_TEMPLATE(cunitary, t_real, decltype(std::tuple<float, double, long double>{}))
{
	using namespace m_ops;

	std::cout << "--------------------------------------------------------------------------------" << std::endl;
	std::cout << "Testing CUnitary with " << ty::type_id_with_cvr<t_real>().pretty_name() << " type." << std::endl;
	std::cout << "--------------------------------------------------------------------------------" << std::endl;

	t_real eps = std::sqrt(std::numeric_limits<t_real>::epsilon());

	t_mat<t_real> X = m::su2_matrix<t_mat<t_real>>(0);
	t_mat<t_real> Y = m::su2_matrix<t_mat<t_real>>(1);
	t_mat<t_real> Z = m::su2_matrix<t_mat<t_real>>(2);

	t_mat<t_real> cu_cnot = cunitary_nqbits<t_mat<t_real>>(X, 2, 0, 1, true);
	t_mat<t_real> cnot = m::cnot_nqbits<t_mat<t_real>>(2, 0, 1);
	std::cout << "CUNITARY(X) = " << cu_cnot << std::endl;
	std::cout << "CX          = " << cnot << std::endl;

	t_mat<t_real> cu_cnot_inv = cunitary_nqbits<t_mat<t_real>>(X, 2, 1, 0, true);
	t_mat<t_real> cnot_inv = m::cnot_nqbits<t_mat<t_real>>(2, 1, 0);

	t_mat<t_real> cu_cz = cunitary_nqbits<t_mat<t_real>>(Z, 2, 0, 1, true);
	t_mat<t_real> cz = m::cz_nqbits<t_mat<t_real>>(2, 0, 1);
	std::cout << "CUNITARY(Z) = " << cu_cz << std::endl;
	std::cout << "CZ          = " << cz << std::endl;

	t_mat<t_real> cu_cz_inv = cunitary_nqbits<t_mat<t_real>>(Z, 2, 1, 0, true);
	t_mat<t_real> cz_inv = m::cz_nqbits<t_mat<t_real>>(2, 1, 0);

	BOOST_TEST((m::equals<t_mat<t_real>>(cu_cnot, cnot, eps)));
	BOOST_TEST((m::equals<t_mat<t_real>>(cu_cnot_inv, cnot_inv, eps)));
	BOOST_TEST((m::equals<t_mat<t_real>>(cu_cz, cz, eps)));
	BOOST_TEST((m::equals<t_mat<t_real>>(cu_cz_inv, cz_inv, eps)));

	std::cout << "--------------------------------------------------------------------------------" << std::endl;
	std::cout << std::endl;
}
