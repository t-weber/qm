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
#include "globals.h"
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
	void DeleteQuantumComponent(QuantumComponentItem *comp);
	const std::vector<QuantumComponentItem*>& GetQuantumComponents() const
	{ return m_components; }

	bool IsQuantumComponent(const QGraphicsItem *item) const;
	InputStates* GetCorrespondingInputState(QuantumComponentItem* comp) const;
	std::vector<QuantumComponentItem*> GetCorrespondingGates(QuantumComponentItem* input_state) const;
	std::vector<QuantumComponentItem*> GetCorrespondingGatesApprox(QuantumComponentItem* input_state) const;
	std::vector<QuantumComponentItem*> GetAllInputStates() const;

	bool Calculate(QuantumComponentItem* input_state) const;
	bool Calculate(std::vector<QuantumComponentItem*>& input_states) const;


protected:
	virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

	virtual void mousePressEvent(QGraphicsSceneMouseEvent *evt) override;
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *evt) override;
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *evt) override;


private:
	std::vector<QuantumComponentItem*> m_components{};
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

	QuantumComponentItem* GetCurItem() { return m_curItem; }
	const QuantumComponentItem* GetCurItem() const { return m_curItem; }

	const QPointF& GetCursorPosition(bool on_grid = true) const
	{ return on_grid ? m_curRasterScenePos : m_curScenePos; }

	QPointF GetSafePos(QGraphicsItem* item, const QPointF& posOrg) const;


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
	// corresponding scene
	QmScene *m_scene = nullptr;

	// currently selected item
	QuantumComponentItem *m_curItem = nullptr;
	QuantumComponentItem *m_copiedItem = nullptr;
	std::vector<QuantumComponentItem*> m_copiedCorrespondingGates{};
	std::tuple<t_int, t_int> m_copiedGridPos{0, 0};
	bool m_curItemIsDragged = false;

	// gates connected to current item
	std::vector<QuantumComponentItem*> m_curGates{};

	// context menu for a selected item
	std::shared_ptr<QMenu> m_context{};
	// context menu in case no item has been selected
	std::shared_ptr<QMenu> m_contextNoItem{};

	// cursor position in scene
	QPointF m_curScenePos{0, 0};
	// cursor position in grid
	QPointF m_curRasterScenePos{0, 0};


public slots:
	// change the configuration of the current component
	void SetCurItemConfig(const ComponentConfigs& cfg);

	void DeleteCurItem();
	void CopyCurItem();
	void PasteItem();

	// functions to delegate to the scene
	bool CalculateCurItem();
	bool Calculate(QuantumComponentItem *input_state);
	void AddQuantumComponent(QuantumComponentItem *comp, bool no_signals=false);
	void DeleteQuantumComponent(QuantumComponentItem *comp);


signals:
	void SignalMouseCoordinates(qreal scene_x, qreal scene_y);
	void SignalSelectedItem(QuantumComponent *item,
		InputStates *associated_input_comp = nullptr);
	void SignalWorkspaceChanged(bool changed = true);
	void SignalNewResults(const QuantumComponent *item,
		const InputStates *associated_input_comp = nullptr,
		bool ok = true);
};


#endif
