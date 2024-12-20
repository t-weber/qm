/**
 * built-in circuit components
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#include <QtGui/QRadialGradient>

#include "components.h"
#include "globals.h"
#include "settings.h"
#include "lib/qm_algos.h"

#include <string_view>
#include <boost/algorithm/string.hpp>



// ----------------------------------------------------------------------------
// input qubit states
// ----------------------------------------------------------------------------
InputStates::InputStates()
{
	setPos(QPointF{0,0});
	setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);

	// to have some initial values
	CalculateInputStates();
}


InputStates::~InputStates()
{
}


QuantumComponentItem* InputStates::clone() const
{
	InputStates *item = new InputStates{};

	item->SetNumQBits(this->GetNumQBits());
	item->SetWidth(this->GetWidth());
	item->m_qbit_names = this->m_qbit_names;

	//*static_cast<QGraphicsItem*>(item) = *this;

	return item;
}


QRectF InputStates::boundingRect() const
{
	t_real w = t_real(m_width) * g_raster_size;
	t_real h = t_real(m_num_qbits) * g_raster_size;

	return QRectF{-g_raster_size*0.5, -g_raster_size*0.5, w, h};
}


void InputStates::paint(QPainter *painter,
	const QStyleOptionGraphicsItem*, QWidget*)
{
	const QColor& colour_fg = get_foreground_colour();

	QPen pen(colour_fg);
	pen.setWidthF(1.5);

	//QFont font = painter->font();
	//font.setPointSize(g_raster_size*0.5);
	//painter->setFont(font);

	painter->setPen(pen);
	painter->setBrush(Qt::NoBrush);

	// iterate qubits
	for(t_uint bit=0; bit<GetNumQBits(); ++bit)
	{
		// write qubit names
		QString name = GetQBitName(bit).c_str();
		QRect rectName = painter->fontMetrics().boundingRect(name);

		qreal x = -g_raster_size*0.5;
		qreal w = g_raster_size;

		// qubit name too long?
		if(rectName.width() > w)
		{
			w = rectName.width();
			//x = g_raster_size*0.5 - w;
		}

		QRectF rectText{
			x, bit*g_raster_size-g_raster_size*0.5,
			w, g_raster_size};
		painter->drawText(rectText, Qt::AlignCenter, name);

		// draw qubit connection lines
		painter->drawLine(
			QPointF(x+w, bit*g_raster_size),
			QPointF((m_width-0.5)*g_raster_size, bit*g_raster_size));
	}
}


/**
 * set the number of qubits
 */
void InputStates::SetNumQBits(t_uint bits)
{
	m_num_qbits = bits;

	auto old_qbits = m_qbits_input;
	m_qbits_input = m::samevalue<t_vec>(GetNumQBits(), 1.);

	// restore old bits
	for(t_uint bit=0; bit<std::min(m_qbits_input.size(), old_qbits.size()); ++bit)
		m_qbits_input[bit] = old_qbits[bit];

	CalculateInputStates();
}


/**
 * set the name of a qbit
 */
void InputStates::SetQBitName(t_uint bit, const std::string& name)
{
	//std::cout << bit << " -> " << name << std::endl;
	m_qbit_names.insert_or_assign(bit, name);
	update();
}


/**
 * get the name of a qbit
 */
std::string InputStates::GetQBitName(t_uint bit) const
{
	if(auto iter = m_qbit_names.find(bit); iter != m_qbit_names.end())
		return iter->second;

	// return a default name if none is set
	std::ostringstream ostr;
	ostr << "|ψ" << (bit+1) << ">";
	return ostr.str();
}


/**
 * set the circuit column operators and calculate the total operator
 */
void InputStates::SetOperators(const std::vector<t_columnop>& ops)
{
	m_ops = ops;
	CalculateTotalOperator();
	CalculateOutputStates();
}


/**
 * set the circuit column operators and calculate the total operator
 */
void InputStates::SetOperators(std::vector<t_columnop>&& ops)
{
	m_ops = std::forward<std::vector<t_columnop>>(ops);
	CalculateTotalOperator();
	CalculateOutputStates();
}


/**
 * set the input qubits and calculate their state vector
 */
void InputStates::SetInputQBits(const t_vec& vec)
{
	m_qbits_input = vec;
	CalculateInputStates();
}


/**
 * set the circuit column operators and calculate the total operator
 */
bool InputStates::CalculateTotalOperator()
{
	using namespace m_ops;
	const auto& ops = GetOperators();

	if(ops.size() && IsOk())
	{
		auto iter = ops.rbegin();
		m_totalop = std::get<t_mat>(*iter);
		std::advance(iter, 1);

		for(; iter!=ops.rend(); std::advance(iter, 1))
			m_totalop = m_totalop * std::get<t_mat>(*iter);
	}
	else
	{
		m_totalop = m::unit<t_mat>(std::pow(2, GetNumQBits()));
	}

	return true;
}


/**
 * calculate input state vector from qubit configuration
 */
bool InputStates::CalculateInputStates()
{
	using namespace m_ops;
	m_state_input.clear();

	if(m_qbits_input.size() != m_num_qbits)
		return false;

	static const t_vec down = m::create<t_vec>({ 1, 0 });
	static const t_vec up = m::create<t_vec>({ 0, 1 });

	for(t_uint bit=0; bit<m_num_qbits; ++bit)
	{
		t_vec comp = m_qbits_input[bit].real()*down + m_qbits_input[bit].imag()*up;

		if(m_state_input.size() == 0)
			m_state_input = comp;
		else
			m_state_input = m::outer_flat<t_vec, t_mat>(m_state_input, comp);
	}

	return true;
}


/**
 * apply the operator to the input state vector
 */
bool InputStates::CalculateOutputStates()
{
	if(m_state_input.size() != m_totalop.size2())
		return false;

	using namespace m_ops;
	m_state_output = m_totalop * m_state_input;

	return true;
}


/**
 * get total calculated operator
 */
t_mat InputStates::GetOperator() const
{
	return m_totalop;
}


template<class t_map>
static std::string serialise_map(const t_map& map)
{
	std::ostringstream ostr;

	// TODO: ensure there's no collision with the separator symbols
	for(const auto& pair : map)
		ostr << pair.first << "=" << pair.second << "#";

	return ostr.str();
}


template<class t_map>
static t_map deserialise_map(const std::string& str)
{
	t_map map;
	using t_key = typename t_map::key_type;
	using t_val = typename t_map::mapped_type;

	// TODO: ensure there's no collision with the separator symbols
	std::vector<std::string> keyvals;
	boost::split(keyvals, str,
		[](auto ch) -> bool
		{
			return ch == '#';
		}, boost::token_compress_on);

	for(const std::string& keyvalstr : keyvals)
	{
		std::vector<std::string> keyval;
		boost::split(keyval, keyvalstr,
			[](auto ch) -> bool
			{
				return ch == '=';
			}, boost::token_compress_on);

		if(keyval.size() != 2)
			continue;

		// convert to correct key/value types
		t_key key{};
		t_val val{};

		if constexpr(std::is_same_v<t_key, std::string>)
		{
			// key is already a string
			key = boost::trim_copy(keyval[0]);
		}
		else
		{
			std::istringstream istr(keyval[0]);
			istr >> key;
		}

		if constexpr(std::is_same_v<t_val, std::string>)
		{
			// value is already a string
			val = boost::trim_copy(keyval[1]);
		}
		else
		{
			std::istringstream istr(keyval[1]);
			istr >> val;
		}

		map.emplace(std::make_pair(key, val));
	}

	return map;
}


ComponentConfigs InputStates::GetConfig() const
{
	ComponentConfigs cfgs;
	cfgs.name = GetName();

	cfgs.configs = std::vector<ComponentConfig>
	{{
		ComponentConfig
		{
			.key = "num_qbits",
			.value = GetNumQBits(),
			.description = "Number of qubits",
			.min_value = GetMinNumQBits(),
		},

		ComponentConfig
		{
			.key = "width",
			.value = GetWidth(),
			.description = "Width",
			.min_value = t_uint(2)
		},

		ComponentConfig
		{
			.key = "qbit_names",
			.value = serialise_map(m_qbit_names),
			.description = "Qubit names",
		},
	}};

	// get individual qubit names
	/*for(t_uint qbit=0; qbit<GetNumQBits(); ++qbit)
	{
		std::ostringstream ostrKey;
		ostrKey << "qbit_" << qbit << "_name";

		ComponentConfig cfg
		{
			.key = ostrKey.str(),
			.value = GetQBitName(qbit),
		};

		cfgs.configs.emplace_back(std::move(cfg));
	}*/

	return cfgs;
}


void InputStates::SetConfig(const ComponentConfigs& cfgs)
{
	// get configuration options
	for(const ComponentConfig& cfg : cfgs.configs)
	{
		if(cfg.key == "num_qbits")
		{
			t_uint bits = std::get<t_uint>(cfg.value);
			SetNumQBits(bits);
		}
		else if(cfg.key == "width")
		{
			t_uint w = std::get<t_uint>(cfg.value);
			SetWidth(w);
		}
		else if(cfg.key == "qbit_names")
		{
			if(std::holds_alternative<std::string>(cfg.value))
			{
				m_qbit_names = deserialise_map<decltype(m_qbit_names)>(
					std::get<std::string>(cfg.value));
				//for(const auto& [qbit, name] : m_qbit_names)
				//	std::cout << qbit << " -> " << name << std::endl;
			}
		}
	}

	// write qubit names individually
	/*for(t_uint qbit=0; qbit<GetNumQBits(); ++qbit)
	{
		std::ostringstream ostrKey;
		ostrKey << "qbit_" << qbit << "_name";
		std::string qbitkey = ostrKey.str();

		auto iterCfg = std::find_if(cfgs.configs.begin(), cfgs.configs.end(),
			[&qbitkey](const ComponentConfig& cfg) -> bool
		{
			return cfg.key == qbitkey;
		});

		if(iterCfg != cfgs.configs.end())
		{
			if(std::holds_alternative<std::string>(iterCfg->value))
				SetQBitName(qbit, std::get<std::string>(iterCfg->value));
		}
	}*/

	// get qubit setting
	for(const QBitConfig& bitcfg : cfgs.qbit_configs)
	{
		if(bitcfg.bit < m_num_qbits)
		{
			if(bitcfg.component == 0)
				m_qbits_input[bitcfg.bit].real(bitcfg.value);
			else if(bitcfg.component == 1)
				m_qbits_input[bitcfg.bit].imag(bitcfg.value);
		}
	}

	CalculateInputStates();
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// Hadamard gate
// @see https://en.wikipedia.org/wiki/Quantum_logic_gate#Hadamard_gate
// ----------------------------------------------------------------------------

HadamardGate::HadamardGate()
{
	setPos(QPointF{0,0});
	setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}


HadamardGate::~HadamardGate()
{
}


QuantumComponentItem* HadamardGate::clone() const
{
	HadamardGate *item = new HadamardGate{};
	return item;
}


QRectF HadamardGate::boundingRect() const
{
	t_real w = g_raster_size;
	t_real h = g_raster_size;

	return QRectF{-g_raster_size*0.5, -g_raster_size*0.5, w, h};
}


void HadamardGate::paint(QPainter *painter,
	const QStyleOptionGraphicsItem*, QWidget*)
{
	const QColor& colour_fg = get_foreground_colour();
	const QColor& colour_bg = get_background_colour();

	QPen pen(colour_fg);
	pen.setWidthF(1.);

	QBrush brush{Qt::SolidPattern};
	brush.setColor(colour_bg);

	//QFont font = painter->font();
	//font.setPointSize(g_raster_size*0.5);
	//painter->setFont(font);

	painter->setPen(pen);
	painter->setBrush(brush);

	t_real size = g_raster_size*0.66;
	QRectF rect{-size*0.5, -size*0.5, size, size};

	painter->drawRect(rect);
	painter->drawText(rect, Qt::AlignCenter, "H");
}


/**
 * get gate operator
 */
t_mat HadamardGate::GetOperator() const
{
	return m::hadamard<t_mat>();
}


ComponentConfigs HadamardGate::GetConfig() const
{
	ComponentConfigs cfgs;
	cfgs.name = GetName();

	return cfgs;
}


void HadamardGate::SetConfig(const ComponentConfigs&)
{
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// Pauli X/Y/Z gate
// @see https://en.wikipedia.org/wiki/Quantum_logic_gate#Pauli_gates_(X,Y,Z)
// ----------------------------------------------------------------------------
PauliGate::PauliGate()
{
	setPos(QPointF{0,0});
	setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}


PauliGate::~PauliGate()
{
}


QuantumComponentItem* PauliGate::clone() const
{
	PauliGate *item = new PauliGate{};

	item->SetDirection(this->GetDirection());

	return item;
}


QRectF PauliGate::boundingRect() const
{
	t_real w = g_raster_size;
	t_real h = g_raster_size;

	return QRectF{-g_raster_size*0.5, -g_raster_size*0.5, w, h};
}


void PauliGate::paint(QPainter *painter,
	const QStyleOptionGraphicsItem*, QWidget*)
{
	const QColor& colour_fg = get_foreground_colour();
	const QColor& colour_bg = get_background_colour();

	QPen pen(colour_fg);
	pen.setWidthF(1.);

	QBrush brush{Qt::SolidPattern};
	brush.setColor(colour_bg);

	//QFont font = painter->font();
	//font.setPointSize(g_raster_size*0.5);
	//painter->setFont(font);

	painter->setPen(pen);
	painter->setBrush(brush);

	t_real size = g_raster_size*0.66;
	QRectF rect{-size*0.5, -size*0.5, size, size};

	painter->drawRect(rect);
	QString texts[]{"X", "Y", "Z"};
	if(m_dir <= 2)
		painter->drawText(rect, Qt::AlignCenter, texts[m_dir]);
}


/**
 * get gate operator
 */
t_mat PauliGate::GetOperator() const
{
	return m::su2_matrix<t_mat>(m_dir);
}


bool PauliGate::IsOk() const
{
	return m_dir < 3;
}



ComponentConfigs PauliGate::GetConfig() const
{
	ComponentConfigs cfgs;
	cfgs.name = GetName();

	cfgs.configs = std::vector<ComponentConfig>
	{{
		ComponentConfig
		{
			.key = "dir",
			.value = GetDirection(),
			.description = "Direction",
			.min_value = t_uint(0),
			.max_value = t_uint(2),
		},
	}};

	return cfgs;
}


void PauliGate::SetConfig(const ComponentConfigs& configs)
{
	for(const ComponentConfig& cfg : configs.configs)
	{
		if(cfg.key == "dir")
		{
			t_uint dir = std::get<t_uint>(cfg.value);
			SetDirection(dir);
		}
	}
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// SU(2) rotation gate
// @see https://en.wikipedia.org/wiki/Quantum_logic_gate#Rotation_operator_gates
// ----------------------------------------------------------------------------
RotationGate::RotationGate()
{
	setPos(QPointF{0,0});
	setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}


RotationGate::~RotationGate()
{
}


QuantumComponentItem* RotationGate::clone() const
{
	RotationGate *item = new RotationGate{};

	item->SetDirection(this->GetDirection());
	item->SetAngle(this->GetAngle());

	return item;
}


QRectF RotationGate::boundingRect() const
{
	t_real w = g_raster_size;
	t_real h = g_raster_size;

	return QRectF{-g_raster_size*0.5, -g_raster_size*0.5, w, h};
}


void RotationGate::paint(QPainter *painter,
	const QStyleOptionGraphicsItem*, QWidget*)
{
	const QColor& colour_fg = get_foreground_colour();
	const QColor& colour_bg = get_background_colour();

	QPen pen(colour_fg);
	pen.setWidthF(1.);

	QBrush brush{Qt::SolidPattern};
	brush.setColor(colour_bg);

	//QFont font = painter->font();
	//font.setPointSize(g_raster_size*0.5);
	//painter->setFont(font);

	painter->setPen(pen);
	painter->setBrush(brush);

	t_real size = g_raster_size*0.66;
	QRectF rect{-size*0.5, -size*0.5, size, size};

	painter->drawRect(rect);
	QString texts[]{"Rx", "Ry", "Rz"};
	if(m_dir <= 2)
		painter->drawText(rect, Qt::AlignCenter, texts[m_dir]);
}


/**
 * get gate operator
 */
t_mat RotationGate::GetOperator() const
{
	return m::su2_rot<t_mat, t_cplx, t_real>(m_dir, m_angle);
}


bool RotationGate::IsOk() const
{
	return m_dir < 3;
}



ComponentConfigs RotationGate::GetConfig() const
{
	ComponentConfigs cfgs;
	cfgs.name = GetName();

	cfgs.configs = std::vector<ComponentConfig>
	{{
		ComponentConfig
		{
			.key = "dir",
			.value = GetDirection(),
			.description = "Direction",
			.min_value = t_uint(0),
			.max_value = t_uint(2),
		},
		ComponentConfig
		{
			.key = "angle",
			.value = GetAngle(),
			.description = "Angle",
			.min_value = t_real(0.),
			.max_value = t_real(4. * m::pi<t_real>),
			.is_phase = true,
		},
	}};

	return cfgs;
}


void RotationGate::SetConfig(const ComponentConfigs& configs)
{
	for(const ComponentConfig& cfg : configs.configs)
	{
		if(cfg.key == "dir")
		{
			t_uint dir = std::get<t_uint>(cfg.value);
			SetDirection(dir);
		}
		if(cfg.key == "angle")
		{
			t_real angle = std::get<t_real>(cfg.value);
			SetAngle(angle);
		}
	}
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// Phase gate
// @see https://en.wikipedia.org/wiki/Quantum_logic_gate#Phase_shift_gates
// ----------------------------------------------------------------------------
PhaseGate::PhaseGate()
{
	setPos(QPointF{0,0});
	setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}


PhaseGate::~PhaseGate()
{
}


QuantumComponentItem* PhaseGate::clone() const
{
	PhaseGate *item = new PhaseGate{};

	item->SetPhase(this->GetPhase());

	return item;
}


QRectF PhaseGate::boundingRect() const
{
	t_real w = g_raster_size;
	t_real h = g_raster_size;

	return QRectF{-g_raster_size*0.5, -g_raster_size*0.5, w, h};
}


void PhaseGate::paint(QPainter *painter,
	const QStyleOptionGraphicsItem*, QWidget*)
{
	const QColor& colour_fg = get_foreground_colour();
	const QColor& colour_bg = get_background_colour();

	QPen pen(colour_fg);
	pen.setWidthF(1.);

	QBrush brush{Qt::SolidPattern};
	brush.setColor(colour_bg);

	//QFont font = painter->font();
	//font.setPointSize(g_raster_size*0.5);
	//painter->setFont(font);

	painter->setPen(pen);
	painter->setBrush(brush);

	t_real size = g_raster_size*0.66;
	QRectF rect{-size*0.5, -size*0.5, size, size};

	painter->drawRect(rect);
	painter->drawText(rect, Qt::AlignCenter, "P");
}


/**
 * get gate operator
 */
t_mat PhaseGate::GetOperator() const
{
	t_mat op = m::phasegate<t_mat, t_cplx>(m_phase);

	//using namespace m_ops;
	//std::cout << m_phase << ", " << op << std::endl;

	return op;
}


ComponentConfigs PhaseGate::GetConfig() const
{
	ComponentConfigs cfgs;
	cfgs.name = GetName();

	cfgs.configs = std::vector<ComponentConfig>
	{{
		ComponentConfig
		{
			.key = "phase",
			.value = GetPhase(),
			.description = "Phase",
			.min_value = t_real(0.),
			.max_value = t_real(2. * m::pi<t_real>),
			.is_phase = true,
		},
	}};

	return cfgs;
}


void PhaseGate::SetConfig(const ComponentConfigs& configs)
{
	for(const ComponentConfig& cfg : configs.configs)
	{
		if(cfg.key == "phase")
		{
			t_real phase = std::get<t_real>(cfg.value);
			SetPhase(phase);
		}
	}
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// Unitary gate
// @see https://en.wikipedia.org/wiki/Quantum_logic_gate#Controlled_gates
// ----------------------------------------------------------------------------
UnitaryGate::UnitaryGate()
{
	setPos(QPointF{0,0});
	setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}


UnitaryGate::~UnitaryGate()
{
}


QuantumComponentItem* UnitaryGate::clone() const
{
	UnitaryGate *item = new UnitaryGate{};

	item->SetMatrix(this->GetMatrix());

	return item;
}


QRectF UnitaryGate::boundingRect() const
{
	t_real w = g_raster_size;
	t_real h = g_raster_size;

	return QRectF{-g_raster_size*0.5, -g_raster_size*0.5, w, h};
}


void UnitaryGate::paint(QPainter *painter,
	const QStyleOptionGraphicsItem*, QWidget*)
{
	const QColor& colour_fg = get_foreground_colour();
	const QColor& colour_bg = get_background_colour();

	QPen pen(colour_fg);
	pen.setWidthF(1.);

	QBrush brush{Qt::SolidPattern};
	brush.setColor(colour_bg);

	//QFont font = painter->font();
	//font.setPointSize(g_raster_size*0.5);
	//painter->setFont(font);

	painter->setPen(pen);
	painter->setBrush(brush);

	t_real size = g_raster_size*0.66;
	QRectF rect{-size*0.5, -size*0.5, size, size};

	painter->drawRect(rect);
	painter->drawText(rect, Qt::AlignCenter, "U");
}


/**
 * get gate operator
 */
t_mat UnitaryGate::GetOperator() const
{
	return m_mat;
}


ComponentConfigs UnitaryGate::GetConfig() const
{
	ComponentConfigs cfgs;
	cfgs.name = GetName();

	cfgs.configs = std::vector<ComponentConfig>
	{{
		ComponentConfig
		{
			.key = "m00",
			.value = GetComponent00(),
			.description = "Matrix element (0,0)",
		},
		ComponentConfig
		{
			.key = "m01",
			.value = GetComponent01(),
			.description = "Matrix element (0,1)",
		},
		ComponentConfig
		{
			.key = "m10",
			.value = GetComponent10(),
			.description = "Matrix element (1,0)",
		},
		ComponentConfig
		{
			.key = "m11",
			.value = GetComponent11(),
			.description = "Matrix element (1,1)",
		},
	}};

	return cfgs;
}


void UnitaryGate::SetConfig(const ComponentConfigs& configs)
{
	for(const ComponentConfig& cfg : configs.configs)
	{
		if(cfg.key == "m00")
		{
			t_cplx m00 = std::get<t_cplx>(cfg.value);
			SetComponent00(m00);
		}
		else if(cfg.key == "m01")
		{
			t_cplx m01 = std::get<t_cplx>(cfg.value);
			SetComponent01(m01);
		}
		else if(cfg.key == "m10")
		{
			t_cplx m10 = std::get<t_cplx>(cfg.value);
			SetComponent10(m10);
		}
		else if(cfg.key == "m11")
		{
			t_cplx m11 = std::get<t_cplx>(cfg.value);
			SetComponent11(m11);
		}
	}
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// SWAP gate
// @see https://en.wikipedia.org/wiki/Quantum_logic_gate#Swap_gate
// ----------------------------------------------------------------------------
SwapGate::SwapGate()
{
	setPos(QPointF{0,0});
	setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}


SwapGate::~SwapGate()
{
}


QuantumComponentItem* SwapGate::clone() const
{
	SwapGate *item = new SwapGate{};

	item->SetNumQBits(this->GetNumQBits());
	item->SetSourceBitPos(this->GetSourceBitPos());
	item->SetTargetBitPos(this->GetTargetBitPos());

	return item;
}


QRectF SwapGate::boundingRect() const
{
	t_real w = g_raster_size;
	t_real h = t_real(m_num_qbits) * g_raster_size;

	return QRectF{-g_raster_size*0.5, -g_raster_size*0.5, w, h};
}


void SwapGate::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	const QColor& colour_fg = get_foreground_colour();
	const QColor& colour_bg = get_background_colour();


	// background rect
	QBrush brushBg{Qt::SolidPattern};
	brushBg.setColor(colour_bg);
	painter->setBrush(brushBg);
	painter->setPen(Qt::NoPen);

	painter->drawRect(boundingRect());


	// crossing qubits
	QPen penLine(colour_fg);
	penLine.setWidthF(1.5);

	painter->setBrush(Qt::NoBrush);
	painter->setPen(penLine);

	const t_real border_pos = g_raster_size*0.5;
	const t_real x_pos = g_raster_size*0.15;

	painter->drawLine(
		QPointF(-border_pos, m_source_bit_pos*g_raster_size),
		QPointF(-x_pos, m_source_bit_pos*g_raster_size));
	painter->drawLine(
		QPointF(x_pos, m_source_bit_pos*g_raster_size),
		QPointF(border_pos, m_source_bit_pos*g_raster_size));

	painter->drawLine(
		QPointF(-border_pos, m_target_bit_pos*g_raster_size),
		QPointF(-x_pos, m_target_bit_pos*g_raster_size));
	painter->drawLine(
		QPointF(x_pos, m_target_bit_pos*g_raster_size),
		QPointF(border_pos, m_target_bit_pos*g_raster_size));

	painter->drawLine(
		QPointF(-x_pos, m_source_bit_pos*g_raster_size),
		QPointF(x_pos, m_target_bit_pos*g_raster_size));
	painter->drawLine(
		QPointF(-x_pos, m_target_bit_pos*g_raster_size),
		QPointF(x_pos, m_source_bit_pos*g_raster_size));


	// non-crossing qubits
	for(t_uint bit=0; bit<m_num_qbits; ++bit)
	{
		if(bit == m_source_bit_pos || bit == m_target_bit_pos)
			continue;

		painter->drawLine(
			QPointF(-border_pos, bit*g_raster_size),
			QPointF(border_pos, bit*g_raster_size));
	}
}


/**
 * get gate operator
 */
t_mat SwapGate::GetOperator() const
{
	return m::swap_nqbits<t_mat>(m_num_qbits,
		m_source_bit_pos, m_target_bit_pos,
		g_reverse_state_numbering);
}


bool SwapGate::IsOk() const
{
	return
		m_source_bit_pos < m_num_qbits &&
		m_target_bit_pos < m_num_qbits;
}


ComponentConfigs SwapGate::GetConfig() const
{
	ComponentConfigs cfgs;
	cfgs.name = GetName();

	cfgs.configs = std::vector<ComponentConfig>
	{{
		ComponentConfig
		{
			.key = "num_qbits",
			.value = GetNumQBits(),
			.description = "Number of qubits",
			.min_value = GetMinNumQBits(),
		},
		ComponentConfig
		{
			.key = "source_bit_pos",
			.value = GetSourceBitPos(),
			.description = "Source qubit position",
			.min_value = t_uint(0),
			.max_value = t_uint(GetNumQBits() - 1)
		},
		ComponentConfig
		{
			.key = "target_bit_pos",
			.value = GetTargetBitPos(),
			.description = "Target qubit position",
			.min_value = t_uint(0),
			.max_value = t_uint(GetNumQBits() - 1)
		},
	}};

	return cfgs;
}


void SwapGate::SetConfig(const ComponentConfigs& configs)
{
	for(const ComponentConfig& cfg : configs.configs)
	{
		if(cfg.key == "num_qbits")
		{
			t_uint bits = std::get<t_uint>(cfg.value);
			SetNumQBits(bits);
		}
		else if(cfg.key == "source_bit_pos")
		{
			t_uint bits = std::get<t_uint>(cfg.value);
			SetSourceBitPos(bits);
		}
		else if(cfg.key == "target_bit_pos")
		{
			t_uint bits = std::get<t_uint>(cfg.value);
			SetTargetBitPos(bits);
		}
	}
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// CNOT gate
// @see https://en.wikipedia.org/wiki/Controlled_NOT_gate
// ----------------------------------------------------------------------------
CNotGate::CNotGate()
{
	setPos(QPointF{0,0});
	setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}


CNotGate::~CNotGate()
{
}


QuantumComponentItem* CNotGate::clone() const
{
	CNotGate *item = new CNotGate{};

	item->SetNumQBits(this->GetNumQBits());
	item->SetControlBitPos(this->GetControlBitPos());
	item->SetTargetBitPos(this->GetTargetBitPos());

	return item;
}


QRectF CNotGate::boundingRect() const
{
	t_real w = g_raster_size;
	t_real h = t_real(m_num_qbits) * g_raster_size;

	return QRectF{-g_raster_size*0.5, -g_raster_size*0.5, w, h};
}


void CNotGate::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	const QColor& colour_fg = get_foreground_colour();
	const QColor& colour_bg = get_background_colour();

	std::array<QColor, 2> colours =
	{
		colour_fg,
		lerp(colour_fg, colour_bg, 0.2),
	};

	QRadialGradient grad{};
	grad.setCenter(0., 0.);
	grad.setRadius(m_control_bit_radius);

	for(std::size_t col=0; col<colours.size(); ++col)
		grad.setColorAt(col/double(colours.size()-1), colours[col]);


	QPen penLine(colour_fg);
	QPen penGrad(*colours.rbegin());
	penLine.setWidthF(1.);
	penGrad.setWidthF(1.);


	// control bit
	painter->setBrush(grad);
	painter->setPen(penGrad);

	QTransform trafo_orig = painter->worldTransform();
	painter->translate(0., m_control_bit_pos*g_raster_size);
	painter->drawEllipse(
		-m_control_bit_radius/2., -m_control_bit_radius/2.,
		m_control_bit_radius, m_control_bit_radius);
	painter->setWorldTransform(trafo_orig);


	// target bit
	painter->setBrush(Qt::NoBrush);
	painter->setPen(penLine);

	trafo_orig = painter->worldTransform();
	painter->translate(0., m_target_bit_pos*g_raster_size);
	painter->drawEllipse(
		-m_target_bit_radius/2., -m_target_bit_radius/2.,
		m_target_bit_radius, m_target_bit_radius);

	painter->drawLine(
		QPointF(-m_target_bit_radius/2., 0.),
		QPointF(m_target_bit_radius/2., 0.));
	painter->drawLine(
		QPointF(0., -m_target_bit_radius/2.),
		QPointF(0., m_target_bit_radius/2.));
	painter->setWorldTransform(trafo_orig);


	// connecting vertical line
	painter->drawLine(
		QPointF(0., m_control_bit_pos*g_raster_size),
		QPointF(0., m_target_bit_pos*g_raster_size));
}


/**
 * get gate operator
 */
t_mat CNotGate::GetOperator() const
{
	return m::cnot_nqbits<t_mat>(m_num_qbits,
		m_control_bit_pos, m_target_bit_pos,
		g_reverse_state_numbering);
}


bool CNotGate::IsOk() const
{
	return
		m_control_bit_pos < m_num_qbits &&
		m_target_bit_pos < m_num_qbits;
}


ComponentConfigs CNotGate::GetConfig() const
{
	ComponentConfigs cfgs;
	cfgs.name = GetName();

	cfgs.configs = std::vector<ComponentConfig>
	{{
		ComponentConfig
		{
			.key = "num_qbits",
			.value = GetNumQBits(),
			.description = "Number of qubits",
			.min_value = GetMinNumQBits(),
		},
		ComponentConfig
		{
			.key = "control_bit_pos",
			.value = GetControlBitPos(),
			.description = "Control qubit position",
			.min_value = t_uint(0),
			.max_value = t_uint(GetNumQBits() - 1)
		},
		ComponentConfig
		{
			.key = "target_bit_pos",
			.value = GetTargetBitPos(),
			.description = "Target qubit position",
			.min_value = t_uint(0),
			.max_value = t_uint(GetNumQBits() - 1)
		},
	}};

	return cfgs;
}


void CNotGate::SetConfig(const ComponentConfigs& configs)
{
	for(const ComponentConfig& cfg : configs.configs)
	{
		if(cfg.key == "num_qbits")
		{
			t_uint bits = std::get<t_uint>(cfg.value);
			SetNumQBits(bits);
		}
		else if(cfg.key == "control_bit_pos")
		{
			t_uint bits = std::get<t_uint>(cfg.value);
			SetControlBitPos(bits);
		}
		else if(cfg.key == "target_bit_pos")
		{
			t_uint bits = std::get<t_uint>(cfg.value);
			SetTargetBitPos(bits);
		}
	}
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// CZ gate
// @see https://en.wikipedia.org/wiki/Quantum_logic_gate#Controlled_gates
// ----------------------------------------------------------------------------
CZGate::CZGate()
{
	setPos(QPointF{0,0});
	setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}


CZGate::~CZGate()
{
}


QuantumComponentItem* CZGate::clone() const
{
	CZGate *item = new CZGate{};

	item->SetNumQBits(this->GetNumQBits());
	item->SetControlBitPos(this->GetControlBitPos());
	item->SetTargetBitPos(this->GetTargetBitPos());

	return item;
}


QRectF CZGate::boundingRect() const
{
	t_real w = g_raster_size;
	t_real h = t_real(m_num_qbits) * g_raster_size;

	return QRectF{-g_raster_size*0.5, -g_raster_size*0.5, w, h};
}


void CZGate::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	const QColor& colour_fg = get_foreground_colour();
	const QColor& colour_bg = get_background_colour();

	std::array<QColor, 2> colours =
	{
		colour_fg,
		lerp(colour_fg, colour_bg, 0.2),
	};

	QRadialGradient grad{};
	grad.setCenter(0., 0.);
	grad.setRadius(m_control_bit_radius);

	for(std::size_t col=0; col<colours.size(); ++col)
		grad.setColorAt(col/double(colours.size()-1), colours[col]);


	QPen penLine(colour_fg);
	QPen penGrad(*colours.rbegin());
	penLine.setWidthF(1.);
	penGrad.setWidthF(1.);


	// control bit
	painter->setBrush(grad);
	painter->setPen(penGrad);

	QTransform trafo_orig = painter->worldTransform();
	painter->translate(0., m_control_bit_pos*g_raster_size);
	painter->drawEllipse(
		-m_control_bit_radius/2., -m_control_bit_radius/2.,
		m_control_bit_radius, m_control_bit_radius);
	painter->setWorldTransform(trafo_orig);


	// target bit
	painter->translate(0., m_target_bit_pos*g_raster_size);
	painter->drawEllipse(
		-m_target_bit_radius/2., -m_target_bit_radius/2.,
		m_target_bit_radius, m_target_bit_radius);
	painter->setWorldTransform(trafo_orig);


	// connecting vertical line
	painter->drawLine(
		QPointF(0., m_control_bit_pos*g_raster_size),
		QPointF(0., m_target_bit_pos*g_raster_size));
}


/**
 * get gate operator
 */
t_mat CZGate::GetOperator() const
{
	return m::cz_nqbits<t_mat>(m_num_qbits,
		m_control_bit_pos, m_target_bit_pos,
		g_reverse_state_numbering);
}


bool CZGate::IsOk() const
{
	return
		m_control_bit_pos < m_num_qbits &&
		m_target_bit_pos < m_num_qbits;
}


ComponentConfigs CZGate::GetConfig() const
{
	ComponentConfigs cfgs;
	cfgs.name = GetName();

	cfgs.configs = std::vector<ComponentConfig>
	{{
		ComponentConfig
		{
			.key = "num_qbits",
			.value = GetNumQBits(),
			.description = "Number of qubits",
			.min_value = GetMinNumQBits(),
		},
		ComponentConfig
		{
			.key = "control_bit_pos",
			.value = GetControlBitPos(),
			.description = "Control qubit position",
			.min_value = t_uint(0),
			.max_value = t_uint(GetNumQBits() - 1)
		},
		ComponentConfig
		{
			.key = "target_bit_pos",
			.value = GetTargetBitPos(),
			.description = "Target qubit position",
			.min_value = t_uint(0),
			.max_value = t_uint(GetNumQBits() - 1)
		},
	}};

	return cfgs;
}


void CZGate::SetConfig(const ComponentConfigs& configs)
{
	for(const ComponentConfig& cfg : configs.configs)
	{
		if(cfg.key == "num_qbits")
		{
			t_uint bits = std::get<t_uint>(cfg.value);
			SetNumQBits(bits);
		}
		else if(cfg.key == "control_bit_pos")
		{
			t_uint bits = std::get<t_uint>(cfg.value);
			SetControlBitPos(bits);
		}
		else if(cfg.key == "target_bit_pos")
		{
			t_uint bits = std::get<t_uint>(cfg.value);
			SetTargetBitPos(bits);
		}
	}
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// Controlled unitary gate
// @see https://en.wikipedia.org/wiki/Quantum_logic_gate#Controlled_gates
// ----------------------------------------------------------------------------
CUnitaryGate::CUnitaryGate()
{
	setPos(QPointF{0,0});
	setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}


CUnitaryGate::~CUnitaryGate()
{
}


QuantumComponentItem* CUnitaryGate::clone() const
{
	CUnitaryGate *item = new CUnitaryGate{};

	item->SetNumQBits(this->GetNumQBits());
	item->SetControlBitPos(this->GetControlBitPos());
	item->SetTargetBitPos(this->GetTargetBitPos());
	item->SetMatrix(this->GetMatrix());

	return item;
}


QRectF CUnitaryGate::boundingRect() const
{
	t_real w = g_raster_size;
	t_real h = t_real(m_num_qbits) * g_raster_size;

	return QRectF{-g_raster_size*0.5, -g_raster_size*0.5, w, h};
}


void CUnitaryGate::paint(QPainter *painter,
	const QStyleOptionGraphicsItem*, QWidget*)
{
	const QColor& colour_fg = get_foreground_colour();
	const QColor& colour_bg = get_background_colour();

	std::array<QColor, 2> colours =
	{
		colour_fg,
		lerp(colour_fg, colour_bg, 0.2),
	};

	QRadialGradient grad{};
	grad.setCenter(0., 0.);
	grad.setRadius(m_control_bit_radius);

	for(std::size_t col=0; col<colours.size(); ++col)
		grad.setColorAt(col/double(colours.size()-1), colours[col]);


	QPen penLine(colour_fg);
	QPen penGrad(*colours.rbegin());
	penLine.setWidthF(1.);
	penGrad.setWidthF(1.);


	// control bit
	painter->setBrush(grad);
	painter->setPen(penGrad);

	QTransform trafo_orig = painter->worldTransform();
	painter->translate(0., m_control_bit_pos*g_raster_size);
	painter->drawEllipse(
		-m_control_bit_radius/2., -m_control_bit_radius/2.,
		m_control_bit_radius, m_control_bit_radius);
	painter->setWorldTransform(trafo_orig);


	// connecting vertical line
	painter->drawLine(
		QPointF(0., m_control_bit_pos*g_raster_size),
		QPointF(0., m_target_bit_pos*g_raster_size));


	// target bit
	painter->translate(0., m_target_bit_pos*g_raster_size);
	QPen pen(colour_fg);
	pen.setWidthF(1.);

	QBrush brush{Qt::SolidPattern};
	brush.setColor(colour_bg);

	painter->setPen(pen);
	painter->setBrush(brush);

	t_real size = g_raster_size*0.66;
	QRectF rect{-size*0.5, -size*0.5, size, size};

	painter->drawRect(rect);
	painter->drawText(rect, Qt::AlignCenter, "U");
	painter->setWorldTransform(trafo_orig);
}


/**
 * get gate operator
 */
t_mat CUnitaryGate::GetOperator() const
{
	return m::cunitary_nqbits<t_mat>(m_mat,
		m_num_qbits,
		m_control_bit_pos, m_target_bit_pos,
		g_reverse_state_numbering);
}


ComponentConfigs CUnitaryGate::GetConfig() const
{
	ComponentConfigs cfgs;
	cfgs.name = GetName();

	cfgs.configs = std::vector<ComponentConfig>
	{{
		ComponentConfig
		{
			.key = "num_qbits",
			.value = GetNumQBits(),
			.description = "Number of qubits",
			.min_value = GetMinNumQBits(),
		},
		ComponentConfig
		{
			.key = "control_bit_pos",
			.value = GetControlBitPos(),
			.description = "Control qubit position",
			.min_value = t_uint(0),
			.max_value = t_uint(GetNumQBits() - 1)
		},
		ComponentConfig
		{
			.key = "target_bit_pos",
			.value = GetTargetBitPos(),
			.description = "Target qubit position",
			.min_value = t_uint(0),
			.max_value = t_uint(GetNumQBits() - 1)
		},
		ComponentConfig
		{
			.key = "m00",
			.value = GetComponent00(),
			.description = "Matrix element (0,0)",
		},
		ComponentConfig
		{
			.key = "m01",
			.value = GetComponent01(),
			.description = "Matrix element (0,1)",
		},
		ComponentConfig
		{
			.key = "m10",
			.value = GetComponent10(),
			.description = "Matrix element (1,0)",
		},
		ComponentConfig
		{
			.key = "m11",
			.value = GetComponent11(),
			.description = "Matrix element (1,1)",
		},
	}};

	return cfgs;
}


void CUnitaryGate::SetConfig(const ComponentConfigs& configs)
{
	for(const ComponentConfig& cfg : configs.configs)
	{
		if(cfg.key == "num_qbits")
		{
			t_uint bits = std::get<t_uint>(cfg.value);
			SetNumQBits(bits);
		}
		else if(cfg.key == "control_bit_pos")
		{
			t_uint bits = std::get<t_uint>(cfg.value);
			SetControlBitPos(bits);
		}
		else if(cfg.key == "target_bit_pos")
		{
			t_uint bits = std::get<t_uint>(cfg.value);
			SetTargetBitPos(bits);
		}
		else if(cfg.key == "m00")
		{
			t_cplx m00 = std::get<t_cplx>(cfg.value);
			SetComponent00(m00);
		}
		else if(cfg.key == "m01")
		{
			t_cplx m01 = std::get<t_cplx>(cfg.value);
			SetComponent01(m01);
		}
		else if(cfg.key == "m10")
		{
			t_cplx m10 = std::get<t_cplx>(cfg.value);
			SetComponent10(m10);
		}
		else if(cfg.key == "m11")
		{
			t_cplx m11 = std::get<t_cplx>(cfg.value);
			SetComponent11(m11);
		}
	}
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// Toffoli gate
// @see https://en.wikipedia.org/wiki/Toffoli_gate
// ----------------------------------------------------------------------------
ToffoliGate::ToffoliGate()
{
	setPos(QPointF{0,0});
	setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}


ToffoliGate::~ToffoliGate()
{
}


QuantumComponentItem* ToffoliGate::clone() const
{
	ToffoliGate *item = new ToffoliGate{};

	item->SetNumQBits(this->GetNumQBits());
	item->SetControlBit1Pos(this->GetControlBit1Pos());
	item->SetControlBit2Pos(this->GetControlBit2Pos());
	item->SetTargetBitPos(this->GetTargetBitPos());

	return item;
}


QRectF ToffoliGate::boundingRect() const
{
	t_real w = g_raster_size;
	t_real h = t_real(m_num_qbits) * g_raster_size;

	return QRectF{-g_raster_size*0.5, -g_raster_size*0.5, w, h};
}


void ToffoliGate::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	const QColor& colour_fg = get_foreground_colour();
	const QColor& colour_bg = get_background_colour();

	std::array<QColor, 2> colours =
	{
		colour_fg,
		lerp(colour_fg, colour_bg, 0.2),
	};

	QRadialGradient grad{};
	grad.setCenter(0., 0.);
	grad.setRadius(m_control_bit_radius);

	for(std::size_t col=0; col<colours.size(); ++col)
		grad.setColorAt(col/double(colours.size()-1), colours[col]);


	QPen penLine(colour_fg);
	QPen penGrad(*colours.rbegin());
	penLine.setWidthF(1.);
	penGrad.setWidthF(1.);


	// control bit 1
	painter->setBrush(grad);
	painter->setPen(penGrad);

	QTransform trafo_orig = painter->worldTransform();
	painter->translate(0., m_control_bit_1_pos*g_raster_size);
	painter->drawEllipse(
		-m_control_bit_radius/2., -m_control_bit_radius/2.,
		m_control_bit_radius, m_control_bit_radius);
	painter->setWorldTransform(trafo_orig);


	// control bit 2
	painter->setBrush(grad);
	painter->setPen(penGrad);

	trafo_orig = painter->worldTransform();
	painter->translate(0., m_control_bit_2_pos*g_raster_size);
	painter->drawEllipse(
		-m_control_bit_radius/2., -m_control_bit_radius/2.,
		m_control_bit_radius, m_control_bit_radius);
	painter->setWorldTransform(trafo_orig);


	// target bit
	painter->setBrush(Qt::NoBrush);
	painter->setPen(penLine);

	trafo_orig = painter->worldTransform();
	painter->translate(0., m_target_bit_pos*g_raster_size);
	painter->drawEllipse(
		-m_target_bit_radius/2., -m_target_bit_radius/2.,
		m_target_bit_radius, m_target_bit_radius);

	painter->drawLine(
		QPointF(-m_target_bit_radius/2., 0.),
		QPointF(m_target_bit_radius/2., 0.));
	painter->drawLine(
		QPointF(0., -m_target_bit_radius/2.),
		QPointF(0., m_target_bit_radius/2.));
	painter->setWorldTransform(trafo_orig);


	// connecting vertical line
	t_real elem_pos[] = 
	{
		m_control_bit_1_pos * g_raster_size,
		m_control_bit_2_pos * g_raster_size,
		m_target_bit_pos * g_raster_size,
	};

	auto [min_elem_y, max_elem_y] = std::minmax_element(
		elem_pos, elem_pos+sizeof(elem_pos)/sizeof(*elem_pos));

	painter->drawLine(
		QPointF(0., *min_elem_y),
		QPointF(0., *max_elem_y));
}


/**
 * get gate operator
 */
t_mat ToffoliGate::GetOperator() const
{
	return m::toffoli_nqbits<t_mat>(m_num_qbits,
		m_control_bit_1_pos, m_control_bit_2_pos,
		m_target_bit_pos,
		g_reverse_state_numbering);
}


bool ToffoliGate::IsOk() const
{
	return
		m_control_bit_1_pos < m_num_qbits &&
		m_control_bit_2_pos < m_num_qbits &&
		m_target_bit_pos < m_num_qbits;
}


ComponentConfigs ToffoliGate::GetConfig() const
{
	ComponentConfigs cfgs;
	cfgs.name = GetName();

	cfgs.configs = std::vector<ComponentConfig>
	{{
		ComponentConfig
		{
			.key = "num_qbits",
			.value = GetNumQBits(),
			.description = "Number of qubits",
			.min_value = GetMinNumQBits(),
		},
		ComponentConfig
		{
			.key = "control_bit_1_pos",
			.value = GetControlBit1Pos(),
			.description = "Control qubit 1 position",
			.min_value = t_uint(0),
			.max_value = t_uint(GetNumQBits() - 1)
		},
		ComponentConfig
		{
			.key = "control_bit_2_pos",
			.value = GetControlBit2Pos(),
			.description = "Control qubit 2 position",
			.min_value = t_uint(0),
			.max_value = t_uint(GetNumQBits() - 1)
		},
		ComponentConfig
		{
			.key = "target_bit_pos",
			.value = GetTargetBitPos(),
			.description = "Target qubit position",
			.min_value = t_uint(0),
			.max_value = t_uint(GetNumQBits() - 1)
		},
	}};

	return cfgs;
}


void ToffoliGate::SetConfig(const ComponentConfigs& configs)
{
	for(const ComponentConfig& cfg : configs.configs)
	{
		if(cfg.key == "num_qbits")
		{
			t_uint bits = std::get<t_uint>(cfg.value);
			SetNumQBits(bits);
		}
		else if(cfg.key == "control_bit_1_pos")
		{
			t_uint bits = std::get<t_uint>(cfg.value);
			SetControlBit1Pos(bits);
		}
		else if(cfg.key == "control_bit_2_pos")
		{
			t_uint bits = std::get<t_uint>(cfg.value);
			SetControlBit2Pos(bits);
		}
		else if(cfg.key == "target_bit_pos")
		{
			t_uint bits = std::get<t_uint>(cfg.value);
			SetTargetBitPos(bits);
		}
	}
}
// ----------------------------------------------------------------------------


/**
 * create the component matching the given id
 */
template<class t_comps, std::size_t ...indices>
static constexpr inline QuantumComponentItem*
create_matching_comp(const std::string& id,
	const std::index_sequence<indices...>&)
{
	QuantumComponentItem *comp = nullptr;

	// loop through all component classes
	(
		[&id, &comp]()
		{
			using t_comp = std::tuple_element_t<indices, t_comps>;

			// create the component with the matching id
			// if the component hasn't already been created
			if(!comp && id == std::string_view{t_comp::GetStaticIdent()})
				comp = new t_comp();
		}(),
	...); // call a sequence of lambda functions

	return comp;
}


/**
 * factory function to create the component with the given id
 */
QuantumComponentItem* QuantumComponentItem::create(const std::string& id)
{
	// iterate through all component classes and create the matching one
	constexpr const std::size_t num_comps = std::tuple_size<t_all_components>();
	constexpr const auto comp_indices = std::make_index_sequence<num_comps>();

	return create_matching_comp<t_all_components>(id, comp_indices);
}
