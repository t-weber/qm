/**
 * quantum mechanics/computing algos
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date oct-2021
 * @license see 'LICENSE' file
 *
 * @see references for algorithms:
 * 	- (Arens15): T. Arens et al., ISBN: 978-3-642-44919-2, DOI: 10.1007/978-3-642-44919-2 (2015).
 * 	- (Arfken13): G. B. Arfken et al., ISBN: 978-0-12-384654-9, DOI: 10.1016/C2009-0-30629-7 (2013).
 *	- (DesktopBronstein08): I. N. Bronstein et al., ISBN: 978-3-8171-2017-8 (2008) [in its HTML version "Desktop Bronstein"].
 * 	- (Bronstein08): I. N. Bronstein et al., ISBN: 978-3-8171-2017-8 (2008) [in its paperback version].
 * 	- (Merziger06): G. Merziger and T. Wirth, ISBN: 3923923333 (2006).
 * 	- (Scarpino11): M. Scarpino, ISBN: 978-1-6172-9017-6 (2011).
 * 	- (Shirane02): G. Shirane et al., ISBN: 978-0-5214-1126-4 (2002).
 * 	- (Kuipers02): J. B. Kuipers, ISBN: 0-691-05872-5 (2002).
 * 	- (FUH 2021): A. Schulz and J. Rollin, "Effiziente Algorithmen", Kurs 1684 (2021), Fernuni Hagen, (https://vu.fernuni-hagen.de/lvuweb/lvu/app/Kurs/01684).
 * 	- (Sellers 2014): G. Sellers et al., ISBN: 978-0-321-90294-8 (2014).
 */

#ifndef __QM_ALGOS_H__
#define __QM_ALGOS_H__

#include "math_algos.h"
#include "math_conts.h"


// math
namespace m {

/**
 * hadamard operator/gate
 * @see (FUH 2021), p. 7
 */
template<class t_mat>
const t_mat& hadamard()
requires is_mat<t_mat> && is_complex<typename t_mat::value_type>
{
	using t_cplx = typename t_mat::value_type;
	using t_real = typename t_cplx::value_type;
	t_cplx c(t_real(1)/std::sqrt(t_real(2)), 0);

	static const t_mat mat = create<t_mat>({{c, c}, {c, -c}});
	return mat;
}


template<class t_val>
std::size_t count_equal_1_bits(t_val val1, t_val val2)
{
	std::size_t count = 0;

	std::size_t N = sizeof(t_val);
	for(std::size_t i=0; i<N; ++i)
	{
		if((val1 & (1<<i)) && (val2 & (1<<i)))
			++count;
	}

	return count;
}


/**
 * hadamard operator of size 2^n (direct calculation without outer product)
 * @see (FUH 2021)
 * @see https://en.wikipedia.org/wiki/Hadamard_transform
 */
template<class t_mat>
t_mat hadamard(std::size_t n)
requires is_mat<t_mat> && is_complex<typename t_mat::value_type>
{
	using t_cplx = typename t_mat::value_type;
	using t_real = typename t_cplx::value_type;

	const t_real factor = std::pow(t_real(1)/std::sqrt(t_real(2)), t_real(n));

	const std::size_t N = std::pow(2, n);
	t_mat mat = create<t_mat>(N, N);

	for(std::size_t i=0; i<N; ++i)
	{
		for(std::size_t j=0; j<N; ++j)
		{
			t_real sign = 1;
			if(count_equal_1_bits(i, j) % 2 != 0)
				sign = -1;

			mat(i,j) = sign * factor;
		}
	}

	return mat;
}


/**
 * hadamard trafo
 * @see (FUH 2021)
 * @see https://en.wikipedia.org/wiki/Hadamard_transform
 */
template<class t_mat>
t_mat hadamard_trafo(const t_mat& M)
requires is_mat<t_mat> && is_complex<typename t_mat::value_type>
{
	std::size_t n = std::log2(M.size1());
	t_mat H = hadamard<t_mat>(n);

	// M_trafo = H^+ M H
	return H * M * H;
}


/**
 * phase gate
 * @see (FUH 2021), p. 12
 * @see (DesktopBronstein08), Ch. 22 (Zusatzkapitel.pdf), p. 25
 */
template<class t_mat, class t_cplx = typename t_mat::value_type, class t_real = typename t_cplx::value_type>
const t_mat& phasegate(t_cplx phase = pi<t_real>/t_real(2))
requires is_mat<t_mat> && is_complex<t_cplx>
{
	constexpr t_cplx c1(1, 0);
	constexpr t_cplx cI(0, 1);

	static const t_mat mat = create<t_mat>({
		{ c1, 0 },
		{  0, std::exp(cI * phase) }
	});
	return mat;
}


/**
 * discrete phase gate
 * @see (DesktopBronstein08), Ch. 22 (Zusatzkapitel.pdf), p. 25
 */
template<class t_mat, class t_cplx = typename t_mat::value_type, class t_real = typename t_cplx::value_type>
const t_mat& phasegate_discrete(t_real k = 1)
requires is_mat<t_mat> && is_complex<t_cplx>
{
	t_real phase = t_real(2)*pi<t_real> / std::pow(t_real(2), k);
	return phasegate<t_mat, t_cplx, t_real>(phase);
}


/**
 * 2-qubit controlled NOT gate ( == controlled unitary gate with U = Pauli-X)
 * @see (FUH 2021), p. 9
 * @see https://en.wikipedia.org/wiki/Controlled_NOT_gate
 */
template<class t_mat>
const t_mat& cnot(bool flipped = false)
requires is_mat<t_mat> && is_complex<typename t_mat::value_type>
{
	using t_cplx = typename t_mat::value_type;
	constexpr t_cplx c(1, 0);

	// C_not
	static const t_mat mat = create<t_mat>({
		{ c, 0, 0, 0 },
		{ 0, c, 0, 0 },
		{ 0, 0, 0, c },
		{ 0, 0, c, 0 },
	});

	// transformed: (H x H)^+ C_not (H x H)
	static const t_mat mat_flipped = create<t_mat>({
		{ c, 0, 0, 0 },
		{ 0, 0, 0, c },
		{ 0, 0, c, 0 },
		{ 0, c, 0, 0 },
	});

	return flipped ? mat_flipped : mat;
}


/**
 * 2-qubit controlled NOT gate ( == controlled unitary gate with U = Pauli-X)
 * flips target bit if control bit is set
 * @see (FUH 2021), p. 9
 * @see https://en.wikipedia.org/wiki/Controlled_NOT_gate
 */
template<class t_mat>
const t_mat cnot_nqbits(std::size_t num_qbits = 2,
	std::size_t control_bit = 0, std::size_t target_bit = 1,
	bool reverse_state_numbering = true)
requires is_mat<t_mat> && is_complex<typename t_mat::value_type>
{
	//using t_cplx = typename t_mat::value_type;
	//constexpr t_cplx c(1, 0);

	if(reverse_state_numbering)
	{
		// numbering of qubits from left-hand side
		control_bit = num_qbits - control_bit - 1;
		target_bit = num_qbits - target_bit - 1;
	}

	const std::size_t N = (1 << num_qbits);
	t_mat mat = zero<t_mat>(N, N);

	const std::size_t control_bit_mask = (1 << control_bit);
	const std::size_t target_bit_mask = (1 << target_bit);

	// iterate all qubits
	for(std::size_t bits=0; bits<N; ++bits)
	{
		std::size_t new_bits = bits;

		bool control_bit_set = (bits & control_bit_mask) != 0;
		if(control_bit_set)
		{
			// filter out target bit
			new_bits = (bits & ~target_bit_mask);

			// flip target bit (set it if it was not set before)
			bool target_bit_set = (bits & target_bit_mask) != 0;
			if(!target_bit_set)
				new_bits |= (1<<target_bit);
		}

		mat(bits, new_bits) = 1;
	}

	return mat;
}


/**
 * 2-qubit SWAP gate
 * flips source and target qubit
 * https://en.wikipedia.org/wiki/Quantum_logic_gate#Swap_gate
 */
template<class t_mat>
const t_mat swap_nqbits(std::size_t num_qbits = 2,
	std::size_t source_bit = 0, std::size_t target_bit = 1,
	bool reverse_state_numbering = true)
requires is_mat<t_mat> && is_complex<typename t_mat::value_type>
{
	if(reverse_state_numbering)
	{
		// numbering of qubit states from left-hand side,
		// e.g.: |11>, |10>, |01>, |00>
		source_bit = num_qbits - source_bit - 1;
		target_bit = num_qbits - target_bit - 1;
	}

	const std::size_t N = (1 << num_qbits);
	t_mat mat = zero<t_mat>(N, N);

	const std::size_t source_bit_mask = (1 << source_bit);
	const std::size_t target_bit_mask = (1 << target_bit);

	// iterate all qubits
	for(std::size_t bits=0; bits<N; ++bits)
	{
		std::size_t new_bits = bits;

		bool source_bit_set = (bits == source_bit_mask);
		if(source_bit_set)
		{
			// set target bit
			new_bits = target_bit_mask;
		}

		bool target_bit_set = (bits == target_bit_mask);
		if(target_bit_set)
		{
			// set source bit
			new_bits = source_bit_mask;
		}

		mat(bits, new_bits) = 1;
	}

	return mat;
}


/**
 * 3-qubit Toffoli gate
 * @see https://en.wikipedia.org/wiki/Toffoli_gate
 */
template<class t_mat>
const t_mat& toffoli()
requires is_mat<t_mat> && is_complex<typename t_mat::value_type>
{
	using t_cplx = typename t_mat::value_type;
	constexpr t_cplx c(1, 0);

	static const t_mat mat = create<t_mat>({
		{ c, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, c, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, c, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, c, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, c, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, c, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, c },
		{ 0, 0, 0, 0, 0, 0, c, 0 },
	});

	return mat;
}


/**
 * 3-qubit Toffoli gate
 * flips target bit if both control bits are set
 * @see https://en.wikipedia.org/wiki/Toffoli_gate
 */
template<class t_mat>
const t_mat toffoli_nqbits(std::size_t num_qbits = 3,
	std::size_t control_bit_1 = 0, std::size_t control_bit_2 = 1, std::size_t target_bit = 2,
	bool reverse_state_numbering = true)
requires is_mat<t_mat> && is_complex<typename t_mat::value_type>
{
	//using t_cplx = typename t_mat::value_type;
	//constexpr t_cplx c(1, 0);

	if(reverse_state_numbering)
	{
		// numbering of qubits from left-hand side
		control_bit_1 = num_qbits - control_bit_1 - 1;
		control_bit_2 = num_qbits - control_bit_2 - 1;
		target_bit = num_qbits - target_bit - 1;
	}

	const std::size_t N = (1 << num_qbits);
	t_mat mat = zero<t_mat>(N, N);

	const std::size_t control_bit_1_mask = (1 << control_bit_1);
	const std::size_t control_bit_2_mask = (1 << control_bit_2);
	const std::size_t target_bit_mask = (1 << target_bit);

	// iterate all qubits
	for(std::size_t bits=0; bits<N; ++bits)
	{
		std::size_t new_bits = bits;

		bool control_bit_1_set = (bits & control_bit_1_mask) != 0;
		bool control_bit_2_set = (bits & control_bit_2_mask) != 0;
		if(control_bit_1_set && control_bit_2_set)
		{
			// filter out target bit
			new_bits = (bits & ~target_bit_mask);

			// flip target bit (set it if it was not set before)
			bool target_bit_set = (bits & target_bit_mask) != 0;
			if(!target_bit_set)
				new_bits |= (1<<target_bit);
		}

		mat(bits, new_bits) = 1;
	}

	return mat;
}


//#define __CALC_C_UNITARY__

/**
 * controlled unitary gate
 * @see (DesktopBronstein08), Ch. 22 (Zusatzkapitel.pdf), p. 27
 */
template<class t_mat>
t_mat cunitary(const t_mat& U22, bool flipped = false)
requires is_mat<t_mat> && is_complex<typename t_mat::value_type>
{
	using t_cplx = typename t_mat::value_type;
	using t_real = typename t_cplx::value_type;

	if(!flipped)
	{
		// C_unitary
		constexpr t_real c1 = 1;

		return create<t_mat>({
			{ c1,       0,        0,        0        },
			{ 0,        c1,       0,        0        },
			{ 0,        0,        U22(0,0), U22(1,0) },
			{ 0,        0,        U22(0,1), U22(1,1) },
		});
	}
	else
	{
		// transformed: (H x H)^+ C_unitary (H x H)
#ifdef __CALC_C_UNITARY__
		t_mat M_unflipped = cunitary<t_mat>(U22, false);
		t_mat M = hadamard_trafo<t_mat>(M_unflipped);

#else
		constexpr t_real c2 = 2;

		const t_cplx& a = U22(0,0);
		const t_cplx& b = U22(0,1);
		const t_cplx& c = U22(1,0);
		const t_cplx& d = U22(1,1);

		t_mat M = create<t_mat>(4,4);

		M(0,0) = c2+a+b+c+d;
		M(0,1) = a-b+c-d;
		M(0,2) = c2-a-b-c-d;
		M(0,3) = -a+b-c+d;

		M(1,0) = std::conj(M(0,1));
		M(1,1) = c2+a-b-c+d;
		M(1,2) = -a-b+c+d;
		M(1,3) = c2-a+b+c-d;

		M(2,0) = std::conj(M(0,2));
		M(2,1) = std::conj(M(1,2));
		M(2,2) = c2+a+b+c+d;
		M(2,3) = a-b+c-d;

		M(3,0) = std::conj(M(0,3));
		M(3,1) = std::conj(M(1,3));
		M(3,2) = std::conj(M(2,3));
		M(3,3) = c2+a-b-c+d;

		M /= t_real(4);
#endif

		return M;
	}
}


/**
 * get total operator of the circuit:
 *
 * qubit 1: ---one_pre_1---|                              |---one_post_1---
 *                         |---two_pre---two---two_post---|
 * qubit 2: ---one_pre_2---|                              |---one_post_2---
 *
 * with one-qubit operators "one*" and two-qubit operator "two"
 */
template<class t_mat> requires m::is_mat<t_mat>
t_mat two_qbit_total_op(
	const t_mat& one_pre_1, const t_mat& one_pre_2,
	const t_mat& two_pre, const t_mat& two, const t_mat& two_post,
	const t_mat& one_post_1, const t_mat& one_post_2)
{
	using namespace m_ops;

	t_mat pre = m::outer<t_mat>(one_pre_1, one_pre_2);
	t_mat post = m::outer<t_mat>(one_post_1, one_post_2);

	return (post * two_post) * two * (two_pre * pre);
}


/**
 * get total operator of the circuit:
 *
 * qubit 1: ---one_pre_1---|           |---one_post_1---
 *                         |           |
 * qubit 2: ---one_pre_2---|---three---|---one_post_2---
 *                         |           |
 * qubit 3: ---one_pre_3---|           |---one_post_3---
 *
 * with one-qubit operators "one*" and two-qubit operator "two"
 */
template<class t_mat> requires m::is_mat<t_mat>
t_mat three_qbit_total_op(
	const t_mat& one_pre_1, const t_mat& one_pre_2, const t_mat& one_pre_3,
	const t_mat& three,
	const t_mat& one_post_1, const t_mat& one_post_2, const t_mat& one_post_3)
{
	using namespace m_ops;

	t_mat pre = m::outer<t_mat>(one_pre_1, one_pre_2);
	pre = m::outer<t_mat>(pre, one_pre_3);
	t_mat post = m::outer<t_mat>(one_post_1, one_post_2);
	post = m::outer<t_mat>(post, one_post_3);

	return post * three * pre;
}

}
#endif
