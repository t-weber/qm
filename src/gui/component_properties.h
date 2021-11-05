/**
 * circuit component properties
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_COMPONENT_PROPERTIES_H__
#define __QM_COMPONENT_PROPERTIES_H__

//#define DOCKWIDGETWRAPPER_USE_DYN_WIDGET

#include <QtWidgets/QWidget>
#include <QtWidgets/QDockWidget>

#include "types.h"



/**
 * widget for component properties
 */
class ComponentProperties : public QWidget
{
public:
	ComponentProperties(QWidget *parent=nullptr);

	virtual ~ComponentProperties();


private:
};


/**
 * wrapper making a widget into a dock widget
 */
template<class t_widget>
class DockWidgetWrapper : public QDockWidget
{
public:
	DockWidgetWrapper(QWidget *parent=nullptr)
		: QDockWidget(parent)
	{}

	virtual ~DockWidgetWrapper() = default;

#ifndef DOCKWIDGETWRAPPER_USE_DYN_WIDGET
	const t_widget* GetWidget() const { return &m_widget; }
	t_widget* GetWidget() { return &m_widget; }
#else
	using t_widgetptr = std::shared_ptr<t_widget>;
	t_widgetptr GetWidget() const { return m_widget; }
	void SetWidget(t_widgetptr widget) { m_widget = widget; }
#endif


private:
#ifndef DOCKWIDGETWRAPPER_USE_DYN_WIDGET
	t_widget m_widget{this};
#else
	t_widgetptr m_widget = std::make_shared<t_widget>(this);
#endif
};

#endif
