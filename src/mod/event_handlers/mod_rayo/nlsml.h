/*
 * mod_rayo for FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2013-2014, Grasshopper
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mod_rayo for FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is Grasshopper
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * Chris Rienzo <chris.rienzo@grasshopper.com>
 *
 * nlsml.h -- Parses / creates NLSML results
 *
 */
#ifndef NLSML_H
#define NLSML_H

#include <iksemel.h>
#include <switch.h>

enum nlsml_match_type {
	NMT_BAD_XML,
	NMT_MATCH,
	NMT_NOINPUT,
	NMT_NOMATCH
};

extern int nlsml_init(void);
extern void nlsml_destroy(void);
enum nlsml_match_type nlsml_parse(const char *result, const char *uuid);
iks *nlsml_normalize(const char *result);
extern iks *nlsml_create_dtmf_match(const char *digits, const char *interpretation);

#endif

/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4 noet
 */
