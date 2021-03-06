/**
 * component states dialog
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#include "component_states.h"
#include "globals.h"
#include "lib/qm_algos.h"

#include <QtCore/QSettings>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QSpacerItem>

#include <sstream>


ComponentStates::ComponentStates(QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle("Qubit States");
	setSizeGripEnabled(true);


	// text edit box
	m_edit = std::make_shared<QTextEdit>(this);
	m_edit->setReadOnly(true);
	SetLineWrap(true);

	//QSpacerItem *spacer_end = new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding);
	//grid->addItem(spacer_end, grid->rowCount(), 0, 1, 2);


	// options checkboxes
	QCheckBox *checkWrap = new QCheckBox("Wrap Text", this);
	checkWrap->setChecked(GetLineWrap());
	connect(checkWrap, &QCheckBox::toggled,
		this, &ComponentStates::SetLineWrap);


	// button box
	QDialogButtonBox *buttonbox = new QDialogButtonBox(this);
	buttonbox->setStandardButtons(QDialogButtonBox::Ok);

	connect(buttonbox, &QDialogButtonBox::clicked,
		[this, buttonbox](QAbstractButton *btn) -> void
	{
		// get button role
		QDialogButtonBox::ButtonRole role = buttonbox->buttonRole(btn);

		if(role == QDialogButtonBox::AcceptRole)
			this->accept();
		else if(role == QDialogButtonBox::RejectRole)
			this->reject();
	});


	// grid layout
	QGridLayout *grid = new QGridLayout(this);
	grid->setSpacing(4);
	grid->setContentsMargins(8, 8, 8, 8);

	grid->addWidget(m_edit.get(), grid->rowCount(), 0, 1, 2);
	grid->addWidget(checkWrap, grid->rowCount(), 0, 1, 1);
	grid->addWidget(buttonbox, grid->rowCount()-1, 1, 1, 1);


	// restore settings
	QSettings settings{this};
	if(settings.contains("dlg_states/wnd_geo"))
	{
		QByteArray arr{settings.value("dlg_states/wnd_geo").toByteArray()};
		this->restoreGeometry(arr);
	}
}


/**
 * set the (output) states to display
 */
void ComponentStates::SetStates(t_uint num_qbits, const t_vec& vecIn, const t_vec& vecOut)
{
	std::ostringstream ostr;
	ostr.precision(g_prec_gui);


	// input and output qubit product state vectors
	ostr << "Input state vector:\n";
	for(std::size_t col=0; col<vecIn.size(); ++col)
	{
		ostr << std::setw(g_prec_gui*4) << std::left << vecIn(col);
		//if(col < vecIn.size()-1)
		//	ostr << ", ";
	}
	ostr << "\n";

	ostr << "\nOutput state vector:\n";
	for(std::size_t col=0; col<vecOut.size(); ++col)
	{
		ostr << std::setw(g_prec_gui*4) << std::left << vecOut(col);
		//if(col < vecOut.size()-1)
		//	ostr << ", ";
	}
	ostr << "\n";


	// interpretation as classical bits (if possible)
	constexpr const t_int bitsize = sizeof(t_int)*8;

	// input qubits
	ostr << "\nInput classical bits:\n";

	auto all_classical_bits_in = measure_qbits_all<t_vec, bitsize, t_int>(
		vecIn, g_classical_threshold);
	for(const auto& classical_bits_in : all_classical_bits_in)
	{
		std::string classical_bits_in_str = classical_bits_in.to_string();
		classical_bits_in_str = classical_bits_in_str.substr(bitsize-num_qbits);
		ostr << classical_bits_in_str << "\n";
	}

	// output qubits
	ostr << "\nOutput classical bits:\n";
	auto all_classical_bits_out = measure_qbits_all<t_vec, bitsize, t_int>(
		vecOut, g_classical_threshold);
	for(const auto& classical_bits_out : all_classical_bits_out)
	{
		std::string classical_bits_out_str = classical_bits_out.to_string();
		classical_bits_out_str = classical_bits_out_str.substr(bitsize-num_qbits);
		ostr << classical_bits_out_str << "\n";
	}


	m_edit->setPlainText(ostr.str().c_str());
}


bool ComponentStates::GetLineWrap() const
{
	return m_edit->lineWrapMode() != QTextEdit::NoWrap;
}


void ComponentStates::SetLineWrap(bool wrap)
{
	if(wrap)
		m_edit->setLineWrapMode(QTextEdit::WidgetWidth);
	else
		m_edit->setLineWrapMode(QTextEdit::NoWrap);
}


void ComponentStates::accept()
{
	// save settings
	QSettings settings{this};
	QByteArray geo{this->saveGeometry()};
	settings.setValue("dlg_states/wnd_geo", geo);

	QDialog::accept();
}


void ComponentStates::reject()
{
	QDialog::reject();
}
