/**
 * global settings variables
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date Nov-2021
 * @license see 'LICENSE' file
 */

#include "globals.h"
#include <limits>


t_real g_eps = std::numeric_limits<t_real>::epsilon();
t_int g_prec_gui = 4;

t_real g_raster_size = 35.;
bool g_snap_on_move = false;
bool g_keep_gates_on_states = true;

bool g_reverse_state_numbering = true;
