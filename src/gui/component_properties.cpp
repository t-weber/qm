/**
 * circuit component properties
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#include "component_properties.h"
#include "globals.h"
#include "helpers.h"

#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFrame>
#include <QtWidgets/QSpacerItem>


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

	if(m_compOperator)
		m_compOperator->SetOperator(t_mat{});
}


/**
 * a component has been selected -> show its properties
 */
void ComponentProperties::SelectedItem(const QuantumComponent *comp)
{
	Clear();
	if(!comp)
		return;

	// component configuration
	const ComponentConfigs& cfgs = comp->GetConfig();
	QLabel *labelName = new QLabel(cfgs.name.c_str(), this);
	QFont fontName = labelName->font();
	fontName.setBold(true);
	labelName->setFont(fontName);
	m_layout->addWidget(labelName, m_layout->rowCount(), 0, 1, 1);

	for(const ComponentConfig& cfg : cfgs.configs)
	{
		// description label
		QLabel *labelDescr = new QLabel(cfg.description.c_str(), this);
		m_layout->addWidget(labelDescr, m_layout->rowCount(), 0, 1, 1);

		// uint value
		if(std::holds_alternative<t_uint>(cfg.value))
		{
			QSpinBox *spinVal = new QSpinBox(this);
			spinVal->setValue(std::get<t_uint>(cfg.value));
			if(cfg.min_value)
				spinVal->setMinimum(std::get<t_uint>(*cfg.min_value));
			if(cfg.max_value)
				spinVal->setMaximum(std::get<t_uint>(*cfg.max_value));

			connect(spinVal, static_cast<void (QSpinBox::*)(int)>
				(&QSpinBox::valueChanged), [this, cfg, comp](int val) -> void
			{
				ComponentConfigs configs;

				configs.configs = std::vector<ComponentConfig>
				{{
					ComponentConfig{.key = cfg.key, .value = t_uint(val)},
				}};

				// send the changes back to the component
				emit this->SignalConfigChanged(configs);

				// also update the operator dialog every time the configuration is changed
				if(m_compOperator)
					m_compOperator->SetOperator(comp->GetOperator());
			});

			m_layout->addWidget(spinVal, m_layout->rowCount(), 0, 1, 1);
		}

		// t_real value
		else if(std::holds_alternative<t_real>(cfg.value))
		{
			t_real scale = t_real(1);
			if(cfg.is_phase)
				scale = t_real(180) / m::pi<t_real>;

			QDoubleSpinBox *spinVal = new QDoubleSpinBox(this);
			spinVal->setDecimals(g_prec_gui);
			spinVal->setValue(std::get<t_real>(cfg.value) * scale);
			if(cfg.min_value)
				spinVal->setMinimum(std::get<t_real>(*cfg.min_value)*scale);
			if(cfg.max_value)
				spinVal->setMaximum(std::get<t_real>(*cfg.max_value)*scale);

			connect(spinVal, static_cast<void (QDoubleSpinBox::*)(double)>
			(&QDoubleSpinBox::valueChanged), [this, cfg, comp, scale](double val) -> void
			{
				ComponentConfigs configs;

				configs.configs = std::vector<ComponentConfig>
				{{
					ComponentConfig{.key = cfg.key, .value = t_real(val)/scale},
				}};

				// send the changes back to the component
				emit this->SignalConfigChanged(configs);

				// also update the operator dialog every time the configuration is changed
				if(m_compOperator)
					m_compOperator->SetOperator(comp->GetOperator());
			});

			m_layout->addWidget(spinVal, m_layout->rowCount(), 0, 1, 1);
		}
	}

	if(comp->GetType() == ComponentType::STATE || comp->GetType() == ComponentType::GATE)
	{
		QFrame *line = new QFrame(this);
		line->setFrameShape(QFrame::HLine);
		m_layout->addWidget(line, m_layout->rowCount(), 0, 1, 1);

		QPushButton *btnOperator = new QPushButton("Operator...", this);
		btnOperator->setToolTip("Show Operator of this Component");
		connect(btnOperator, &QAbstractButton::clicked, [this, comp]()
		{
			if(!m_compOperator)
				m_compOperator = std::make_shared<ComponentOperator>(this);

			// the pointer to comp is still valid in this lambda function,
			// because the current item is still selected as long as its
			// properties are displayed
			m_compOperator->SetOperator(comp->GetOperator());
			show_dialog(m_compOperator.get());
		});
		m_layout->addWidget(btnOperator, m_layout->rowCount(), 0, 1, 1);

		if(comp->GetType() == ComponentType::STATE)
		{
			QPushButton *btnStates = new QPushButton("States...", this);
			btnStates->setToolTip("Show Input and Output State Vectors");
			m_layout->addWidget(btnStates, m_layout->rowCount(), 0, 1, 1);
		}
	}

	QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
	m_layout->addItem(spacer, m_layout->rowCount(), 0, 1, 1);

	// also update the operator dialog every time a new item is selected
	if(m_compOperator)
		m_compOperator->SetOperator(comp->GetOperator());
}
