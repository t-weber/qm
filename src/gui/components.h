/**
 * circuit components
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_COMPONENTS_H__
#define __QM_COMPONENTS_H__

#include <QtWidgets/QGraphicsItem>
#include <QtGui/QPainter>

#include <memory>

#include "types.h"


/**
 * basic interface for a quantum gate
 */
class QuantumGate
{
public:
	virtual ~QuantumGate() = default;

	virtual t_mat GetOperator() const = 0;
};


class QuantumGateItem : public QuantumGate, public QGraphicsItem
{
public:
	QuantumGateItem() = default;
	virtual ~QuantumGateItem() = default;
};

using t_gateptr = std::shared_ptr<QuantumGateItem>;


/**
 * Controlled NOT gate
 * @see https://en.wikipedia.org/wiki/Controlled_NOT_gate
 */
class CNotGate : public QuantumGateItem
{
public:
	CNotGate();
	virtual ~CNotGate();

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

	virtual t_mat GetOperator() const override;


private:
	std::size_t m_num_qbits = 2;
	std::size_t m_control_bit_pos = 0;
	std::size_t m_target_bit_pos = 1;

	t_real m_control_bit_radius = 10.;
	t_real m_target_bit_radius = 25.;
};



/**
 * Toffoli gate
 * @see https://en.wikipedia.org/wiki/Toffoli_gate
 */
class ToffoliGate : public QuantumGateItem
{
public:
	ToffoliGate();
	virtual ~ToffoliGate();

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

	virtual t_mat GetOperator() const override;


private:
	std::size_t m_num_qbits = 3;
	std::size_t m_control_bit_1_pos = 0;
	std::size_t m_control_bit_2_pos = 1;
	std::size_t m_target_bit_pos = 2;

	t_real m_control_bit_radius = 10.;
	t_real m_target_bit_radius = 25.;
};


#endif
