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
	t_real w = std::max(m_control_bit_radius, m_target_bit_radius);

	// centre positions
	t_real ctrl_pos = m_control_bit_pos * g_raster_size;
	t_real target_pos = m_target_bit_pos * g_raster_size;

	// outer positions
	t_real y_min = 0, y_max = 0;
	if(m_control_bit_pos < m_target_bit_pos)
	{
		y_min = ctrl_pos - m_control_bit_radius;
		y_max = target_pos + m_target_bit_radius;
	}
	else
	{
		y_min = target_pos - m_target_bit_radius;
		y_max = ctrl_pos + m_control_bit_radius;
	}

	return QRectF{-w/2., y_min, w, y_max-y_min};
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


ComponentConfigs CNotGate::GetConfig() const
{
	ComponentConfigs cfgs;
	cfgs.name = GetName();

	cfgs.configs = std::vector<ComponentConfig>
	{{
		ComponentConfig{.key = "num_qbits",
			.value = GetNumQBits(),
			.description = "Number of qubits",
			.min_value = std::size_t(2)},
		ComponentConfig{.key = "control_bit_pos",
			.value = GetControlBitPos(),
			.description = "Control qubit position",
			.min_value = std::size_t(0),
			.max_value = std::size_t(GetNumQBits() - 1)},
		ComponentConfig{.key = "target_bit_pos",
			.value = GetTargetBitPos(),
			.description = "Target qubit position",
			.min_value = std::size_t(0),
			.max_value = std::size_t(GetNumQBits() - 1)},
	}};

	return cfgs;
}


void CNotGate::SetConfig(const ComponentConfigs& configs)
{
	for(const ComponentConfig& cfg : configs.configs)
	{
		if(cfg.key == "num_qbits")
		{
			std::size_t bits = std::get<std::size_t>(cfg.value);
			SetNumQBits(bits);
		}
		else if(cfg.key == "control_bit_pos")
		{
			std::size_t bits = std::get<std::size_t>(cfg.value);
			SetControlBitPos(bits);
		}
		else if(cfg.key == "target_bit_pos")
		{
			std::size_t bits = std::get<std::size_t>(cfg.value);
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
	t_real w = std::max(m_control_bit_radius, m_target_bit_radius);

	// centre positions
	t_real pos[] =
	{
		m_control_bit_1_pos * g_raster_size,
		m_control_bit_2_pos * g_raster_size,
		m_target_bit_pos * g_raster_size,
	};

	// element radii
	t_real radii[] =
	{
		m_control_bit_radius,
		m_control_bit_radius,
		m_target_bit_radius,
	};

	//get indices of minimum and maximum element
	auto [_min, _max] = std::minmax_element(
		pos, pos+sizeof(pos)/sizeof(*pos));
	std::size_t min_idx = _min - pos;
	std::size_t max_idx = _max - pos;

	// outer positions
	t_real y_min = pos[min_idx] - radii[min_idx];
	t_real y_max = pos[max_idx] + radii[max_idx];

	return QRectF{-w/2., y_min, w, y_max-y_min};
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


ComponentConfigs ToffoliGate::GetConfig() const
{
	ComponentConfigs cfgs;
	cfgs.name = GetName();

	cfgs.configs = std::vector<ComponentConfig>
	{{
		ComponentConfig{.key = "num_qbits",
			.value = GetNumQBits(),
			.description = "Number of qubits",
			.min_value = std::size_t(3)},
		ComponentConfig{.key = "control_bit_1_pos",
			.value = GetControlBit1Pos(),
			.description = "Control qubit 1 position",
			.min_value = std::size_t(0),
			.max_value = std::size_t(GetNumQBits() - 1)},
		ComponentConfig{.key = "control_bit_2_pos",
			.value = GetControlBit2Pos(),
			.description = "Control qubit 2 position",
			.min_value = std::size_t(0),
			.max_value = std::size_t(GetNumQBits() - 1)},
		ComponentConfig{.key = "target_bit_pos",
			.value = GetTargetBitPos(),
			.description = "Target qubit position",
			.min_value = std::size_t(0),
			.max_value = std::size_t(GetNumQBits() - 1)},
	}};

	return cfgs;
}


void ToffoliGate::SetConfig(const ComponentConfigs& configs)
{
	for(const ComponentConfig& cfg : configs.configs)
	{
		if(cfg.key == "num_qbits")
		{
			std::size_t bits = std::get<std::size_t>(cfg.value);
			SetNumQBits(bits);
		}
		else if(cfg.key == "control_bit_1_pos")
		{
			std::size_t bits = std::get<std::size_t>(cfg.value);
			SetControlBit1Pos(bits);
		}
		else if(cfg.key == "control_bit_2_pos")
		{
			std::size_t bits = std::get<std::size_t>(cfg.value);
			SetControlBit2Pos(bits);
		}
		else if(cfg.key == "target_bit_pos")
		{
			std::size_t bits = std::get<std::size_t>(cfg.value);
			SetTargetBitPos(bits);
		}
	}
}
// ----------------------------------------------------------------------------
