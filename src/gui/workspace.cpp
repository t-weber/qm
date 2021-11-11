/**
 * qm graphics workspace
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Oct-2021
 * @license see 'LICENSE' file
 */

#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QGraphicsSceneMouseEvent>

#include <cmath>

#include "qm_gui.h"
#include "helpers.h"


// ----------------------------------------------------------------------------
// graphics scene
// ----------------------------------------------------------------------------
QmScene::QmScene(QWidget* parent) : QGraphicsScene(parent)
{
}


QmScene::~QmScene()
{
}


/**
 * insert a quantum gate into the scene
 */
void QmScene::AddQuantumComponent(QuantumComponentItem *gate)
{
	m_components.push_back(gate);
	addItem(gate);
}


/**
 * clear all components in the scene
 */
void QmScene::Clear()
{
	clear();
	m_components.clear();
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

	QPointF halfgrid(0.5*g_raster_size, 0.5*g_raster_size);
	QPointF start = snap_to_grid(topLeft) - halfgrid;
	QPointF end = snap_to_grid(bottomRight) + halfgrid;

	QPen pen(colour_line);
	pen.setWidthF(0.5);
	painter->setPen(pen);

	// horizontal guide lines
	for(t_real y=start.y(); y<end.y(); y+=g_raster_size)
	{
		painter->drawLine(
			QPointF(start.x(), y),
			QPointF(end.x(), y));
	}

	// vertical guide lines
	for(t_real x=start.x(); x<end.x(); x+=g_raster_size)
	{
		painter->drawLine(
			QPointF(x, start.y()),
			QPointF(x, end.y()));
	}
}


void QmScene::mousePressEvent(QGraphicsSceneMouseEvent *evt)
{
	QGraphicsScene::mousePressEvent(evt);
}


void QmScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *evt)
{
	// get the item being dragged
	QGraphicsItem* item = mouseGrabberItem();

	// finish dragging the item
	QGraphicsScene::mouseReleaseEvent(evt);

	// snap the item to the grid
	if(item)
		item->setPos(snap_to_grid(item->scenePos()));
}


void QmScene::mouseMoveEvent(QGraphicsSceneMouseEvent *evt)
{
	m_curScenePos = evt->scenePos();
	m_curRasterScenePos = snap_to_grid(m_curScenePos);

	// get the item being dragged
	QGraphicsItem* item = mouseGrabberItem();

	// drag the item
	QGraphicsScene::mouseMoveEvent(evt);

	// snap the item to the grid
	if(item && g_snap_on_move)
		item->setPos(snap_to_grid(item->scenePos()));
}
// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// graphics view
// ----------------------------------------------------------------------------
QmView::QmView(QmScene *scene, QWidget *parent)
	: QGraphicsView(scene, parent), m_scene{scene}
{
	// context menus
	m_context = std::make_shared<QMenu>(this);
	m_contextNoItem = std::make_shared<QMenu>(this);

	QIcon iconCopy = QIcon::fromTheme("edit-copy");
	QAction *actionCopy = new QAction(iconCopy, "Copy Component", m_context.get());

	QIcon iconPaste = QIcon::fromTheme("edit-paste");
	QAction *actionPaste = new QAction(iconPaste, "Paste Component", m_context.get());

	QIcon iconDelete = QIcon::fromTheme("edit-delete");
	QAction *actionDelete = new QAction(iconDelete, "Delete Component", m_context.get());

	m_context->addAction(actionCopy);
	m_context->addAction(actionPaste);
	m_context->addSeparator();
	m_context->addAction(actionDelete);

	m_contextNoItem->addAction(actionPaste);


	// connections
	connect(actionCopy, &QAction::triggered, this, &QmView::CopyCurItem);
	connect(actionPaste, &QAction::triggered, this, &QmView::PasteItem);
	connect(actionDelete, &QAction::triggered, this, &QmView::DeleteCurItem);


	// settings
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	setInteractive(true);
	setMouseTracking(true);
	setDragMode(QGraphicsView::NoDrag);

	setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
}


QmView::~QmView()
{
	// remove any previous copy
	if(m_copiedItem)
	{
		delete m_copiedItem;
		m_copiedItem = nullptr;
	}
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


/**
 * delete the currently selected component
 */
void QmView::DeleteCurItem()
{
	if(!m_curItem)
		return;

	delete m_curItem;
	m_curItem = nullptr;
	emit SignalSelectedItem(nullptr);
}


/**
 * copy the currently selected component
 */
void QmView::CopyCurItem()
{
	// remove any previous copy
	if(m_copiedItem)
	{
		delete m_copiedItem;
		m_copiedItem = nullptr;
	}

	// clone the currently selected item
	if(m_curItem)
		m_copiedItem = m_curItem->clone();
}


/**
 * paste a new component
 */
void QmView::PasteItem()
{
	if(!m_copiedItem)
		return;

	auto *clonedItem = m_copiedItem->clone();
	clonedItem->setPos(m_scene->GetCursorPosition(true));
	m_scene->AddQuantumComponent(clonedItem);
}


void QmView::resizeEvent(QResizeEvent* evt)
{
	//QPointF pt1{mapToScene(QPoint{0,0})};
	QPointF topLeft{this->sceneRect().topLeft()};
	QPointF pt1{-g_raster_size*0.5, -g_raster_size*0.5};
	pt1.rx() = std::min(pt1.x(), topLeft.x());
	pt1.ry() = std::min(pt1.y(), topLeft.y());

	QPointF pt2{mapToScene(QPoint{evt->size().width(), evt->size().height()})};

	QRectF sceneRect{pt1, pt2};
	FitAreaToScene(&sceneRect);

	QGraphicsView::resizeEvent(evt);
}


void QmView::FitAreaToScene(const QRectF *_sceneRect)
{
	if(!m_scene)
		return;

	// current scene rectangle
	QRectF sceneRect = _sceneRect ? *_sceneRect : this->sceneRect();
	QPointF pt1{sceneRect.topLeft()};
	QPointF pt2{sceneRect.bottomRight()};

	if(m_scene)
	{
		// include the bounds from all items in the scene
		for(const QGraphicsItem *item : m_scene->items())
		{
			QTransform trafo = item->sceneTransform();
			QPolygonF polyScene = trafo.map(item->boundingRect());

			for(const QPointF& pt : polyScene)
			{
				if(pt.x() < pt1.x()) pt1.setX(pt.x());
				if(pt.y() < pt1.y()) pt1.setY(pt.y());
				if(pt.x() > pt2.x()) pt2.setX(pt.x());
				if(pt.y() > pt2.y()) pt2.setY(pt.y());
			}
		}
	}

	setSceneRect(QRectF{pt1, pt2});
}


void QmView::mousePressEvent(QMouseEvent* evt)
{
	bool mouse_fully_handled = false;

	QPoint posVP = evt->pos();
	//QPointF posScene = mapToScene(posVP);

	QList<QGraphicsItem*> items = this->items(posVP);
	const QGraphicsItem *oldItem = m_curItem;

	// get current item under the cursor
	if(items.size())
		m_curItem = dynamic_cast<QuantumComponentItem*>(*items.begin());
	else
		m_curItem = nullptr;

	// a new item was selected
	if(m_curItem != oldItem)
	{
		emit SignalSelectedItem(m_curItem);
		viewport()->update();
	}

	// show context menu on right click on a component
	if(evt->buttons()&Qt::RightButton)
	{
		QPoint posGlobal = mapToGlobal(posVP);
		posGlobal.rx() += 8;
		posGlobal.ry() += 8;

		// show different context menu depending on whether an item is selected
		if(m_curItem)
			m_context->popup(posGlobal);
		else
			m_contextNoItem->popup(posGlobal);

		mouse_fully_handled = true;
	}

	if(!mouse_fully_handled)
		QGraphicsView::mousePressEvent(evt);
}


void QmView::mouseReleaseEvent(QMouseEvent* evt)
{
	if(m_curItem)
		FitAreaToScene();
	QGraphicsView::mouseReleaseEvent(evt);
}


void QmView::mouseMoveEvent(QMouseEvent* evt)
{
	QPoint posVP = evt->pos();
	QPointF posScene = mapToScene(posVP);

	emit SignalMouseCoordinates(posScene.x(), posScene.y());

	QGraphicsView::mouseMoveEvent(evt);
}


void QmView::wheelEvent(QWheelEvent *evt)
{
	const t_real angular_speed = 0.0075;
	const t_real angle_deg = evt->angleDelta().y() / 8.;
	const t_real zoom_factor = std::pow(2., angle_deg*angular_speed);

	if(angle_deg > 0.)
		centerOn(evt->position());
	scale(zoom_factor, zoom_factor);

	//QGraphicsView::wheelEvent(evt);
}


void QmView::keyPressEvent(QKeyEvent *evt)
{
	if(evt->key() == Qt::Key_Control)
		setDragMode(QGraphicsView::ScrollHandDrag);

	QGraphicsView::keyPressEvent(evt);
}


void QmView::keyReleaseEvent(QKeyEvent *evt)
{
	if(evt->key() == Qt::Key_Control)
		setDragMode(QGraphicsView::NoDrag);

	QGraphicsView::keyReleaseEvent(evt);
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
