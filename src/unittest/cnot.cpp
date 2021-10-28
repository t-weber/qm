/**
 * cnot gate test
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date 23-oct-2021
 * @license see 'LICENSE' file
 *
 * References:
 *  * https://www.boost.org/doc/libs/1_76_0/libs/test/doc/html/index.html
 *
 * g++ -std=c++20 -I.. -Wall -Wextra -Weffc++ -o cnot cnot.cpp
 */

#define BOOST_TEST_MODULE test_cnot

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


BOOST_AUTO_TEST_CASE_TEMPLATE(cnot, t_real, decltype(std::tuple<float, double, long double>{}))
{
	using namespace m_ops;

	std::cout << "--------------------------------------------------------------------------------" << std::endl;
	std::cout << "Testing CNOT with " << ty::type_id_with_cvr<t_real>().pretty_name() << " type." << std::endl;
	std::cout << "--------------------------------------------------------------------------------" << std::endl;

	t_real eps = std::sqrt(std::numeric_limits<t_real>::epsilon());
	const t_mat<t_real> I = m::unit<t_mat<t_real>>(2);
	const t_mat<t_real>& H = m::hadamard<t_mat<t_real>>();
	t_vec<t_real> down = m::create<t_vec<t_real>>({ 1, 0 });
	t_vec<t_real> up = m::create<t_vec<t_real>>({ 0, 1 });
	t_vec<t_real> upup = m::outer_flat<t_vec<t_real>, t_mat<t_real>>(up, up);
	t_vec<t_real> downdown = m::outer_flat<t_vec<t_real>, t_mat<t_real>>(down, down);
	t_vec<t_real> downup = m::outer_flat<t_vec<t_real>, t_mat<t_real>>(down, up);
	t_vec<t_real> updown = m::outer_flat<t_vec<t_real>, t_mat<t_real>>(up, down);

	t_mat<t_real> X = m::su2_matrix<t_mat<t_real>>(0);
	t_mat<t_real> Y = m::su2_matrix<t_mat<t_real>>(1);
	t_mat<t_real> Z = m::su2_matrix<t_mat<t_real>>(2);
	t_mat<t_real> C1 = m::cnot<t_mat<t_real>>(false);
	t_mat<t_real> C2 = m::cnot<t_mat<t_real>>(true);
	t_mat<t_real> C1b = m::cnot_nqbits<t_mat<t_real>>(2, 0, 1);
	t_mat<t_real> C2b = m::cnot_nqbits<t_mat<t_real>>(2, 1, 0);
	t_mat<t_real> I4 = m::unit<t_mat<t_real>>(4);


	// cnot
	std::cout << "CNOT           = " << C1 << std::endl;
	std::cout << "CNOT (flipped) = " << C2 << std::endl;

	BOOST_TEST((m::equals<t_mat<t_real>>(C1, C1b, eps)));
	BOOST_TEST((m::equals<t_mat<t_real>>(C2, C2b, eps)));


	// cnot in a 3-qbit circuit
	t_mat<t_real> C3 = m::outer<t_mat<t_real>>(C1, I);
	t_mat<t_real> C4 = m::outer<t_mat<t_real>>(I, C1);
	t_mat<t_real> C3b = m::cnot_nqbits<t_mat<t_real>>(3, 0, 1);
	t_mat<t_real> C4b = m::cnot_nqbits<t_mat<t_real>>(3, 1, 2);
	BOOST_TEST((m::equals<t_mat<t_real>>(C3, C3b, eps)));
	BOOST_TEST((m::equals<t_mat<t_real>>(C4, C4b, eps)));


	t_mat<t_real> circ1_op = m::two_qbit_total_op<t_mat<t_real>>(Y, X, C1, I4, I4, X, Y);
	std::cout << "\ncircuit total operator: " << circ1_op << std::endl;


	// see: https://en.wikipedia.org/wiki/Controlled_NOT_gate
	t_mat<t_real> cnot_flipped_op = m::two_qbit_total_op<t_mat<t_real>>(H, H, I4, C1, I4, H, H);
	BOOST_TEST((m::equals<t_mat<t_real>>(cnot_flipped_op, C2, eps)));


	t_mat<t_real> U1 = m::cunitary<t_mat<t_real>>(Y, 0);
	t_mat<t_real> U2 = m::cunitary<t_mat<t_real>>(Y, 1);
	t_mat<t_real> U3 = m::cunitary<t_mat<t_real>>(X, 1);
	t_mat<t_real> cunitary_flipped_op = m::two_qbit_total_op<t_mat<t_real>>(H, H, I4, U1, I4, H, H);
	std::cout << "\n" << cunitary_flipped_op << "\n" << U2 << std::endl;
	BOOST_TEST((m::equals<t_mat<t_real>>(cunitary_flipped_op, U2, eps)));
	BOOST_TEST((m::equals<t_mat<t_real>>(U3, C2, eps)));


	// swap state
	// see: (Bronstein08): I. N. Bronstein et al., ISBN: 978-3-8171-2017-8 (2008), Ch. 22 (Zusatzkapitel.pdf), p. 28
	t_mat<t_real> swap_op = m::two_qbit_total_op<t_mat<t_real>>(I, I, C1, C2, C1, I, I);
	std::cout << "\nSWAP |up down> = " << swap_op * updown << std::endl;
	std::cout << "SWAP |down up> = " << swap_op * downup << std::endl;
	BOOST_TEST((m::equals<t_vec<t_real>>(swap_op * updown, downup, eps)));
	BOOST_TEST((m::equals<t_vec<t_real>>(swap_op * downup, updown, eps)));

	std::cout << "--------------------------------------------------------------------------------" << std::endl;
	std::cout << std::endl;
}


BOOST_AUTO_TEST_CASE_TEMPLATE(toffoli, t_real, decltype(std::tuple<float, double, long double>{}))
{
	using namespace m_ops;

	std::cout << "--------------------------------------------------------------------------------" << std::endl;
	std::cout << "Testing TOFFOLI with " << ty::type_id_with_cvr<t_real>().pretty_name() << " type." << std::endl;
	std::cout << "--------------------------------------------------------------------------------" << std::endl;

	t_real eps = std::sqrt(std::numeric_limits<t_real>::epsilon());
	const t_mat<t_real> I = m::unit<t_mat<t_real>>(2);
	const t_mat<t_real>& H = m::hadamard<t_mat<t_real>>();

	const t_mat<t_real>& T = m::toffoli<t_mat<t_real>>();
	t_mat<t_real> toffoli_flipped_1_3 = m::three_qbit_total_op<t_mat<t_real>>(H, I, H, T, H, I, H);
	t_mat<t_real> toffoli_flipped_2_3 = m::three_qbit_total_op<t_mat<t_real>>(I, H, H, T, I, H, H);
	t_mat<t_real> Tb = m::toffoli_nqbits<t_mat<t_real>>(3, 0, 1, 2);
	t_mat<t_real> toffoli_flipped_1_3b = m::toffoli_nqbits<t_mat<t_real>>(3, 2, 1, 0);
	t_mat<t_real> toffoli_flipped_2_3b = m::toffoli_nqbits<t_mat<t_real>>(3, 0, 2, 1);

	std::cout << "Toffoli                       = " << T << std::endl;
	std::cout << "Toffoli (flipped bit 1 and 3) = " << toffoli_flipped_1_3 << std::endl;
	std::cout << "Toffoli (flipped bit 2 and 3) = " << toffoli_flipped_2_3 << std::endl;

	BOOST_TEST((m::equals<t_mat<t_real>>(T, Tb, eps)));
	BOOST_TEST((m::equals<t_mat<t_real>>(toffoli_flipped_1_3, toffoli_flipped_1_3b, eps)));
	BOOST_TEST((m::equals<t_mat<t_real>>(toffoli_flipped_2_3, toffoli_flipped_2_3b, eps)));

	std::cout << "--------------------------------------------------------------------------------" << std::endl;
	std::cout << std::endl;
}
