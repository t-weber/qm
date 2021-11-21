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
#include <optional>
#include <variant>
#include <vector>

#include "types.h"


// ----------------------------------------------------------------------------
// configuration structs
// ----------------------------------------------------------------------------
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

	bool is_phase{false};
};


/**
 * set the configuration of a qubit
 */
struct QBitConfig
{
	t_uint bit{};

	// 0:down, 1:up
	t_uint component{};

	// component value
	t_real value{};
};


/**
 * struct for the exchange of component configuration options
 */
struct ComponentConfigs
{
	std::string name{};

	std::vector<ComponentConfig> configs{};
	std::vector<QBitConfig> qbit_configs{};
};
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// interface classes for quantum circuit components
// ----------------------------------------------------------------------------
enum class ComponentType
{
	STATE,
	GATE
};


/**
 * basic interface for a quantum component:
 * either a collection of states or a gate
 */
class QuantumComponent
{
public:
	virtual ~QuantumComponent() = default;

	virtual std::string GetIdent() const = 0;
	virtual std::string GetName() const = 0;

	virtual ComponentType GetType() const = 0;
	virtual t_mat GetOperator() const = 0;

	virtual ComponentConfigs GetConfig() const = 0;
	virtual void SetConfig(const ComponentConfigs&) = 0;

	virtual t_uint GetNumQBits() const = 0;
};


/**
 * a graphical representation of a quantum component
 */
class QuantumComponentItem : public QuantumComponent, public QGraphicsItem
{
public:
	QuantumComponentItem() = default;
	virtual ~QuantumComponentItem() = default;

	virtual QuantumComponentItem* clone() const = 0;

	// factory function
	static QuantumComponentItem* create(const std::string& id);

	std::tuple<t_int, t_int> GetGridPos() const;
};


// shared pointer to a QuantumComponentItem
using t_gateptr = std::shared_ptr<QuantumComponentItem>;
// [column index, column operator]
using t_columnop = std::tuple<std::size_t, t_mat>;
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// quantum state vectors
// ----------------------------------------------------------------------------
class InputStates : public QuantumComponentItem
{
public:
	InputStates();
	virtual ~InputStates();

	virtual QuantumComponentItem* clone() const override;

	// setter
	void SetNumQBits(t_uint bits);
	void SetWidth(t_uint w) { m_width = w; }
	void SetInputQBits(const t_vec& vec);
	void SetOperators(const std::vector<t_columnop>& ops);
	void SetOperators(std::vector<t_columnop>&& ops);

	// getter
	virtual t_uint GetNumQBits() const override { return m_num_qbits; }
	t_uint GetWidth() const { return m_width; }
	const t_vec& GetInputQBits() const { return m_qbits_input; }
	const std::vector<t_columnop>& GetOperators() const { return m_ops; }
	const t_vec& GetInputState() const { return m_state_input; }
	const t_vec& GetOutputState() const { return m_state_output; }

	static const char* GetStaticIdent() { return "input_states"; }
	virtual std::string GetIdent() const override { return InputStates::GetStaticIdent(); }
	virtual std::string GetName() const override { return "Input States"; }

	virtual ComponentType GetType() const override { return ComponentType::STATE; }
	virtual t_mat GetOperator() const override;

	virtual ComponentConfigs GetConfig() const override;
	virtual void SetConfig(const ComponentConfigs&) override;

	bool CalculateTotalOperator();
	bool CalculateInputStates();
	bool CalculateOutputStates();

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;


private:
	t_uint m_num_qbits = 4;
	t_uint m_width = 8;

	// calculated operators for each column of the grid
	std::vector<t_columnop> m_ops{};
	t_mat m_totalop = m::unit<t_mat>(std::pow(2, GetNumQBits()));

	t_vec m_qbits_input = m::samevalue<t_vec>(GetNumQBits(), 1.);
	t_vec m_state_input = m::zero<t_vec>(std::pow(2, GetNumQBits()));
	t_vec m_state_output = m::zero<t_vec>(std::pow(2, GetNumQBits()));
};
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// quantum gates
// ----------------------------------------------------------------------------
/**
 * Hadamard gate
 * @see https://en.wikipedia.org/wiki/Quantum_logic_gate#Hadamard_gate
 */
class HadamardGate : public QuantumComponentItem
{
public:
	HadamardGate();
	virtual ~HadamardGate();

	virtual QuantumComponentItem* clone() const override;

	virtual t_uint GetNumQBits() const override { return 1; }

	static const char* GetStaticIdent() { return "hadamard"; }
	virtual std::string GetIdent() const override { return HadamardGate::GetStaticIdent(); }
	virtual std::string GetName() const override { return "Hadamard Gate"; }

	virtual ComponentType GetType() const override { return ComponentType::GATE; }
	virtual t_mat GetOperator() const override;

	virtual ComponentConfigs GetConfig() const override;
	virtual void SetConfig(const ComponentConfigs&) override;

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;
};



/**
 * Pauli X/Y/Z gate
 * @see https://en.wikipedia.org/wiki/Quantum_logic_gate#Pauli_gates_(X,Y,Z)
 */
class PauliGate : public QuantumComponentItem
{
public:
	PauliGate();
	virtual ~PauliGate();

	virtual QuantumComponentItem* clone() const override;

	// setter
	void SetDirection(t_uint dir) { m_dir = dir; }

	// getter
	t_uint GetDirection() const { return m_dir; }
	virtual t_uint GetNumQBits() const override { return 1; }

	static const char* GetStaticIdent() { return "pauli"; }
	virtual std::string GetIdent() const override { return PauliGate::GetStaticIdent(); }
	virtual std::string GetName() const override { return "Pauli Gate"; }

	virtual ComponentType GetType() const override { return ComponentType::GATE; }
	virtual t_mat GetOperator() const override;

	virtual ComponentConfigs GetConfig() const override;
	virtual void SetConfig(const ComponentConfigs&) override;

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;


private:
	t_uint m_dir = 0;
};



/**
 * Phase gate
 * @see https://en.wikipedia.org/wiki/Quantum_logic_gate#Phase_shift_gates
 */
class PhaseGate : public QuantumComponentItem
{
public:
	PhaseGate();
	virtual ~PhaseGate();

	virtual QuantumComponentItem* clone() const override;

	// setter
	void SetPhase(t_real phase) { m_phase = phase; }

	// getter
	t_real GetPhase() const { return m_phase; }
	virtual t_uint GetNumQBits() const override { return 1; }

	static const char* GetStaticIdent() { return "phase"; }
	virtual std::string GetIdent() const override { return PhaseGate::GetStaticIdent(); }
	virtual std::string GetName() const override { return "Phase Gate"; }

	virtual ComponentType GetType() const override { return ComponentType::GATE; }
	virtual t_mat GetOperator() const override;

	virtual ComponentConfigs GetConfig() const override;
	virtual void SetConfig(const ComponentConfigs&) override;

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;


private:
	t_real m_phase = m::pi<t_real> * t_real(0.5);
};



/**
 * SWAP gate
 * @see https://en.wikipedia.org/wiki/Quantum_logic_gate#Swap_gate
 */
class SwapGate : public QuantumComponentItem
{
public:
	SwapGate();
	virtual ~SwapGate();

	virtual QuantumComponentItem* clone() const override;

	// setter
	void SetNumQBits(t_uint bits) { m_num_qbits = bits; }
	void SetSourceBitPos(t_uint pos) { m_source_bit_pos = pos; }
	void SetTargetBitPos(t_uint pos) { m_target_bit_pos = pos; }

	// getter
	virtual t_uint GetNumQBits() const override { return m_num_qbits; }
	t_uint GetSourceBitPos() const { return m_source_bit_pos; }
	t_uint GetTargetBitPos() const { return m_target_bit_pos; }

	static const char* GetStaticIdent() { return "swap"; }
	virtual std::string GetIdent() const override { return SwapGate::GetStaticIdent(); }
	virtual std::string GetName() const override { return "SWAP Gate"; }

	virtual ComponentType GetType() const override { return ComponentType::GATE; }
	virtual t_mat GetOperator() const override;

	virtual ComponentConfigs GetConfig() const override;
	virtual void SetConfig(const ComponentConfigs&) override;

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;


private:
	t_uint m_num_qbits = 2;
	t_uint m_source_bit_pos = 0;
	t_uint m_target_bit_pos = 1;
};



/**
 * Controlled NOT gate
 * @see https://en.wikipedia.org/wiki/Controlled_NOT_gate
 */
class CNotGate : public QuantumComponentItem
{
public:
	CNotGate();
	virtual ~CNotGate();

	virtual QuantumComponentItem* clone() const override;

	// setter
	void SetNumQBits(t_uint bits) { m_num_qbits = bits; }
	void SetControlBitPos(t_uint pos) { m_control_bit_pos = pos; }
	void SetTargetBitPos(t_uint pos) { m_target_bit_pos = pos; }

	// getter
	virtual t_uint GetNumQBits() const override { return m_num_qbits; }
	t_uint GetControlBitPos() const { return m_control_bit_pos; }
	t_uint GetTargetBitPos() const { return m_target_bit_pos; }

	static const char* GetStaticIdent() { return "cnot"; }
	virtual std::string GetIdent() const override { return CNotGate::GetStaticIdent(); }
	virtual std::string GetName() const override { return "CNOT Gate"; }

	virtual ComponentType GetType() const override { return ComponentType::GATE; }
	virtual t_mat GetOperator() const override;

	virtual ComponentConfigs GetConfig() const override;
	virtual void SetConfig(const ComponentConfigs&) override;

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;


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
class ToffoliGate : public QuantumComponentItem
{
public:
	ToffoliGate();
	virtual ~ToffoliGate();

	virtual QuantumComponentItem* clone() const override;

	// setter
	void SetNumQBits(t_uint bits) { m_num_qbits = bits; }
	void SetControlBit1Pos(t_uint pos) { m_control_bit_1_pos = pos; }
	void SetControlBit2Pos(t_uint pos) { m_control_bit_2_pos = pos; }
	void SetTargetBitPos(t_uint pos) { m_target_bit_pos = pos; }

	// getter
	virtual t_uint GetNumQBits() const override { return m_num_qbits; }
	t_uint GetControlBit1Pos() const { return m_control_bit_1_pos; }
	t_uint GetControlBit2Pos() const { return m_control_bit_2_pos; }
	t_uint GetTargetBitPos() const { return m_target_bit_pos; }

	virtual ComponentType GetType() const override { return ComponentType::GATE; }

	static const char* GetStaticIdent() { return "toffoli"; }
	virtual std::string GetIdent() const override { return ToffoliGate::GetStaticIdent(); }
	virtual std::string GetName() const override { return "Toffoli Gate"; }

	virtual t_mat GetOperator() const override;

	virtual ComponentConfigs GetConfig() const override;
	virtual void SetConfig(const ComponentConfigs&) override;

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;


private:
	t_uint m_num_qbits = 3;
	t_uint m_control_bit_1_pos = 0;
	t_uint m_control_bit_2_pos = 1;
	t_uint m_target_bit_pos = 2;

	t_real m_control_bit_radius = 10.;
	t_real m_target_bit_radius = 25.;
};
// ----------------------------------------------------------------------------


#endif
