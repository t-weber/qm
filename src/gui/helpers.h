/**
 * helper functions
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_HELPERS_H__
#define __QM_HELPERS_H__

#include <QtCore/QPointF>
#include <QtGui/QColor>
#include <QtWidgets/QWidget>

#include <tuple>
#include <cmath>

#include "types.h"
#include "globals.h"

#include "lib/math_concepts.h"


/**
 * concept for a point, e.g. a QPointF
 */
template<class t_point>
concept is_point = requires(const t_point& pt, decltype(pt.x()) x, decltype(pt.y()) y)
{
	t_point(x, y);

	pt.x();
	pt.y();
};


/**
 * interpolate between two colours
 */
template<class t_real = qreal> requires m::is_scalar<t_real>
QColor lerp(const QColor& col1, const QColor& col2, t_real t)
{
	return QColor::fromRgbF(
		std::lerp(col1.redF(), col2.redF(), static_cast<qreal>(t)),
		std::lerp(col1.greenF(), col2.greenF(), static_cast<qreal>(t)),
		std::lerp(col1.blueF(), col2.blueF(), static_cast<qreal>(t)),
		std::lerp(col1.alphaF(), col2.alphaF(), static_cast<qreal>(t))
	);
}


/**
 * get grid tile index from position
 */
template<class t_real = qreal> requires m::is_scalar<t_real>
std::tuple<t_int, t_int> get_grid_indices(
	t_real x, t_real y,
	t_real raster_w = g_raster_size,
	t_real raster_h = g_raster_size)
{
	t_int raster_x = static_cast<t_int>(std::round(x / raster_w));
	t_int raster_y = static_cast<t_int>(std::round(y / raster_h));

	return std::make_tuple(raster_x, raster_y);
}


/**
 * get grid tile index from position
 */
template<class t_point = QPointF, class t_real = decltype(t_point{}.x())>
requires is_point<t_point> && m::is_scalar<t_real>
std::tuple<t_int, t_int> get_grid_indices(
	const t_point& pt,
	t_real raster_w = g_raster_size,
	t_real raster_h = g_raster_size)
{
	return get_grid_indices<t_real>(pt.x(), pt.y(), raster_w, raster_h);
}


/**
 * snap to position on the grid
 */
template<class t_point = QPointF, class t_real = decltype(t_point{}.x())>
requires is_point<t_point> && m::is_scalar<t_real>
t_point snap_to_grid(
	const t_point& pt,
	t_real raster_w = g_raster_size,
	t_real raster_h = g_raster_size)
{
	t_real raster_x = std::round(pt.x() / raster_w);
	t_real raster_y = std::round(pt.y() / raster_h);

	raster_x *= raster_w;
	raster_y *= raster_h;

	return t_point{raster_x, raster_y};
}


/**
 * show a dialog
 * @see https://doc.qt.io/qt-5/qdialog.html#code-examples
 */
static inline void show_dialog(QWidget *dlg)
{
	if(!dlg)
		return;

	dlg->show();
	dlg->raise();
	dlg->activateWindow();
}

#endif
