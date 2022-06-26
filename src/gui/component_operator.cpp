/**
 * component operator dialog
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#include "component_operator.h"
#include "globals.h"

#include <QtCore/QSettings>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>

#include <sstream>


ComponentOperator::ComponentOperator(QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle("Operator");
	setSizeGripEnabled(true);


	// text edit box
	m_edit = std::make_shared<QTextEdit>(this);
	m_edit->setReadOnly(true);
	SetLineWrap(true);


	// options checkboxes
	QCheckBox *checkWrap = new QCheckBox("Wrap Text", this);
	checkWrap->setChecked(GetLineWrap());
	connect(checkWrap, &QCheckBox::toggled,
		this, &ComponentOperator::SetLineWrap);


	//QSpacerItem *spacer_end = new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding);
	//grid->addItem(spacer_end, grid->rowCount(), 0, 1, 2);


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
	if(settings.contains("dlg_operator/wnd_geo"))
	{
		QByteArray arr{settings.value("dlg_operator/wnd_geo").toByteArray()};
		this->restoreGeometry(arr);
	}
}


/**
 * set the operator matrix to display
 */
void ComponentOperator::SetOperator(const t_mat& op)
{
	std::ostringstream ostr;
	ostr.precision(g_prec_gui);
	ostr << "<table style=\"border:0px; border-spacing:2px\">";

	for(std::size_t row=0; row<op.size1(); ++row)
	{
		ostr << "<tr>";

		for(std::size_t col=0; col<op.size2(); ++col)
		{
			ostr << "<td style=\"padding-top:2px; padding-bottom:2px; padding-left:4px; padding-right:4px\">";
			ostr << op(row, col);
			ostr << "</td>";
		}

		ostr << "</tr>";
	}

	ostr << "</table>";

	m_edit->setHtml(ostr.str().c_str());
}


bool ComponentOperator::GetLineWrap() const
{
	return m_edit->lineWrapMode() != QTextEdit::NoWrap;
}


void ComponentOperator::SetLineWrap(bool wrap)
{
	if(wrap)
		m_edit->setLineWrapMode(QTextEdit::WidgetWidth);
	else
		m_edit->setLineWrapMode(QTextEdit::NoWrap);
}


void ComponentOperator::accept()
{
	// save settings
	QSettings settings{this};
	QByteArray geo{this->saveGeometry()};
	settings.setValue("dlg_operator/wnd_geo", geo);

	QDialog::accept();
}


void ComponentOperator::reject()
{
	QDialog::reject();
}
