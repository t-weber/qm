/**
 * built-in circuit components
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_COMPONENTS_H__
#define __QM_COMPONENTS_H__

#include "component_interface.h"
#include <unordered_map>


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
	void SetQBitName(t_uint bit, const std::string& name);

	// getter
	virtual t_uint GetNumQBits() const override { return m_num_qbits; }
	t_uint GetWidth() const { return m_width; }
	const t_vec& GetInputQBits() const { return m_qbits_input; }
	const std::vector<t_columnop>& GetOperators() const { return m_ops; }
	const t_vec& GetInputState() const { return m_state_input; }
	const t_vec& GetOutputState() const { return m_state_output; }
	std::string GetQBitName(t_uint bit) const;

	static t_uint GetMinNumQBits() { return 1; }
	static const char* GetStaticIdent() { return "input_states"; }
	static const char* GetStaticName() { return "Input Qubits"; }
	virtual std::string GetIdent() const override { return InputStates::GetStaticIdent(); }
	virtual std::string GetName() const override { return InputStates::GetStaticName(); }

	static ComponentType GetStaticType() { return ComponentType::STATE; }
	virtual ComponentType GetType() const override { return InputStates::GetStaticType(); }
	virtual t_mat GetOperator() const override;

	virtual bool IsOk() const override { return m_ok; }
	void SetOk(bool ok) { m_ok = ok; }

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
	std::unordered_map<t_uint, std::string> m_qbit_names{};
	bool m_ok = true;

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

	virtual t_uint GetNumQBits() const override { return HadamardGate::GetMinNumQBits(); }

	static t_uint GetMinNumQBits() { return 1; }
	static const char* GetStaticIdent() { return "hadamard"; }
	static const char* GetStaticName() { return "Hadamard Gate"; }

	virtual std::string GetIdent() const override { return HadamardGate::GetStaticIdent(); }
	virtual std::string GetName() const override { return HadamardGate::GetStaticName(); }

	static ComponentType GetStaticType() { return ComponentType::GATE; }
	virtual ComponentType GetType() const override { return HadamardGate::GetStaticType(); }
	virtual t_mat GetOperator() const override;

	virtual bool IsOk() const override { return true; }

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
	virtual t_uint GetNumQBits() const override { return PauliGate::GetMinNumQBits(); }

	static t_uint GetMinNumQBits() { return 1; }
	static const char* GetStaticIdent() { return "pauli"; }
	static const char* GetStaticName() { return "Pauli Gate"; }

	virtual std::string GetIdent() const override { return PauliGate::GetStaticIdent(); }
	virtual std::string GetName() const override { return PauliGate::GetStaticName(); }

	static ComponentType GetStaticType() { return ComponentType::GATE; }
	virtual ComponentType GetType() const override { return PauliGate::GetStaticType(); }
	virtual t_mat GetOperator() const override;

	virtual bool IsOk() const override;

	virtual ComponentConfigs GetConfig() const override;
	virtual void SetConfig(const ComponentConfigs&) override;

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;


private:
	t_uint m_dir = 0;
};



/**
 * SU(2) rotation gate
 * @see https://en.wikipedia.org/wiki/Quantum_logic_gate#Rotation_operator_gates
 */
class RotationGate : public QuantumComponentItem
{
public:
	RotationGate();
	virtual ~RotationGate();

	virtual QuantumComponentItem* clone() const override;

	// setter
	void SetDirection(t_uint dir) { m_dir = dir; }
	void SetAngle(t_real angle) { m_angle = angle; }

	// getter
	t_uint GetDirection() const { return m_dir; }
	t_real GetAngle() const { return m_angle; }
	virtual t_uint GetNumQBits() const override { return RotationGate::GetMinNumQBits(); }

	static t_uint GetMinNumQBits() { return 1; }
	static const char* GetStaticIdent() { return "rotation"; }
	static const char* GetStaticName() { return "SU(2) Rotation Gate"; }

	virtual std::string GetIdent() const override { return RotationGate::GetStaticIdent(); }
	virtual std::string GetName() const override { return RotationGate::GetStaticName(); }

	static ComponentType GetStaticType() { return ComponentType::GATE; }
	virtual ComponentType GetType() const override { return RotationGate::GetStaticType(); }
	virtual t_mat GetOperator() const override;

	virtual bool IsOk() const override;

	virtual ComponentConfigs GetConfig() const override;
	virtual void SetConfig(const ComponentConfigs&) override;

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;


private:
	t_uint m_dir = 0;
	t_real m_angle = 0;
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
	virtual t_uint GetNumQBits() const override { return PhaseGate::GetMinNumQBits(); }

	static t_uint GetMinNumQBits() { return 1; }
	static const char* GetStaticIdent() { return "phase"; }
	static const char* GetStaticName() { return "Phase Gate"; }

	virtual std::string GetIdent() const override { return PhaseGate::GetStaticIdent(); }
	virtual std::string GetName() const override { return PhaseGate::GetStaticName(); }

	static ComponentType GetStaticType() { return ComponentType::GATE; }
	virtual ComponentType GetType() const override { return PhaseGate::GetStaticType(); }
	virtual t_mat GetOperator() const override;

	virtual bool IsOk() const override { return true; }

	virtual ComponentConfigs GetConfig() const override;
	virtual void SetConfig(const ComponentConfigs&) override;

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;


private:
	t_real m_phase = m::pi<t_real> * t_real(0.5);
};



/**
 * Unitary gate
 * @see https://en.wikipedia.org/wiki/Quantum_logic_gate#Controlled_gates
 */
class UnitaryGate : public QuantumComponentItem
{
public:
	UnitaryGate();
	virtual ~UnitaryGate();

	virtual QuantumComponentItem* clone() const override;

	// setter
	void SetComponent00(const t_cplx& m00) { m_mat(0,0) = m00; }
	void SetComponent01(const t_cplx& m01) { m_mat(0,1) = m01; }
	void SetComponent10(const t_cplx& m10) { m_mat(1,0) = m10; }
	void SetComponent11(const t_cplx& m11) { m_mat(1,1) = m11; }
	void SetMatrix(const t_mat& mat) { m_mat = mat; }

	// getter
	const t_cplx& GetComponent00() const { return m_mat(0,0); }
	const t_cplx& GetComponent01() const { return m_mat(0,1); }
	const t_cplx& GetComponent10() const { return m_mat(1,0); }
	const t_cplx& GetComponent11() const { return m_mat(1,1); }
	const t_mat& GetMatrix() const { return m_mat; }
	virtual t_uint GetNumQBits() const override { return UnitaryGate::GetMinNumQBits(); }

	static t_uint GetMinNumQBits() { return 1; }
	static const char* GetStaticIdent() { return "unitary"; }
	static const char* GetStaticName() { return "Unitary Gate"; }

	virtual std::string GetIdent() const override { return UnitaryGate::GetStaticIdent(); }
	virtual std::string GetName() const override { return UnitaryGate::GetStaticName(); }

	static ComponentType GetStaticType() { return ComponentType::GATE; }
	virtual ComponentType GetType() const override { return UnitaryGate::GetStaticType(); }
	virtual t_mat GetOperator() const override;

	virtual bool IsOk() const override { return true; }

	virtual ComponentConfigs GetConfig() const override;
	virtual void SetConfig(const ComponentConfigs&) override;

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;


private:
	t_mat m_mat = m::unit<t_mat>(2);
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

	static t_uint GetMinNumQBits() { return 2; }
	static const char* GetStaticIdent() { return "swap"; }
	static const char* GetStaticName() { return "SWAP Gate"; }

	virtual std::string GetIdent() const override { return SwapGate::GetStaticIdent(); }
	virtual std::string GetName() const override { return SwapGate::GetStaticName(); }

	static ComponentType GetStaticType() { return ComponentType::GATE; }
	virtual ComponentType GetType() const override { return PhaseGate::GetStaticType(); }
	virtual t_mat GetOperator() const override;

	virtual bool IsOk() const override;

	virtual ComponentConfigs GetConfig() const override;
	virtual void SetConfig(const ComponentConfigs&) override;

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;


private:
	t_uint m_num_qbits = SwapGate::GetMinNumQBits();
	t_uint m_source_bit_pos = 0;
	t_uint m_target_bit_pos = 1;
};



/**
 * Controlled NOT gate (CNOT / CX)
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

	static t_uint GetMinNumQBits() { return 2; }
	static const char* GetStaticIdent() { return "cnot"; }
	static const char* GetStaticName() { return "CNOT/CX Gate"; }

	virtual std::string GetIdent() const override { return CNotGate::GetStaticIdent(); }
	virtual std::string GetName() const override { return CNotGate::GetStaticName(); }

	static ComponentType GetStaticType() { return ComponentType::GATE; }
	virtual ComponentType GetType() const override { return CNotGate::GetStaticType(); }
	virtual t_mat GetOperator() const override;

	virtual bool IsOk() const override;

	virtual ComponentConfigs GetConfig() const override;
	virtual void SetConfig(const ComponentConfigs&) override;

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;


private:
	t_uint m_num_qbits = CNotGate::GetMinNumQBits();
	t_uint m_control_bit_pos = 0;
	t_uint m_target_bit_pos = 1;

	t_real m_control_bit_radius = 10.;
	t_real m_target_bit_radius = 25.;
};



/**
 * Controlled Pauli-Z gate (CZ)
 * @see https://en.wikipedia.org/wiki/Quantum_logic_gate#Controlled_gates
 */
class CZGate : public QuantumComponentItem
{
public:
	CZGate();
	virtual ~CZGate();

	virtual QuantumComponentItem* clone() const override;

	// setter
	void SetNumQBits(t_uint bits) { m_num_qbits = bits; }
	void SetControlBitPos(t_uint pos) { m_control_bit_pos = pos; }
	void SetTargetBitPos(t_uint pos) { m_target_bit_pos = pos; }

	// getter
	virtual t_uint GetNumQBits() const override { return m_num_qbits; }
	t_uint GetControlBitPos() const { return m_control_bit_pos; }
	t_uint GetTargetBitPos() const { return m_target_bit_pos; }

	static t_uint GetMinNumQBits() { return 2; }
	static const char* GetStaticIdent() { return "cz"; }
	static const char* GetStaticName() { return "CZ Gate"; }

	virtual std::string GetIdent() const override { return CZGate::GetStaticIdent(); }
	virtual std::string GetName() const override { return CZGate::GetStaticName(); }

	static ComponentType GetStaticType() { return ComponentType::GATE; }
	virtual ComponentType GetType() const override { return CZGate::GetStaticType(); }
	virtual t_mat GetOperator() const override;

	virtual bool IsOk() const override;

	virtual ComponentConfigs GetConfig() const override;
	virtual void SetConfig(const ComponentConfigs&) override;

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;


private:
	t_uint m_num_qbits = CZGate::GetMinNumQBits();
	t_uint m_control_bit_pos = 0;
	t_uint m_target_bit_pos = 1;

	t_real m_control_bit_radius = 10.;
	t_real m_target_bit_radius = 10.;
};



/**
 * Controlled unitary gate
 * @see https://en.wikipedia.org/wiki/Quantum_logic_gate#Controlled_gates
 */
class CUnitaryGate : public QuantumComponentItem
{
public:
	CUnitaryGate();
	virtual ~CUnitaryGate();

	virtual QuantumComponentItem* clone() const override;

	// setter
	void SetNumQBits(t_uint bits) { m_num_qbits = bits; }
	void SetControlBitPos(t_uint pos) { m_control_bit_pos = pos; }
	void SetTargetBitPos(t_uint pos) { m_target_bit_pos = pos; }
	void SetComponent00(const t_cplx& m00) { m_mat(0,0) = m00; }
	void SetComponent01(const t_cplx& m01) { m_mat(0,1) = m01; }
	void SetComponent10(const t_cplx& m10) { m_mat(1,0) = m10; }
	void SetComponent11(const t_cplx& m11) { m_mat(1,1) = m11; }
	void SetMatrix(const t_mat& mat) { m_mat = mat; }

	// getter
	virtual t_uint GetNumQBits() const override { return m_num_qbits; }
	t_uint GetControlBitPos() const { return m_control_bit_pos; }
	t_uint GetTargetBitPos() const { return m_target_bit_pos; }
	const t_cplx& GetComponent00() const { return m_mat(0,0); }
	const t_cplx& GetComponent01() const { return m_mat(0,1); }
	const t_cplx& GetComponent10() const { return m_mat(1,0); }
	const t_cplx& GetComponent11() const { return m_mat(1,1); }
	const t_mat& GetMatrix() const { return m_mat; }

	static t_uint GetMinNumQBits() { return 2; }
	static const char* GetStaticIdent() { return "cunitary"; }
	static const char* GetStaticName() { return "CUnitary Gate"; }

	virtual std::string GetIdent() const override { return CUnitaryGate::GetStaticIdent(); }
	virtual std::string GetName() const override { return CUnitaryGate::GetStaticName(); }

	static ComponentType GetStaticType() { return ComponentType::GATE; }
	virtual ComponentType GetType() const override { return CUnitaryGate::GetStaticType(); }
	virtual t_mat GetOperator() const override;

	virtual bool IsOk() const override { return true; }

	virtual ComponentConfigs GetConfig() const override;
	virtual void SetConfig(const ComponentConfigs&) override;

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;


private:
	t_mat m_mat = m::unit<t_mat>(2);

	t_uint m_num_qbits = CUnitaryGate::GetMinNumQBits();
	t_uint m_control_bit_pos = 0;
	t_uint m_target_bit_pos = 1;

	t_real m_control_bit_radius = 10.;
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

	static t_uint GetMinNumQBits() { return 3; }
	static const char* GetStaticIdent() { return "toffoli"; }
	static const char* GetStaticName() { return "Toffoli/CCNOT Gate"; }

	virtual std::string GetIdent() const override { return ToffoliGate::GetStaticIdent(); }
	virtual std::string GetName() const override { return ToffoliGate::GetStaticName(); }

	static ComponentType GetStaticType() { return ComponentType::GATE; }
	virtual ComponentType GetType() const override { return ToffoliGate::GetStaticType(); }
	virtual t_mat GetOperator() const override;

	virtual bool IsOk() const override;

	virtual ComponentConfigs GetConfig() const override;
	virtual void SetConfig(const ComponentConfigs&) override;

	virtual QRectF boundingRect() const override;
	virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) override;


private:
	t_uint m_num_qbits = ToffoliGate::GetMinNumQBits();
	t_uint m_control_bit_1_pos = 0;
	t_uint m_control_bit_2_pos = 1;
	t_uint m_target_bit_pos = 2;

	t_real m_control_bit_radius = 10.;
	t_real m_target_bit_radius = 25.;
};
// ----------------------------------------------------------------------------


// list of all internally known components classes
using t_all_components = std::tuple
<
	InputStates,
	HadamardGate, PauliGate,
	PhaseGate, RotationGate, UnitaryGate,
	SwapGate, CNotGate, CZGate, CUnitaryGate,
	ToffoliGate
>;


#endif
