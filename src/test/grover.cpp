/**
 * tests the container-agnostic math algorithms
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date jun-2021
 * @license see 'LICENSE' file
 *
 * g++ -std=c++20 -I.. -Wall -Wextra -Weffc++ -o grover grover.cpp
 */

#include <vector>
#include <iostream>
#include <fstream>

#include "lib/math_algos.h"
#include "lib/math_conts.h"
#include "lib/qm_algos.h"

using namespace m;
using namespace m_ops;


template<class t_vec> requires is_vec<t_vec>
void write_state(std::ostream& ostr, const t_vec& state)
{
	using t_cplx = typename t_vec::value_type;
	using t_real = typename t_cplx::value_type;

	for(std::size_t i=0; i<state.size(); ++i)
	{
		t_real n = std::norm(state[i]);
		ostr << std::left << std::setw(16) << n << " ";
	}

	ostr << std::endl;
}


/**
 * grover search algorithm
 * @see https://en.wikipedia.org/wiki/Grover%27s_algorithm
 * @see "Effiziente Algorithmen" (2021), Kurs 1684, Fernuni Hagen (https://vu.fernuni-hagen.de/lvuweb/lvu/app/Kurs/01684), pp. 26-31.
 */
template<class t_mat, class t_vec>
requires is_mat<t_mat> && is_vec<t_vec>
static bool check_grover(std::size_t n, std::size_t num_steps, std::size_t idx_to_find)
{
	using t_val = typename t_vec::value_type;

	t_vec down = create<t_vec>({ 1, 0 });
	t_vec state = down;

	for(std::size_t i=0; i<n-1; ++i)
		state = outer_flat<t_vec, t_mat>(state, down);

	t_mat H = hadamard<t_mat>(n);
	state = H * state;
	std::cout << "state: " << state << std::endl;

	t_mat mirror = -ortho_mirror_op<t_mat, t_vec>(state, true);
	//std::cout << "mirror = " << mirror << std::endl;

	t_vec oracle_vec = m::zero<t_vec>(state.size());
	oracle_vec[idx_to_find] = 1;	// index to find
	t_mat mirror_oracle = ortho_mirror_op<t_mat, t_vec>(oracle_vec, true);
	//std::cout << "mirror_oracle = " << mirror_oracle << std::endl;


	std::ofstream ofstr("grover.dat");

	for(std::size_t step=0; step<num_steps; ++step)
	{
		state = mirror_oracle * state;
		std::cout << "\nmirror sign: " << state << std::endl;

		state = mirror * state;
		std::cout << "mirror at mean: " << state << std::endl;

		write_state(ofstr, state);
	}


	// check if the correct index has been recovered
	auto iter_max = std::max_element(state.begin(), state.end(),
		[](const t_val& val1, const t_val& val2) -> bool
			{ return std::norm(val1) < std::norm(val2); });

	return (iter_max - state.begin()) == std::ptrdiff_t(idx_to_find);
}


int main()
{
	using t_real = double;
	using t_cplx = std::complex<t_real>;
	using t_vec = vec<t_cplx, std::vector>;
	using t_mat = mat<t_cplx, std::vector>;

	std::cout << "\n" << std::boolalpha << check_grover<t_mat, t_vec>(4, 8, 5) << std::endl;

	return 0;
}
