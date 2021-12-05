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
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QDoubleSpinBox>

#include <memory>

#include "types.h"
#include "components.h"
#include "component_operator.h"
#include "component_states.h"


/**
 * widget for component properties
 */
class ComponentProperties : public QWidget
{ Q_OBJECT
public:
	ComponentProperties(QWidget *parent=nullptr);
	virtual ~ComponentProperties();

	virtual QSize sizeHint() const override
	{
		QSize size = QWidget::sizeHint();
		size.rwidth() += 128;
		return size;
	}


protected:
	void Clear();


private:
	std::shared_ptr<QWidget> m_scrollwidget{};
	std::shared_ptr<QGridLayout> m_layout{};

	std::shared_ptr<ComponentOperator> m_compOperator{};
	std::shared_ptr<ComponentStates> m_compStates{};

	std::vector<std::pair<QDoubleSpinBox*, QDoubleSpinBox*>> m_spins_qbit{};


public slots:
	// a component has been selected
	void SelectedItem(const QuantumComponent *item,
		const InputStates *associated_input_comp = nullptr);

	// new results are available
	void UpdateResults(const QuantumComponent *comp,
		const InputStates *associated_input_comp = nullptr,
		bool ok = true);


signals:
	// the current component's configuration has been changed
	void SignalConfigChanged(const ComponentConfigs& cfg);
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
