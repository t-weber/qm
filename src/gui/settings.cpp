/**
 * settings
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#include "settings.h"

#include <QtCore/QSettings>
#include <QtGui/QGuiApplication>
#include <QtGui/QPalette>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>


t_real g_raster_size = 35.;
bool g_snap_on_move = false;
bool g_keep_gates_on_states = true;


const QColor& get_foreground_colour()
{
	QPalette palette = dynamic_cast<QGuiApplication*>(
		QGuiApplication::instance())->palette();
	const QColor& col = palette.color(QPalette::WindowText);

	return col;
}


const QColor& get_background_colour()
{
	QPalette palette = dynamic_cast<QGuiApplication*>(
		QGuiApplication::instance())->palette();
	const QColor& col = palette.color(QPalette::Window);

	return col;
}


// ----------------------------------------------------------------------------
// settings dialog
// ----------------------------------------------------------------------------

Settings::Settings(QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle("Settings");
	setSizeGripEnabled(true);


	// grid layout
	m_grid = std::make_shared<QGridLayout>(this);
	m_grid->setSpacing(4);
	m_grid->setContentsMargins(8, 8, 8, 8);


	// buttons
	QDialogButtonBox *buttonbox = new QDialogButtonBox(this);
	buttonbox->setStandardButtons(
		QDialogButtonBox::Ok |
		QDialogButtonBox::Apply |
		QDialogButtonBox::Cancel |
		QDialogButtonBox::RestoreDefaults);
	buttonbox->button(QDialogButtonBox::Ok)->setDefault(true);

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

	m_grid->addWidget(buttonbox, m_grid->rowCount(), 0, 1, 1);


	// restore settings
	QSettings settings{this};
	if(settings.contains("dlg_settings/wnd_geo"))
	{
		QByteArray arr{settings.value("dlg_settings/wnd_geo").toByteArray()};
		this->restoreGeometry(arr);
	}
}


void Settings::accept()
{
	// save settings
	QSettings settings{this};
	QByteArray geo{this->saveGeometry()};
	settings.setValue("dlg_settings/wnd_geo", geo);

	QDialog::accept();
}


void Settings::reject()
{
	QDialog::reject();
}

// TODO
// ----------------------------------------------------------------------------
