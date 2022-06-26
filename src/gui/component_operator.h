/**
 * component operator dialog
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_OPDLG_H__
#define __QM_OPDLG_H__

#include <QtWidgets/QDialog>
#include <QtWidgets/QTextEdit>

#include <memory>

#include "types.h"


class ComponentOperator : public QDialog
{
public:
	ComponentOperator(QWidget *parent=nullptr);
	virtual ~ComponentOperator() = default;

	ComponentOperator(const ComponentOperator&) = delete;
	const ComponentOperator& operator=(const ComponentOperator&) = delete;

	void SetOperator(const t_mat& op);

	bool GetLineWrap() const;
	void SetLineWrap(bool b);


protected:
	virtual void accept() override;
	virtual void reject() override;


private:
	std::shared_ptr<QTextEdit> m_edit{};
};

#endif
