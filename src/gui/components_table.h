/**
 * table of components representing a circuit
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date 14-nov-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_COMPONENTS_TABLE_H__
#define __QM_COMPONENTS_TABLE_H__

#include "components.h"

#include <vector>


class ComponentsTable
{
public:
	using value_type = const QuantumComponentItem*;
	using container_type = std::vector<value_type>;

	ComponentsTable(std::size_t ROWS, std::size_t COLS);
	~ComponentsTable() = default;

	// get table size
	std::size_t row_size() const { return m_rowsize; }
	std::size_t col_size() const { return m_colsize; }

	// access table elements
	const value_type& operator()(std::size_t row, std::size_t col) const { return m_data[row*m_colsize + col]; }
	value_type& operator()(std::size_t row, std::size_t col) { return m_data[row*m_colsize + col]; }

	bool HasGates(std::size_t col) const;

	// check circuit
	bool CheckCircuit() const;

	// calculate circuit
	std::vector<t_columnop> CalculateCircuitOperators() const;
	t_mat CalculateCircuitOperator(std::size_t col) const;

	// print table
	friend std::ostream& operator<<(std::ostream& ostr, const ComponentsTable& tab);


private:
	container_type m_data;
	std::size_t m_rowsize, m_colsize;
};


#endif
