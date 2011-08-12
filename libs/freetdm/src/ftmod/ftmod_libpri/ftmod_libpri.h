/*
 * Copyright (c) 2009, Anthony Minessale II
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of the original author; nor the names of any contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

typedef enum {
        SERVICE_CHANGE_STATUS_INSERVICE = 0,
        SERVICE_CHANGE_STATUS_MAINTENANCE = 1,
        SERVICE_CHANGE_STATUS_OUTOFSERVICE = 2
} service_change_status_t;     

#ifndef FTMOD_LIBPRI_H
#define FTMOD_LIBPRI_H
#include "freetdm.h"
#include "lpwrap_pri.h"

typedef enum {
	FTMOD_LIBPRI_OPT_NONE = 0,
	FTMOD_LIBPRI_OPT_SUGGEST_CHANNEL = (1 << 0),
	FTMOD_LIBPRI_OPT_OMIT_DISPLAY_IE = (1 << 1),
	FTMOD_LIBPRI_OPT_OMIT_REDIRECTING_NUMBER_IE = (1 << 2),
	FTMOD_LIBPRI_OPT_FACILITY_AOC = (1 << 3),

	FTMOD_LIBPRI_OPT_MAX = (1 << 4)
} ftdm_isdn_opts_t;

typedef enum {
	FTMOD_LIBPRI_RUNNING = (1 << 0)
} ftdm_isdn_flag_t;

typedef enum {
	FTMOD_LIBPRI_OVERLAP_NONE    = 0,
	FTMOD_LIBPRI_OVERLAP_RECEIVE = (1 << 0),
	FTMOD_LIBPRI_OVERLAP_SEND    = (1 << 1)
#define FTMOD_LIBPRI_OVERLAP_BOTH	(FTMOD_LIBPRI_OVERLAP_RECEIVE | FTMOD_LIBPRI_OVERLAP_SEND)
} ftdm_isdn_overlap_t;

struct ftdm_libpri_data {
	ftdm_channel_t *dchan;
	ftdm_isdn_opts_t opts;
	uint32_t flags;
	uint32_t debug_mask;

	int mode;
	int dialect;
	int overlap;		/*!< Overlap dial flags */
	unsigned int layer1;
	unsigned int ton;
	unsigned int service_message_support;

	lpwrap_pri_t spri;
};

typedef struct ftdm_libpri_data ftdm_libpri_data_t;

#endif

/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4:
 */

