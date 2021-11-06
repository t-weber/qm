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

	void AddGate(QuantumGateItem *gate);
	void Clear();


protected:
	virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

	virtual void mousePressEvent(QGraphicsSceneMouseEvent *evt) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *evt) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *evt) override;


private:
	QWidget *m_parent {nullptr};
	std::vector<QuantumGateItem*> m_gates{};
};



class QmView : public QGraphicsView
{ Q_OBJECT
public:
	QmView(QmScene *scene = nullptr, QWidget *parent = nullptr);
	virtual ~QmView();

	QmView(QmView&) = delete;
	const QmView& operator=(const QmView&) const = delete;

	void Clear();


protected:
	virtual void mousePressEvent(QMouseEvent* evt) override;
	virtual void mouseReleaseEvent(QMouseEvent* evt) override;
	virtual void mouseMoveEvent(QMouseEvent* evt) override;

	virtual void resizeEvent(QResizeEvent* evt) override;

	virtual void paintEvent(QPaintEvent* evt) override;


private:
	QmScene *m_scene = nullptr;
	QuantumGateItem *m_curItem = nullptr;

	std::shared_ptr<QMenu> m_context;


public slots:
	// change the configuration of the current component
	void SetCurItemConfig(const ComponentConfigs& cfg);


signals:
	void SignalMouseCoordinates(double x, double y);
	void SignalSelectedItem(const QuantumGate *item);
};


#endif
