/**
 * table of components representing a circuit
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date 14-nov-2021
 * @license see 'LICENSE' file
 */

#include "components_table.h"

#include <iostream>
#include <iomanip>


ComponentsTable::ComponentsTable(std::size_t ROWS, std::size_t COLS)
	: m_data(ROWS*COLS), m_rowsize{ROWS}, m_colsize{COLS}
{}


/**
 * are there any gates in the given column?
 */
bool ComponentsTable::HasGates(std::size_t col) const
{
	const std::size_t rows = row_size();

	for(std::size_t row=0; row<rows; ++row)
	{
		if(operator()(row, col))
			return true;
	}

	return false;
}


/**
 * check if the circuit is correct
 */
bool ComponentsTable::CheckCircuit() const
{
	const std::size_t cols = col_size();
	const std::size_t rows = row_size();

	for(std::size_t col=0; col<cols; ++col)
	{
		for(std::size_t row=0; row<rows; ++row)
		{
			const QuantumComponentItem* comp = operator()(row, col);
			if(!comp)
				continue;

			// part of gate outside input state component?
			t_uint h = comp->GetNumQBits();
			if(row + h > rows)
				return false;

			// check for overlapping gates
			for(t_uint i=1; i<h; ++i)
			{
				// following rows have to be empty
				if(operator()(row+i, col))
					return false;
			}
		}
	}

	return true;
}


/**
 * calculate the individual column operators of the circuit
 * @returns [column index, column operator]
 */
std::vector<t_columnop>
ComponentsTable::CalculateCircuitOperators() const
{
	const std::size_t cols = col_size();
	const std::size_t rows = row_size();

	std::vector<t_columnop> ops;
	ops.reserve(rows);

	for(std::size_t col=0; col<cols; ++col)
	{
		// no gates in this column -> unit matrix
		if(!HasGates(col))
			continue;

		auto [ok, col_op] = CalculateCircuitOperator(col);
		ops.emplace_back(std::make_tuple(ok, col, std::move(col_op)));
	}

	return ops;
}


/**
 * calculate an individual column operator of the circuit
 */
std::tuple<bool, t_mat> ComponentsTable::CalculateCircuitOperator(std::size_t col) const
{
	const std::size_t rows = row_size();

	const t_mat I = m::unit<t_mat>(2);
	t_mat col_op;

	for(std::size_t row=0; row<rows; ++row)
	{
		const QuantumComponent* gate = operator()(row, col);

		t_mat row_op = gate ? gate->GetOperator() : I;

		if(!col_op.size1())
			col_op = row_op;
		else
			col_op = m::outer<t_mat>(col_op, row_op);

		// skip over the rest of the gate on the grid
		if(gate)
			row += gate->GetNumQBits()-1;
	}

	bool ok = (col_op.size1() == std::pow(2, rows));
	return std::make_tuple(ok, col_op);
}


/**
 * print table
 */
std::ostream& operator<<(std::ostream& ostr, const ComponentsTable& tab)
{
	const std::size_t rows = tab.row_size();
	const std::size_t cols = tab.col_size();

	const t_int width = 12;

	for(std::size_t row=0; row<rows; ++row)
	{
		for(std::size_t col=0; col<cols; ++col)
		{
			const ComponentsTable::value_type entry = tab(row, col);

			ostr << std::setw(width) << std::left
				<< (entry ? entry->GetIdent() : "---");
		}
		ostr << "\n";
	}

	return ostr;
}
