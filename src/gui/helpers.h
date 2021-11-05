/**
 * helper functions
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_HELPERS_H__
#define __QM_HELPERS_H__

#include <cmath>

#include <QtGui/QColor>


/**
 * interpolate between two colours
 */
template<class t_real = qreal>
QColor lerp(const QColor& col1, const QColor& col2, t_real t)
{
	return QColor::fromRgbF(
		std::lerp(col1.redF(), col2.redF(), t),
		std::lerp(col1.greenF(), col2.greenF(), t),
		std::lerp(col1.blueF(), col2.blueF(), t),
		std::lerp(col1.alphaF(), col2.alphaF(), t)
	);
}

#endif
