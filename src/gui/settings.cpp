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
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>


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


	// button box
	m_buttonbox = std::make_shared<QDialogButtonBox>(this);
	m_buttonbox->setStandardButtons(
		QDialogButtonBox::Ok |
		QDialogButtonBox::Apply |
		QDialogButtonBox::Cancel |
		QDialogButtonBox::RestoreDefaults);
	m_buttonbox->button(QDialogButtonBox::Ok)->setDefault(true);

	connect(m_buttonbox.get(), &QDialogButtonBox::clicked,
			[this](QAbstractButton *btn) -> void
	{
		// get button role
		QDialogButtonBox::ButtonRole role = m_buttonbox->buttonRole(btn);

		if(role == QDialogButtonBox::AcceptRole)
			this->accept();
		else if(role == QDialogButtonBox::RejectRole)
			this->reject();
		else if(role == QDialogButtonBox::ApplyRole)
			ApplySettings();
		else if(role == QDialogButtonBox::ResetRole)
			RestoreDefaultSettings();
	});


	// restore settings
	QSettings settings{this};
	if(settings.contains("dlg_settings/wnd_geo"))
	{
		QByteArray arr{settings.value("dlg_settings/wnd_geo").toByteArray()};
		this->restoreGeometry(arr);
	}
}


/**
 * adds a check box to the end of the grid layout
 */
void Settings::AddCheckbox(const QString& key, const QString& descr, bool value)
{
	bool initial_value = value;

	// look for already saved value
	QSettings settings{this};
	if(settings.contains(key))
		value = settings.value(key).toBool();

	QCheckBox *box = new QCheckBox(this);
	box->setText(descr);
	box->setChecked(value);

	box->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	int row = m_grid->rowCount();
	m_grid->addWidget(box, row, 0, 1, 2);

	m_checkboxes.push_back(std::make_tuple(box, key, initial_value));
}


/**
 * adds a spacer to the end of the grid layout
 */
void Settings::AddSpacer(int size_v)
{
	QSizePolicy::Policy policy_h = QSizePolicy::Fixed;
	QSizePolicy::Policy policy_v = QSizePolicy::Fixed;

	// expanding spacer?
	if(size_v < 0)
	{
		policy_v = QSizePolicy::Expanding;
		size_v = 1;
	}

	QSpacerItem *spacer_end = new QSpacerItem(1, size_v, policy_h, policy_v);
	m_grid->addItem(spacer_end, m_grid->rowCount(), 0, 1, 2);
}


/**
 * adds the button box at the end of the grid layout
 */
void Settings::FinishSetup()
{
	AddSpacer();
	m_grid->addWidget(m_buttonbox.get(), m_grid->rowCount(), 0, 1, 1);

	ApplySettings();
}


/**
 * get the value of the check box with the given key
 */
bool Settings::GetCheckboxValue(const QString& key)
{
	for(auto& [box, box_key, initial] : m_checkboxes)
	{
		if(key == box_key)
			return box->isChecked();
	}

	return false;
}


/**
 * save settings and notice any listeners
 */
void Settings::ApplySettings()
{
	// save current settings
	QSettings settings{this};
	for(const auto& [box, key, initial] : m_checkboxes)
	{
		bool value = box->isChecked();
		settings.setValue(key, value);
	}

	// signal changes
	emit this->SignalApplySettings();
}


/**
 * restore the initial defaults
 */
void Settings::RestoreDefaultSettings()
{
	for(auto& [box, key, initial] : m_checkboxes)
		box->setChecked(initial);

	// signal changes
	emit this->SignalApplySettings();
}


void Settings::accept()
{
	ApplySettings();

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
