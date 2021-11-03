/**
 * qm gui
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Oct-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_GUI_H__
#define __QM_GUI_H__

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsSceneMouseEvent>

#include <memory>
#include <vector>

#include "recent.h"
#include "components.h"

#include "types.h"
#include "settings.h"
#include "lib/qm_algos.h"


class QmScene : public QGraphicsScene
{
public:
	QmScene(QWidget* parent);
	virtual ~QmScene();

	QmScene(QmScene&) = delete;
	const QmScene& operator=(const QmScene&) const = delete;


protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *evt) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *evt) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *evt) override;


private:
	QWidget *m_parent = nullptr;
};



class QmView : public QGraphicsView
{ Q_OBJECT
public:
	QmView(QmScene *scene = nullptr, QWidget *parent = nullptr);
	virtual ~QmView();

	QmView(QmView&) = delete;
	const QmView& operator=(const QmView&) const = delete;


protected:
	virtual void mousePressEvent(QMouseEvent *evt) override;
	virtual void mouseReleaseEvent(QMouseEvent *evt) override;
	virtual void mouseMoveEvent(QMouseEvent *evt) override;

	virtual void resizeEvent(QResizeEvent *evt) override;


private:
	QmScene *m_scene = nullptr;
	bool m_dragging = false;


signals:
	void SignalMouseCoordinates(double x, double y);
};


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


private:
	virtual void closeEvent(QCloseEvent *) override;


private:
	QString m_gui_theme{};
	bool m_gui_native{false};

	RecentFiles m_recent{this, 16};

	std::shared_ptr<QmScene> m_scene;
	std::shared_ptr<QmView> m_view;
	std::shared_ptr<QLabel> m_statusLabel;
};


#endif
