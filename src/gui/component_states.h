/**
 * component states dialog
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_STATESDLG_H__
#define __QM_STATESDLG_H__

#include <QtWidgets/QDialog>
#include <QtWidgets/QTextEdit>

#include <memory>

#include "types.h"


class ComponentStates : public QDialog
{
public:
	ComponentStates(QWidget *parent=nullptr);
	virtual ~ComponentStates() = default;

	ComponentStates(const ComponentStates&) = delete;
	const ComponentStates& operator=(const ComponentStates&) = delete;

	void SetStates(t_uint num_qbits, const t_vec& vecIn, const t_vec& vecOut);


protected:
	virtual void accept() override;
	virtual void reject() override;


private:
	std::shared_ptr<QTextEdit> m_edit{};
};

#endif
