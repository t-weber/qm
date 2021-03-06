/**
 * adder test
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date 26-oct-2021
 * @license see 'LICENSE' file
 *
 * References:
 *  * https://www.boost.org/doc/libs/1_76_0/libs/test/doc/html/index.html
 *  * https://en.wikipedia.org/wiki/Quantum_logic_gate
 *
 * g++ -std=c++20 -I.. -Wall -Wextra -Weffc++ -o adder adder.cpp
 */

#define BOOST_TEST_MODULE test_adder

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


BOOST_AUTO_TEST_CASE_TEMPLATE(adder, t_real, decltype(std::tuple<float, double, long double>{}))
{
	using namespace m_ops;

	std::cout << "--------------------------------------------------------------------------------" << std::endl;
	std::cout << "Testing with " << ty::type_id_with_cvr<t_real>().pretty_name() << " type." << std::endl;
	std::cout << "--------------------------------------------------------------------------------" << std::endl;

	using t_int = std::uint8_t;
	constexpr const std::size_t num_qbits = 4; //sizeof(t_int)*8;


	t_vec<t_real> down = m::create<t_vec<t_real>>({ 1, 0 });
	t_vec<t_real> up = m::create<t_vec<t_real>>({ 0, 1 });
	t_vec<t_real> inval =
		m::outer_flat<t_vec<t_real>, t_mat<t_real>>(
			m::outer_flat<t_vec<t_real>, t_mat<t_real>>(
				m::outer_flat<t_vec<t_real>, t_mat<t_real>>(up, up), down), down);

	auto inval_bits = measure_qbits<t_vec<t_real>, num_qbits, t_int>(inval);

	std::cout << "in =  " << inval << std::endl;
	std::cout << "in qubits: " << inval_bits << std::endl;

	BOOST_TEST((inval_bits.test(3) == true));  // a
	BOOST_TEST((inval_bits.test(2) == true));  // b
	BOOST_TEST((inval_bits.test(1) == false)); // carry in
	BOOST_TEST((inval_bits.test(0) == false)); // unused (down)


	// adder, see: https://en.wikipedia.org/wiki/Quantum_logic_gate#/media/File:Quantum_Full_Adder.png
	t_mat<t_real> adder1 = m::toffoli_nqbits<t_mat<t_real>>(num_qbits, 0, 1, 3);
	t_mat<t_real> adder2 = m::cnot_nqbits<t_mat<t_real>>(num_qbits, 0, 1);
	t_mat<t_real> adder3 = m::toffoli_nqbits<t_mat<t_real>>(num_qbits, 1, 2, 3);
	t_mat<t_real> adder4 = m::cnot_nqbits<t_mat<t_real>>(num_qbits, 1, 2);
	t_mat<t_real> adder5 = m::cnot_nqbits<t_mat<t_real>>(num_qbits, 0, 1);
	t_mat<t_real> adder = adder5 * adder4 * adder3 * adder2 * adder1;

	BOOST_TEST((adder.size1() == adder.size2()));
	BOOST_TEST((inval.size() == adder.size1()));

	t_vec<t_real> sum = adder * inval;
	auto sum_bits = measure_qbits<t_vec<t_real>, num_qbits, t_int>(sum);

	std::cout << "out = " << sum << std::endl;
	std::cout << "out qubits: " << sum_bits << std::endl;

	BOOST_TEST((sum.size() == adder.size1()));

	BOOST_TEST((sum_bits.test(3) == true));  // a
	BOOST_TEST((sum_bits.test(2) == true));  // b
	BOOST_TEST((sum_bits.test(1) == false)); // sum
	BOOST_TEST((sum_bits.test(0) == true));  // carry out


	std::cout << "--------------------------------------------------------------------------------" << std::endl;
	std::cout << std::endl;
}


BOOST_AUTO_TEST_CASE_TEMPLATE(two_adders, t_real, decltype(std::tuple<float, double, long double>{}))
{
	using namespace m_ops;

	std::cout << "--------------------------------------------------------------------------------" << std::endl;
	std::cout << "Testing with " << ty::type_id_with_cvr<t_real>().pretty_name() << " type." << std::endl;
	std::cout << "--------------------------------------------------------------------------------" << std::endl;

	using t_int = std::uint8_t;
	constexpr const std::size_t num_qbits = 7;


	t_vec<t_real> down = m::create<t_vec<t_real>>({ 1, 0 });
	t_vec<t_real> up = m::create<t_vec<t_real>>({ 0, 1 });

	const t_vec<t_real>& digit1_bit0 = up;
	const t_vec<t_real>& digit1_bit1 = down;
	const t_vec<t_real>& digit2_bit0 = up;
	const t_vec<t_real>& digit2_bit1 = up;

	t_vec<t_real> inval = m::outer_flat<t_vec<t_real>, t_mat<t_real>>({
		down, digit1_bit0, digit2_bit0, down,
		digit1_bit1, digit2_bit1, down});
	auto inval_bits = measure_qbits<t_vec<t_real>, num_qbits, t_int>(inval);

	std::cout << "in =  " << inval << std::endl;
	std::cout << "in qubits: " << inval_bits << std::endl;

	BOOST_TEST((inval_bits.test(6) == false));             // carry in 1
	BOOST_TEST((inval_bits.test(5) == (digit1_bit0==up))); // digit 1, bit 0
	BOOST_TEST((inval_bits.test(4) == (digit2_bit0==up))); // digit 2, bit 0
	BOOST_TEST((inval_bits.test(3) == false));             // unused (down)
	BOOST_TEST((inval_bits.test(2) == (digit1_bit1==up))); // digit 1, bit 1
	BOOST_TEST((inval_bits.test(1) == (digit2_bit1==up))); // digit 2, bit 1
	BOOST_TEST((inval_bits.test(0) == false));             // unused (down)


	// adder, see: https://en.wikipedia.org/wiki/Quantum_logic_gate#/media/File:Quantum_Full_Adder.png
	t_mat<t_real> adder1 = m::toffoli_nqbits<t_mat<t_real>>(num_qbits, 1, 2, 3);
	t_mat<t_real> adder2 = m::cnot_nqbits<t_mat<t_real>>(num_qbits, 1, 2);
	t_mat<t_real> adder3 = m::toffoli_nqbits<t_mat<t_real>>(num_qbits, 2, 0, 3);
	t_mat<t_real> adder4 = m::cnot_nqbits<t_mat<t_real>>(num_qbits, 2, 0);
	t_mat<t_real> adder5 = m::cnot_nqbits<t_mat<t_real>>(num_qbits, 1, 2);

	// connect first carry out to second carry in
	t_mat<t_real> adder6 = m::toffoli_nqbits<t_mat<t_real>>(num_qbits, 1+3, 2+3, 3+3);
	t_mat<t_real> adder7 = m::cnot_nqbits<t_mat<t_real>>(num_qbits, 1+3, 2+3);
	t_mat<t_real> adder8 = m::toffoli_nqbits<t_mat<t_real>>(num_qbits, 2+3, 0+3, 3+3);
	t_mat<t_real> adder9 = m::cnot_nqbits<t_mat<t_real>>(num_qbits, 2+3, 0+3);
	t_mat<t_real> adder10 = m::cnot_nqbits<t_mat<t_real>>(num_qbits, 1+3, 2+3);

	t_mat<t_real> adder = adder10*adder9*adder8*adder7*adder6 * adder5*adder4*adder3*adder2*adder1;
	BOOST_TEST((adder.size1() == adder.size2()));
	BOOST_TEST((inval.size() == adder.size1()));

	t_vec<t_real> sum = adder * inval;
	auto sum_bits = measure_qbits<t_vec<t_real>, num_qbits, t_int>(sum);
	BOOST_TEST((sum.size() == adder.size1()));

	std::cout << "out = " << sum << std::endl;
	std::cout << "out qubits: " << sum_bits << std::endl;

	//BOOST_TEST((sum_bits.test(6) == false));            // sum 1
	BOOST_TEST((sum_bits.test(5) == inval_bits.test(5))); // digit 1, bit 0
	BOOST_TEST((sum_bits.test(4) == inval_bits.test(4))); // digit 2, bit 0
	//BOOST_TEST((sum_bits.test(3) == true));             // sum 2
	BOOST_TEST((sum_bits.test(2) == inval_bits.test(2))); // digit 1, bit 1
	BOOST_TEST((sum_bits.test(1) == inval_bits.test(1))); // digit 2, bit 1
	//BOOST_TEST((sum_bits.test(0) == false));            // carry out 2

	t_int num1 = inval_bits.test(2)*(1<<1) + inval_bits.test(5)*(1<<0);
	t_int num2 = inval_bits.test(1)*(1<<1) + inval_bits.test(4)*(1<<0);
	t_int result = sum_bits.test(0)*(1<<2) + sum_bits.test(3)*(1<<1) + sum_bits.test(6)*(1<<0);
	BOOST_TEST((num1 + num2 == result));
	std::cout << "result: " << (int)num1 << " + " << (int)num2 << " = " << (int)result << std::endl;

	std::cout << "--------------------------------------------------------------------------------" << std::endl;
	std::cout << std::endl;
}
