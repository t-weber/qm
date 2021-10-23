/**
 * hadamard operator test
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date 23-oct-2021
 * @license see 'LICENSE' file
 *
 * References:
 *  * http://www.boost.org/doc/libs/1_76_0/libs/geometry/doc/html/index.html
 *  * https://www.boost.org/doc/libs/1_76_0/libs/geometry/doc/html/geometry/reference/algorithms/buffer/buffer_7_with_strategies.html
 *  * https://github.com/boostorg/geometry/tree/develop/example
 *  * https://www.boost.org/doc/libs/1_76_0/libs/test/doc/html/index.html
 *
 * g++ -std=c++20 -I.. -Wall -Wextra -Weffc++ -o hadamard hadamard.cpp
 */

#define BOOST_TEST_MODULE test_hadamard

#include <complex>
#include <vector>
#include <tuple>

#include <boost/test/included/unit_test.hpp>
#include <boost/type_index.hpp>
namespace test = boost::unit_test;
namespace ty = boost::typeindex;

#include "lib/math_algos.h"
#include "lib/math_conts.h"

template<class t_real> using t_cplx = std::complex<t_real>;
template<class t_real> using t_vec = m::vec<t_cplx<t_real>, std::vector>;
template<class t_real> using t_mat = m::mat<t_cplx<t_real>, std::vector>;


template<class t_mat> requires m::is_mat<t_mat>
static bool check_hadamard(std::size_t n)
{
	using namespace m;
	using namespace m_ops;

	t_mat H = hadamard<t_mat>();
	t_mat H1 = H;
	for(std::size_t i=1; i<n; ++i)
		H1 = outer<t_mat>(H1, H);

	t_mat H2 = hadamard<t_mat>(n);

	//operator<<(std::cout, H1) << std::endl;
	//operator<<(std::cout, H2) << std::endl;

	return equals<t_mat>(H1, H2, 1e-6);
}


BOOST_AUTO_TEST_CASE_TEMPLATE(hadamard, t_real, decltype(std::tuple<float, double, long double>{}))
{
	using namespace m;
	using namespace m_ops;

	std::cout << "Testing with " << ty::type_id_with_cvr<t_real>().pretty_name() << " type." << std::endl;

	for(int i=1; i<=8; ++i)
	{
		int SIZE = std::pow(2, i);
		std::cout << "Testing " << SIZE << "x" << SIZE << " hadamard operator." << std::endl;

		bool ok = check_hadamard<t_mat<t_real>>(i);
		BOOST_TEST((ok));
	}

	std::cout << std::endl;
}
