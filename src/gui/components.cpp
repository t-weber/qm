/**
 * circuit components
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#include <QtGui/QRadialGradient>

#include "components.h"
#include "helpers.h"
#include "globals.h"
#include "settings.h"

#include "lib/qm_algos.h"

#include <string_view>
#include <tuple>



// ----------------------------------------------------------------------------
// graphical representation of a quantum component
// ----------------------------------------------------------------------------
std::tuple<t_int, t_int> QuantumComponentItem::GetGridPos() const
{
	return get_grid_indices(scenePos());
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// input qubit states
// ----------------------------------------------------------------------------
InputStates::InputStates()
{
	setPos(QPointF{0,0});
	setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}


InputStates::~InputStates()
{
}


QuantumComponentItem* InputStates::clone() const
{
	InputStates *item = new InputStates{};

	item->SetNumQBits(this->GetNumQBits());
	item->SetWidth(this->GetWidth());
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
	for(t_uint bit=0; bit<m_num_qbits; ++bit)
	{
		// write qubit state numbers
		QRectF rectText{
			-g_raster_size*0.5, bit*g_raster_size-g_raster_size*0.5,
			g_raster_size, g_raster_size};
		QString textState = QString{"|ψ%1>"}.arg(bit+1);
		painter->drawText(rectText, Qt::AlignCenter, textState);

		// draw qubit connection lines
		painter->drawLine(
			QPointF(0.5*g_raster_size, bit*g_raster_size),
			QPointF((m_width-0.5)*g_raster_size, bit*g_raster_size));
	}
}


/**
 * get total calculated operator
 */
t_mat InputStates::GetOperator() const
{
	using namespace m_ops;
	const auto& ops = GetOperators();

	if(ops.size())
	{
		t_mat op_total;

		auto iter = ops.rbegin();
		op_total = std::get<t_mat>(*iter);
		std::advance(iter, 1);

		for(; iter!=ops.rend(); std::advance(iter, 1))
			op_total = op_total * std::get<t_mat>(*iter);

		return op_total;
	}
	else
	{
		// corresponds to identity
		return t_mat{};
	}
}


t_vec InputStates::GetState() const
{
	// TODO
	return t_vec{};
};


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
			.min_value = t_uint(1)
		},
		ComponentConfig
		{
			.key = "width",
			.value = GetWidth(),
			.description = "Width",
			.min_value = t_uint(2)
		},
	}};

	return cfgs;
}


void InputStates::SetConfig(const ComponentConfigs& configs)
{
	for(const ComponentConfig& cfg : configs.configs)
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
	}
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


t_vec HadamardGate::GetState() const
{
	return t_vec{};
};


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


t_vec PauliGate::GetState() const
{
	return t_vec{};
};


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
	return m::phasegate<t_mat, t_cplx>(m_phase);
}


t_vec PhaseGate::GetState() const
{
	return t_vec{};
};


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


t_vec SwapGate::GetState() const
{
	return t_vec{};
};


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
			.min_value = t_uint(2)
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


t_vec CNotGate::GetState() const
{
	return t_vec{};
};


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
			.min_value = t_uint(2)
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


t_vec ToffoliGate::GetState() const
{
	return t_vec{};
};


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
			.min_value = t_uint(3)
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
	... ); // call a sequence of lambda functions

	return comp;
}


/**
 * factory function to create the component with the given id
 */
QuantumComponentItem* QuantumComponentItem::create(const std::string& id)
{
	// possible component classes to create
	using t_comps = std::tuple
	<
		InputStates,
		HadamardGate, PauliGate, PhaseGate,
		SwapGate, CNotGate,
		ToffoliGate
	>;

	// iterate through all component classes and create the matching one
	constexpr const std::size_t num_comps = std::tuple_size<t_comps>();
	constexpr const auto comp_indices = std::make_index_sequence<num_comps>();
	return create_matching_comp<t_comps>(id, comp_indices);
}
