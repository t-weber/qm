/**
 * qm graphics workspace
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Oct-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_WORKSPACE_H__
#define __QM_WORKSPACE_H__

#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QMenu>

#include <memory>
#include <vector>

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

	void Clear();
	void AddQuantumComponent(QuantumComponentItem *comp);
	const std::vector<QuantumComponentItem*>& GetQuantumComponents() const
	{ return m_components; }

	const QPointF& GetCursorPosition(bool on_grid = true) const
	{ return on_grid ? m_curRasterScenePos : m_curScenePos; }


protected:
	virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

	virtual void mousePressEvent(QGraphicsSceneMouseEvent *evt) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *evt) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *evt) override;


private:
	std::vector<QuantumComponentItem*> m_components{};

	// cursor position in scene
	QPointF m_curScenePos{0, 0};
	// cursor position in grid
	QPointF m_curRasterScenePos{0, 0};
};



class QmView : public QGraphicsView
{ Q_OBJECT
public:
	QmView(QmScene *scene = nullptr, QWidget *parent = nullptr);
	virtual ~QmView();

	QmView(QmView&) = delete;
	const QmView& operator=(const QmView&) const = delete;

	void Clear();
	void FitAreaToScene(const QRectF *_sceneRect = nullptr);


protected:
	virtual void mousePressEvent(QMouseEvent *evt) override;
	virtual void mouseReleaseEvent(QMouseEvent *evt) override;
	virtual void mouseMoveEvent(QMouseEvent *evt) override;
	virtual void wheelEvent(QWheelEvent *evt) override;

	virtual void keyPressEvent(QKeyEvent *evt) override;
	virtual void keyReleaseEvent(QKeyEvent *evt) override;

	virtual void paintEvent(QPaintEvent* evt) override;
	virtual void resizeEvent(QResizeEvent* evt) override;


private:
	QmScene *m_scene = nullptr;
	QuantumComponentItem *m_curItem = nullptr;
	QuantumComponentItem *m_copiedItem = nullptr;

	// context menu for a selected item
	std::shared_ptr<QMenu> m_context{};
	// context menu in case no item has been selected
	std::shared_ptr<QMenu> m_contextNoItem{};


public slots:
	// change the configuration of the current component
	void SetCurItemConfig(const ComponentConfigs& cfg);

	void DeleteCurItem();
	void CopyCurItem();
	void PasteItem();


signals:
	void SignalMouseCoordinates(qreal scene_x, qreal scene_y);
	void SignalSelectedItem(const QuantumComponent *item);
};


#endif
