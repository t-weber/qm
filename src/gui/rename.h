/**
 * component rename dialog
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Jun-2022
 * @license see 'LICENSE' file
 */

#ifndef __QM_RENAME_H__
#define __QM_RENAME_H__

#include <QtWidgets/QDialog>
#include <QtWidgets/QTableWidget>

#include "components.h"


class Rename : public QDialog
{ Q_OBJECT
public:
	Rename(QWidget *parent = nullptr);
	virtual ~Rename() = default;

	Rename(const Rename&) = delete;
	const Rename& operator=(const Rename&) = delete;


public slots:
	void SelectedItem(QuantumComponent *item,
		InputStates *associated_input_comp = nullptr);


protected:
	void SaveChanges();

	virtual void accept() override;
	virtual void reject() override;


private:
	InputStates *m_curInputStates{};
	QTableWidget *m_tableQBits{};
};

#endif
