/**
 * qm graphics workspace
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Oct-2021
 * @license see 'LICENSE' file
 */

#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QGraphicsSceneMouseEvent>

#include <cmath>

#include "qm_gui.h"
#include "helpers.h"


// ----------------------------------------------------------------------------
// graphics scene
// ----------------------------------------------------------------------------
QmScene::QmScene(QWidget* parent)
	: QGraphicsScene(parent), m_parent{parent}
{
}


QmScene::~QmScene()
{
}


/**
 * insert a quantum gate into the scene
 */
void QmScene::AddGate(QuantumGateItem *gate)
{
	m_gates.push_back(gate);
	addItem(gate);
}


/**
 * clear all components in the scene
 */
void QmScene::Clear()
{
	clear();
	m_gates.clear();
}


/**
 * draw background grid
 */
void QmScene::drawBackground(QPainter* painter, const QRectF& rect)
{
	QGraphicsScene::drawBackground(painter, rect);

	const QColor& colour_fg = get_foreground_colour();
	const QColor& colour_bg = get_background_colour();
	QColor colour_line = lerp(colour_fg, colour_bg, 0.75);

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

	QPen pen(colour_line);
	pen.setWidthF(0.5);
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
	// context menu
	m_context = std::make_shared<QMenu>(this);
	QIcon iconDelete = QIcon::fromTheme("edit-delete");
	QAction *actionDelete = new QAction(iconDelete, "Delete Component", m_context.get());
	m_context->addAction(actionDelete);


	// connections
	connect(actionDelete, &QAction::triggered, [this]()
	{
		if(m_curItem)
		{
			delete m_curItem;
			m_curItem = nullptr;
			emit SignalSelectedItem(nullptr);
		}
	});


	// settings
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	setInteractive(true);
	setMouseTracking(true);
	setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
}


QmView::~QmView()
{
}


/**
 * clear currently selected component
 */
void QmView::Clear()
{
	m_curItem = nullptr;
	emit SignalSelectedItem(nullptr);
}


/**
 * change the configuration of the current component
 */
void QmView::SetCurItemConfig(const ComponentConfigs& cfg)
{
	if(!m_curItem)
		return;

	m_curItem->SetConfig(cfg);

	// redraw the component
	//m_curItem->update();
	viewport()->update();

	// refresh properties widget in case the ranges have changed
	if(cfg.configs.size() && cfg.configs.begin()->key == "num_qbits")
	{
		QMetaObject::invokeMethod(this, [this]() -> void
		{
			emit this->SignalSelectedItem(this->m_curItem);
		}, Qt::QueuedConnection);
	}
}


void QmView::resizeEvent(QResizeEvent* evt)
{
	QPointF pt1{mapToScene(QPoint{0,0})};
	QPointF pt2{mapToScene(QPoint{evt->size().width(), evt->size().height()})};

	// TODO: include bounds given by all components

	setSceneRect(QRectF{pt1, pt2});
	QGraphicsView::resizeEvent(evt);
}


void QmView::mousePressEvent(QMouseEvent* evt)
{
	bool mouse_fully_handled = false;

	QPoint posVP = evt->pos();
	//QPointF posScene = mapToScene(posVP);

	QList<QGraphicsItem*> items = this->items(posVP);
	const QGraphicsItem *oldItem = m_curItem;

	if(items.size())
		m_curItem = dynamic_cast<QuantumGateItem*>(*items.begin());
	else
		m_curItem = nullptr;

	if(m_curItem != oldItem)
	{
		emit SignalSelectedItem(m_curItem);
		viewport()->update();
	}

	// show context menu on right click on a component
	if(m_curItem && (evt->buttons()&Qt::RightButton))
	{
		QPoint posGlobal = mapToGlobal(posVP);
		posGlobal.rx() += 8;
		posGlobal.ry() += 8;
		m_context->popup(posGlobal);

		mouse_fully_handled = true;
	}

	if(!mouse_fully_handled)
		QGraphicsView::mousePressEvent(evt);
}


void QmView::mouseReleaseEvent(QMouseEvent* evt)
{
	QGraphicsView::mouseReleaseEvent(evt);
}


void QmView::mouseMoveEvent(QMouseEvent* evt)
{
	QPoint posVP = evt->pos();
	QPointF posScene = mapToScene(posVP);

	emit SignalMouseCoordinates(posScene.x(), posScene.y());

	QGraphicsView::mouseMoveEvent(evt);
}


void QmView::paintEvent(QPaintEvent* evt)
{
	QGraphicsView::paintEvent(evt);

	// draw a selection rectangle around the current item
	if(m_curItem)
	{
		// local to scene coordinate trafo
		QTransform trafo = m_curItem->sceneTransform();
		QPolygonF polyScene = trafo.map(m_curItem->boundingRect());
		// scene to viewport coordinate trafo
		QPolygon poly = mapFromScene(polyScene);

		// line colour
		const QColor& colour_fg = get_foreground_colour();
		const QColor& colour_bg = get_background_colour();
		QColor colour_line = lerp(colour_fg, colour_bg, 0.5);

		// selection rectangle pen
		QPen pen(colour_line);
		pen.setWidthF(1.);
		pen.setDashPattern(QVector<qreal>{{ 4, 2 }});

		// paint the selection rectangle
		QPainter painter(viewport());
		painter.setPen(pen);
		painter.drawPolygon(poly);
	}
}

// ----------------------------------------------------------------------------
