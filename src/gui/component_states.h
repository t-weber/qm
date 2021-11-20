/**
 * component states dialog
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_STATESDLG_H__
#define __QM_STATESDLG_H__

#include <QtWidgets/QDialog>

#include <memory>

#include "types.h"


class ComponentStates : public QDialog
{
public:
	ComponentStates(QWidget *parent=nullptr);
	virtual ~ComponentStates() = default;

	ComponentStates(const ComponentStates&) = delete;
	const ComponentStates& operator=(const ComponentStates&) = delete;

	void SetStates(const t_vec& op);


protected:
	virtual void accept() override;
	virtual void reject() override;


private:
};

#endif