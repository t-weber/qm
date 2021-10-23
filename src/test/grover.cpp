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


template<class t_mat, class t_vec>
requires is_mat<t_mat> && is_vec<t_vec>
static bool check_measurements(const t_vec& up, const t_vec& down, const t_vec& twobitstate)
{
	using t_val = typename t_vec::value_type;
	const t_mat I = unit<t_mat>(2);

	// measurement operators
	t_mat up_proj = projector<t_mat, t_vec>(up, false);
	t_mat down_proj = projector<t_mat, t_vec>(down, false);
	t_mat up_proj_bit1 = outer<t_mat>(up_proj, I);
	t_mat down_proj_bit1 = outer<t_mat>(down_proj, I);
	t_mat up_proj_bit2 = outer<t_mat>(I, up_proj);
	t_mat down_proj_bit2 = outer<t_mat>(I, down_proj);

	/*std::cout << "|down><down| x I = " << down_proj_bit1 << std::endl;
	std::cout << "|up><up| x I = " << up_proj_bit1 << std::endl;
	std::cout << "I x |down><down| = " << down_proj_bit2 << std::endl;
	std::cout << "I x |up><up| = " << up_proj_bit2 << std::endl;*/

	// numbering: 0=|down down>, 1=|down up>, 2=|up down>, 3=|up up>
	std::cout << "2-bit state: " << twobitstate << std::endl;

	t_val bit1_down1 = sum(down_proj_bit1*twobitstate);
	t_val bit1_down2 = twobitstate[0] + twobitstate[1];
	t_val bit1_up1 = sum(up_proj_bit1*twobitstate);
	t_val bit1_up2 = twobitstate[2] + twobitstate[3];

	t_val bit2_down1 = sum(down_proj_bit2*twobitstate);
	t_val bit2_down2 = twobitstate[0] + twobitstate[2];
	t_val bit2_up1 = sum(up_proj_bit2*twobitstate);
	t_val bit2_up2 = twobitstate[1] + twobitstate[3];

	bool bit1downok = m::equals<t_val>(bit1_down1, bit1_down2);
	bool bit1upok = m::equals<t_val>(bit1_up1, bit1_up2);
	bool bit2downok = m::equals<t_val>(bit2_down1, bit2_down2);
	bool bit2upok = m::equals<t_val>(bit2_up1, bit2_up2);

	std::cout << "bit1_down: " << bit1_down1 << " == " << bit1_down2 << ": " << bit1downok << std::endl;
	std::cout << "bit1_up: " << bit1_up1 << " == " << bit1_up2 << ": " << bit1upok << std::endl;

	std::cout << "bit2_down: " << bit2_down1 << " == " << bit2_down2 << ": " << bit2downok << std::endl;
	std::cout << "bit2_up: " << bit2_up1 << " == " << bit2_up2 << ": "  << bit2upok << std::endl;

	return bit1downok && bit1upok && bit2downok && bit2upok;
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
