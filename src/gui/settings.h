/**
 * settings
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_SETTINGS_H__
#define __QM_SETTINGS_H__

#include <QtGui/QColor>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QCheckBox>

#include <memory>

#include "types.h"


// get basic colours
extern const QColor& get_foreground_colour();
extern const QColor& get_background_colour();



/**
 * settings dialog
 */
class Settings : public QDialog
{ Q_OBJECT
public:
	Settings(QWidget *parent=nullptr);
	virtual ~Settings() = default;

	Settings(const Settings&) = delete;
	const Settings& operator=(const Settings&) = delete;

	void AddCheckbox(const QString& key, const QString& descr, bool value);
	void AddSpacer(int size_v=-1);
	void FinishSetup();

	bool GetCheckboxValue(const QString& key);


protected:
	virtual void accept() override;
	virtual void reject() override;

	void ApplySettings();
	void RestoreDefaultSettings();


private:
	std::shared_ptr<QGridLayout> m_grid{};
	std::shared_ptr<QDialogButtonBox> m_buttonbox{};

	// checkbox, key, and initial value
	using t_checkboxinfo = std::tuple<QCheckBox*, QString, bool>;
	std::vector<t_checkboxinfo> m_checkboxes{};


signals:
	void SignalApplySettings();
};

#endif
