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
#include <string>
#include <variant>
#include <vector>

#include "types.h"


/**
 * component configuration options
 */
struct ComponentConfig
{
	// key
	std::string key{};

	// value
	std::variant<
		t_int, t_uint,
		t_real, std::string> value{};

	// description of the entry
	std::string description{};

	// optional minimum and maximum values
	std::optional<std::variant<
		t_int, t_uint,
		t_real, std::string>> min_value{};
	std::optional<std::variant<
		t_int, t_uint,
		t_real, std::string>> max_value{};
};


/**
 * struct for the exchange of component configuration options
 */
struct ComponentConfigs
{
	std::string name{};
	std::vector<ComponentConfig> configs{};
};


/**
 * basic interface for a quantum gate
 */
class QuantumGate
{
public:
	virtual ~QuantumGate() = default;

	virtual std::string GetIdent() const = 0;
	virtual std::string GetName() const = 0;
	virtual t_mat GetOperator() const = 0;

	virtual ComponentConfigs GetConfig() const = 0;
	virtual void SetConfig(const ComponentConfigs&) = 0;
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

	// getter
	void SetNumQBits(t_uint bits) { m_num_qbits = bits; }
	void SetControlBitPos(t_uint pos) { m_control_bit_pos = pos; }
	void SetTargetBitPos(t_uint pos) { m_target_bit_pos = pos; }

	// setter
	t_uint GetNumQBits() const { return m_num_qbits; }
	t_uint GetControlBitPos() const { return m_control_bit_pos; }
	t_uint GetTargetBitPos() const { return m_target_bit_pos; }

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

	virtual std::string GetIdent() const override { return "cnot"; }
	virtual std::string GetName() const override { return "CNOT Gate"; }
	virtual t_mat GetOperator() const override;

	virtual ComponentConfigs GetConfig() const override;
	virtual void SetConfig(const ComponentConfigs&) override;


private:
	t_uint m_num_qbits = 2;
	t_uint m_control_bit_pos = 0;
	t_uint m_target_bit_pos = 1;

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

	// getter
	void SetNumQBits(t_uint bits) { m_num_qbits = bits; }
	void SetControlBit1Pos(t_uint pos) { m_control_bit_1_pos = pos; }
	void SetControlBit2Pos(t_uint pos) { m_control_bit_2_pos = pos; }
	void SetTargetBitPos(t_uint pos) { m_target_bit_pos = pos; }

	// setter
	t_uint GetNumQBits() const { return m_num_qbits; }
	t_uint GetControlBit1Pos() const { return m_control_bit_1_pos; }
	t_uint GetControlBit2Pos() const { return m_control_bit_2_pos; }
	t_uint GetTargetBitPos() const { return m_target_bit_pos; }

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;

	virtual std::string GetIdent() const override { return "toffoli"; }
	virtual std::string GetName() const override { return "Toffoli Gate"; }
	virtual t_mat GetOperator() const override;

	virtual ComponentConfigs GetConfig() const override;
	virtual void SetConfig(const ComponentConfigs&) override;


private:
	t_uint m_num_qbits = 3;
	t_uint m_control_bit_1_pos = 0;
	t_uint m_control_bit_2_pos = 1;
	t_uint m_target_bit_pos = 2;

	t_real m_control_bit_radius = 10.;
	t_real m_target_bit_radius = 25.;
};


#endif
