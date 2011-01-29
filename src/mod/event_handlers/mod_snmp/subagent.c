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
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Anthony Minessale II <anthm@freeswitch.org>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * Daniel Swarbrick <daniel.swarbrick@seventhsignal.de>
 * Stefan Knoblich <s.knoblich@axsentis.de>
 * 
 * mod_snmp.c -- SNMP AgentX Subagent Module
 *
 */
#include <switch.h>
#include <switch_version.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "subagent.h"

netsnmp_table_registration_info *ch_table_info;
netsnmp_tdata *ch_table;
netsnmp_handler_registration *ch_reginfo;
uint32_t idx;


static int sql_count_callback(void *pArg, int argc, char **argv, char **columnNames)
{
	uint32_t *count = (uint32_t *) pArg;
	*count = atoi(argv[0]);
	return 0;
}


static int channelList_callback(void *pArg, int argc, char **argv, char **columnNames)
{
	chan_entry_t *entry;
	netsnmp_tdata_row *row;

	switch_zmalloc(entry, sizeof(chan_entry_t));
	if (!entry)
		return 0;

	row = netsnmp_tdata_create_row();
	if (!row) {
		switch_safe_free(entry);
		return 0;
	}
	row->data = entry;

	entry->idx = idx++;
	strncpy(entry->uuid, argv[0], sizeof(entry->uuid));
	strncpy(entry->direction, argv[1], sizeof(entry->direction));
	strncpy(entry->created, argv[2], sizeof(entry->created));
	strncpy(entry->name, argv[4], sizeof(entry->name));
	strncpy(entry->state, argv[5], sizeof(entry->state));
	strncpy(entry->cid_name, argv[6], sizeof(entry->cid_name));
	strncpy(entry->cid_num, argv[7], sizeof(entry->cid_num));

	netsnmp_tdata_row_add_index(row, ASN_INTEGER, &entry->idx, sizeof(entry->idx));
	netsnmp_tdata_add_row(ch_table, row);
	return 0;
}


void channelList_free(netsnmp_cache *cache, void *magic)
{
	netsnmp_tdata_row *row = netsnmp_tdata_row_first(ch_table);

	/* Delete table rows one by one */
	while (row) {
		netsnmp_tdata_remove_and_delete_row(ch_table, row);
		switch_safe_free(row->data);
		row = netsnmp_tdata_row_first(ch_table);
	}
}


int channelList_load(netsnmp_cache *cache, void *vmagic)
{
	switch_cache_db_handle_t *dbh;
	char sql[1024] = "", hostname[256] = "";

	channelList_free(cache, NULL);

	if (switch_core_db_handle(&dbh) != SWITCH_STATUS_SUCCESS) {
		return 0;
	}

	idx = 1;
	gethostname(hostname, sizeof(hostname));
	sprintf(sql, "SELECT * FROM channels WHERE hostname='%s' ORDER BY created_epoch", hostname);
	switch_cache_db_execute_sql_callback(dbh, sql, channelList_callback, NULL, NULL);

	switch_cache_db_release_db_handle(&dbh);

	return 0;
}


void init_subagent(switch_memory_pool_t *pool)
{
	static oid identity_oid[] = { 1,3,6,1,4,1,27880,1,1 };
	static oid systemStats_oid[] = { 1,3,6,1,4,1,27880,1,2 };
	static oid channelList_oid[] = { 1,3,6,1,4,1,27880,1,9 };

	DEBUGMSGTL(("init_subagent", "mod_snmp subagent initializing\n"));

	netsnmp_register_scalar_group(netsnmp_create_handler_registration("identity", handle_identity, identity_oid, OID_LENGTH(identity_oid), HANDLER_CAN_RONLY), 1, 2);
	netsnmp_register_scalar_group(netsnmp_create_handler_registration("systemStats", handle_systemStats, systemStats_oid, OID_LENGTH(systemStats_oid), HANDLER_CAN_RONLY), 1, 7);

	ch_table_info = switch_core_alloc(pool, sizeof(netsnmp_table_registration_info));
	netsnmp_table_helper_add_index(ch_table_info, ASN_INTEGER);
	ch_table_info->min_column = 1;
	ch_table_info->max_column = 7;
	ch_table = netsnmp_tdata_create_table("channelList", 0);
	ch_reginfo = netsnmp_create_handler_registration("channelList", handle_channelList, channelList_oid, OID_LENGTH(channelList_oid), HANDLER_CAN_RONLY);
	netsnmp_tdata_register(ch_reginfo, ch_table, ch_table_info);
	netsnmp_inject_handler(ch_reginfo, netsnmp_get_cache_handler(5, channelList_load, channelList_free, channelList_oid, OID_LENGTH(channelList_oid)));
}


int handle_identity(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
	netsnmp_request_info *request = NULL;
	oid subid;
	static char const version[] = SWITCH_VERSION_FULL;
	char uuid[40] = "";

	switch(reqinfo->mode) {
	case MODE_GET:
		subid = requests->requestvb->name[reginfo->rootoid_len - 2];

		switch (subid) {
		case ID_VERSION_STR:
			snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, (u_char *) &version, strlen(version));
			break;
		case ID_UUID:
			strncpy(uuid, switch_core_get_uuid(), sizeof(uuid));
			snmp_set_var_typed_value(requests->requestvb, ASN_OCTET_STR, (u_char *) &uuid, strlen(uuid));
			break;
		default:
			snmp_log(LOG_WARNING, "Unregistered OID-suffix requested (%d)\n", (int) subid);
			netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHOBJECT);
		}
		break;

	default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "Unknown mode (%d) in handle_identity\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	return SNMP_ERR_NOERROR;
}


int handle_systemStats(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
	netsnmp_request_info *request = NULL;
	oid subid;
	switch_time_t uptime;
	uint32_t int_val = 0;

	switch(reqinfo->mode) {
	case MODE_GET:
		subid = requests->requestvb->name[reginfo->rootoid_len - 2];

		switch (subid) {
		case SS_UPTIME:
			uptime = switch_core_uptime() / 10000;
			snmp_set_var_typed_value(requests->requestvb, ASN_TIMETICKS, (u_char *) &uptime, sizeof(uptime));
			break;
		case SS_SESSIONS_SINCE_STARTUP:
			int_val = switch_core_session_id() - 1;
			snmp_set_var_typed_value(requests->requestvb, ASN_COUNTER, (u_char *) &int_val, sizeof(int_val));
			break;
		case SS_CURRENT_SESSIONS:
			int_val = switch_core_session_count();
			snmp_set_var_typed_value(requests->requestvb, ASN_GAUGE, (u_char *) &int_val, sizeof(int_val));
			break;
		case SS_MAX_SESSIONS:
			switch_core_session_ctl(SCSC_MAX_SESSIONS, &int_val);
			snmp_set_var_typed_value(requests->requestvb, ASN_GAUGE, (u_char *) &int_val, sizeof(int_val));
			break;
		case SS_CURRENT_CALLS:
			{
			switch_cache_db_handle_t *dbh;
			char sql[1024] = "", hostname[256] = "";

			if (switch_core_db_handle(&dbh) != SWITCH_STATUS_SUCCESS) {
				return SNMP_ERR_GENERR;
			}

			gethostname(hostname, sizeof(hostname));
			sprintf(sql, "SELECT COUNT(*) FROM calls WHERE hostname='%s'", hostname);
			switch_cache_db_execute_sql_callback(dbh, sql, sql_count_callback, &int_val, NULL);
			snmp_set_var_typed_value(requests->requestvb, ASN_GAUGE, (u_char *) &int_val, sizeof(int_val));
			switch_cache_db_release_db_handle(&dbh);
			}
			break;
		case SS_SESSIONS_PER_SECOND:
			switch_core_session_ctl(SCSC_LAST_SPS, &int_val);
			snmp_set_var_typed_value(requests->requestvb, ASN_GAUGE, (u_char *) &int_val, sizeof(int_val));
			break;
		case SS_MAX_SESSIONS_PER_SECOND:
			switch_core_session_ctl(SCSC_SPS, &int_val);
			snmp_set_var_typed_value(requests->requestvb, ASN_GAUGE, (u_char *) &int_val, sizeof(int_val));
			break;
		default:
			snmp_log(LOG_WARNING, "Unregistered OID-suffix requested (%d)\n", (int) subid);
			netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHOBJECT);
		}
		break;

	default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "Unknown mode (%d) in handle_systemStats\n", reqinfo->mode);
		return SNMP_ERR_GENERR;
	}

	return SNMP_ERR_NOERROR;
}


int handle_channelList(netsnmp_mib_handler *handler, netsnmp_handler_registration *reginfo, netsnmp_agent_request_info *reqinfo, netsnmp_request_info *requests)
{
	netsnmp_request_info *request;
	netsnmp_table_request_info *table_info;
	chan_entry_t *entry;

	switch (reqinfo->mode) {
	case MODE_GET:
		for (request = requests; request; request = request->next) {
			table_info = netsnmp_extract_table_info(request);
			entry = (chan_entry_t *) netsnmp_tdata_extract_entry(request);

			switch (table_info->colnum) {
			case CH_UUID:
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) entry->uuid, strlen(entry->uuid));
				break;
			case CH_DIRECTION:
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) entry->direction, strlen(entry->direction));
				break;
			case CH_CREATED:
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) entry->created, strlen(entry->created));
				break;
			case CH_NAME:
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) entry->name, strlen(entry->name));
				break;
			case CH_STATE:
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) entry->state, strlen(entry->state));
				break;
			case CH_CID_NAME:
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) entry->cid_name, strlen(entry->cid_name));
				break;
			case CH_CID_NUM:
				snmp_set_var_typed_value(request->requestvb, ASN_OCTET_STR, (u_char *) entry->cid_num, strlen(entry->cid_num));
				break;
			default:
				snmp_log(LOG_WARNING, "Unregistered OID-suffix requested (%d)\n", table_info->colnum);
				netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHOBJECT);
			}
		}
		break;
	default:
		/* we should never get here, so this is a really bad error */
		snmp_log(LOG_ERR, "Unknown mode (%d) in handle_foo\n", reqinfo->mode );
		return SNMP_ERR_GENERR;
	}

	return SNMP_ERR_NOERROR;
}


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
