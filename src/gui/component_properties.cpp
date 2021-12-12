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
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QFrame>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QScrollArea>


ComponentProperties::ComponentProperties(QWidget* parent)
	: QWidget(parent)
{
	QScrollArea *scroll = new QScrollArea(this);
	scroll->setWidgetResizable(true);
	//scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll->setFrameStyle(QFrame::NoFrame);

	m_scrollwidget = std::make_shared<QWidget>(scroll);
	scroll->setWidget(m_scrollwidget.get());

	m_layout = std::make_shared<QGridLayout>(m_scrollwidget.get());
	m_layout->setContentsMargins(4, 4, 4, 4);
	m_layout->setVerticalSpacing(4);
	m_layout->setHorizontalSpacing(4);

	QGridLayout *mainlayout = new QGridLayout(this);
	mainlayout->setContentsMargins(4, 4, 4, 4);
	mainlayout->setVerticalSpacing(4);
	mainlayout->setHorizontalSpacing(4);
	mainlayout->addWidget(scroll, 0, 0, 1, 1);
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
		m_compStates->SetStates(0, t_vec{}, t_vec{});
}


/**
 * update the results shown in the operator and states dialogs
 */
void ComponentProperties::UpdateResults(
	const QuantumComponent *comp,
	const InputStates *associated_input_comp,
	bool /*ok*/)
{
	if(comp && comp->GetType() == ComponentType::STATE)
		associated_input_comp = static_cast<const InputStates*>(comp);

	// update operator
	if(m_compOperator && comp)
	{
		m_compOperator->SetOperator(comp->GetOperator());
	}

	// update states vector
	if(m_compStates && associated_input_comp)
	{
		m_compStates->SetStates(
			associated_input_comp->GetNumQBits(),
			associated_input_comp->GetInputState(),
			associated_input_comp->GetOutputState());
	}
}


/**
 * a component has been selected -> show its properties
 */
void ComponentProperties::SelectedItem(const QuantumComponent *comp, const InputStates *associated_input_comp)
{
	Clear();
	if(!comp)
		return;

	if(comp->GetType() == ComponentType::STATE)
		associated_input_comp = static_cast<const InputStates*>(comp);


	// get input state and send them to the component
	auto update_states = [this, comp, associated_input_comp]()
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
		UpdateResults(comp, associated_input_comp);
	};


	// number of columns in layout
	const int num_cols = 3;


	// component configuration
	const ComponentConfigs& cfgs = comp->GetConfig();

	// name label
	QLabel *labelName = new QLabel(cfgs.name.c_str(), this);
	QFont fontName = labelName->font();
	fontName.setBold(true);
	labelName->setFont(fontName);
	m_layout->addWidget(labelName, m_layout->rowCount(), 0, 1, num_cols);

	// is the component's error flag set
	if(!comp->IsOk())
	{
		// error label
		QLabel *labelErr = new QLabel("Configuration error!", this);

		// font
		QFont fontErr = labelErr->font();
		fontErr.setBold(true);
		labelErr->setFont(fontErr);

		// palette
		QPalette paletteErr = labelErr->palette();
		paletteErr.setColor(labelErr->foregroundRole(), QColor(0xff, 0, 0));
		labelErr->setPalette(paletteErr);

		m_layout->addWidget(labelErr, m_layout->rowCount(), 0, 1, num_cols);
	}


	// add component configuration options
	for(const ComponentConfig& cfg : cfgs.configs)
	{
		// uint value
		if(std::holds_alternative<t_uint>(cfg.value))
		{
			QSpinBox *spinVal = new QSpinBox(this);
			spinVal->setPrefix((cfg.description + ": ").c_str());
			spinVal->setValue(std::get<t_uint>(cfg.value));
			if(cfg.min_value)
				spinVal->setMinimum(std::get<t_uint>(*cfg.min_value));
			if(cfg.max_value)
				spinVal->setMaximum(std::get<t_uint>(*cfg.max_value));

			connect(spinVal, static_cast<void (QSpinBox::*)(int)>
				(&QSpinBox::valueChanged), [this, cfg, comp, associated_input_comp](int val) -> void
			{
				ComponentConfigs configs;

				configs.configs = std::vector<ComponentConfig>
				{{
					ComponentConfig
					{
						.key = cfg.key,
						.value = t_uint(val)
					},
				}};

				// send the changes back to the component
				emit this->SignalConfigChanged(configs);

				// update the dialogs every time the configuration is changed
				UpdateResults(comp, associated_input_comp);
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
			spinVal->setPrefix((cfg.description + ": ").c_str());
			spinVal->setDecimals(g_prec_gui);
			spinVal->setValue(std::get<t_real>(cfg.value) * scale);
			if(cfg.min_value)
				spinVal->setMinimum(std::get<t_real>(*cfg.min_value)*scale);
			if(cfg.max_value)
				spinVal->setMaximum(std::get<t_real>(*cfg.max_value)*scale);

			connect(spinVal, static_cast<void (QDoubleSpinBox::*)(double)>
			(&QDoubleSpinBox::valueChanged), [this, cfg, scale, comp, associated_input_comp](double val) -> void
			{
				ComponentConfigs configs;

				configs.configs = std::vector<ComponentConfig>
				{{
					ComponentConfig
					{
						.key = cfg.key,
						.value = t_real(val)/scale
					},
				}};

				// send the changes back to the component
				emit this->SignalConfigChanged(configs);

				// update the dialogs every time the configuration is changed
				UpdateResults(comp, associated_input_comp);
			});

			m_layout->addWidget(spinVal, m_layout->rowCount(), 0, 1, num_cols);
		}

		// t_cplx value
		else if(std::holds_alternative<t_cplx>(cfg.value))
		{
			// description label
			QLabel *labelDescr = new QLabel(cfg.description.c_str(), this);
			m_layout->addWidget(labelDescr, m_layout->rowCount(), 0, 1, num_cols);

			QLineEdit *editVal = new QLineEdit(this);
			std::ostringstream ostrVal;
			ostrVal.precision(g_prec_gui);
			ostrVal << std::get<t_cplx>(cfg.value);
			editVal->setText(ostrVal.str().c_str());

			connect(editVal, static_cast<void (QLineEdit::*)(const QString&)>
			(&QLineEdit::textChanged), [this, cfg, comp, associated_input_comp](const QString& val) -> void
			{
				ComponentConfigs configs;

				std::istringstream istrVal(val.toStdString());
				t_cplx cplx_val{0, 0};
				istrVal >> cplx_val;

				configs.configs = std::vector<ComponentConfig>
				{{
					ComponentConfig
					{
						.key = cfg.key,
						.value = cplx_val
					},
				}};

				// send the changes back to the component
				emit this->SignalConfigChanged(configs);

				// update the dialogs every time the configuration is changed
				UpdateResults(comp, associated_input_comp);
			});

			m_layout->addWidget(editVal, m_layout->rowCount(), 0, 1, num_cols);
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
			QString textState = QString{"Qubit |ψ%1> = a⋅|0> + b⋅|1>"}.arg(bit+1);
			QLabel *labelDescr = new QLabel(textState, this);
			m_layout->addWidget(labelDescr, m_layout->rowCount(), 0, 1, num_cols);

			QDoubleSpinBox *spinDown = new QDoubleSpinBox(this);
			spinDown->setDecimals(g_prec_gui);
			spinDown->setValue(bit < vec.size() ? vec[bit].real() : 0);
			spinDown->setSingleStep(0.1);
			spinDown->setMinimum(0);
			spinDown->setMaximum(1);
			spinDown->setToolTip(QString("Component a of qubit %1.").arg(bit+1));
			spinDown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

			QToolButton *btnSwap = new QToolButton(this);
			btnSwap->setText("↔");
			btnSwap->setToolTip(QString("Swap the a and b components of qubit %1.").arg(bit+1));
			QSizePolicy policySwap = btnSwap->sizePolicy();
			policySwap.setHorizontalStretch(0);
			policySwap.setHeightForWidth(false);
			policySwap.setHorizontalPolicy(QSizePolicy::Fixed);
			btnSwap->setSizePolicy(policySwap);

			QDoubleSpinBox *spinUp = new QDoubleSpinBox(this);
			spinUp->setDecimals(g_prec_gui);
			spinUp->setValue(bit < vec.size() ? vec[bit].imag() : 0);
			spinUp->setSingleStep(0.1);
			spinUp->setMinimum(0);
			spinUp->setMaximum(1);
			spinUp->setToolTip(QString("Component b of qubit %1.").arg(bit+1));
			spinUp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

			for(QDoubleSpinBox *spin : {spinDown, spinUp})
			{
				connect(spin, static_cast<void (QDoubleSpinBox::*)(double)>
					(&QDoubleSpinBox::valueChanged), [update_states](double)
					{
						update_states();
					});
			}

			connect(btnSwap, &QAbstractButton::clicked, [spinDown, spinUp]
			{
				qreal val = spinDown->value();
				spinDown->setValue(spinUp->value());
				spinUp->setValue(val);
			});

			m_spins_qbit.push_back(std::make_pair(spinDown, spinUp));

			int row = m_layout->rowCount();
			m_layout->addWidget(spinDown, row, 0, 1, 1);
			m_layout->addWidget(btnSwap, row, 1, 1, 1);
			m_layout->addWidget(spinUp, row, 2, 1, 1);
		}
	}


	// add operator and state dialog buttons
	if(comp->GetType() == ComponentType::STATE || comp->GetType() == ComponentType::GATE)
	{
		QFrame *line = new QFrame(this);
		line->setFrameShape(QFrame::HLine);
		m_layout->addWidget(line, m_layout->rowCount(), 0, 1, num_cols);

		QPushButton *btnOperator = new QPushButton("Operator...", this);
		btnOperator->setToolTip("Show the operator of this component.");
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
			btnStates->setToolTip("Show the input and output state vectors");
			connect(btnStates, &QAbstractButton::clicked, [this, input_comp]()
			{
				if(!m_compStates)
					m_compStates = std::make_shared<ComponentStates>(this);

				m_compStates->SetStates(input_comp->GetNumQBits(),
					input_comp->GetInputState(), input_comp->GetOutputState());
				show_dialog(m_compStates.get());
			});
			m_layout->addWidget(btnStates, m_layout->rowCount(), 0, 1, num_cols);
		}
	}


	QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
	m_layout->addItem(spacer, m_layout->rowCount(), 0, 1, num_cols);

	// also update dialogs every time a new item is selected
	UpdateResults(comp, associated_input_comp);

	// TODO: find minimum width
	//m_scrollwidget->adjustSize();
	//m_scrollwidget->setMinimumWidth(m_layout->sizeHint().width());
}
