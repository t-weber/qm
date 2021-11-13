/**
 * settings
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_SETTINGS_H__
#define __QM_SETTINGS_H__

#include <QtGui/QColor>
#include <QtWidgets/QDialog>

#include "types.h"


// size of the components grid
extern t_real g_raster_size;

// snap component to the grid while dragging
extern bool g_snap_on_move;

// keep the relative position of the gates when
// moving the input states component
extern bool g_keep_gates_on_states;


// basic colours
extern const QColor& get_foreground_colour();
extern const QColor& get_background_colour();



/**
 * settings dialog
 */
class Settings : public QDialog
{
public:
	Settings(QWidget *parent=nullptr);
	virtual ~Settings() = default;
};

#endif
