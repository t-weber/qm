/**
 * settings
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_SETTINGS_H__
#define __QM_SETTINGS_H__

#include <QtGui/QColor>

#include "types.h"


// size of the components grid
extern t_real g_raster_size;


// basic colours
extern const QColor& get_foreground_colour();
extern const QColor& get_background_colour();


#endif
