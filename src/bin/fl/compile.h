//-------------------------------------------------------------------
// Copyright 2020 Carl-Johan Seger
// SPDX-License-Identifier: Apache-2.0
//-------------------------------------------------------------------

/********************************************************************
*                                                                   *
*     Original author: Carl-Johan Seger 1995                        *
*                                                                   *
*********************************************************************/
/* compile.h -- header for compile.c */

#ifdef EXPORT_FORWARD_DECL
/* --- Forward declarations that need to be exported to earlier .h files --- */

/* -------- Function prototypes for exported functions -------- */
g_ptr	Optimise(g_ptr node);
g_ptr   reorder(g_ptr node);

#else /* EXPORT_FORWARD_DECL */
/* ----------------------- Main include file ------------------------------- */
#ifndef COMPILE_H
#define COMPILE_H
#include "fl.h"	/* Global data types and include files 		     */

typedef struct env_rec	*env_ptr;

typedef struct env_rec {
	string	var;
	int	n;
	env_ptr	next;
} env_rec;

#endif /* COMPILE_H */
#endif /* EXPORT_FORWARD_DECL */
