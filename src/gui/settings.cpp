/**
 * settings
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#include "settings.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QPalette>


t_real g_raster_size = 35.;


const QColor& get_foreground_colour()
{
	QPalette palette = dynamic_cast<QGuiApplication*>(
		QGuiApplication::instance())->palette();
	const QColor& col = palette.color(QPalette::WindowText);

	return col;
}


const QColor& get_background_colour()
{
	QPalette palette = dynamic_cast<QGuiApplication*>(
		QGuiApplication::instance())->palette();
	const QColor& col = palette.color(QPalette::Window);

	return col;
}
