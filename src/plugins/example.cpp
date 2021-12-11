/**
 * built-in circuit components
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Dec-2021
 * @license see 'LICENSE' file
 *
 * g++ -std=c++20 -Wall -Wextra -I.. -I/usr/include/qt5 -shared -fPIC -fvisibility=hidden -o example.so example.cpp
 */

#include "example.h"
#include "gui/globals.h"
#include "gui/settings.h"
#include "lib/qm_algos.h"

#include <vector>


// ----------------------------------------------------------------------------
// component
// ----------------------------------------------------------------------------
IdentityGate::IdentityGate()
{
	setPos(QPointF{0,0});
	setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}


IdentityGate::~IdentityGate()
{
}


QuantumComponentItem* IdentityGate::clone() const
{
	IdentityGate *item = new IdentityGate{};
	return item;
}


QRectF IdentityGate::boundingRect() const
{
	// TODO: test if the global variables bind to the correct symbols
	t_real w = g_raster_size;
	t_real h = g_raster_size;

	return QRectF{-g_raster_size*0.5, -g_raster_size*0.5, w, h};
}


void IdentityGate::paint(QPainter *painter,
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
	painter->drawText(rect, Qt::AlignCenter, "I");
}


/**
 * get gate operator
 */
t_mat IdentityGate::GetOperator() const
{
	return m::unit<t_mat>(2);
}


ComponentConfigs IdentityGate::GetConfig() const
{
	ComponentConfigs cfgs;
	cfgs.name = GetName();

	return cfgs;
}


void IdentityGate::SetConfig(const ComponentConfigs&)
{
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// plug-in interface
// @see https://www.boost.org/doc/libs/1_78_0/doc/html/boost_dll.html
// @see https://github.com/boostorg/dll/tree/develop/example
// ----------------------------------------------------------------------------

/**
 * list of component descriptors
 */
extern std::vector<PluginComponentDescriptor> plugin_get_component_descriptors()
{
	return std::vector<PluginComponentDescriptor>
	{{
		PluginComponentDescriptor
		{
			.ident = IdentityGate::GetStaticIdent(),
			.name = IdentityGate::GetStaticName(),
			.ty = IdentityGate::GetStaticType(),
			.min_qbits = IdentityGate::GetMinNumQBits(),
		},
	}};
}


/**
 * create the component with the given identifier
 */
extern QuantumComponentItem* plugin_create_component(const std::string& ident)
{
	if(ident == IdentityGate::GetStaticIdent())
		return new IdentityGate{};

	return nullptr;
}


#include <boost/dll/alias.hpp>

BOOST_DLL_ALIAS_SECTIONED(plugin_get_component_descriptors, QM_PLUGIN_FUNC_GET_COMP_DESCR, qm);
BOOST_DLL_ALIAS_SECTIONED(plugin_create_component, QM_PLUGIN_FUNC_CREATE_COMP, qm);
// ----------------------------------------------------------------------------
