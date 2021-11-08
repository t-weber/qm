/**
 * circuit components
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#include <QtGui/QRadialGradient>

#include "components.h"
#include "helpers.h"
#include "settings.h"

#include "lib/qm_algos.h"



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
	//font.setPixelSize(g_raster_size*0.5);
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


t_mat InputStates::GetOperator() const
{
	return t_mat{};
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
		ComponentConfig{.key = "num_qbits",
			.value = GetNumQBits(),
			.description = "Number of qubits",
			.min_value = t_uint(1)},
		ComponentConfig{.key = "width",
			.value = GetWidth(),
			.description = "Width",
			.min_value = t_uint(2)},
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
// ----------------------------------------------------------------------------
HadamardGate::HadamardGate()
{
	setPos(QPointF{0,0});
	setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}


HadamardGate::~HadamardGate()
{
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
	//font.setPixelSize(g_raster_size*0.5);
	//painter->setFont(font);

	painter->setPen(pen);
	painter->setBrush(brush);

	t_real size = g_raster_size*0.66;
	QRectF rect{-size*0.5, -size*0.5, size, size};

	painter->drawRect(rect);
	painter->drawText(rect, Qt::AlignCenter, "H");
}


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
// ----------------------------------------------------------------------------
PauliGate::PauliGate()
{
	setPos(QPointF{0,0});
	setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}


PauliGate::~PauliGate()
{
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
	//font.setPixelSize(g_raster_size*0.5);
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
		ComponentConfig{.key = "dir",
			.value = GetDirection(),
			.description = "Direction",
			.min_value = t_uint(0),
			.max_value = t_uint(2),},
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
// CNOT gate
// ----------------------------------------------------------------------------
CNotGate::CNotGate()
{
	setPos(QPointF{0,0});
	setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}


CNotGate::~CNotGate()
{
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


t_mat CNotGate::GetOperator() const
{
	return m::cnot_nqbits<t_mat>(m_num_qbits,
		m_control_bit_pos, m_target_bit_pos);
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
		ComponentConfig{.key = "num_qbits",
			.value = GetNumQBits(),
			.description = "Number of qubits",
			.min_value = t_uint(2)},
		ComponentConfig{.key = "control_bit_pos",
			.value = GetControlBitPos(),
			.description = "Control qubit position",
			.min_value = t_uint(0),
			.max_value = t_uint(GetNumQBits() - 1)},
		ComponentConfig{.key = "target_bit_pos",
			.value = GetTargetBitPos(),
			.description = "Target qubit position",
			.min_value = t_uint(0),
			.max_value = t_uint(GetNumQBits() - 1)},
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
// ----------------------------------------------------------------------------
ToffoliGate::ToffoliGate()
{
	setPos(QPointF{0,0});
	setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}


ToffoliGate::~ToffoliGate()
{
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


t_mat ToffoliGate::GetOperator() const
{
	return m::toffoli_nqbits<t_mat>(m_num_qbits,
		m_control_bit_1_pos, m_control_bit_2_pos,
		m_target_bit_pos);
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
		ComponentConfig{.key = "num_qbits",
			.value = GetNumQBits(),
			.description = "Number of qubits",
			.min_value = t_uint(3)},
		ComponentConfig{.key = "control_bit_1_pos",
			.value = GetControlBit1Pos(),
			.description = "Control qubit 1 position",
			.min_value = t_uint(0),
			.max_value = t_uint(GetNumQBits() - 1)},
		ComponentConfig{.key = "control_bit_2_pos",
			.value = GetControlBit2Pos(),
			.description = "Control qubit 2 position",
			.min_value = t_uint(0),
			.max_value = t_uint(GetNumQBits() - 1)},
		ComponentConfig{.key = "target_bit_pos",
			.value = GetTargetBitPos(),
			.description = "Target qubit position",
			.min_value = t_uint(0),
			.max_value = t_uint(GetNumQBits() - 1)},
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
 * factory function to create the component with the given id
 */
QuantumComponentItem* QuantumComponentItem::create(const std::string& id)
{
	QuantumComponentItem *comp = nullptr;

	if(id == "input_states")
		comp = new InputStates();
	else if(id == "hadamard")
		comp = new HadamardGate();
	else if(id == "pauli")
		comp = new PauliGate();
	else if(id == "cnot")
		comp = new CNotGate();
	else if(id == "toffoli")
		comp = new ToffoliGate();

	return comp;
}
