/**
 * qm graphics workspace
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Oct-2021
 * @license see 'LICENSE' file
 */

#include "qm_gui.h"

#include <QtGui/QMouseEvent>

#include <cmath>

// ----------------------------------------------------------------------------
// graphics scene
// ----------------------------------------------------------------------------
QmScene::QmScene(QWidget* parent)
	: QGraphicsScene(parent), m_parent{parent}
{
	// test
	//addItem(new CNot());
	//addItem(new Toffoli());
}


QmScene::~QmScene()
{
}


/**
 * draw background grid
 */
void QmScene::drawBackground(QPainter* painter, const QRectF& rect)
{
	QGraphicsScene::drawBackground(painter, rect);

	QPointF topLeft = rect.topLeft();
	QPointF bottomRight = rect.bottomRight();

	t_real start_y = std::round(topLeft.y() / g_raster_size) *
		g_raster_size - 0.5*g_raster_size;
	t_real end_y = std::round(bottomRight.y() / g_raster_size) *
		g_raster_size + 0.5*g_raster_size;

	t_real start_x = std::round(topLeft.x() / g_raster_size) *
		g_raster_size - 0.5*g_raster_size;
	t_real end_x = std::round(bottomRight.x() / g_raster_size) *
		g_raster_size + 0.5*g_raster_size;

	QPen pen(QColor::fromRgbF(0.5, 0.5, 0.5, 0.5));
	pen.setWidthF(0.5);
	//pen.setDashPattern(QVector<qreal>{{ 
	//	1./4.*g_raster_size, 1./2*g_raster_size }});
	painter->setPen(pen);

	// horizontal guide lines
	for(t_real y=start_y; y<end_y; y+=g_raster_size)
	{
		painter->drawLine(
			QPointF(start_x, y),
			QPointF(end_x, y));
	}

	// vertical guide lines
	for(t_real x=start_x; x<end_x; x+=g_raster_size)
	{
		painter->drawLine(
			QPointF(x, start_y),
			QPointF(x, end_y));
	}
}


void QmScene::mousePressEvent(QGraphicsSceneMouseEvent *evt)
{
	QGraphicsScene::mousePressEvent(evt);
}


void QmScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *evt)
{
	QGraphicsScene::mouseReleaseEvent(evt);
}


void QmScene::mouseMoveEvent(QGraphicsSceneMouseEvent *evt)
{
	QPointF posScene = evt->scenePos();

	if(QGraphicsItem* item = mouseGrabberItem(); item)
	{
		qreal raster_x = std::round(posScene.x() / g_raster_size);
		qreal raster_y = std::round(posScene.y() / g_raster_size);

		item->setX(raster_x * g_raster_size);
		item->setY(raster_y * g_raster_size);
	}
	else
	{
		QGraphicsScene::mouseMoveEvent(evt);
	}
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// graphics view
// ----------------------------------------------------------------------------
QmView::QmView(QmScene *scene, QWidget *parent)
	: QGraphicsView(scene, parent), m_scene{scene}
{
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	setInteractive(true);
	setMouseTracking(true);
}


QmView::~QmView()
{
}


void QmView::resizeEvent(QResizeEvent *evt)
{
	QPointF pt1{mapToScene(QPoint{0,0})};
	QPointF pt2{mapToScene(QPoint{evt->size().width(), evt->size().height()})};

	// TODO: include bounds given by all components

	setSceneRect(QRectF{pt1, pt2});
	QGraphicsView::resizeEvent(evt);
}


void QmView::mousePressEvent(QMouseEvent *evt)
{
	QPoint posVP = evt->pos();
	//QPointF posScene = mapToScene(posVP);

	QList<QGraphicsItem*> items = this->items(posVP);

	// TODO

	QGraphicsView::mousePressEvent(evt);
}


void QmView::mouseReleaseEvent(QMouseEvent *evt)
{
	// TODO

	QGraphicsView::mouseReleaseEvent(evt);
}


void QmView::mouseMoveEvent(QMouseEvent *evt)
{
	QGraphicsView::mouseMoveEvent(evt);

	// TODO

	QPoint posVP = evt->pos();
	QPointF posScene = mapToScene(posVP);

	emit SignalMouseCoordinates(posScene.x(), posScene.y());
}
// ----------------------------------------------------------------------------
