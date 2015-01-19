/*
 * SpanDSP - a series of DSP components for telephony
 *
 * v29_tests.c
 *
 * Written by Steve Underwood <steveu@coppice.org>
 *
 * Copyright (C) 2003 Steve Underwood
 *
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*! \page v29_tests_page V.29 modem tests
\section v29_tests_page_sec_1 What does it do?
These tests test one way paths, as V.29 is a half-duplex modem. They allow either:

 - A V.29 transmit modem to feed a V.29 receive modem through a telephone line
   model. BER testing is then used to evaluate performance under various line
   conditions. This is effective for testing the basic performance of the
   receive modem. It is also the only test mode provided for evaluating the
   transmit modem.

 - A V.29 receive modem is used to decode V.29 audio, stored in an audio file.
   This is good way to evaluate performance with audio recorded from other
   models of modem, and with real world problematic telephone lines.

If the appropriate GUI environment exists, the tests are built such that a visual
display of modem status is maintained.

\section v29_tests_page_sec_2 How is it used?
*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(HAVE_FL_FL_H)  &&  defined(HAVE_FL_FL_CARTESIAN_H)  &&  defined(HAVE_FL_FL_AUDIO_METER_H)
#define ENABLE_GUI
#endif

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sndfile.h>
#include <signal.h>
#if defined(HAVE_FENV_H)
#define __USE_GNU
#include <fenv.h>
#endif

#define SPANDSP_EXPOSE_INTERNAL_STRUCTURES

#include "spandsp.h"
#include "spandsp-sim.h"

#if defined(ENABLE_GUI)
#include "modem_monitor.h"
#include "line_model_monitor.h"
#endif

#define BLOCK_LEN       160

#define OUT_FILE_NAME   "v29.wav"

char *decode_test_file = NULL;
bool use_gui = false;

int symbol_no = 0;
int rx_bits = 0;

bert_state_t bert;
one_way_line_model_state_t *line_model;

#if defined(ENABLE_GUI)
qam_monitor_t *qam_monitor;
#endif

bert_results_t latest_results;

static void reporter(void *user_data, int reason, bert_results_t *results)
{
    switch (reason)
    {
    case BERT_REPORT_REGULAR:
        fprintf(stderr, "BERT report regular - %d bits, %d bad bits, %d resyncs\n", results->total_bits, results->bad_bits, results->resyncs);
        memcpy(&latest_results, results, sizeof(latest_results));
        break;
    default:
        fprintf(stderr, "BERT report %s\n", bert_event_to_str(reason));
        break;
    }
}
/*- End of function --------------------------------------------------------*/

static void v29_rx_status(void *user_data, int status)
{
    v29_rx_state_t *s;
    int i;
    int len;
#if defined(SPANDSP_USE_FIXED_POINT)
    complexi16_t *coeffs;
#else
    complexf_t *coeffs;
#endif

    printf("V.29 rx status is %s (%d)\n", signal_status_to_str(status), status);
    s = (v29_rx_state_t *) user_data;
    switch (status)
    {
    case SIG_STATUS_TRAINING_SUCCEEDED:
        printf("Training succeeded\n");
        if ((len = v29_rx_equalizer_state(s, &coeffs)))
        {
            printf("Equalizer:\n");
            for (i = 0;  i < len;  i++)
#if defined(SPANDSP_USE_FIXED_POINT)
                printf("%3d (%15.5f, %15.5f)\n", i, coeffs[i].re/V29_CONSTELLATION_SCALING_FACTOR, coeffs[i].im/V29_CONSTELLATION_SCALING_FACTOR);
#else
                printf("%3d (%15.5f, %15.5f) -> %15.5f\n", i, coeffs[i].re, coeffs[i].im, powerf(&coeffs[i]));
#endif
        }
        break;
    }
}
/*- End of function --------------------------------------------------------*/

static void v29putbit(void *user_data, int bit)
{
    if (bit < 0)
    {
        v29_rx_status(user_data, bit);
        return;
    }

    if (decode_test_file)
        printf("Rx bit %d - %d\n", rx_bits++, bit);
    else
        bert_put_bit(&bert, bit);
}
/*- End of function --------------------------------------------------------*/

static void v29_tx_status(void *user_data, int status)
{
    printf("V.29 tx status is %s (%d)\n", signal_status_to_str(status), status);
}
/*- End of function --------------------------------------------------------*/

static int v29getbit(void *user_data)
{
    return bert_get_bit(&bert);
}
/*- End of function --------------------------------------------------------*/

#if defined(SPANDSP_USE_FIXED_POINT)
static void qam_report(void *user_data, const complexi16_t *constel, const complexi16_t *target, int symbol)
#else
static void qam_report(void *user_data, const complexf_t *constel, const complexf_t *target, int symbol)
#endif
{
    int i;
    int len;
#if defined(SPANDSP_USE_FIXED_POINT)
    complexi16_t *coeffs;
#else
    complexf_t *coeffs;
#endif
    complexf_t constel_point;
    complexf_t target_point;
    float fpower;
    v29_rx_state_t *rx;
    static float smooth_power = 0.0f;
    static int update_interval = 100;

    rx = (v29_rx_state_t *) user_data;
    if (constel)
    {
        constel_point.re = constel->re/V29_CONSTELLATION_SCALING_FACTOR;
        constel_point.im = constel->im/V29_CONSTELLATION_SCALING_FACTOR;
        target_point.re = target->re/V29_CONSTELLATION_SCALING_FACTOR,
        target_point.im = target->im/V29_CONSTELLATION_SCALING_FACTOR,
        fpower = (constel_point.re - target_point.re)*(constel_point.re - target_point.re)
               + (constel_point.im - target_point.im)*(constel_point.im - target_point.im);
        smooth_power = 0.95f*smooth_power + 0.05f*fpower;
#if defined(ENABLE_GUI)
        if (use_gui)
        {
            qam_monitor_update_constel(qam_monitor, &constel_point);
            qam_monitor_update_carrier_tracking(qam_monitor, v29_rx_carrier_frequency(rx));
            //qam_monitor_update_carrier_tracking(qam_monitor, (fpower)  ?  fpower  :  0.001f);
            qam_monitor_update_symbol_tracking(qam_monitor, v29_rx_symbol_timing_correction(rx));
        }
#endif
        printf("%8d [%8.4f, %8.4f] [%8.4f, %8.4f] %2x %8.4f %8.4f %9.4f %7.3f %7.4f\n",
               symbol_no,
               constel_point.re,
               constel_point.im,
               target_point.re,
               target_point.im,
               symbol,
               fpower,
               smooth_power,
               v29_rx_carrier_frequency(rx),
               v29_rx_signal_power(rx),
               v29_rx_symbol_timing_correction(rx));
        symbol_no++;
        if (--update_interval <= 0)
        {
            if ((len = v29_rx_equalizer_state(rx, &coeffs)))
            {
                printf("Equalizer A:\n");
                for (i = 0;  i < len;  i++)
#if defined(SPANDSP_USE_FIXED_POINT)
                    printf("%3d (%15.5f, %15.5f)\n", i, coeffs[i].re/V29_CONSTELLATION_SCALING_FACTOR, coeffs[i].im/V29_CONSTELLATION_SCALING_FACTOR);
#else
                    printf("%3d (%15.5f, %15.5f) -> %15.5f\n", i, coeffs[i].re, coeffs[i].im, powerf(&coeffs[i]));
#endif
#if defined(ENABLE_GUI)
                if (use_gui)
                {
#if defined(SPANDSP_USE_FIXED_POINT)
                    qam_monitor_update_int_equalizer(qam_monitor, coeffs, len);
#else
                    qam_monitor_update_equalizer(qam_monitor, coeffs, len);
#endif
                }
#endif
            }
            update_interval = 100;
        }
    }
}
/*- End of function --------------------------------------------------------*/

#if defined(HAVE_FENV_H)
static void sigfpe_handler(int sig_num, siginfo_t *info, void *data)
{
    switch (sig_num)
    {
    case SIGFPE:
        switch (info->si_code)
        {
        case FPE_INTDIV:
            fprintf(stderr, "integer divide by zero at %p\n", info->si_addr);
            break;
        case FPE_INTOVF:
            fprintf(stderr, "integer overflow at %p\n", info->si_addr);
            break;
        case FPE_FLTDIV:
            fprintf(stderr, "FP divide by zero at %p\n", info->si_addr);
            break;
        case FPE_FLTOVF:
            fprintf(stderr, "FP overflow at %p\n", info->si_addr);
            break;
        case FPE_FLTUND:
            fprintf(stderr, "FP underflow at %p\n", info->si_addr);
            break;
        case FPE_FLTRES:
            fprintf(stderr, "FP inexact result at %p\n", info->si_addr);
            break;
        case FPE_FLTINV:
            fprintf(stderr, "FP invalid operation at %p\n", info->si_addr);
            break;
        case FPE_FLTSUB:
            fprintf(stderr, "subscript out of range at %p\n", info->si_addr);
            break;
        }
        break;
    default:
        fprintf(stderr, "Unexpected signal %d\n", sig_num);
        break;
    }
    exit(2);
}
/*- End of function --------------------------------------------------------*/

static void fpe_trap_setup(void)
{
    struct sigaction trap;

    sigemptyset(&trap.sa_mask);
    trap.sa_flags = SA_SIGINFO;
    trap.sa_sigaction = sigfpe_handler;

    sigaction(SIGFPE, &trap, NULL);
    //feenableexcept(FE_DIVBYZERO | FE_INEXACT | FE_INVALID | FE_OVERFLOW | FE_UNDERFLOW);
    //feenableexcept(FE_ALL_EXCEPT);
    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
}
/*- End of function --------------------------------------------------------*/
#endif

int main(int argc, char *argv[])
{
    v29_rx_state_t *rx;
    v29_tx_state_t *tx;
    bert_results_t bert_results;
    int16_t gen_amp[BLOCK_LEN];
    int16_t amp[BLOCK_LEN];
    SNDFILE *inhandle;
    SNDFILE *outhandle;
    int outframes;
    int samples;
    int test_bps;
    int noise_level;
    int signal_level;
    int bits_per_test;
    int line_model_no;
    int block_no;
    int channel_codec;
    int rbs_pattern;
    int opt;
    bool tep;
    bool log_audio;
    logging_state_t *logging;

    channel_codec = MUNGE_CODEC_NONE;
    rbs_pattern = 0;
    test_bps = 9600;
    tep = false;
    line_model_no = 0;
    decode_test_file = NULL;
    use_gui = false;
    noise_level = -70;
    signal_level = -13;
    bits_per_test = 50000;
    log_audio = false;
    while ((opt = getopt(argc, argv, "b:B:c:d:glm:n:r:s:t")) != -1)
    {
        switch (opt)
        {
        case 'b':
            test_bps = atoi(optarg);
            if (test_bps != 9600  &&  test_bps != 7200  &&  test_bps != 4800)
            {
                fprintf(stderr, "Invalid bit rate specified\n");
                exit(2);
            }
            break;
        case 'B':
            bits_per_test = atoi(optarg);
            break;
        case 'c':
            channel_codec = atoi(optarg);
            break;
        case 'd':
            decode_test_file = optarg;
            break;
        case 'g':
#if defined(ENABLE_GUI)
            use_gui = true;
#else
            fprintf(stderr, "Graphical monitoring not available\n");
            exit(2);
#endif
            break;
        case 'l':
            log_audio = true;
            break;
        case 'm':
            line_model_no = atoi(optarg);
            break;
        case 'n':
            noise_level = atoi(optarg);
            break;
        case 'r':
            rbs_pattern = atoi(optarg);
            break;
        case 's':
            signal_level = atoi(optarg);
            break;
        case 't':
            tep = true;
            break;
        default:
            //usage();
            exit(2);
            break;
        }
    }
    inhandle = NULL;
    outhandle = NULL;

#if defined(HAVE_FENV_H)
    fpe_trap_setup();
#endif

    if (log_audio)
    {
        if ((outhandle = sf_open_telephony_write(OUT_FILE_NAME, 1)) == NULL)
        {
            fprintf(stderr, "    Cannot create audio file '%s'\n", OUT_FILE_NAME);
            exit(2);
        }
    }

    if (decode_test_file)
    {
        /* We will decode the audio from a file. */
        tx = NULL;
        if ((inhandle = sf_open_telephony_read(decode_test_file, 1)) == NULL)
        {
            fprintf(stderr, "    Cannot open audio file '%s'\n", decode_test_file);
            exit(2);
        }
    }
    else
    {
        /* We will generate V.29 audio, and add some noise to it. */
        tx = v29_tx_init(NULL, test_bps, tep, v29getbit, NULL);
        logging = v29_tx_get_logging_state(tx);
        span_log_set_level(logging, SPAN_LOG_SHOW_SEVERITY | SPAN_LOG_SHOW_PROTOCOL | SPAN_LOG_FLOW);
        span_log_set_tag(logging, "V.29-tx");
        v29_tx_power(tx, signal_level);
        v29_tx_set_modem_status_handler(tx, v29_tx_status, (void *) tx);
#if defined(SPANDSP_EXPOSE_INTERNAL_STRUCTURES)
        /* Move the carrier off a bit */
        tx->carrier_phase_rate = dds_phase_ratef(1710.0f);
        tx->carrier_phase = 0;
#endif

        bert_init(&bert, bits_per_test, BERT_PATTERN_ITU_O152_11, test_bps, 20);
        bert_set_report(&bert, 10000, reporter, NULL);

        if ((line_model = one_way_line_model_init(line_model_no, (float) noise_level, channel_codec, rbs_pattern)) == NULL)
        {
            fprintf(stderr, "    Failed to create line model\n");
            exit(2);
        }
    }

    rx = v29_rx_init(NULL, test_bps, v29putbit, NULL);
    logging = v29_rx_get_logging_state(rx);
    span_log_set_level(logging, SPAN_LOG_SHOW_SEVERITY | SPAN_LOG_SHOW_PROTOCOL | SPAN_LOG_FLOW);
    span_log_set_tag(logging, "V.29-rx");
    v29_rx_signal_cutoff(rx, -45.5f);
    v29_rx_set_modem_status_handler(rx, v29_rx_status, (void *) rx);
    v29_rx_set_qam_report_handler(rx, qam_report, (void *) rx);
#if defined(SPANDSP_EXPOSE_INTERNAL_STRUCTURES)
    /* Rotate the starting phase */
    rx->carrier_phase = 0x80000000;
#endif

#if defined(ENABLE_GUI)
    if (use_gui)
    {
        qam_monitor = qam_monitor_init(6.0f, V29_CONSTELLATION_SCALING_FACTOR, NULL);
        if (!decode_test_file)
        {
            start_line_model_monitor(129);
            line_model_monitor_line_model_update(line_model->near_filter, line_model->near_filter_len);
        }
    }
#endif

    memset(&latest_results, 0, sizeof(latest_results));
    for (block_no = 0;  ;  block_no++)
    {
        if (decode_test_file)
        {
            samples = sf_readf_short(inhandle, amp, BLOCK_LEN);
#if defined(ENABLE_GUI)
            if (use_gui)
                qam_monitor_update_audio_level(qam_monitor, amp, samples);
#endif
            if (samples == 0)
                break;
        }
        else
        {
            samples = v29_tx(tx, gen_amp, BLOCK_LEN);
#if defined(ENABLE_GUI)
            if (use_gui)
                qam_monitor_update_audio_level(qam_monitor, gen_amp, samples);
#endif
            if (samples == 0)
            {
                /* Push a little silence through, to ensure all the data bits get out of the buffers */
                vec_zeroi16(amp, BLOCK_LEN);
                v29_rx(rx, amp, BLOCK_LEN);

                /* Note that we might get a few bad bits as the carrier shuts down. */
                bert_result(&bert, &bert_results);
                fprintf(stderr, "Final result %ddBm0/%ddBm0, %d bits, %d bad bits, %d resyncs\n", signal_level, noise_level, bert_results.total_bits, bert_results.bad_bits, bert_results.resyncs);
                fprintf(stderr, "Last report  %ddBm0/%ddBm0, %d bits, %d bad bits, %d resyncs\n", signal_level, noise_level, latest_results.total_bits, latest_results.bad_bits, latest_results.resyncs);
                /* See if bit errors are appearing yet. Also check we are getting enough bits out of the receiver. The last regular report
                   should be error free, though the final report will generally contain bits errors as the carrier was dying. The total
                   number of bits out of the receiver should be at least the number we sent. Also, since BERT sync should have occurred
                   rapidly at the start of transmission, the last report should have occurred at not much less than the total number of
                   bits we sent. */
                if (bert_results.total_bits < bits_per_test
                    ||
                    latest_results.total_bits < bits_per_test - 100
                    ||
                    latest_results.bad_bits != 0)
                {
                    break;
                }
                memset(&latest_results, 0, sizeof(latest_results));
                signal_level--;
                v29_tx_restart(tx, test_bps, tep);
                v29_tx_power(tx, signal_level);
                v29_rx_restart(rx, test_bps, false);
#if defined(SPANDSP_EXPOSE_INTERNAL_STRUCTURES)
                rx->eq_put_step = rand()%(48*10/3);
#endif
                bert_init(&bert, bits_per_test, BERT_PATTERN_ITU_O152_11, test_bps, 20);
                bert_set_report(&bert, 10000, reporter, NULL);
                one_way_line_model_free(line_model);
                if ((line_model = one_way_line_model_init(line_model_no, (float) noise_level, channel_codec, 0)) == NULL)
                {
                    fprintf(stderr, "    Failed to create line model\n");
                    exit(2);
                }
            }
            if (log_audio)
            {
                outframes = sf_writef_short(outhandle, gen_amp, samples);
                if (outframes != samples)
                {
                    fprintf(stderr, "    Error writing audio file\n");
                    exit(2);
                }
            }
            one_way_line_model(line_model, amp, gen_amp, samples);
        }
#if defined(ENABLE_GUI)
        if (use_gui  &&  !decode_test_file)
            line_model_monitor_line_spectrum_update(amp, samples);
#endif
        v29_rx(rx, amp, samples);
    }
    if (!decode_test_file)
    {
        bert_result(&bert, &bert_results);
        fprintf(stderr, "At completion:\n");
        fprintf(stderr, "Final result %ddBm0/%ddBm0, %d bits, %d bad bits, %d resyncs\n", signal_level, noise_level, bert_results.total_bits, bert_results.bad_bits, bert_results.resyncs);
        fprintf(stderr, "Last report  %ddBm0/%ddBm0, %d bits, %d bad bits, %d resyncs\n", signal_level, noise_level, latest_results.total_bits, latest_results.bad_bits, latest_results.resyncs);
        one_way_line_model_free(line_model);

        if (signal_level > -43)
        {
            printf("Tests failed.\n");
            exit(2);
        }

        printf("Tests passed.\n");
    }
    v29_rx_free(rx);
    if (tx)
        v29_tx_free(tx);
    bert_release(&bert);
#if defined(ENABLE_GUI)
    if (use_gui)
        qam_wait_to_end(qam_monitor);
#endif
    if (decode_test_file)
    {
        if (sf_close_telephony(inhandle))
        {
            fprintf(stderr, "    Cannot close audio file '%s'\n", decode_test_file);
            exit(2);
        }
    }
    if (log_audio)
    {
        if (sf_close_telephony(outhandle))
        {
            fprintf(stderr, "    Cannot close audio file '%s'\n", OUT_FILE_NAME);
            exit(2);
        }
    }
    return 0;
}
/*- End of function --------------------------------------------------------*/
/*- End of file ------------------------------------------------------------*/
