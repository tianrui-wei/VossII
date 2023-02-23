//-------------------------------------------------------------------
// Copyright 2020 Carl-Johan Seger
// SPDX-License-Identifier: Apache-2.0
//-------------------------------------------------------------------

/************************************************
 *       Original author: Peter Seger 2017       *
 ************************************************/

#include "list_util.h"
#include <stdio.h>
#include <stdlib.h>

int Find_sep_distance(list_ptr one, list_ptr two);
list_ptr Shift_top_line(int dist, list_ptr line);
list_ptr Shift_bottom_line(int dist, list_ptr line);
list_ptr Get_top(list_ptr line1, list_ptr line2);
list_ptr Get_bottom(list_ptr line1, list_ptr line2);
list_ptr Shift_width(int dist, list_ptr line);
