/* 
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2011, Anthony Minessale II <anthm@freeswitch.org>
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
 * The Original Code is FreeSWITCH mod_spandsp.
 *
 * The Initial Developer of the Original Code is
 * Michael Jerris <mike@jerris.com
 *
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Massimo Cetra <devel@navynet.it>
 * Anthony Minessale II <anthm@freeswitch.org>
 * Brian West <brian@freeswitch.org>
 * Steve Underwood <steveu@coppice.org>
 * Antonio Gallo <agx@linux.it>
 * Christopher M. Rienzo <chris@rienzo.net>
 * mod_spandsp.c -- Module implementing spandsp fax, dsp, and codec functionality
 *
 */

#include "mod_spandsp.h"
#include <spandsp/version.h>
#include "mod_spandsp_modem.h"

/* **************************************************************************
   FREESWITCH MODULE DEFINITIONS
   ************************************************************************* */

struct spandsp_globals spandsp_globals = { 0 };

#define SPANFAX_RX_USAGE "<filename>"
#define SPANFAX_TX_USAGE "<filename>"

SWITCH_MODULE_LOAD_FUNCTION(mod_spandsp_init);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_spandsp_shutdown);
SWITCH_MODULE_DEFINITION(mod_spandsp, mod_spandsp_init, mod_spandsp_shutdown, NULL);

static switch_event_node_t *NODE = NULL;

SWITCH_STANDARD_APP(spanfax_tx_function)
{
	mod_spandsp_fax_process_fax(session, data, FUNCTION_TX);
}

SWITCH_STANDARD_APP(spanfax_rx_function)
{
	mod_spandsp_fax_process_fax(session, data, FUNCTION_RX);
}

SWITCH_STANDARD_APP(dtmf_session_function)
{
	spandsp_inband_dtmf_session(session);
}

SWITCH_STANDARD_APP(stop_dtmf_session_function)
{
	spandsp_stop_inband_dtmf_session(session);
}


SWITCH_STANDARD_APP(spandsp_fax_detect_session_function)
{
    int argc = 0;
	char *argv[4] = { 0 };
    char *dupdata;
    const char *app = NULL, *arg = NULL;
    int timeout = 0;
	int tone_type = MODEM_CONNECT_TONES_FAX_CNG;

    if (!zstr(data) && (dupdata = switch_core_session_strdup(session, data))) {
		if ((argc = switch_split(dupdata, ' ', argv)) >= 2) {
            app = argv[0];
            arg = argv[1];
			if (argc > 2) {
				timeout = atoi(argv[2]);
				if (timeout < 0) {
					timeout = 0;
				}
			}
			if (argc > 3) {
				if (!strcmp(argv[3], "ced")) {
					tone_type = MODEM_CONNECT_TONES_FAX_CED_OR_PREAMBLE;
				} else {
					tone_type = MODEM_CONNECT_TONES_FAX_CNG;
				}
			}
        }
    }
    
    if (app) {
        switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_NOTICE, "Enabling fax detection '%s' '%s'\n", argv[0], argv[1]);
		spandsp_fax_detect_session(session, "rw", timeout, tone_type, 1, app, arg, NULL);
    } else {
        switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, "Cannot Enable fax detection '%s' '%s'\n", argv[0], argv[1]);
    }
}

SWITCH_STANDARD_APP(spandsp_stop_fax_detect_session_function)
{
	switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_NOTICE, "Disabling fax detection\n");
    spandsp_fax_stop_detect_session(session);
}

static void event_handler(switch_event_t *event)
{
    load_configuration(1);
}

SWITCH_STANDARD_APP(t38_gateway_function)
{
    switch_channel_t *channel = switch_core_session_get_channel(session);
    time_t timeout = switch_epoch_time_now(NULL) + 20;
    const char *var;
    int argc = 0;
    char *argv[2] = { 0 };
    char *dupdata;
    const char *direction = NULL, *flags = NULL;
    
    if (!zstr(data) && (dupdata = switch_core_session_strdup(session, data))) {
        if ((argc = switch_split(dupdata, ' ', argv))) {
            if (argc > 0) {
                direction = argv[0];
            }
            
            if (argc > 1) {
                flags = argv[1];
            }
        }
    }

    if (zstr(direction) || strcasecmp(direction, "self")) {
        direction = "peer";
    }
    
    switch_channel_set_variable(channel, "t38_leg", direction);

    if (!zstr(flags) && !strcasecmp(flags, "nocng")) {
        t38_gateway_start(session, direction, NULL);
    } else {
        if ((var = switch_channel_get_variable(channel, "t38_gateway_detect_timeout"))) {
            long to = atol(var);
            if (to > -1) {
                timeout = (time_t) (switch_epoch_time_now(NULL) + to);
            } else {
                switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_WARNING, "%s invalid timeout value.\n", switch_channel_get_name(channel));
            }
        }

        //switch_ivr_tone_detect_session(session, "t38", "1100.0", "rw", timeout, 1, direction, NULL, t38_gateway_start);        
		spandsp_fax_detect_session(session, "rw", timeout, MODEM_CONNECT_TONES_FAX_CED_OR_PREAMBLE, 1, direction, NULL, t38_gateway_start);
    }
}

/**
 * Start tone detector application
 *
 * @param data the command string
 */
SWITCH_STANDARD_APP(start_tone_detect_app)
{
	switch_channel_t *channel;
	if (!session) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No session\n");
		return;
	}
	channel = switch_core_session_get_channel(session);
	if (zstr(data)) {
		switch_channel_set_variable(channel, SWITCH_CURRENT_APPLICATION_RESPONSE_VARIABLE, "-ERR missing descriptor name");
	} else if (callprogress_detector_start(session, data) != SWITCH_STATUS_SUCCESS) {
		switch_channel_set_variable(channel, SWITCH_CURRENT_APPLICATION_RESPONSE_VARIABLE, "-ERR failed to start tone detector");
	} else {
		switch_channel_set_variable(channel, SWITCH_CURRENT_APPLICATION_RESPONSE_VARIABLE, "+OK started");
	}
}

/**
 * Start tone detector API
 */
SWITCH_STANDARD_API(start_tone_detect_api)
{
	switch_status_t status = SWITCH_STATUS_SUCCESS;
	 switch_core_session_t *psession = NULL;

	if (zstr(cmd)) {
		stream->write_function(stream, "-ERR missing descriptor name\n");
		return SWITCH_STATUS_SUCCESS;
	}

    if (!(psession = switch_core_session_locate(cmd))) {
        stream->write_function(stream, "-ERR Cannot locate session\n");
        return SWITCH_STATUS_SUCCESS;
    }


	status = callprogress_detector_start(psession, cmd);

	if (status == SWITCH_STATUS_SUCCESS) {
		stream->write_function(stream, "+OK started\n");
	} else {
		stream->write_function(stream, "-ERR failed to start tone detector\n");
	}
    
    switch_core_session_rwunlock(psession);

	return status;
}

/**
 * Stop tone detector application
 *
 * @param data the command string
 */
SWITCH_STANDARD_APP(stop_tone_detect_app)
{
	switch_channel_t *channel;
	if (!session) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No session\n");
		return;
	}
	channel = switch_core_session_get_channel(session);
	callprogress_detector_stop(session);
	switch_channel_set_variable(channel, SWITCH_CURRENT_APPLICATION_RESPONSE_VARIABLE, "+OK stopped");
}

/**
 * Stop tone detector API
 */
SWITCH_STANDARD_API(stop_tone_detect_api)
{
	switch_status_t status = SWITCH_STATUS_SUCCESS;
	 switch_core_session_t *psession = NULL;

	if (zstr(cmd)) {
		stream->write_function(stream, "-ERR missing descriptor name\n");
		return SWITCH_STATUS_SUCCESS;
	}

    if (!(psession = switch_core_session_locate(cmd))) {
        stream->write_function(stream, "-ERR Cannot locate session\n");
        return SWITCH_STATUS_SUCCESS;
    }

    callprogress_detector_stop(psession);
    stream->write_function(stream, "+OK stopped\n");
    switch_core_session_rwunlock(psession);
    
	return status;
}


void mod_spandsp_indicate_data(switch_core_session_t *session, switch_bool_t self, switch_bool_t on)
{
    switch_core_session_t *target_session = NULL;
    int locked = 0;

    if (self) {
        target_session = session;
    } else {
        if (switch_core_session_get_partner(session, &target_session) == SWITCH_STATUS_SUCCESS) {
            locked = 1;
        } else {
            target_session = NULL;
        }
    }

    if (target_session) {
        switch_core_session_message_t *msg;
        
        msg = switch_core_session_alloc(target_session, sizeof(*msg));
        MESSAGE_STAMP_FFL(msg);
        msg->message_id = SWITCH_MESSAGE_INDICATE_AUDIO_DATA;
        msg->from = __FILE__;
        msg->numeric_arg = on;
		
        switch_core_session_queue_message(target_session, msg);

        if (locked) {
            switch_core_session_rwunlock(target_session);
            locked = 0;
        }
    }
}


/* **************************************************************************
   CONFIGURATION
   ************************************************************************* */

switch_status_t load_configuration(switch_bool_t reload)
{
	switch_xml_t xml = NULL, x_lists = NULL, x_list = NULL, cfg = NULL, callprogress = NULL, xdescriptor = NULL;
    switch_status_t status = SWITCH_STATUS_FALSE;

    switch_mutex_lock(spandsp_globals.mutex);

	if (spandsp_globals.tones) {
        switch_core_hash_destroy(&spandsp_globals.tones);
    }
    
    if (spandsp_globals.config_pool) {
        switch_core_destroy_memory_pool(&spandsp_globals.config_pool);
    }

    switch_core_new_memory_pool(&spandsp_globals.config_pool);
	switch_core_hash_init(&spandsp_globals.tones, spandsp_globals.config_pool);

    spandsp_globals.modem_dialplan = "XML";
    spandsp_globals.modem_context = "default";
    spandsp_globals.modem_count = 0;


    spandsp_globals.enable_t38 = 1;
	spandsp_globals.total_sessions = 0;
	spandsp_globals.verbose = 0;
	spandsp_globals.use_ecm = 1;
	spandsp_globals.disable_v17 = 0;
	spandsp_globals.prepend_string = switch_core_strdup(spandsp_globals.config_pool, "fax");
	spandsp_globals.spool = switch_core_strdup(spandsp_globals.config_pool, "/tmp");
    spandsp_globals.ident = "SpanDSP Fax Ident";
    spandsp_globals.header = "SpanDSP Fax Header";
    spandsp_globals.timezone = "";

	/* TODO make configuration param */
	spandsp_globals.tonedebug = 0;

	if ((xml = switch_xml_open_cfg("spandsp.conf", &cfg, NULL)) || (xml = switch_xml_open_cfg("fax.conf", &cfg, NULL))) {
        status = SWITCH_STATUS_SUCCESS;

		if ((x_lists = switch_xml_child(cfg, "modem-settings"))) {
			for (x_list = switch_xml_child(x_lists, "param"); x_list; x_list = x_list->next) {
				const char *name = switch_xml_attr(x_list, "name");
				const char *value = switch_xml_attr(x_list, "value");
                
				if (zstr(name)) {
					continue;
				}

				if (zstr(value)) {
					continue;
				}

                
				if (!reload && !strcmp(name, "total-modems")) {
                    int tmp = atoi(value);

                    if (tmp > -1 && tmp < MAX_MODEMS) {
                        spandsp_globals.modem_count = tmp;
                    } else {
                        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Invalid value [%d] for total-modems\n", tmp);
                    }
				} else if (!strcmp(name, "dialplan")) {
                    spandsp_globals.modem_dialplan = switch_core_strdup(spandsp_globals.config_pool, value);
				} else if (!strcmp(name, "context")) {
                    spandsp_globals.modem_context = switch_core_strdup(spandsp_globals.config_pool, value);
				} else if (!strcmp(name, "verbose")) {
                    if (switch_true(value)) {
                        spandsp_globals.modem_verbose = 1;
                    } else {
                        spandsp_globals.modem_verbose = 0;
                    }
                }
            }
        }

		if ((x_lists = switch_xml_child(cfg, "fax-settings")) || (x_lists = switch_xml_child(cfg, "settings"))) {
			for (x_list = switch_xml_child(x_lists, "param"); x_list; x_list = x_list->next) {
				const char *name = switch_xml_attr(x_list, "name");
				const char *value = switch_xml_attr(x_list, "value");

				if (zstr(name)) {
					continue;
				}

				if (zstr(value)) {
					continue;
				}

				if (!strcmp(name, "use-ecm")) {
					if (switch_true(value))
						spandsp_globals.use_ecm = 1;
					else
						spandsp_globals.use_ecm = 0;
				} else if (!strcmp(name, "verbose")) {
					if (switch_true(value))
						spandsp_globals.verbose = 1;
					else
						spandsp_globals.verbose = 0;
				} else if (!strcmp(name, "disable-v17")) {
					if (switch_true(value))
						spandsp_globals.disable_v17 = 1;
					else
						spandsp_globals.disable_v17 = 0;
				} else if (!strcmp(name, "enable-t38")) {
					if (switch_true(value)) {
						spandsp_globals.enable_t38= 1;
                    } else {
						spandsp_globals.enable_t38 = 0;
                    }
				} else if (!strcmp(name, "enable-t38-request")) {
					if (switch_true(value)) {
						spandsp_globals.enable_t38_request = 1;
                    } else {
						spandsp_globals.enable_t38_request = 0;
                    }
				} else if (!strcmp(name, "ident")) {
                    spandsp_globals.ident = switch_core_strdup(spandsp_globals.config_pool, value);
				} else if (!strcmp(name, "header")) {
                    spandsp_globals.header = switch_core_strdup(spandsp_globals.config_pool, value);
				} else if (!strcmp(name, "spool-dir")) {
					spandsp_globals.spool = switch_core_strdup(spandsp_globals.config_pool, value);
				} else if (!strcmp(name, "file-prefix")) {
					spandsp_globals.prepend_string = switch_core_strdup(spandsp_globals.config_pool, value);
				} else {
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Unknown parameter %s\n", name);
				}

			}
		}

        /* Configure call progress detector */
        if ((callprogress = switch_xml_child(cfg, "descriptors"))) {
            for (xdescriptor = switch_xml_child(callprogress, "descriptor"); xdescriptor; xdescriptor = switch_xml_next(xdescriptor)) {
                const char *name = switch_xml_attr(xdescriptor, "name");
                const char *tone_name = NULL;
                switch_xml_t tone = NULL, element = NULL;
                tone_descriptor_t *descriptor = NULL;

                /* create descriptor */
                if (zstr(name)) {
                    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Missing <descriptor> name\n");
                    switch_goto_status(SWITCH_STATUS_FALSE, done);
                }
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Adding tone_descriptor: %s\n", name);
                if (tone_descriptor_create(&descriptor, name, spandsp_globals.config_pool) != SWITCH_STATUS_SUCCESS) {
                    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Unable to allocate tone_descriptor: %s\n", name);
                    switch_goto_status(SWITCH_STATUS_FALSE, done);
                }
                switch_core_hash_insert(spandsp_globals.tones, name, descriptor);

                /* add tones to descriptor */
                for (tone = switch_xml_child(xdescriptor, "tone"); tone; tone = switch_xml_next(tone)) {
                    int id = 0;
                    tone_name = switch_xml_attr(tone, "name");
                    if (zstr(tone_name)) {
                        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Missing <tone> name for <descriptor> %s\n", name);
                        switch_goto_status(SWITCH_STATUS_FALSE, done);
                    }
                    id = tone_descriptor_add_tone(descriptor, tone_name);
                    if (id == -1) {
                        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, 
                                          "Unable to add tone_descriptor: %s, tone: %s.  (too many tones)\n", name, tone_name);
                        switch_goto_status(SWITCH_STATUS_FALSE, done);
                    }
                    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, 
                                      "Adding tone_descriptor: %s, tone: %s(%d)\n", name, tone_name, id);
                    /* add elements to tone */
                    for (element = switch_xml_child(tone, "element"); element; element = switch_xml_next(element)) {
                        const char *freq1_attr = switch_xml_attr(element, "freq1");
                        const char *freq2_attr = switch_xml_attr(element, "freq2");
                        const char *min_attr = switch_xml_attr(element, "min");
                        const char *max_attr = switch_xml_attr(element, "max");
                        int freq1, freq2, min, max;
                        if (zstr(freq1_attr)) {
                            freq1 = 0;
                        } else {
                            freq1 = atoi(freq1_attr);
                        }
                        if (zstr(freq2_attr)) {
                            freq2 = 0;
                        } else {
                            freq2 = atoi(freq2_attr);
                        }
                        if (zstr(min_attr)) {
                            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, 
                                              "Missing min in <element> of <descriptor> %s <tone> %s(%d)\n", name, tone_name, id);
                            switch_goto_status(SWITCH_STATUS_FALSE, done);
                        }
                        min = atoi(min_attr);
                        if (zstr(max_attr)) {
                            max = 0;
                        } else {
                            max = atoi(max_attr);
                        }
                        /* check params */
                        if ((freq1 < 0 || freq2 < 0 || min < 0 || max < 0) || (freq1 == 0 && min == 0 && max == 0)) {
                            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Invalid element param.\n");
                            switch_goto_status(SWITCH_STATUS_FALSE, done);
                        }
                        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, 
                                          "Adding tone_descriptor: %s, tone: %s(%d), element (%d, %d, %d, %d)\n", name, tone_name, id, freq1, freq2, min, max);
                        tone_descriptor_add_tone_element(descriptor, id, freq1, freq2, min, max);
                    }
                }
            }
        }
        
    done:
        
		switch_xml_free(xml);
	}

    switch_mutex_unlock(spandsp_globals.mutex);

    return status;
}


SWITCH_MODULE_LOAD_FUNCTION(mod_spandsp_init)
{
	switch_application_interface_t *app_interface;
	switch_api_interface_t *api_interface;

	memset(&spandsp_globals, 0, sizeof(spandsp_globals));
	spandsp_globals.pool = pool;

	*module_interface = switch_loadable_module_create_module_interface(pool, modname);
	switch_mutex_init(&spandsp_globals.mutex, SWITCH_MUTEX_NESTED, pool);

	SWITCH_ADD_APP(app_interface, "t38_gateway", "Convert to T38 Gateway if tones are heard", "Convert to T38 Gateway if tones are heard", 
                   t38_gateway_function, "", SAF_MEDIA_TAP);

	SWITCH_ADD_APP(app_interface, "rxfax", "FAX Receive Application", "FAX Receive Application", spanfax_rx_function, SPANFAX_RX_USAGE,
				   SAF_SUPPORT_NOMEDIA);
	SWITCH_ADD_APP(app_interface, "txfax", "FAX Transmit Application", "FAX Transmit Application", spanfax_tx_function, SPANFAX_TX_USAGE,
				   SAF_SUPPORT_NOMEDIA);

	SWITCH_ADD_APP(app_interface, "spandsp_stop_dtmf", "stop inband dtmf", "Stop detecting inband dtmf.", stop_dtmf_session_function, "", SAF_NONE);
	SWITCH_ADD_APP(app_interface, "spandsp_start_dtmf", "Detect dtmf", "Detect inband dtmf on the session", dtmf_session_function, "", SAF_MEDIA_TAP);

	SWITCH_ADD_APP(app_interface, "spandsp_start_fax_detect", "start fax detect", "start fax detect", spandsp_fax_detect_session_function, 
				   "<app>[ <arg>][ <timeout>][ <tone_type>]", SAF_NONE);

	SWITCH_ADD_APP(app_interface, "spandsp_stop_fax_detect", "stop fax detect", "stop fax detect", spandsp_stop_fax_detect_session_function, "", SAF_NONE);
                   
    load_configuration(0);

	mod_spandsp_fax_load(pool);
    mod_spandsp_codecs_load(module_interface, pool);

    
	if (mod_spandsp_dsp_load(module_interface, pool) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Couldn't load or process spandsp.conf, not adding tone_detect applications\n");
    } else {
        SWITCH_ADD_APP(app_interface, "start_tone_detect", "Start background tone detection with cadence", "", start_tone_detect_app, "[name]", SAF_NONE);
        SWITCH_ADD_APP(app_interface, "stop_tone_detect", "Stop background tone detection with cadence", "", stop_tone_detect_app, "", SAF_NONE);
        SWITCH_ADD_API(api_interface, "start_tone_detect", "Start background tone detection with cadence", start_tone_detect_api, "[name]");
        SWITCH_ADD_API(api_interface, "stop_tone_detect", "Stop background tone detection with cadence", stop_tone_detect_api, "");
	}

	if ((switch_event_bind_removable(modname, SWITCH_EVENT_RELOADXML, NULL, event_handler, NULL, &NODE) != SWITCH_STATUS_SUCCESS)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Couldn't bind our reloadxml handler!\n");
		/* Not such severe to prevent loading */
	}


	modem_global_init(module_interface, pool);

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "mod_spandsp loaded, using spandsp library version [%s]\n", SPANDSP_RELEASE_DATETIME_STRING);

	return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_spandsp_shutdown)
{
	switch_event_unbind(&NODE);

	mod_spandsp_fax_shutdown();
	mod_spandsp_dsp_shutdown();
	modem_global_shutdown();

    if (spandsp_globals.tones) {
        switch_core_hash_destroy(&spandsp_globals.tones);
    }

    if (spandsp_globals.config_pool) {
        switch_core_destroy_memory_pool(&spandsp_globals.config_pool);
    }

	return SWITCH_STATUS_UNLOAD;
}

/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:nil
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4:
 */
