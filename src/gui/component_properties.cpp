/**
 * circuit component properties
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#include "component_properties.h"
#include "settings.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>


ComponentProperties::ComponentProperties(QWidget* parent)
	: QWidget(parent)
{
	m_layout->setContentsMargins(4, 4, 4, 4);
	m_layout->setVerticalSpacing(4);
	m_layout->setHorizontalSpacing(4);
}


ComponentProperties::~ComponentProperties()
{
}


/**
 * clear all widgets in the grid layout
 */
void ComponentProperties::Clear()
{
	while(m_layout->count())
	{
		QLayoutItem* item = m_layout->itemAt(0);
		if(!item)
			break;
		m_layout->removeItem(item);

		if(item->widget())
			delete item->widget();
		delete item;
	}
}


/**
 * a component has been selected -> show its properties
 */
void ComponentProperties::SelectedItem(const QuantumComponent* item)
{
	Clear();

	if(!item)
		return;

	// component configuration
	const ComponentConfigs& cfgs = item->GetConfig();
	int layout_y = 0;
	QLabel *labelName = new QLabel(cfgs.name.c_str(), this);
	QFont fontName = labelName->font();
	fontName.setBold(true);
	labelName->setFont(fontName);
	m_layout->addWidget(labelName, layout_y++, 0, 1, 1);

	for(const ComponentConfig& cfg : cfgs.configs)
	{
		// description label
		QLabel *labelDescr = new QLabel(cfg.description.c_str(), this);
		m_layout->addWidget(labelDescr, layout_y++, 0, 1, 1);

		// value
		QSpinBox *spinVal = new QSpinBox(this);
		spinVal->setValue(std::get<t_uint>(cfg.value));
		if(cfg.min_value)
			spinVal->setMinimum(std::get<t_uint>(*cfg.min_value));
		if(cfg.max_value)
			spinVal->setMaximum(std::get<t_uint>(*cfg.max_value));

		connect(spinVal, static_cast<void (QSpinBox::*)(int)>
			(&QSpinBox::valueChanged), [this, cfg](int val) -> void
		{
			ComponentConfigs configs;

			configs.configs = std::vector<ComponentConfig>
			{{
				ComponentConfig{.key = cfg.key, .value = t_uint(val)},
			}};

			// send the changes back to the component
			emit this->SignalConfigChanged(configs);
		});

		m_layout->addWidget(spinVal, layout_y++, 0, 1, 1);
	}

	QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
	m_layout->addItem(spacer, layout_y++, 0, 1, 1);
}
