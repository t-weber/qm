/**
 * qm gui
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Oct-2021
 * @license: see 'LICENSE' file
 */

#ifndef __QM_GUI_H__
#define __QM_GUI_H__

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsItem>

#include <memory>
#include <vector>

#include "recent.h"
#include "lib/qm_algos.h"
using t_real = double;


class QmWnd : public QMainWindow
{ Q_OBJECT
public:
	using QMainWindow::QMainWindow;

	QmWnd(QWidget* pParent = nullptr);
	~QmWnd();

	void SetStatusMessage(const QString& msg);

	void FileNew();
	void FileLoad();
	void FileSave();
	void FileSaveAs();

	bool SaveFile(const QString& filename) const;
	bool LoadFile(const QString& filename);

	void AddRecentFile(const QString& filename);


protected:
	void CreateRecentFileMenu();


private:
	virtual void closeEvent(QCloseEvent *) override;


private:
	QString m_gui_theme{};
	bool m_gui_native{false};

	RecentFiles m_recent{this, 16};

	//std::shared_ptr<QGraphicsScene> m_scene;
	//std::shared_ptr<QGraphicsView> m_view;
	std::shared_ptr<QLabel> m_statusLabel;
};


#endif
