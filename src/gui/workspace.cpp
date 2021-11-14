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
#include <QtWidgets/QMessageBox>

#include <cmath>

#include "qm_gui.h"
#include "helpers.h"
#include "components_table.h"


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
 * is the given graphics item a known component?
 */
bool QmScene::IsQuantumComponent(const QGraphicsItem *item) const
{
	for(const QuantumComponentItem *comp : GetQuantumComponents())
	{
		if(item == comp)
			return true;
	}

	return false;
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
 * get the input state component associated with a given gate
 */
QuantumComponentItem*
QmScene::GetCorrespondingInputState(QuantumComponentItem* comp) const
{
	if(!comp)
		return nullptr;

	// the component already is an input state component
	if(comp->GetType() == ComponentType::STATE)
		return comp;

	// grid position of the selected component
	auto [col_pos, row_pos] = comp->GetGridPos();

	for(auto* _input_comp : GetQuantumComponents())
	{
		// look for input state components
		if(!_input_comp || _input_comp->GetType() != ComponentType::STATE)
			continue;

		// check if the components lies within the bounds of the input state
		InputStates *input_comp = static_cast<InputStates*>(_input_comp);
		auto [input_col_pos, input_row_pos] = input_comp->GetGridPos();
		t_int input_height = input_comp->GetNumQBits();
		t_int input_width = input_comp->GetWidth();

		if((col_pos >= input_col_pos && col_pos < input_col_pos+input_width) &&
			(row_pos >= input_row_pos && row_pos < input_row_pos+input_height))
			return input_comp;
	}

	return nullptr;
}


/**
 * get all gates associated with a given input gate
 */
std::vector<QuantumComponentItem*>
QmScene::GetCorrespondingGates(QuantumComponentItem* _input_comp) const
{
	std::vector<QuantumComponentItem*> gates;

	if(!_input_comp || _input_comp->GetType() != ComponentType::STATE)
		return gates;

	// get input state dimensions
	InputStates *input_comp = static_cast<InputStates*>(_input_comp);
	auto [input_col_pos, input_row_pos] = input_comp->GetGridPos();
	t_int input_height = input_comp->GetNumQBits();
	t_int input_width = input_comp->GetWidth();

	for(auto* gate : GetQuantumComponents())
	{
		// look for gate components
		if(!gate || gate->GetType() != ComponentType::GATE)
			continue;

		// grid position of the selected component
		auto [col_pos, row_pos] = gate->GetGridPos();

		// check if the gate lies within the bounds of the input state
		if((col_pos >= input_col_pos && col_pos < input_col_pos+input_width) &&
			(row_pos >= input_row_pos && row_pos < input_row_pos+input_height))
			gates.push_back(gate);
	}

	return gates;
}


/**
 * get all gates associated with a given input gate
 * (approximate version using scene coordinates used for dragging)
 */
std::vector<QuantumComponentItem*>
QmScene::GetCorrespondingGatesApprox(QuantumComponentItem* _input_comp) const
{
	std::vector<QuantumComponentItem*> gates;

	if(!_input_comp || _input_comp->GetType() != ComponentType::STATE)
		return gates;

	// get input state dimensions
	InputStates *input_comp = static_cast<InputStates*>(_input_comp);
	QPointF input_pos = input_comp->scenePos();
	t_real input_height = input_comp->GetNumQBits() * g_raster_size;
	t_real input_width = input_comp->GetWidth() * g_raster_size;

	t_real eps = g_raster_size * 0.25;

	for(auto* gate : GetQuantumComponents())
	{
		// look for gate components
		if(!gate || gate->GetType() != ComponentType::GATE)
			continue;

		// grid position of the selected component
		QPointF pos = gate->scenePos();

		// check if the gate lies within the bounds of the input state
		if((pos.x()+eps >= input_pos.x() && pos.x()-eps <= input_pos.x()+input_width) &&
			(pos.y()+eps >= input_pos.y() && pos.y()-eps <= input_pos.y()+input_height))
			gates.push_back(gate);
	}

	return gates;
}


/**
 * return all input state components
 */
std::vector<QuantumComponentItem*> QmScene::GetAllInputStates() const
{
	std::vector<QuantumComponentItem*> states;

	for(auto* comp : GetQuantumComponents())
	{
		if(comp && comp->GetType() == ComponentType::STATE)
			states.push_back(comp);
	}

	return states;
}


/**
 * calculate the circuit associated with the given input state
 */
bool QmScene::Calculate(QuantumComponentItem* _input_comp) const
{
	if(!_input_comp || _input_comp->GetType() != ComponentType::STATE)
		return false;

	// arrange the gates in a table
	InputStates *input_comp = static_cast<InputStates*>(_input_comp);
	std::vector<QuantumComponentItem*> gates = GetCorrespondingGates(input_comp);

	auto [input_col_pos, input_row_pos] = input_comp->GetGridPos();
	ComponentsTable tab{input_comp->GetNumQBits(), input_comp->GetWidth()};

	for(const QuantumComponentItem* gate : gates)
	{
		auto [gate_col_pos, gate_row_pos] = gate->GetGridPos();
		t_int col_pos = gate_col_pos - input_col_pos;
		t_int row_pos = gate_row_pos - input_row_pos;

		if(col_pos<0 || row_pos<0 || col_pos>=t_int(tab.col_size()) || row_pos>=t_int(tab.row_size()))
		{
			QString err{"Component \"%1\" is outside the grid. Position: (%2, %3)."};
			err = err.arg(gate->GetIdent().c_str()).arg(col_pos).arg(row_pos);

			QMessageBox::critical(static_cast<QWidget*>(parent()), "Error", err);
			return false;
		}

		if(tab(row_pos, col_pos))
		{
			QString err{"Cannot insert \"%1\" since position (%2, %3) is already occupied by \"%4\"."};
			err = err.arg(gate->GetIdent().c_str()).
				arg(col_pos).arg(row_pos).
				arg(tab(row_pos, col_pos)->GetIdent().c_str());

			QMessageBox::critical(static_cast<QWidget*>(parent()), "Error", err);
			return false;
		}

		tab(row_pos, col_pos) = gate;
	}

	// is the circuit correct?
	if(!tab.CheckCircuit())
	{
		QMessageBox::critical(static_cast<QWidget*>(parent()),
			"Error", "Invalid circuit configuration.");
		return false;
	}


	//std::cout << tab << std::endl;
	// TODO: calculate total gate of the circuit

	return true;
}


/**
 * calculate the circuits associated with the given input states
 */
bool QmScene::Calculate(std::vector<QuantumComponentItem*>& input_states) const
{
	bool ok = true;

	for(QuantumComponentItem* input_state : input_states)
	{
		if(!Calculate(input_state))
			ok = false;
	}

	return ok;
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
	//m_clickScenePos = evt->scenePos();

	QGraphicsScene::mousePressEvent(evt);
}


void QmScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *evt)
{
	QGraphicsScene::mouseReleaseEvent(evt);
}


void QmScene::mouseMoveEvent(QGraphicsSceneMouseEvent *evt)
{
	QGraphicsScene::mouseMoveEvent(evt);
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

	QAction *actionCalc = new QAction(iconDelete, "Calculate Circuit", m_context.get());

	m_context->addAction(actionCopy);
	m_context->addAction(actionPaste);
	m_context->addAction(actionDelete);
	m_context->addSeparator();
	m_context->addAction(actionCalc);

	m_contextNoItem->addAction(actionPaste);


	// connections
	connect(actionCopy, &QAction::triggered, this, &QmView::CopyCurItem);
	connect(actionPaste, &QAction::triggered, this, &QmView::PasteItem);
	connect(actionDelete, &QAction::triggered, this, &QmView::DeleteCurItem);
	connect(actionCalc, &QAction::triggered, this, &QmView::CalculateCurItem);


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
 * calculate the circuit associated with the currently selected item
 */
void QmView::CalculateCurItem()
{
	auto *selected_comp = GetCurItem();
	auto *input_comp = m_scene->GetCorrespondingInputState(selected_comp);

	if(!input_comp)
	{
		QMessageBox::critical(this, "Error", "No input state component was selected.");
		return;
	}

	m_scene->Calculate(input_comp);
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
	clonedItem->setPos(GetCursorPosition(true));
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
	if(items.size() && m_scene->IsQuantumComponent(*items.begin()))
	{
		m_curItem = static_cast<QuantumComponentItem*>(*items.begin());
		m_curGates = m_scene->GetCorrespondingGates(m_curItem);
	}
	else
	{
		m_curItem = nullptr;
		m_curGates.clear();
	}

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


	// get the item being dragged
	QGraphicsItem* item = m_scene->mouseGrabberItem();

	// finish dragging the item
	QGraphicsView::mouseReleaseEvent(evt);

	// snap the item to the grid
	if(item)
	{
		item->setPos(snap_to_grid(item->scenePos()));

		// also snap the corresponding gates to the grid
		if(item==m_curItem && g_keep_gates_on_states)
		{
			// if this is an input state, move its gate by the same amount
			for(auto *gate : m_curGates)
				gate->setPos(snap_to_grid(gate->scenePos()));
		}
	}
}


void QmView::mouseMoveEvent(QMouseEvent* evt)
{
	QPoint posVP = evt->pos();
	m_curScenePos = mapToScene(posVP);
	m_curRasterScenePos = snap_to_grid(m_curScenePos);

	// get the item being dragged
	QGraphicsItem* item = m_scene->mouseGrabberItem();

	// original position of the item
	QPointF posOrig{0, 0};
	if(item)
		posOrig = item->scenePos();

	// drag the item
	QGraphicsView::mouseMoveEvent(evt);

	if(item)
	{
		// snap the item to the grid
		if(g_snap_on_move)
			item->setPos(snap_to_grid(item->scenePos()));

		if(item==m_curItem && g_keep_gates_on_states)
		{
			// new position of the item
			QPointF posNew = item->scenePos();
			QPointF vecMoved = posNew - posOrig;

			// if this is an input state, move its gate by the same amount
			for(auto *gate : m_curGates)
			{
				gate->setPos(gate->pos() + vecMoved);

				if(g_snap_on_move)
					gate->setPos(snap_to_grid(gate->scenePos()));
			}
		}
	}


	emit SignalMouseCoordinates(m_curScenePos.x(), m_curScenePos.y());
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
