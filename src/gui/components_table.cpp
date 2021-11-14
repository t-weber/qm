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
 * check if the circuit is correct
 */
bool ComponentsTable::CheckCircuit() const
{
	for(std::size_t col=0; col<col_size(); ++col)
	{
		for(std::size_t row=0; row<row_size(); ++row)
		{
			const QuantumComponentItem* comp = operator()(row, col);
			if(!comp)
				continue;

			// part of gate outside input state component?
			t_uint h = comp->GetNumQBits();
			if(row + h > row_size())
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
 * print table
 */
std::ostream& operator<<(std::ostream& ostr, const ComponentsTable& tab)
{
	const t_int width = 12;

	for(std::size_t row=0; row<tab.row_size(); ++row)
	{
		for(std::size_t col=0; col<tab.col_size(); ++col)
		{
			const ComponentsTable::value_type entry = tab(row, col);

			ostr << std::setw(width) << std::left
				<< (entry ? entry->GetIdent() : "---");
		}
		ostr << "\n";
	}

	return ostr;
}
