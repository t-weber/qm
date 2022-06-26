/**
 * component rename dialog
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Jun-2022
 * @license see 'LICENSE' file
 */

#include "rename.h"

#include <QtCore/QSettings>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>


Rename::Rename(QWidget *parent) : QDialog(parent)
{
	setWindowTitle("Rename Component");
	setSizeGripEnabled(true);


	// qbits table
	m_tableQBits = new QTableWidget(this);
	m_tableQBits->setShowGrid(true);
	m_tableQBits->setSortingEnabled(true);

	m_tableQBits->setColumnCount(2);
	m_tableQBits->setHorizontalHeaderItem(0, new QTableWidgetItem{"Qubit"});
	m_tableQBits->setHorizontalHeaderItem(1, new QTableWidgetItem{"Name"});

	m_tableQBits->horizontalHeader()->setDefaultSectionSize(200);
	m_tableQBits->verticalHeader()->setVisible(false);

	m_tableQBits->setColumnWidth(0, 100);
	m_tableQBits->setColumnWidth(1, 250);


	// buttons
	QDialogButtonBox *buttonbox = new QDialogButtonBox(this);
	buttonbox->setStandardButtons(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);

	connect(buttonbox, &QDialogButtonBox::clicked,
		[this, buttonbox](QAbstractButton *btn) -> void
	{
		// get button role
		QDialogButtonBox::ButtonRole role = buttonbox->buttonRole(btn);

		if(role == QDialogButtonBox::AcceptRole)
			this->accept();
		else if(role == QDialogButtonBox::ApplyRole)
			this->SaveChanges();
		else if(role == QDialogButtonBox::RejectRole)
			this->reject();
	});

	buttonbox->button(QDialogButtonBox::Ok)->setAutoDefault(true);
	buttonbox->button(QDialogButtonBox::Ok)->setDefault(true);
	buttonbox->button(QDialogButtonBox::Ok)->setFocus();


	// grid layout
	auto *grid = new QGridLayout(this);
	grid->setSpacing(4);
	grid->setContentsMargins(8, 8, 8, 8);
	grid->addWidget(m_tableQBits, grid->rowCount(), 0, 1, 1);
	grid->addWidget(buttonbox, grid->rowCount(), 0, 1, 1);


	// restore settings
	QSettings settings{this};
	if(settings.contains("dlg_rename/wnd_geo"))
	{
		QByteArray arr{settings.value("dlg_rename/wnd_geo").toByteArray()};
		this->restoreGeometry(arr);
	}
}


/**
 * a new component has been selected
 */
void Rename::SelectedItem(
	[[maybe_unused]]QuantumComponent *item,  // TODO
	InputStates *input_comp)
{
	m_tableQBits->clearContents();

	m_curInputStates = input_comp;
	if(!m_curInputStates)
	{
		m_tableQBits->setRowCount(0);
		return;
	}

	const t_uint num_qbits = m_curInputStates->GetNumQBits();
	m_tableQBits->setRowCount(num_qbits);

	for(t_uint qbit=0; qbit<num_qbits; ++qbit)
	{
		std::string name = m_curInputStates->GetQBitName(qbit);

		m_tableQBits->setItem(qbit, 0, new QTableWidgetItem(QString("%1").arg(qbit+1)));
		m_tableQBits->setItem(qbit, 1, new QTableWidgetItem(name.c_str()));
	}
}


/**
 * apply changes
 */
void Rename::SaveChanges()
{
	// apply changes
	if(m_curInputStates)
	{
		const t_uint num_qbits = m_curInputStates->GetNumQBits();

		// get qbit names
		for(int row=0; row<m_tableQBits->rowCount(); ++row)
		{
			t_uint idx = m_tableQBits->item(row, 0)->text().toUInt() - 1;
			QString name = m_tableQBits->item(row, 1)->text();

			if(idx < num_qbits)
			{
				//std::cout << idx << " -> " << name.toStdString() << std::endl;
				m_curInputStates->SetQBitName(idx, name.toStdString());
			}
		}
	}
}


/**
 * apply changes and accept
 */
void Rename::accept()
{
	// save settings
	SaveChanges();
	QSettings settings{this};
	QByteArray geo{this->saveGeometry()};
	settings.setValue("dlg_rename/wnd_geo", geo);

	QDialog::accept();
}


/**
 * cancel
 */
void Rename::reject()
{
	QDialog::reject();
}
