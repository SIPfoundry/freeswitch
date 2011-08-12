/*
 * Copyright (c) 2009, Konrad Hammel <konrad@sangoma.com>
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

/* INCLUDE ********************************************************************/
#include "ftmod_sangoma_ss7_main.h"
/******************************************************************************/

/* DEFINES ********************************************************************/

/* GLOBALS ********************************************************************/

/* FUNCTIONS ******************************************************************/
void ft_to_sngss7_iam (ftdm_channel_t * ftdmchan)
{	
	SiConEvnt 			iam;
	sngss7_chan_data_t	*sngss7_info = ftdmchan->call_data;;
	
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);
	
	sngss7_info->suInstId 	= get_unique_id ();
	sngss7_info->spInstId 	= 0;
	sngss7_info->spId 		= 1;
	
	memset (&iam, 0x0, sizeof (iam));

	if (sngss7_info->circuit->transparent_iam &&
		sngss7_retrieve_iam(ftdmchan, &iam) == FTDM_SUCCESS) {
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Tx IAM (Transparent)\n", sngss7_info->circuit->cic);
	} else {
		/* Nature of Connection Indicators */
		copy_natConInd_to_sngss7(ftdmchan, &iam.natConInd);

		/* Forward Call Indicators */
		copy_fwdCallInd_to_sngss7(ftdmchan, &iam.fwdCallInd);

		/* Transmission medium requirements */
		copy_txMedReq_to_sngss7(ftdmchan, &iam.txMedReq);

		if (SNGSS7_SWITCHTYPE_ANSI(g_ftdm_sngss7_data.cfg.isupCkt[sngss7_info->circuit->id].switchType)) {
			/* User Service Info A */
			copy_usrServInfoA_to_sngss7(ftdmchan, &iam.usrServInfoA);
		}
		
		/* Called Number information */
		copy_cdPtyNum_to_sngss7(ftdmchan, &iam.cdPtyNum);
		
		/* Calling Number information */
		copy_cgPtyNum_to_sngss7(ftdmchan, &iam.cgPtyNum);

		/* Generic Number information */
		copy_genNmb_to_sngss7(ftdmchan, &iam.genNmb);

		/* Calling Party's Category */
		copy_cgPtyCat_to_sngss7(ftdmchan, &iam.cgPtyCat);

		/* Redirecting Number */
		copy_redirgNum_to_sngss7(ftdmchan, &iam.redirgNum);

		/* Access Transport */
		copy_accTrnspt_to_sngss7(ftdmchan, &iam.accTrnspt);

		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Tx IAM clg = \"%s\" (NADI=%d), cld = \"%s\" (NADI=%d)\n",
									sngss7_info->circuit->cic,
									ftdmchan->caller_data.cid_num.digits,
									iam.cgPtyNum.natAddrInd.val,
									ftdmchan->caller_data.dnis.digits,
									iam.cdPtyNum.natAddrInd.val);
	}

	sng_cc_con_request (sngss7_info->spId,
						sngss7_info->suInstId,
						sngss7_info->spInstId,
						sngss7_info->circuit->id,
						&iam,
						0);

	SS7_FUNC_TRACE_EXIT (__FUNCTION__);
	return;
}

void ft_to_sngss7_acm (ftdm_channel_t * ftdmchan)
{
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);
	
	sngss7_chan_data_t	*sngss7_info = ftdmchan->call_data;
	SiCnStEvnt acm;
	
	memset (&acm, 0x0, sizeof (acm));
	
	/* fill in the needed information for the ACM */
	acm.bckCallInd.eh.pres 				= PRSNT_NODEF;
	acm.bckCallInd.chrgInd.pres			= PRSNT_NODEF;
	acm.bckCallInd.chrgInd.val			= CHRG_CHRG;
	acm.bckCallInd.cadPtyStatInd.pres	= PRSNT_NODEF;
	acm.bckCallInd.cadPtyStatInd.val	= 0x01;
	acm.bckCallInd.cadPtyCatInd.pres	= PRSNT_NODEF;
	acm.bckCallInd.cadPtyCatInd.val		= CADCAT_ORDSUBS;
	acm.bckCallInd.end2EndMethInd.pres	= PRSNT_NODEF;
	acm.bckCallInd.end2EndMethInd.val	= E2EMTH_NOMETH;
	acm.bckCallInd.intInd.pres			= PRSNT_NODEF;
	acm.bckCallInd.intInd.val 			= INTIND_NOINTW;
	acm.bckCallInd.end2EndInfoInd.pres	= PRSNT_NODEF;
	acm.bckCallInd.end2EndInfoInd.val	= E2EINF_NOINFO;
	acm.bckCallInd.isdnUsrPrtInd.pres	= PRSNT_NODEF;
	acm.bckCallInd.isdnUsrPrtInd.val	= ISUP_USED;
	acm.bckCallInd.holdInd.pres			= PRSNT_NODEF;
	acm.bckCallInd.holdInd.val			= HOLD_NOTREQD;
	acm.bckCallInd.isdnAccInd.pres		= PRSNT_NODEF;
	acm.bckCallInd.isdnAccInd.val		= ISDNACC_NONISDN;
	acm.bckCallInd.echoCtrlDevInd.pres	= PRSNT_NODEF;
	switch (ftdmchan->caller_data.bearer_capability) {
	/**********************************************************************/
	case (FTDM_BEARER_CAP_SPEECH):
		acm.bckCallInd.echoCtrlDevInd.val	= 0x1;
		break;
	/**********************************************************************/
	case (FTDM_BEARER_CAP_UNRESTRICTED):
		acm.bckCallInd.echoCtrlDevInd.val	= 0x0;
		break;
	/**********************************************************************/
	case (FTDM_BEARER_CAP_3_1KHZ_AUDIO):
		acm.bckCallInd.echoCtrlDevInd.val	= 0x1;
		break;
	/**********************************************************************/
	default:
		SS7_ERROR_CHAN(ftdmchan, "Unknown Bearer capability falling back to speech%s\n", " ");
		acm.bckCallInd.echoCtrlDevInd.val	= 0x1;
		break;
	/**********************************************************************/
	} /* switch (ftdmchan->caller_data.bearer_capability) */
	acm.bckCallInd.sccpMethInd.pres		= PRSNT_NODEF;
	acm.bckCallInd.sccpMethInd.val		= SCCPMTH_NOIND;

	/* fill in any optional parameters */
	if (sngss7_test_options(&g_ftdm_sngss7_data.cfg.isupCkt[sngss7_info->circuit->id], SNGSS7_ACM_OBCI_BITA)) {
		SS7_DEBUG_CHAN(ftdmchan, "Found ACM_OBCI_BITA flag:0x%X\n", g_ftdm_sngss7_data.cfg.isupCkt[sngss7_info->circuit->id].options);
		acm.optBckCalInd.eh.pres				= PRSNT_NODEF;
		acm.optBckCalInd.inbndInfoInd.pres		= PRSNT_NODEF;
		acm.optBckCalInd.inbndInfoInd.val		= 0x1;
		acm.optBckCalInd.caFwdMayOcc.pres		= PRSNT_DEF;
		acm.optBckCalInd.simpleSegmInd.pres		= PRSNT_DEF;
		acm.optBckCalInd.mlppUserInd.pres		= PRSNT_DEF;
		acm.optBckCalInd.usrNetIneractInd.pres	= PRSNT_DEF;
	} /* if (sngss7_test_options(isup_intf, SNGSS7_ACM_OBCI_BITA)) */

	/* send the ACM request to LibSngSS7 */
	sng_cc_con_status  (1,
						sngss7_info->suInstId,
						sngss7_info->spInstId,
						sngss7_info->circuit->id, 
						&acm, 
						ADDRCMPLT);
	
	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Tx ACM\n", sngss7_info->circuit->cic);

	SS7_FUNC_TRACE_EXIT (__FUNCTION__);
	return;
}

void ft_to_sngss7_anm (ftdm_channel_t * ftdmchan)
{
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);
	
	sngss7_chan_data_t *sngss7_info = ftdmchan->call_data;
	SiConEvnt anm;
	
	memset (&anm, 0x0, sizeof (anm));
	
	/* send the ANM request to LibSngSS7 */
	sng_cc_con_response(1,
						sngss7_info->suInstId,
						sngss7_info->spInstId,
						sngss7_info->circuit->id, 
						&anm, 
						5);

  SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Tx ANM\n", sngss7_info->circuit->cic);

  SS7_FUNC_TRACE_EXIT (__FUNCTION__);
  return;
}

/******************************************************************************/
void ft_to_sngss7_rel (ftdm_channel_t * ftdmchan)
{
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);
	
	sngss7_chan_data_t *sngss7_info = ftdmchan->call_data;
	SiRelEvnt rel;
	
	memset (&rel, 0x0, sizeof (rel));
	
	rel.causeDgn.eh.pres = PRSNT_NODEF;
	rel.causeDgn.location.pres = PRSNT_NODEF;
	rel.causeDgn.location.val = 0x01;
	rel.causeDgn.cdeStand.pres = PRSNT_NODEF;
	rel.causeDgn.cdeStand.val = 0x00;
	rel.causeDgn.recommend.pres = NOTPRSNT;
	rel.causeDgn.causeVal.pres = PRSNT_NODEF;
	rel.causeDgn.causeVal.val = (uint8_t) ftdmchan->caller_data.hangup_cause;
	rel.causeDgn.dgnVal.pres = NOTPRSNT;
	
	/* send the REL request to LibSngSS7 */
	sng_cc_rel_request (1,
						sngss7_info->suInstId,
						sngss7_info->spInstId, 
						sngss7_info->circuit->id, 
						&rel);
	
	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Tx REL cause=%d \n",
							sngss7_info->circuit->cic,
							ftdmchan->caller_data.hangup_cause );

	SS7_FUNC_TRACE_EXIT (__FUNCTION__);
	return;
}

/******************************************************************************/
void ft_to_sngss7_rlc (ftdm_channel_t * ftdmchan)
{
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);
	
	sngss7_chan_data_t *sngss7_info = ftdmchan->call_data;
	SiRelEvnt rlc;
	
	memset (&rlc, 0x0, sizeof (rlc));
	
	/* send the RLC request to LibSngSS7 */
	sng_cc_rel_response (1,
						sngss7_info->suInstId,
						sngss7_info->spInstId, 
						sngss7_info->circuit->id, 
						&rlc);
	
	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Tx RLC\n", sngss7_info->circuit->cic);

	SS7_FUNC_TRACE_EXIT (__FUNCTION__);
	return;
}

/******************************************************************************/
void ft_to_sngss7_rsc (ftdm_channel_t * ftdmchan)
{
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);
	
	sngss7_chan_data_t *sngss7_info = ftdmchan->call_data;
	
	sng_cc_sta_request (1,
						sngss7_info->suInstId,
						sngss7_info->spInstId,
						sngss7_info->circuit->id,
						sngss7_info->globalFlg, 
						SIT_STA_CIRRESREQ, 
						NULL);
	
	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Tx RSC\n", sngss7_info->circuit->cic);

	SS7_FUNC_TRACE_EXIT (__FUNCTION__);
	return;
}

/******************************************************************************/
void ft_to_sngss7_rsca (ftdm_channel_t * ftdmchan)
{
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);
	
	sngss7_chan_data_t *sngss7_info = ftdmchan->call_data;
	
	sng_cc_sta_request (1,
						sngss7_info->suInstId,
						sngss7_info->spInstId,
						sngss7_info->circuit->id,
						sngss7_info->globalFlg, 
						SIT_STA_CIRRESRSP, 
						NULL);
	
	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Tx RSC-RLC\n", sngss7_info->circuit->cic);

	SS7_FUNC_TRACE_EXIT (__FUNCTION__);
  return;
}

/******************************************************************************/
void ft_to_sngss7_blo (ftdm_channel_t * ftdmchan)
{
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);
	
	sngss7_chan_data_t *sngss7_info = ftdmchan->call_data;
	
	sng_cc_sta_request (1,
						0,
						0,
						sngss7_info->circuit->id,
						sngss7_info->globalFlg, 
						SIT_STA_CIRBLOREQ, 
						NULL);
	
	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Tx BLO\n", sngss7_info->circuit->cic);

	SS7_FUNC_TRACE_EXIT (__FUNCTION__);
	return;
}

/******************************************************************************/
void ft_to_sngss7_bla (ftdm_channel_t * ftdmchan)
{
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);
	
	sngss7_chan_data_t *sngss7_info = ftdmchan->call_data;
	
	sng_cc_sta_request (1,
						0,
						0,
						sngss7_info->circuit->id,
						sngss7_info->globalFlg, 
						SIT_STA_CIRBLORSP, 
						NULL);
	
	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Tx BLA\n", sngss7_info->circuit->cic);

	SS7_FUNC_TRACE_EXIT (__FUNCTION__);
	return;
}

/******************************************************************************/
void
ft_to_sngss7_ubl (ftdm_channel_t * ftdmchan)
{
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);
	
	sngss7_chan_data_t *sngss7_info = ftdmchan->call_data;
	
	sng_cc_sta_request (1,
						0,
						0,
						sngss7_info->circuit->id,
						sngss7_info->globalFlg, 
						SIT_STA_CIRUBLREQ, 
						NULL);
	
	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Tx UBL\n", sngss7_info->circuit->cic);

	SS7_FUNC_TRACE_EXIT (__FUNCTION__);
	return;
}

/******************************************************************************/
void ft_to_sngss7_uba (ftdm_channel_t * ftdmchan)
{
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);
	
	sngss7_chan_data_t *sngss7_info = ftdmchan->call_data;
	
	sng_cc_sta_request (1,
						0,
						0,
						sngss7_info->circuit->id,
						sngss7_info->globalFlg, 
						SIT_STA_CIRUBLRSP, 
						NULL);
	
	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Tx UBA\n", sngss7_info->circuit->cic);

	SS7_FUNC_TRACE_EXIT (__FUNCTION__);
	return;
}

/******************************************************************************/
void ft_to_sngss7_lpa (ftdm_channel_t * ftdmchan)
{
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);
	
	sngss7_chan_data_t *sngss7_info = ftdmchan->call_data;
	
	sng_cc_sta_request (1,
						sngss7_info->suInstId,
						sngss7_info->spInstId,
						sngss7_info->circuit->id,
						sngss7_info->globalFlg, 
						SIT_STA_LOOPBACKACK, 
						NULL);
	
	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Tx LPA\n", sngss7_info->circuit->cic);

	SS7_FUNC_TRACE_EXIT (__FUNCTION__);
return;
}

/******************************************************************************/
void ft_to_sngss7_gra (ftdm_channel_t * ftdmchan)
{
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);
	
	sngss7_chan_data_t *sngss7_info = ftdmchan->call_data;
	SiStaEvnt	gra;
	
	/* clean out the gra struct */
	memset (&gra, 0x0, sizeof (gra));

	gra.rangStat.eh.pres = PRSNT_NODEF;

	/* fill in the range */	
	gra.rangStat.range.pres = PRSNT_NODEF;
	gra.rangStat.range.val = sngss7_info->rx_grs.range;

	/* fill in the status */
	gra.rangStat.status.pres = PRSNT_NODEF;
	gra.rangStat.status.len = ((sngss7_info->rx_grs.range + 1) >> 3) + (((sngss7_info->rx_grs.range + 1) & 0x07) ? 1 : 0); 
	
	/* the status field should be 1 if blocked for maintenace reasons 
	* and 0 is not blocked....since we memset the struct nothing to do
	*/
	
	/* send the GRA to LibSng-SS7 */
	sng_cc_sta_request (1,
						0,
						0,
						sngss7_info->rx_grs.circuit,
						0,
						SIT_STA_GRSRSP,
						&gra);
	
	SS7_INFO_CHAN(ftdmchan, "[CIC:%d]Tx GRA (%d:%d)\n",
							sngss7_info->circuit->cic,
							sngss7_info->circuit->cic,
							(sngss7_info->circuit->cic + sngss7_info->rx_grs.range));
	

	SS7_FUNC_TRACE_EXIT (__FUNCTION__);
	return;
}

/******************************************************************************/
void ft_to_sngss7_grs (ftdm_channel_t *fchan)
{
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);
	
	sngss7_chan_data_t *cinfo = fchan->call_data;
	
	SiStaEvnt grs;
	
	ftdm_assert(sngss7_test_ckt_flag(cinfo, FLAG_GRP_RESET_TX) && 
		   !sngss7_test_ckt_flag(cinfo, FLAG_GRP_RESET_SENT), "Incorrect flags\n");

	memset (&grs, 0x0, sizeof(grs));
	grs.rangStat.eh.pres    = PRSNT_NODEF;
	grs.rangStat.range.pres = PRSNT_NODEF;
	grs.rangStat.range.val  = cinfo->tx_grs.range;

	sng_cc_sta_request (1,
		0,
		0,
		cinfo->tx_grs.circuit,
		0,
		SIT_STA_GRSREQ,
		&grs);

	SS7_INFO_CHAN(fchan, "[CIC:%d]Tx GRS (%d:%d)\n",
		cinfo->circuit->cic,
		cinfo->circuit->cic,
		(cinfo->circuit->cic + cinfo->tx_grs.range));

	memset(&cinfo->tx_grs, 0, sizeof(cinfo->tx_grs));

	sngss7_set_ckt_flag(cinfo, FLAG_GRP_RESET_SENT);

	SS7_FUNC_TRACE_EXIT (__FUNCTION__);
}

/******************************************************************************/
void ft_to_sngss7_cgba(ftdm_channel_t * ftdmchan)
{	
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);
	
	sngss7_span_data_t 	*sngss7_span = ftdmchan->span->signal_data;
	sngss7_chan_data_t	*sngss7_info = ftdmchan->call_data;
	int					x = 0;
	
	SiStaEvnt cgba;

	memset (&cgba, 0x0, sizeof(cgba));

	/* fill in the circuit group supervisory message */
	cgba.cgsmti.eh.pres = PRSNT_NODEF;
	cgba.cgsmti.typeInd.pres = PRSNT_NODEF;
	cgba.cgsmti.typeInd.val = sngss7_span->rx_cgb.type;

	cgba.rangStat.eh.pres = PRSNT_NODEF;
	/* fill in the range */	
	cgba.rangStat.range.pres = PRSNT_NODEF;
	cgba.rangStat.range.val = sngss7_span->rx_cgb.range;
	/* fill in the status */
	cgba.rangStat.status.pres = PRSNT_NODEF;
	cgba.rangStat.status.len = ((sngss7_span->rx_cgb.range + 1) >> 3) + (((sngss7_span->rx_cgb.range + 1) & 0x07) ? 1 : 0);
	for(x = 0; x < cgba.rangStat.status.len; x++){
		cgba.rangStat.status.val[x] = sngss7_span->rx_cgb.status[x];
	}

	sng_cc_sta_request (1,
						0,
						0,
						sngss7_span->rx_cgb.circuit,
						0,
						SIT_STA_CGBRSP,
						&cgba);
	
	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Tx CGBA (%d:%d)\n",
							sngss7_info->circuit->cic,
							sngss7_info->circuit->cic,
							(sngss7_info->circuit->cic + sngss7_span->rx_cgb.range));

	/* clean out the saved data */
	memset(&sngss7_span->rx_cgb, 0x0, sizeof(sngss7_group_data_t));

	SS7_FUNC_TRACE_EXIT (__FUNCTION__);
	return;
}

/******************************************************************************/
void ft_to_sngss7_cgua(ftdm_channel_t * ftdmchan)
{	
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);
	
	sngss7_span_data_t 	*sngss7_span = ftdmchan->span->signal_data;
	sngss7_chan_data_t	*sngss7_info = ftdmchan->call_data;
	int					x = 0;
	
	SiStaEvnt cgua;

	memset (&cgua, 0x0, sizeof(cgua));

	/* fill in the circuit group supervisory message */
	cgua.cgsmti.eh.pres = PRSNT_NODEF;
	cgua.cgsmti.typeInd.pres = PRSNT_NODEF;
	cgua.cgsmti.typeInd.val = sngss7_span->rx_cgu.type;

	cgua.rangStat.eh.pres = PRSNT_NODEF;
	/* fill in the range */	
	cgua.rangStat.range.pres = PRSNT_NODEF;
	cgua.rangStat.range.val = sngss7_span->rx_cgu.range;
	/* fill in the status */
	cgua.rangStat.status.pres = PRSNT_NODEF;
	cgua.rangStat.status.len = ((sngss7_span->rx_cgu.range + 1) >> 3) + (((sngss7_span->rx_cgu.range + 1) & 0x07) ? 1 : 0);
	for(x = 0; x < cgua.rangStat.status.len; x++){
		cgua.rangStat.status.val[x] = sngss7_span->rx_cgu.status[x];
	}

	sng_cc_sta_request (1,
						0,
						0,
						sngss7_span->rx_cgu.circuit,
						0,
						SIT_STA_CGURSP,
						&cgua);
	
	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Tx CGUA (%d:%d)\n",
							sngss7_info->circuit->cic,
							sngss7_info->circuit->cic,
							(sngss7_info->circuit->cic + sngss7_span->rx_cgu.range));

	/* clean out the saved data */
	memset(&sngss7_span->rx_cgu, 0x0, sizeof(sngss7_group_data_t));


	SS7_FUNC_TRACE_EXIT (__FUNCTION__);
	return;
}

/******************************************************************************/
void ft_to_sngss7_cgb(ftdm_channel_t * ftdmchan)
{
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);

	sngss7_span_data_t 	*sngss7_span = ftdmchan->span->signal_data;
	sngss7_chan_data_t	*sngss7_info = ftdmchan->call_data;
	SiStaEvnt 			cgb;
	int					x = 0;


	memset (&cgb, 0x0, sizeof(cgb));

	/* fill in the circuit group supervisory message */
	cgb.cgsmti.eh.pres			= PRSNT_NODEF;
	cgb.cgsmti.typeInd.pres		= PRSNT_NODEF;
	cgb.cgsmti.typeInd.val		= sngss7_span->tx_cgb.type;

	/* fill in the range */	
	cgb.rangStat.eh.pres 		= PRSNT_NODEF;
	cgb.rangStat.range.pres		= PRSNT_NODEF;
	cgb.rangStat.range.val 		= sngss7_span->tx_cgb.range;

	/* fill in the status */
	cgb.rangStat.status.pres	= PRSNT_NODEF;
	cgb.rangStat.status.len 	= ((sngss7_span->tx_cgb.range + 1) >> 3) + (((sngss7_span->tx_cgb.range + 1) & 0x07) ? 1 : 0);
	for(x = 0; x < cgb.rangStat.status.len; x++){
		cgb.rangStat.status.val[x] = sngss7_span->tx_cgb.status[x];
	}

	sng_cc_sta_request (1,
						0,
						0,
						sngss7_span->tx_cgb.circuit,
						0,
						SIT_STA_CGBREQ,
						&cgb);
	
	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Tx CGB (%d:%d)\n",
							sngss7_info->circuit->cic,
							sngss7_info->circuit->cic,
							(sngss7_info->circuit->cic + sngss7_span->tx_cgb.range));

	/* clean out the saved data */
	memset(&sngss7_span->tx_cgb, 0x0, sizeof(sngss7_group_data_t));


	SS7_FUNC_TRACE_EXIT (__FUNCTION__);
	return;
}

/******************************************************************************/
void ft_to_sngss7_cgu(ftdm_channel_t * ftdmchan)
{
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);

	sngss7_span_data_t 	*sngss7_span = ftdmchan->span->signal_data;
	sngss7_chan_data_t	*sngss7_info = ftdmchan->call_data;
	SiStaEvnt 			cgu;
	int					x = 0;


	memset (&cgu, 0x0, sizeof(cgu));

	/* fill in the circuit group supervisory message */
	cgu.cgsmti.eh.pres			= PRSNT_NODEF;
	cgu.cgsmti.typeInd.pres		= PRSNT_NODEF;
	cgu.cgsmti.typeInd.val		= sngss7_span->tx_cgu.type;

	/* fill in the range */	
	cgu.rangStat.eh.pres 		= PRSNT_NODEF;
	cgu.rangStat.range.pres		= PRSNT_NODEF;
	cgu.rangStat.range.val 		= sngss7_span->tx_cgu.range;

	/* fill in the status */
	cgu.rangStat.status.pres	= PRSNT_NODEF;
	cgu.rangStat.status.len 	= ((sngss7_span->tx_cgu.range + 1) >> 3) + (((sngss7_span->tx_cgu.range + 1) & 0x07) ? 1 : 0);
	for(x = 0; x < cgu.rangStat.status.len; x++){
		cgu.rangStat.status.val[x] = sngss7_span->tx_cgu.status[x];
	}

	sng_cc_sta_request (1,
						0,
						0,
						sngss7_span->tx_cgu.circuit,
						0,
						SIT_STA_CGUREQ,
						&cgu);
	
	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Tx CGU (%d:%d)\n",
							sngss7_info->circuit->cic,
							sngss7_info->circuit->cic,
							(sngss7_info->circuit->cic + sngss7_span->tx_cgu.range));

	/* clean out the saved data */
	memset(&sngss7_span->tx_cgu, 0x0, sizeof(sngss7_group_data_t));


	SS7_FUNC_TRACE_EXIT (__FUNCTION__);
	return;
}

/* French SPIROU send Charge Unit */
/* No one calls this function yet, but it has been implemented to complement TXA messages */
void ft_to_sngss7_itx (ftdm_channel_t * ftdmchan)
{
#ifndef SANGOMA_SPIROU
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);
	ftdm_log_chan_msg(ftdmchan, FTDM_LOG_CRIT, "ITX message not supported!, please update your libsng_ss7\n");
#else
	const char* var = NULL;
	sngss7_chan_data_t	*sngss7_info = ftdmchan->call_data;
	SiCnStEvnt itx;
	
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);

	memset (&itx, 0x0, sizeof (itx));

	itx.msgNum.eh.pres = PRSNT_NODEF;
	itx.msgNum.msgNum.pres = PRSNT_NODEF;
	var = ftdm_usrmsg_get_var(ftdmchan->usrmsg, "ss7_itx_msg_num");
	if (!ftdm_strlen_zero(var)) {
		itx.msgNum.msgNum.val = atoi(var);
	} else {
		itx.msgNum.msgNum.val = 0x1;
	}
	
	itx.chargUnitNum.eh.pres = PRSNT_NODEF;
	itx.chargUnitNum.chargUnitNum.pres = PRSNT_NODEF;
	var = ftdm_usrmsg_get_var(ftdmchan->usrmsg, "ss7_itx_charge_unit");
	if (!ftdm_strlen_zero(var)) {
		itx.chargUnitNum.chargUnitNum.val = atoi(var);
	} else {
		itx.chargUnitNum.chargUnitNum.val = 0x1;
	}

	ftdm_log_chan(ftdmchan, FTDM_LOG_INFO, "ITX Charging Unit:%d Msg Num:%d\n", itx.chargUnitNum.chargUnitNum.val, itx.msgNum.msgNum.val);
	sng_cc_con_status  (1, sngss7_info->suInstId, sngss7_info->spInstId, sngss7_info->circuit->id, &itx, CHARGE_UNIT);

	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Tx ITX\n", sngss7_info->circuit->cic);
#endif
	SS7_FUNC_TRACE_EXIT (__FUNCTION__);
	return;
}

/* French SPIROU send Charging Acknowledgement */
void ft_to_sngss7_txa (ftdm_channel_t * ftdmchan)
{	
#ifndef SANGOMA_SPIROU
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);
	ftdm_log_chan_msg(ftdmchan, FTDM_LOG_CRIT, "TXA message not supported!, please update your libsng_ss7\n");	
#else
	SiCnStEvnt txa;
	sngss7_chan_data_t	*sngss7_info = ftdmchan->call_data;
	SS7_FUNC_TRACE_ENTER (__FUNCTION__);
		
	memset (&txa, 0x0, sizeof(txa));

	sng_cc_con_status(1, sngss7_info->suInstId, sngss7_info->spInstId, sngss7_info->circuit->id, &txa, CHARGE_ACK);
	
	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Tx TXA\n", sngss7_info->circuit->cic);
#endif
	SS7_FUNC_TRACE_EXIT (__FUNCTION__);
	return;
}

/******************************************************************************/
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
/******************************************************************************/
