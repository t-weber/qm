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

	if(m_compStates)
		m_compStates->SetStates(t_vec{});
}


/**
 * update the results shown in the operator and states dialogs
 */
void ComponentProperties::UpdateResults(const QuantumComponent *comp, bool /*ok*/)
{
	if(!comp)
		return;

	// update operator
	if(m_compOperator)
	{
		m_compOperator->SetOperator(comp->GetOperator());
	}

	// update states vector
	if(m_compStates && comp->GetType() == ComponentType::STATE)
	{
		const InputStates *input_comp = static_cast<const InputStates*>(comp);
		m_compStates->SetStates(input_comp->GetOutputState());
	}
}


/**
 * a component has been selected -> show its properties
 */
void ComponentProperties::SelectedItem(const QuantumComponent *comp)
{
	Clear();
	if(!comp)
		return;


	// get input state and send them to the component
	auto update_states = [this, comp]()
	{
		ComponentConfigs configs;

		// get all qubit configurations
		for(t_uint bit=0; bit<m_spins_qbit.size(); ++bit)
		{
			// down
			configs.qbit_configs.emplace_back(QBitConfig
			{
				.bit = bit,
				.component = 0,
				.value = std::get<0>(m_spins_qbit[bit])->value(),
			});

			// up
			configs.qbit_configs.emplace_back(QBitConfig
			{
				.bit = bit,
				.component = 1,
				.value = std::get<1>(m_spins_qbit[bit])->value(),
			});
		}

		// send the changes back to the component
		emit this->SignalConfigChanged(configs);

		// update the dialogs every time the configuration is changed
		UpdateResults(comp);
	};

	const int num_cols = 2;

	// component configuration
	const ComponentConfigs& cfgs = comp->GetConfig();
	QLabel *labelName = new QLabel(cfgs.name.c_str(), this);
	QFont fontName = labelName->font();
	fontName.setBold(true);
	labelName->setFont(fontName);
	m_layout->addWidget(labelName, m_layout->rowCount(), 0, 1, num_cols);


	// add component configuration options
	for(const ComponentConfig& cfg : cfgs.configs)
	{
		// description label
		QLabel *labelDescr = new QLabel(cfg.description.c_str(), this);
		m_layout->addWidget(labelDescr, m_layout->rowCount(), 0, 1, num_cols);

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

				// update the dialogs every time the configuration is changed
				UpdateResults(comp);
			});

			m_layout->addWidget(spinVal, m_layout->rowCount(), 0, 1, num_cols);
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
			(&QDoubleSpinBox::valueChanged), [this, cfg, scale, comp](double val) -> void
			{
				ComponentConfigs configs;

				configs.configs = std::vector<ComponentConfig>
				{{
					ComponentConfig{.key = cfg.key, .value = t_real(val)/scale},
				}};

				// send the changes back to the component
				emit this->SignalConfigChanged(configs);

				// update the dialogs every time the configuration is changed
				UpdateResults(comp);
			});

			m_layout->addWidget(spinVal, m_layout->rowCount(), 0, 1, num_cols);
		}
	}


	// add input qubit states for state components
	m_spins_qbit.clear();

	if(comp->GetType() == ComponentType::STATE)
	{
		QFrame *line = new QFrame(this);
		line->setFrameShape(QFrame::HLine);
		m_layout->addWidget(line, m_layout->rowCount(), 0, 1, num_cols);

		const InputStates *input_comp = static_cast<const InputStates*>(comp);
		const t_vec& vec = input_comp->GetInputQBits();

		for(t_uint bit=0; bit<input_comp->GetNumQBits(); ++bit)
		{
			// description label
			QString textState = QString{"Qubit |ψ%1> = a⋅|↓> + b⋅|↑>"}.arg(bit+1);
			QLabel *labelDescr = new QLabel(textState, this);
			m_layout->addWidget(labelDescr, m_layout->rowCount(), 0, 1, num_cols);

			QDoubleSpinBox *spinDown = new QDoubleSpinBox(this);
			spinDown->setDecimals(g_prec_gui);
			spinDown->setValue(bit < vec.size() ? vec[bit].real() : 0);
			spinDown->setSingleStep(0.1);
			spinDown->setMinimum(0);
			spinDown->setMaximum(1);

			QDoubleSpinBox *spinUp = new QDoubleSpinBox(this);
			spinUp->setDecimals(g_prec_gui);
			spinUp->setValue(bit < vec.size() ? vec[bit].imag() : 0);
			spinUp->setSingleStep(0.1);
			spinUp->setMinimum(0);
			spinUp->setMaximum(1);

			for(QDoubleSpinBox *spin : {spinDown, spinUp})
			{
				connect(spin, static_cast<void (QDoubleSpinBox::*)(double)>
				(&QDoubleSpinBox::valueChanged), [update_states](double)
				{
					update_states();
				});
			}

			m_spins_qbit.push_back(std::make_pair(spinDown, spinUp));

			int row = m_layout->rowCount();
			m_layout->addWidget(spinDown, row, 0, 1, 1);
			m_layout->addWidget(spinUp, row, 1, 1, 1);
		}
	}


	// add operator and state dialog buttons
	if(comp->GetType() == ComponentType::STATE || comp->GetType() == ComponentType::GATE)
	{
		QFrame *line = new QFrame(this);
		line->setFrameShape(QFrame::HLine);
		m_layout->addWidget(line, m_layout->rowCount(), 0, 1, num_cols);

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
		m_layout->addWidget(btnOperator, m_layout->rowCount(), 0, 1, num_cols);

		if(comp->GetType() == ComponentType::STATE)
		{
			const InputStates *input_comp = static_cast<const InputStates*>(comp);

			QPushButton *btnStates = new QPushButton("States...", this);
			btnStates->setToolTip("Show Input and Output State Vectors");
			connect(btnStates, &QAbstractButton::clicked, [this, input_comp]()
			{
				if(!m_compStates)
					m_compStates = std::make_shared<ComponentStates>(this);

				m_compStates->SetStates(input_comp->GetOutputState());
				show_dialog(m_compStates.get());
			});
			m_layout->addWidget(btnStates, m_layout->rowCount(), 0, 1, num_cols);
		}
	}


	QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
	m_layout->addItem(spacer, m_layout->rowCount(), 0, 1, num_cols);

	// also update dialogs every time a new item is selected
	UpdateResults(comp);
}
