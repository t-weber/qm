/**
 * global settings variables
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#ifndef __QM_GLOBALS_H__
#define __QM_GLOBALS_H__

#include "types.h"


// epsilon and precision values
extern t_real g_eps;
extern t_int g_prec_gui;


// size of the components grid
extern t_real g_raster_size;

// snap component to the grid while dragging
extern bool g_snap_on_move;

// keep the relative position of the gates when
// moving the input states component
extern bool g_keep_gates_on_states;


#endif
