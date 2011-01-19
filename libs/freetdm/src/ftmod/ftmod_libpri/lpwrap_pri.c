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

//#define IODEBUG

#include "private/ftdm_core.h"
#include "lpwrap_pri.h"

#ifndef HAVE_GETTIMEOFDAY
#ifdef WIN32
#include <mmsystem.h>

static __inline int gettimeofday(struct timeval *tp, void *nothing)
{
#ifdef WITHOUT_MM_LIB
	SYSTEMTIME st;
	time_t tt;
	struct tm tmtm;
	/* mktime converts local to UTC */
	GetLocalTime (&st);
	tmtm.tm_sec = st.wSecond;
	tmtm.tm_min = st.wMinute;
	tmtm.tm_hour = st.wHour;
	tmtm.tm_mday = st.wDay;
	tmtm.tm_mon = st.wMonth - 1;
	tmtm.tm_year = st.wYear - 1900;  tmtm.tm_isdst = -1;
	tt = mktime (&tmtm);
	tp->tv_sec = tt;
	tp->tv_usec = st.wMilliseconds * 1000;
#else
	/**
	 ** The earlier time calculations using GetLocalTime
	 ** had a time resolution of 10ms.The timeGetTime, part
	 ** of multimedia apis offer a better time resolution
	 ** of 1ms.Need to link against winmm.lib for this
	 **/
	unsigned long Ticks = 0;
	unsigned long Sec =0;
	unsigned long Usec = 0;
	Ticks = timeGetTime();

	Sec = Ticks/1000;
	Usec = (Ticks - (Sec*1000))*1000;
	tp->tv_sec = Sec;
	tp->tv_usec = Usec;
#endif /* WITHOUT_MM_LIB */
	(void)nothing;
	return 0;
}
#endif /* WIN32 */
#endif /* HAVE_GETTIMEOFDAY */

static struct lpwrap_pri_event_list LPWRAP_PRI_EVENT_LIST[] = {
	{0, LPWRAP_PRI_EVENT_ANY, "ANY"},
	{1, LPWRAP_PRI_EVENT_DCHAN_UP, "DCHAN_UP"},
	{2, LPWRAP_PRI_EVENT_DCHAN_DOWN, "DCHAN_DOWN"},
	{3, LPWRAP_PRI_EVENT_RESTART, "RESTART"},
	{4, LPWRAP_PRI_EVENT_CONFIG_ERR, "CONFIG_ERR"},
	{5, LPWRAP_PRI_EVENT_RING, "RING"},
	{6, LPWRAP_PRI_EVENT_HANGUP, "HANGUP"},
	{7, LPWRAP_PRI_EVENT_RINGING, "RINGING"},
	{8, LPWRAP_PRI_EVENT_ANSWER, "ANSWER"},
	{9, LPWRAP_PRI_EVENT_HANGUP_ACK, "HANGUP_ACK"},
	{10, LPWRAP_PRI_EVENT_RESTART_ACK, "RESTART_ACK"},
	{11, LPWRAP_PRI_EVENT_FACILITY, "FACILITY"},
	{12, LPWRAP_PRI_EVENT_INFO_RECEIVED, "INFO_RECEIVED"},
	{13, LPWRAP_PRI_EVENT_PROCEEDING, "PROCEEDING"},
	{14, LPWRAP_PRI_EVENT_SETUP_ACK, "SETUP_ACK"},
	{15, LPWRAP_PRI_EVENT_HANGUP_REQ, "HANGUP_REQ"},
	{16, LPWRAP_PRI_EVENT_NOTIFY, "NOTIFY"},
	{17, LPWRAP_PRI_EVENT_PROGRESS, "PROGRESS"},
	{18, LPWRAP_PRI_EVENT_KEYPAD_DIGIT, "KEYPAD_DIGIT"},
	{19, LPWRAP_PRI_EVENT_IO_FAIL, "IO_FAIL"},
};

#define LINE "--------------------------------------------------------------------------------"

const char *lpwrap_pri_event_str(lpwrap_pri_event_t event_id)
{
	if (event_id < 0 || event_id >= LPWRAP_PRI_EVENT_MAX)
		return "";

	return LPWRAP_PRI_EVENT_LIST[event_id].name;
}

static int __pri_lpwrap_read(struct pri *pri, void *buf, int buflen)
{
	struct lpwrap_pri *spri = (struct lpwrap_pri *) pri_get_userdata(pri);
	ftdm_size_t len = buflen;
	ftdm_status_t zst;
	int res;

	if ((zst = ftdm_channel_read(spri->dchan, buf, &len)) != FTDM_SUCCESS) {
		if (zst == FTDM_FAIL) {
			ftdm_log(FTDM_LOG_CRIT, "span %d D-READ FAIL! [%s]\n", spri->span->span_id, spri->dchan->last_error);
			spri->errs++;
		} else {
			ftdm_log(FTDM_LOG_CRIT, "span %d D-READ TIMEOUT\n", spri->span->span_id);
		}
		/* we cannot return -1, libpri seems to expect values >= 0 */
		return 0;
	}
	spri->errs = 0;
	res = (int)len;

	if (res > 0) {
		memset(&((unsigned char*)buf)[res], 0, 2);
		res += 2;
#ifdef IODEBUG
		{
			char bb[2048] = { 0 };

			print_hex_bytes(buf, res - 2, bb, sizeof(bb));
			ftdm_log(FTDM_LOG_DEBUG, "READ %d\n", res - 2);
		}
#endif
	}
	return res;
}

static int __pri_lpwrap_write(struct pri *pri, void *buf, int buflen)
{
	struct lpwrap_pri *spri = (struct lpwrap_pri *) pri_get_userdata(pri);
	ftdm_size_t len = buflen - 2;

	if (ftdm_channel_write(spri->dchan, buf, buflen, &len) != FTDM_SUCCESS) {
		ftdm_log(FTDM_LOG_CRIT, "span %d D-WRITE FAIL! [%s]\n", spri->span->span_id, spri->dchan->last_error);
		/* we cannot return -1, libpri seems to expect values >= 0 */
		return 0;
	}

#ifdef IODEBUG
	{
		char bb[2048] = { 0 };

		print_hex_bytes(buf, buflen - 2, bb, sizeof(bb));
		ftdm_log(FTDM_LOG_DEBUG, "WRITE %d\n", (int)buflen - 2);
	}
#endif
	return (int)buflen;
}

int lpwrap_init_pri(struct lpwrap_pri *spri, ftdm_span_t *span, ftdm_channel_t *dchan, int swtype, int node, int debug)
{
	int ret = -1;

	memset(spri, 0, sizeof(struct lpwrap_pri));
	spri->dchan = dchan;
	spri->span  = span;

	if (!spri->dchan) {
		ftdm_log(FTDM_LOG_ERROR, "No D-Channel available, unable to create PRI\n");
		return ret;
	}

	if ((spri->pri = pri_new_cb(spri->dchan->sockfd, node, swtype, __pri_lpwrap_read, __pri_lpwrap_write, spri))) {
		unsigned char buf[4] = { 0 };
		size_t buflen = sizeof(buf), len = 0;

		pri_set_debug(spri->pri, debug);
#ifdef HAVE_LIBPRI_AOC
		pri_aoc_events_enable(spri->pri, 1);
#endif
		ftdm_channel_write(spri->dchan, buf, buflen, &len);

		ret = 0;
	} else {
		ftdm_log(FTDM_LOG_ERROR, "Unable to create PRI\n");
	}
	return ret;
}

int lpwrap_init_bri(struct lpwrap_pri *spri, ftdm_span_t *span, ftdm_channel_t *dchan, int swtype, int node, int ptp, int debug)
{
	int ret = -1;

#ifdef HAVE_LIBPRI_BRI
	memset(spri, 0, sizeof(struct lpwrap_pri));
	spri->dchan = dchan;
	spri->span  = span;

	if (!spri->dchan) {
		ftdm_log(FTDM_LOG_ERROR, "No D-Channel available, unable to create BRI\n");
		return ret;
	}

	if ((spri->pri = pri_new_bri_cb(spri->dchan->sockfd, ptp, node, swtype, __pri_lpwrap_read, __pri_lpwrap_write, spri))) {
		unsigned char buf[4] = { 0 };
		size_t buflen = sizeof(buf), len = 0;

		pri_set_debug(spri->pri, debug);
#ifdef HAVE_LIBPRI_AOC
		pri_aoc_events_enable(spri->pri, 1);
#endif
		ftdm_channel_write(spri->dchan, buf, buflen, &len);

		ret = 0;
	} else {
		ftdm_log(FTDM_LOG_ERROR, "Unable to create BRI\n");
	}
#else
	ftdm_log(FTDM_LOG_ERROR, "Installed libpri version (%s) has no BRI support\n",
			pri_get_version());
#endif
	return ret;
}


int lpwrap_one_loop(struct lpwrap_pri *spri)
{
	fd_set rfds, efds;
	struct timeval now = {0,0}, *next = NULL;
	pri_event *event = NULL;
	event_handler handler;
	int sel;

	if (spri->on_loop) {
		if ((sel = spri->on_loop(spri)) < 0) {
			return sel;
		}
	}

	if (spri->errs >= 2) {
		spri->errs = 0;
		return -1;
	}

	FD_ZERO(&rfds);
	FD_ZERO(&efds);

#ifdef _MSC_VER
	//Windows macro for FD_SET includes a warning C4127: conditional expression is constant
#pragma warning(push)
#pragma warning(disable:4127)
#endif

	FD_SET(pri_fd(spri->pri), &rfds);
	FD_SET(pri_fd(spri->pri), &efds);

#ifdef _MSC_VER
#pragma warning(pop)
#endif

	now.tv_sec = 0;
	now.tv_usec = 100000;

	sel = select(pri_fd(spri->pri) + 1, &rfds, NULL, &efds, &now);
	if (!sel) {
		if ((next = pri_schedule_next(spri->pri))) {
			gettimeofday(&now, NULL);
			if (now.tv_sec >= next->tv_sec && (now.tv_usec >= next->tv_usec || next->tv_usec <= 100000)) {
				//ftdm_log(FTDM_LOG_DEBUG, "Check event\n");
				event = pri_schedule_run(spri->pri);
			}
		}
	} else if (sel > 0) {
		event = pri_check_event(spri->pri);
	}

	if (event) {
		/* 0 is catchall event handler */
		if (event->e < 0 || event->e >= LPWRAP_PRI_EVENT_MAX) {
			handler = spri->eventmap[0];
		} else if (spri->eventmap[event->e]) {
			handler = spri->eventmap[event->e];
		} else {
			handler = spri->eventmap[0];
		}

		if (handler) {
			handler(spri, event->e, event);
		} else {
			ftdm_log(FTDM_LOG_CRIT, "No event handler found for event %d.\n", event->e);
		}
	}
	return sel;
}

int lpwrap_run_pri(struct lpwrap_pri *spri)
{
	int ret = 0;

	for (;;) {
		if ((ret = lpwrap_one_loop(spri)) < 0) {
#ifndef WIN32 //This needs to be adressed fror WIN32 still
			if (errno == EINTR){
				/* Igonore an interrupted system call */
				continue;
			}
#endif
			ftdm_log(FTDM_LOG_CRIT, "Error = %i [%s]\n", ret, strerror(errno));
			break;
		}
	}
	return ret;
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
