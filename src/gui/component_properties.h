/**
 * circuit component properties
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_COMPONENT_PROPERTIES_H__
#define __QM_COMPONENT_PROPERTIES_H__

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
	{
		this->setWidget(GetWidget());
	}

	virtual ~DockWidgetWrapper() = default;

	const t_widget* GetWidget() const { return &m_widget; }
	t_widget* GetWidget() { return &m_widget; }


private:
	t_widget m_widget{this};
};

#endif
