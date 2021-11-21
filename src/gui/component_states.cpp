/**
 * component states dialog
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#include "component_states.h"
#include "globals.h"

#include <QtCore/QSettings>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>

#include <sstream>


ComponentStates::ComponentStates(QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle("Qubit States");
	setSizeGripEnabled(true);


	// grid layout
	QGridLayout *grid = new QGridLayout(this);
	grid->setSpacing(4);
	grid->setContentsMargins(8, 8, 8, 8);


	m_edit = std::make_shared<QTextEdit>(this);
	m_edit->setReadOnly(true);
	grid->addWidget(m_edit.get(), grid->rowCount(), 0, 1, 1);

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

	grid->addWidget(buttonbox, grid->rowCount(), 0, 1, 1);


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
void ComponentStates::SetStates(const t_vec& vec)
{
	std::ostringstream ostr;
	ostr << "<table style=\"border:0px; border-spacing:2px\">";
	ostr.precision(g_prec_gui);

	for(std::size_t col=0; col<vec.size(); ++col)
	{
		ostr << "<tr>";
		ostr << "<td style=\"padding-top:2px; padding-bottom:2px; padding-left:4px; padding-right:4px\">";
		ostr << vec(col);
		ostr << "</td>";
		ostr << "</tr>";
	}

	ostr << "</table>";

	m_edit->setHtml(ostr.str().c_str());
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
