/*
 * Copyright (c) 2009 Konrad Hammel <konrad@sangoma.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms|with or without
 * modification|are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice|this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 * notice|this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of the original author; nor the names of any contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES|INCLUDING|BUT NOT
 * LIMITED TO|THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT|INDIRECT|INCIDENTAL|SPECIAL,
 * EXEMPLARY|OR CONSEQUENTIAL DAMAGES (INCLUDING|BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE|DATA|OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY|WHETHER IN CONTRACT|STRICT LIABILITY|OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE|EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* INCLUDE ********************************************************************/
#include "ftmod_sangoma_ss7_main.h"
/******************************************************************************/

/* DEFINES ********************************************************************/
/******************************************************************************/

/* GLOBALS ********************************************************************/
/******************************************************************************/

/* PROTOTYPES *****************************************************************/
ftdm_status_t handle_con_ind(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, SiConEvnt *siConEvnt);
ftdm_status_t handle_con_sta(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, SiCnStEvnt *siCnStEvnt, uint8_t evntType);
ftdm_status_t handle_con_cfm(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, SiConEvnt *siConEvnt);
ftdm_status_t handle_rel_ind(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, SiRelEvnt *siRelEvnt);
ftdm_status_t handle_rel_cfm(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, SiRelEvnt *siRelEvnt);
ftdm_status_t handle_dat_ind(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, SiInfoEvnt *siInfoEvnt);
ftdm_status_t handle_fac_ind(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t evntType, SiFacEvnt *siFacEvnt);
ftdm_status_t handle_fac_cfm(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t evntType, SiFacEvnt *siFacEvnt);
ftdm_status_t handle_umsg_ind(uint32_t suInstId, uint32_t spInstId, uint32_t circuit);
ftdm_status_t handle_susp_ind(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, SiSuspEvnt *siSuspEvnt);
ftdm_status_t handle_resm_ind(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, SiResmEvnt *siResmEvnt);
ftdm_status_t handle_sta_ind(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);

ftdm_status_t handle_reattempt(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_pause(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_resume(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_cot_start(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_cot_stop(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_cot(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_rsc_req(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_local_rsc_req(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_rsc_rsp(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_grs_req(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_grs_rsp(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_blo_req(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_blo_rsp(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_ubl_req(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_ubl_rsp(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_local_blk(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_local_ubl(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_ucic(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_cgb_req(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_cgu_req(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
ftdm_status_t handle_olm_msg(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt);
/******************************************************************************/

/* FUNCTIONS ******************************************************************/
ftdm_status_t handle_con_ind(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, SiConEvnt *siConEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t  *sngss7_info = NULL;
	ftdm_channel_t	  	*ftdmchan = NULL;
	char				nadi[2];

	memset(nadi, '\0', sizeof(nadi));

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	if (sngss7_test_ckt_flag(sngss7_info, FLAG_GLARE)) {
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx IAM (glare)\n", sngss7_info->circuit->cic);
	} else {
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx IAM\n", sngss7_info->circuit->cic);
	}

	/* check if the circuit has a remote block */
	if ((sngss7_test_ckt_flag(sngss7_info, FLAG_CKT_MN_BLOCK_RX)) ||
		(sngss7_test_ckt_flag(sngss7_info, FLAG_GRP_HW_BLOCK_RX)) ||
		(sngss7_test_ckt_flag(sngss7_info, FLAG_GRP_MN_BLOCK_RX))) {

		/* as per Q.764, 2.8.2.3 xiv ... remove the block from this channel */
		sngss7_clear_ckt_flag(sngss7_info, FLAG_CKT_MN_BLOCK_RX);
		sngss7_clear_ckt_flag(sngss7_info, FLAG_GRP_HW_BLOCK_RX);
		sngss7_clear_ckt_flag(sngss7_info, FLAG_GRP_MN_BLOCK_RX);

		/* KONRAD FIX ME : check in case there is a ckt and grp block */
	}

	/* check whether the ftdm channel is in a state to accept a call */
	switch (ftdmchan->state) {
	/**************************************************************************/
	case (FTDM_CHANNEL_STATE_DOWN):	 /* only state it is valid to get IAM (except if there is glare */

		/* check if there is any reason why we can't use this channel */
		if (ftdm_test_flag(ftdmchan, FTDM_CHANNEL_INUSE)) {
			/* channel is already requested for use by the ftdm core */
			goto handle_glare;
		} else if(ftdm_channel_open_chan(ftdmchan) != FTDM_SUCCESS) {
			/* channel is not inuse but we can't open it...fail the call */
			SS7_ERROR("Failed to open span: %d, chan: %d\n",
						ftdmchan->physical_span_id,
						ftdmchan->physical_chan_id);

			 /* set the flag to indicate this hangup is started from the local side */
			sngss7_set_ckt_flag(sngss7_info, FLAG_LOCAL_REL);

			ftdmchan->caller_data.hangup_cause = 41;

			/* move the state to CANCEL */
			ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_CANCEL);

		} else {

			/* fill in the channels SS7 Stack information */
			sngss7_info->suInstId = get_unique_id();
			sngss7_info->spInstId = spInstId;

			/* fill in calling party information */
			if (siConEvnt->cgPtyNum.eh.pres) {
				if (siConEvnt->cgPtyNum.addrSig.pres) {
					/* fill in cid_num */
					copy_tknStr_from_sngss7(siConEvnt->cgPtyNum.addrSig,
											ftdmchan->caller_data.cid_num.digits, 
											siConEvnt->cgPtyNum.oddEven);

					/* fill in cid Name */
					ftdm_set_string(ftdmchan->caller_data.cid_name, ftdmchan->caller_data.cid_num.digits);

					/* fill in ANI */
					ftdm_set_string(ftdmchan->caller_data.ani.digits, ftdmchan->caller_data.cid_num.digits);
				}

				if (siConEvnt->cgPtyNum.scrnInd.pres) {
					/* fill in the screening indication value */
					ftdmchan->caller_data.screen = siConEvnt->cgPtyNum.scrnInd.val;
				}

				if (siConEvnt->cgPtyNum.presRest.pres) {
					/* fill in the presentation value */
					ftdmchan->caller_data.pres = siConEvnt->cgPtyNum.presRest.val;
				}	
			} else {
				SS7_INFO_CHAN(ftdmchan,"No Calling party (ANI) information in IAM!%s\n", " ");
			}

			/* fill in called party infomation */
			if (siConEvnt->cdPtyNum.eh.pres) {
				if (siConEvnt->cdPtyNum.addrSig.pres) {
					/* fill in the called number/dnis */
					copy_tknStr_from_sngss7(siConEvnt->cdPtyNum.addrSig, 
											ftdmchan->caller_data.dnis.digits, 
											siConEvnt->cdPtyNum.oddEven);
				}
			} else {
				SS7_INFO_CHAN(ftdmchan,"No Called party (DNIS) information in IAM!%s\n", " ");
			}

			/* fill in rdnis information*/
			if (siConEvnt->redirgNum.eh.pres) {
				if (siConEvnt->redirgNum.addrSig.pres) {
					/* fill in the rdnis digits */
					copy_tknStr_from_sngss7(siConEvnt->redirgNum.addrSig, 
											ftdmchan->caller_data.rdnis.digits, 
											siConEvnt->cgPtyNum.oddEven);
				}
			}   else {
				SS7_DEBUG_CHAN(ftdmchan,"No RDNIS party information in IAM!%s\n", " ");
			}

			/* fill in the TMR/bearer capability */
			if (siConEvnt->txMedReq.eh.pres) {
				if (siConEvnt->txMedReq.trMedReq.pres) {
					/* fill in the bearer type */
					ftdmchan->caller_data.bearer_capability = siConEvnt->txMedReq.trMedReq.val;
				}
			} else {
				SS7_DEBUG_CHAN(ftdmchan,"No TMR/Bearer Cap information in IAM!%s\n", " ");
			}

			/* add any special variables for the dialplan */
			sprintf(nadi, "%d", siConEvnt->cgPtyNum.natAddrInd.val);
			ftdm_call_add_var(&ftdmchan->caller_data, "ss7_clg_nadi", nadi);

			sprintf(nadi, "%d", siConEvnt->cdPtyNum.natAddrInd.val);
			ftdm_call_add_var(&ftdmchan->caller_data, "ss7_cld_nadi", nadi);


			/* check if a COT test is requested */
			if ((siConEvnt->natConInd.eh.pres) && 
				(siConEvnt->natConInd.contChkInd.pres) &&
				(siConEvnt->natConInd.contChkInd.val)) {

				SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Found COT Request\n", sngss7_info->circuit->cic);

				/* tell the core to loop the channel */
				ftdm_channel_command(ftdmchan, FTDM_COMMAND_ENABLE_LOOP, NULL);

				/* move to in loop state */
				ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_IN_LOOP);
			} else {
				/* set the state of the channel to collecting...the rest is done by the chan monitor */
				ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_COLLECT);
			}

		} /* if (channel is usable */

		break;
	/**************************************************************************/
	case (FTDM_CHANNEL_STATE_DIALING):
	case (FTDM_CHANNEL_STATE_TERMINATING):
	case (FTDM_CHANNEL_STATE_HANGUP):
	case (FTDM_CHANNEL_STATE_HANGUP_COMPLETE):
handle_glare:
		/* the core already has plans for this channel...glare */
		SS7_INFO_CHAN(ftdmchan, "Got IAM on channel that is already inuse (state=%s|inuse=%d)...glare!\n", 
								ftdm_channel_state2str (ftdmchan->state),
								ftdm_test_flag(ftdmchan, FTDM_CHANNEL_INUSE));

		/* save the info so that we can use it later on */
		sngss7_info->glare.spInstId = spInstId;
		sngss7_info->glare.circuit = circuit;
		memcpy(&sngss7_info->glare.iam, siConEvnt, sizeof(*siConEvnt));

		if (!(sngss7_test_ckt_flag(sngss7_info, FLAG_GLARE))) {
			/* glare, throw the flag */
			sngss7_set_ckt_flag(sngss7_info, FLAG_GLARE);
		
			/* setup the hangup cause */
			ftdmchan->caller_data.hangup_cause = 34;	/* Circuit Congrestion */
		
			/* this is a remote hangup request */
			sngss7_set_ckt_flag(sngss7_info, FLAG_REMOTE_REL);
		
			/* move the state of the channel to Terminating to end the call */
			ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_TERMINATING);
		} /* if (!(sngss7_test_ckt_flag(sngss7_info, FLAG_GLARE))) */
		break;
	/**************************************************************************/
	default:	/* should not have gotten an IAM while in this state */
		SS7_ERROR_CHAN(ftdmchan, "Got IAM on channel in invalid state(%s)...reset!\n", ftdm_channel_state2str (ftdmchan->state));

		/* reset the cic */
		sngss7_set_ckt_flag(sngss7_info, FLAG_RESET_TX);

		/* move the state of the channel to RESTART to force a reset */
		ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_RESTART);

		break;
	/**************************************************************************/
	} /* switch (ftdmchan->state) */

	/* unlock the channel */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_con_sta(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, SiCnStEvnt *siCnStEvnt, uint8_t evntType)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t  *sngss7_info ;
	ftdm_channel_t	  *ftdmchan;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	switch (evntType) {
	/**************************************************************************/
	case (ADDRCMPLT):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx ACM\n", sngss7_info->circuit->cic);

		switch (ftdmchan->state) {
		/**********************************************************************/
		case FTDM_CHANNEL_STATE_DIALING:
			/* KONRAD: should we confirm the instance ids ? */

			/* need to grab the sp instance id */ 
			sngss7_info->spInstId = spInstId;

			if ((siCnStEvnt->optBckCalInd.eh.pres) && 
				(siCnStEvnt->optBckCalInd.inbndInfoInd.pres)) {

				if (siCnStEvnt->optBckCalInd.inbndInfoInd.val) {
					/* go to PROGRESS_MEDIA */
					ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_PROGRESS_MEDIA);
				} else {
					/* go to PROGRESS */
					ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_PROGRESS);
				} /* if (inband) */
			} else {
				/* go to PROGRESS_MEDIA */
				ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_PROGRESS_MEDIA);
			}
			
			break;
		/**********************************************************************/
		default:	/* incorrect state...reset the CIC */
			SS7_ERROR_CHAN(ftdmchan, "RX ACM in invalid state :%s...resetting CIC\n", 
									ftdm_channel_state2str (ftdmchan->state));

			/* reset the cic */
			sngss7_set_ckt_flag(sngss7_info, FLAG_RESET_TX);

			/* go to RESTART */
			ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_RESTART);
			break;
		/**********************************************************************/
		} /* switch (ftdmchan->state) */

		break;
	/**************************************************************************/
	case (MODIFY):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx MODIFY\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (MODCMPLT):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx MODIFY-COMPLETE\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (MODREJ):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx MODIFY-REJECT\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (PROGRESS):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx CPG\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (FRWDTRSFR):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx FOT\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (INFORMATION):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx INF\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (INFORMATREQ):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx INR\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (SUBSADDR):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx SAM\n", sngss7_info->circuit->cic);

		/* check the channel state */
		switch (ftdmchan->state) {
		/**********************************************************************/
		case (FTDM_CHANNEL_STATE_COLLECT):
			
			/* confirm that the event contains the subsquent number field */
			if (siCnStEvnt->subNum.eh.pres && siCnStEvnt->subNum.addrSig.pres) {
				/* add the digits to the ftdm channel variable */
				append_tknStr_from_sngss7(siCnStEvnt->subNum.addrSig, 
											ftdmchan->caller_data.dnis.digits, 
											siCnStEvnt->subNum.oddEven);
			} else {
				SS7_INFO_CHAN(ftdmchan,"No Called party (DNIS) information in SAM!%s\n", " ");
			}

			/* go to idle so that collect state is processed again */
			ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_IDLE);

			break;
		/**********************************************************************/
		default:
			SS7_ERROR_CHAN(ftdmchan, "RX SAM in invalid state :%s...ignoring\n", 
										ftdm_channel_state2str (ftdmchan->state));
			break;
		/**********************************************************************/
		} /* switch (ftdmchan->state) */

		break;
	/**************************************************************************/
	case (EXIT):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx EXIT\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (NETRESMGT):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx NRM\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (IDENTREQ):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx IDR\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (IDENTRSP):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx IRS\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (MALCLLPRNT):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx MALICIOUS CALL\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (CHARGE):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx CRG\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (TRFFCHGE):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx CRG-TARIFF\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (CHARGEACK):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx CRG-ACK\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (CALLOFFMSG):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx CALL-OFFER\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (LOOPPRVNT):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx LOP\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (TECT_TIMEOUT):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx ECT-Timeout\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (RINGSEND):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx RINGING-SEND\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (CALLCLEAR):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx CALL-LINE Clear\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (PRERELEASE):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx PRI\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (APPTRANSPORT):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx APM\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (OPERATOR):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx OPERATOR\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (METPULSE):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx METERING-PULSE\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (CLGPTCLR):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx CALLING_PARTY_CLEAR\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	case (SUBDIRNUM):
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx SUB-DIR\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	default:
	   	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx Unknown Msg\n", sngss7_info->circuit->cic);
		break;
	/**************************************************************************/
	}

	/* unlock the channel */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_con_cfm(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, SiConEvnt *siConEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t  *sngss7_info ;
	ftdm_channel_t	  *ftdmchan;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	/* check whether the ftdm channel is in a state to accept a call */
	switch (ftdmchan->state) {
	/**************************************************************************/
	case FTDM_CHANNEL_STATE_PROGRESS:
	case FTDM_CHANNEL_STATE_PROGRESS_MEDIA:

		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx ANM\n", sngss7_info->circuit->cic);

		/* go to UP */
		ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_UP);

		break;
	/**************************************************************************/
	case FTDM_CHANNEL_STATE_DIALING:

		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx CON\n", sngss7_info->circuit->cic);

		/* go to UP */
		ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_UP);

		break;		
	/**************************************************************************/
	default:	/* incorrect state...reset the CIC */

		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx ANM/CON\n", sngss7_info->circuit->cic);

		/* throw the TX reset flag */
		sngss7_set_ckt_flag(sngss7_info, FLAG_GRP_RESET_TX);

		/* go to RESTART */
		ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_RESTART);

		break;
	/**************************************************************************/
	}

	/* unlock the channel */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_rel_ind(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, SiRelEvnt *siRelEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t  *sngss7_info ;
	ftdm_channel_t	  *ftdmchan;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx REL cause=%d\n",
							sngss7_info->circuit->cic, 
							siRelEvnt->causeDgn.causeVal.val);

	/* check whether the ftdm channel is in a state to release a call */
	switch (ftdmchan->state) {
	/**************************************************************************/
	case FTDM_CHANNEL_STATE_DIALING:

		/* pass the release code up to FTDM */
		if (siRelEvnt->causeDgn.causeVal.pres) {
			ftdmchan->caller_data.hangup_cause = siRelEvnt->causeDgn.causeVal.val;
		} else {
			SS7_ERROR("REL does not have a cause code!\n");
			ftdmchan->caller_data.hangup_cause = 0;
		}

		/* this is a remote hangup request */
		sngss7_set_ckt_flag(sngss7_info, FLAG_REMOTE_REL);
ftdm_channel_command(ftdmchan, FTDM_COMMAND_DISABLE_LOOP, NULL);
		/* move the state of the channel to CANCEL to end the call */
		ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_TERMINATING);

		break;
	/**************************************************************************/
	case FTDM_CHANNEL_STATE_RING:
	case FTDM_CHANNEL_STATE_PROGRESS:
	case FTDM_CHANNEL_STATE_PROGRESS_MEDIA:
	case FTDM_CHANNEL_STATE_UP:

		/* pass the release code up to FTDM */
		if (siRelEvnt->causeDgn.causeVal.pres) {
			ftdmchan->caller_data.hangup_cause = siRelEvnt->causeDgn.causeVal.val;
		} else {
			SS7_ERROR("REL does not have a cause ftdm_channel_command(ftdmchan, FTDM_COMMAND_DISABLE_LOOP, NULL);code!\n");
			ftdmchan->caller_data.hangup_cause = 0;
		}

		/* this is a remote hangup request */
		sngss7_set_ckt_flag(sngss7_info, FLAG_REMOTE_REL);

		/* move the state of the channel to TERMINATING to end the call */
		ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_TERMINATING);

		break;
	/**************************************************************************/
	case FTDM_CHANNEL_STATE_IN_LOOP:

		/* inform the core to unloop the channel*/
		ftdm_channel_command(ftdmchan, FTDM_COMMAND_DISABLE_LOOP, NULL);

		/* since we need to acknowledge the hang up set the flag for remote release */
		sngss7_set_ckt_flag(sngss7_info, FLAG_REMOTE_REL);

		/* go to hangup complete to send the RLC */
		ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_HANGUP_COMPLETE);

		/* save the call info for the RLC */
		sngss7_info->suInstId = get_unique_id();
		sngss7_info->spInstId = spInstId;

		break;
	/**************************************************************************/
	default:

		/* throw the reset flag */
		sngss7_set_ckt_flag(sngss7_info, FLAG_RESET_RX);

		/* set the state to RESTART */
		ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_RESTART);
		break;
	/**************************************************************************/
	} /* switch (ftdmchan->state) */


	/* unlock the channel */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_rel_cfm(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, SiRelEvnt *siRelEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);
	
	sngss7_chan_data_t  *sngss7_info ;
	ftdm_channel_t	  *ftdmchan;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx RLC\n", sngss7_info->circuit->cic);

	/* check whether the ftdm channel is in a state to accept a call */
	switch (ftdmchan->state) {
	/**************************************************************************/
	case FTDM_CHANNEL_STATE_HANGUP_COMPLETE:

		/* go to DOWN */
		ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_DOWN);

		break;
	/**************************************************************************/
	case FTDM_CHANNEL_STATE_DOWN:
		/* do nothing, just drop the message */
		break;
	/**************************************************************************/
	default:	
		/* KONRAD: should just stop the call...but a reset is easier for now (since it does hangup the call) */

		/* go to RESTART */
		ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_RESTART);

		break;
	/**************************************************************************/
	}

	/* unlock the channel */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_dat_ind(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, SiInfoEvnt *siInfoEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);
	
	sngss7_chan_data_t  *sngss7_info ;
	ftdm_channel_t	  *ftdmchan;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx DATA IND\n", sngss7_info->circuit->cic);

	/* unlock the channel */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_fac_ind(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t evntType, SiFacEvnt *siFacEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);
	
	sngss7_chan_data_t  *sngss7_info ;
	ftdm_channel_t	  *ftdmchan;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx FAC\n", sngss7_info->circuit->cic);

	/* unlock the channel */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_fac_cfm(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t evntType, SiFacEvnt *siFacEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);
	
	sngss7_chan_data_t  *sngss7_info ;
	ftdm_channel_t	  *ftdmchan;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx FAC-CON\n", sngss7_info->circuit->cic);

	/* unlock the channel */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_umsg_ind(uint32_t suInstId, uint32_t spInstId, uint32_t circuit)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);
	
	sngss7_chan_data_t  *sngss7_info ;
	ftdm_channel_t	  *ftdmchan;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx USER-USER msg\n", sngss7_info->circuit->cic);

	/* unlock the channel */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_susp_ind(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, SiSuspEvnt *siSuspEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);
	
	sngss7_chan_data_t  *sngss7_info ;
	ftdm_channel_t	  *ftdmchan;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx Call-Suspend msg\n", sngss7_info->circuit->cic);

	/* unlock the channel */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_resm_ind(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, SiResmEvnt *siResmEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);
	
	sngss7_chan_data_t  *sngss7_info ;
	ftdm_channel_t	  *ftdmchan;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx Call-Resume msg\n", sngss7_info->circuit->cic);

	/* unlock the channel */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_sta_ind(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	sngss7_chan_data_t  *sngss7_info ;
	ftdm_channel_t	  *ftdmchan;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	switch (evntType) {
	/**************************************************************************/
	case SIT_STA_REATTEMPT:		 /* reattempt indication */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx Reattempt indication\n", sngss7_info->circuit->cic);
		handle_reattempt(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_ERRORIND:		  /* error indication */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx Error indication\n", sngss7_info->circuit->cic);
		SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));
		break;
	/**************************************************************************/
	case SIT_STA_CONTCHK:		   /* continuity check */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx CCR start\n", sngss7_info->circuit->cic);
		handle_cot_start(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_CONTREP:		   /* continuity report */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx COT report\n", sngss7_info->circuit->cic);
		handle_cot(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_STPCONTIN:		 /* stop continuity */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx CCR stop\n", sngss7_info->circuit->cic);
		handle_cot_stop(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_CGQRYRSP:		  /* circuit grp query response from far end forwarded to upper layer by ISUP */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx CQM\n", sngss7_info->circuit->cic);
		SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));
		break;
	/**************************************************************************/
	case SIT_STA_CONFUSION:		 /* confusion */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx CFN\n", sngss7_info->circuit->cic);
		SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));
		break;
	/**************************************************************************/
	case SIT_STA_LOOPBACKACK:	   /* loop-back acknowledge */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx LPA\n", sngss7_info->circuit->cic);
		SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));
		break;
	/**************************************************************************/
	case SIT_STA_CIRRSRVREQ:		/* circuit reservation request */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx Ckt Resveration req\n", sngss7_info->circuit->cic);
		SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));
		break;
	/**************************************************************************/
	case SIT_STA_CIRRSRVACK:		/* circuit reservation acknowledgement */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx Ckt Res ack\n", sngss7_info->circuit->cic);
		SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));
		break;
	/**************************************************************************/
	case SIT_STA_CIRBLOREQ:		 /* circuit blocking request */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx BLO\n", sngss7_info->circuit->cic);
		handle_blo_req(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_CIRBLORSP:		 /* circuit blocking response   */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx BLA\n", sngss7_info->circuit->cic);
		handle_blo_rsp(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_CIRUBLREQ:		 /* circuit unblocking request */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx UBL\n", sngss7_info->circuit->cic);
		handle_ubl_req(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_CIRUBLRSP:		 /* circuit unblocking response */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx UBA\n", sngss7_info->circuit->cic);
		handle_ubl_rsp(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_CIRRESREQ:		 /* circuit reset request - RSC */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx RSC\n", sngss7_info->circuit->cic);
		handle_rsc_req(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_CIRLOCRES:		 /* reset initiated locally by the software */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx Local RSC\n", sngss7_info->circuit->cic);
		handle_local_rsc_req(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_CIRRESRSP:		 /* circuit reset response */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx RSC-RLC\n", sngss7_info->circuit->cic);
		handle_rsc_rsp(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_CGBREQ:			/* CGB request */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx CGB\n", sngss7_info->circuit->cic);
		handle_cgb_req(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_CGUREQ:			/* CGU request */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx CGU\n", sngss7_info->circuit->cic);
		handle_cgu_req(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_CGQRYREQ:		  /* circuit group query request */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx CQM\n", sngss7_info->circuit->cic);
		SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));
		break;
	/**************************************************************************/
	case SIT_STA_CGBRSP:			/* mntc. oriented CGB response */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx mntc CGB\n", sngss7_info->circuit->cic);
		/*handle_cgb_req(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);*/
		break;
	/**************************************************************************/
	case SIT_STA_CGURSP:			/* mntc. oriented CGU response */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx mntc CGU\n", sngss7_info->circuit->cic);
		/*SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));*/
		break;
	/**************************************************************************/
	case SIT_STA_GRSREQ:			/* circuit group reset request */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx GRS\n", sngss7_info->circuit->cic);
		handle_grs_req(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_CIRUNEQPD:		 /* circuit unequipped indication */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx UCIC\n", sngss7_info->circuit->cic);
		handle_ucic(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_GRSRSP:			/* circuit group reset response */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx GRA\n", sngss7_info->circuit->cic);
		handle_grs_rsp(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_PAUSEIND:		  /* pause indication */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx SUS\n", sngss7_info->circuit->cic);
		handle_pause(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_RESUMEIND:		 /* resume indication */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx RES\n", sngss7_info->circuit->cic);
		handle_resume(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_USRPARTA:		  /* user part available */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx UPA\n", sngss7_info->circuit->cic);
		SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));
		break;
	/**************************************************************************/
	case SIT_STA_RMTUSRUNAV:		/* remote user not available */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx Remote User not Available\n", sngss7_info->circuit->cic);
		SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));
		break;
	/**************************************************************************/
	case SIT_STA_MTPCONG0:		  /* congestion indication level 0 */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx Congestion L0\n", sngss7_info->circuit->cic);
		SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));
		break;
	/**************************************************************************/
	case SIT_STA_MTPCONG1:		  /* congestion indication level 1 */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx Congestion L1\n", sngss7_info->circuit->cic);
		SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));
		break;
	/**************************************************************************/
	case SIT_STA_MTPCONG2:		  /* congestion indication level 2 */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx Congestion L2\n", sngss7_info->circuit->cic);
		SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));
		break;
	/**************************************************************************/
	case SIT_STA_MTPCONG3:		  /* congestion indication level 3 */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx Congestion L3\n", sngss7_info->circuit->cic);
		SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));
		break;
	/**************************************************************************/
	case SIT_STA_MTPSTPCONG:		/* stop congestion indication level 0 */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx Stop Congestion\n", sngss7_info->circuit->cic);
		SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));
		break; 
	/**************************************************************************/
	case SIT_STA_CIRLOCALBLOIND:	/* Mngmt local blocking */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx Local BLO\n", sngss7_info->circuit->cic);
		handle_local_blk(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_CIRLOCALUBLIND:	/* Mngmt local unblocking */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx Local UBL\n", sngss7_info->circuit->cic);
		handle_local_ubl(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_OVERLOAD:		  /* Overload */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx Overload\n", sngss7_info->circuit->cic);
		handle_olm_msg(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);
		break;
	/**************************************************************************/
	case SIT_STA_LMCGBREQ:		  /* when LM requests ckt grp blocking */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx LM CGB\n", sngss7_info->circuit->cic);
		SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));
		break;
	/**************************************************************************/
	case SIT_STA_LMCGUREQ:		  /* when LM requests ckt grp unblocking */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx LM CGU\n", sngss7_info->circuit->cic);
		SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));
		break;
	/**************************************************************************/
	case SIT_STA_LMGRSREQ:		  /* when LM requests ckt grp reset */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx LM RSC\n", sngss7_info->circuit->cic);
		SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));
		break;
	/**************************************************************************/
	case SIT_STA_CGBINFOIND:		/* circuit grp blking ind , no resp req */
		/*SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx CGB no resp req\n", sngss7_info->circuit->cic);*/
/*		handle_cgb_req(suInstId, spInstId, circuit, globalFlg, evntType, siStaEvnt);*/
		break;
	/**************************************************************************/
	case SIT_STA_LMCQMINFOREQ:	  /* when LM requests ckt grp query */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx LM CQM\n", sngss7_info->circuit->cic);
// 		SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));
		break;
	/**************************************************************************/
	case SIT_STA_CIRLOCGRS:		 /* group reset initiated locally by the software */
		SS7_INFO_CHAN(ftdmchan,"[CIC:%d]Rx Local GRS\n", sngss7_info->circuit->cic);
		SS7_WARN(" %s indication not currently supported\n", DECODE_LCC_EVENT(evntType));
		break;
	/**************************************************************************/
	default:
		SS7_INFO("[SNG-CC] Received Unknown indication %d\n", evntType);
		break;
	} /* switch (evntType) */

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;

}

/******************************************************************************/
ftdm_status_t handle_reattempt(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t  *sngss7_info = NULL;
	ftdm_channel_t	  *ftdmchan = NULL;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for ISUP circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	if (sngss7_test_ckt_flag(sngss7_info, FLAG_GLARE)) {
		/* the glare flag is already up so it was caught ... do nothing */
		SS7_DEBUG_CHAN(ftdmchan, "Glare flag is already up...nothing to do!%s\n", " ");
	} else {
		SS7_DEBUG_CHAN(ftdmchan, "Glare flag is not up yet...indicating glare from reattempt!%s\n", " ");
		/* glare, throw the flag */
		sngss7_set_ckt_flag(sngss7_info, FLAG_GLARE);

		/* clear any existing glare data from the channel */
		memset(&sngss7_info->glare, 0x0, sizeof(sngss7_glare_data_t));

		/* setup the hangup cause */
		ftdmchan->caller_data.hangup_cause = 34;	/* Circuit Congrestion */

		/* this is a remote hangup request */
		sngss7_set_ckt_flag(sngss7_info, FLAG_REMOTE_REL);

		/* move the state of the channel to Terminating to end the call */
		ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_TERMINATING);
	}

	/* unlock the channel again before we exit */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_pause(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);
	
	sngss7_chan_data_t  *sngss7_info = NULL;
	ftdm_channel_t	  *ftdmchan = NULL;
	int				 infId;
	int				 i;
	
	/* extract the affected infId from the circuit structure */
	infId = g_ftdm_sngss7_data.cfg.isupCkt[circuit].infId;

	/* set the interface to paused */
	sngss7_set_flag(&g_ftdm_sngss7_data.cfg.isupIntf[infId], SNGSS7_PAUSED);
	
	/* go through all the circuits now and find any other circuits on this infId */
	i = (g_ftdm_sngss7_data.cfg.procId * 1000) + 1;
	while (g_ftdm_sngss7_data.cfg.isupCkt[i].id != 0) {
		
		/* check that the infId matches and that this is not a siglink */
		if ((g_ftdm_sngss7_data.cfg.isupCkt[i].infId == infId) && 
			(g_ftdm_sngss7_data.cfg.isupCkt[i].type == VOICE)) {
	
			/* get the ftdmchan and ss7_chan_data from the circuit */
			if (extract_chan_data(i, &sngss7_info, &ftdmchan)) {
				SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
				i++;
				continue;
			}
	
			/* lock the channel */
			ftdm_mutex_lock(ftdmchan->mutex);
	
			/* check if the circuit is fully started */
			if (ftdm_test_flag(ftdmchan->span, FTDM_SPAN_IN_THREAD)) {
				SS7_DEBUG_CHAN(ftdmchan, "Rx PAUSE%s\n", "");
				/* set the pause flag on the channel */
				sngss7_set_ckt_flag(sngss7_info, FLAG_INFID_PAUSED);
			}
	
			/* unlock the channel again before we exit */
			ftdm_mutex_unlock(ftdmchan->mutex);
	
		} /* if (g_ftdm_sngss7_data.cfg.isupCkt[i].infId == infId) */
	
		/* move to the next circuit */
		i++;
	
	} /* while (g_ftdm_sngss7_data.cfg.isupCkt[i].id != 0) */
	
	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_resume(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t  *sngss7_info = NULL;
	ftdm_channel_t	  *ftdmchan = NULL;
	int				 infId;
	int				 i;

	/* extract the affect infId from the circuit structure */
	infId = g_ftdm_sngss7_data.cfg.isupCkt[circuit].infId;

	/* set the interface to resumed */
	sngss7_clear_flag(&g_ftdm_sngss7_data.cfg.isupIntf[infId], SNGSS7_PAUSED);

	/* go through all the circuits now and find any other circuits on this infId */
	i = (g_ftdm_sngss7_data.cfg.procId * 1000) + 1;
	while (g_ftdm_sngss7_data.cfg.isupCkt[i].id != 0) {

		/* check that the infId matches and that this is not a siglink */
		if ((g_ftdm_sngss7_data.cfg.isupCkt[i].infId == infId) && 
			(g_ftdm_sngss7_data.cfg.isupCkt[i].type == VOICE)) {

			/* get the ftdmchan and ss7_chan_data from the circuit */
			if (extract_chan_data(i, &sngss7_info, &ftdmchan)) {
				SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
				i++;
				continue;
			}

			/* lock the channel */
			ftdm_mutex_lock(ftdmchan->mutex);

			/* only resume if we are paused */
			if (sngss7_test_ckt_flag(sngss7_info, FLAG_INFID_PAUSED)) {
				SS7_DEBUG_CHAN(ftdmchan, "Rx RESUME%s\n", "");

				/* set the resume flag on the channel */
				sngss7_set_ckt_flag(sngss7_info, FLAG_INFID_RESUME);

				/* clear the paused flag */
				sngss7_clear_ckt_flag(sngss7_info, FLAG_INFID_PAUSED);
			}
			
			/* unlock the channel again before we exit */
			ftdm_mutex_unlock(ftdmchan->mutex);

		} /* if (g_ftdm_sngss7_data.cfg.isupCkt[i].infId == infId) */

		/* move to the next circuit */
		i++;

	} /* while (g_ftdm_sngss7_data.cfg.isupCkt[i].id != 0) */

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_cot_start(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t  *sngss7_info = NULL;
	ftdm_channel_t	  *ftdmchan = NULL;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	/* open the channel if it is not open */
	if (!ftdm_test_flag(ftdmchan, FTDM_CHANNEL_OPEN)) {
		if (ftdm_channel_open_chan(ftdmchan) != FTDM_SUCCESS) {
			SS7_ERROR("Failed to open CIC %d for CCR test!\n", sngss7_info->circuit->cic);
			/* KONRAD FIX ME */
			SS7_FUNC_TRACE_EXIT(__FUNCTION__);
			return FTDM_FAIL;
		}
	}

	/* tell the core to loop the channel */
	ftdm_channel_command(ftdmchan, FTDM_COMMAND_ENABLE_LOOP, NULL);

	/* switch to the IN_LOOP state */
	ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_IN_LOOP);

	/* unlock the channel again before we exit */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_cot_stop(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t  *sngss7_info = NULL;
	ftdm_channel_t	  *ftdmchan = NULL;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	/* tell the core to stop looping the channel */
	ftdm_channel_command(ftdmchan, FTDM_COMMAND_DISABLE_LOOP, NULL);

	/* exit out of the LOOP state to the last state */
	ftdm_set_state_locked(ftdmchan, ftdmchan->last_state);

	/* unlock the channel again before we exit */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_cot(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t  *sngss7_info = NULL;
	ftdm_channel_t	  *ftdmchan = NULL;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	switch (ftdmchan->state) {
	/**************************************************************************/
	case (FTDM_CHANNEL_STATE_IN_LOOP):
		/* tell the core to stop looping the channel */
		ftdm_channel_command(ftdmchan, FTDM_COMMAND_DISABLE_LOOP, NULL);
	
		/* exit out of the LOOP state and go to collect */
		ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_COLLECT);
		
		break;
	/**************************************************************************/
	default:
		/* exit out of the LOOP state to the last state */
		ftdm_set_state_locked(ftdmchan, ftdmchan->last_state);

		break;
	/**************************************************************************/
	} /* switch (ftdmchan->state) */

	if ( (siStaEvnt->contInd.eh.pres > 0) && (siStaEvnt->contInd.contInd.pres > 0)) {
		SS7_INFO("Continuity Test result for CIC = %d (span %d, chan %d) is: \"%s\"\n",
					g_ftdm_sngss7_data.cfg.isupCkt[circuit].cic,
					g_ftdm_sngss7_data.cfg.isupCkt[circuit].span,
					g_ftdm_sngss7_data.cfg.isupCkt[circuit].chan,
					(siStaEvnt->contInd.contInd.val) ? "PASS" : "FAIL");
	} else {
		SS7_ERROR("Recieved Continuity report containing no results!\n");
	}

	/* unlock the channel again before we exit */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_blo_req(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t  *sngss7_info = NULL;
	ftdm_channel_t	  *ftdmchan = NULL;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	/* check if the circuit is already blocked or not */
	if (sngss7_test_ckt_flag(sngss7_info, FLAG_CKT_MN_BLOCK_RX)) {
		SS7_WARN("Received BLO on circuit that is already blocked!\n");
	}

	/* throw the ckt block flag */
	sngss7_set_ckt_flag(sngss7_info, FLAG_CKT_MN_BLOCK_RX);

	/* set the channel to suspended state */
	ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_SUSPENDED);

	/* unlock the channel again before we exit */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_blo_rsp(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t  *sngss7_info = NULL;
	ftdm_channel_t	  *ftdmchan = NULL;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	/* KONRAD FIX ME */

	/* unlock the channel again before we exit */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_ubl_req(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t  *sngss7_info = NULL;
	ftdm_channel_t	  *ftdmchan = NULL;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	/* check if the channel is blocked */
	if (!(sngss7_test_ckt_flag(sngss7_info, FLAG_CKT_MN_BLOCK_RX))) {
		SS7_WARN("Received UBL on circuit that is not blocked!\n");
	}

	/* throw the unblock flag */
	sngss7_set_ckt_flag(sngss7_info, FLAG_CKT_MN_UNBLK_RX);

	/* clear the block flag */
	sngss7_clear_ckt_flag(sngss7_info, FLAG_CKT_MN_BLOCK_RX);

	/* set the channel to suspended state */
	ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_SUSPENDED);

	/* unlock the channel again before we exit */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_ubl_rsp(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t  *sngss7_info = NULL;
	ftdm_channel_t	  *ftdmchan = NULL;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	/* KONRAD FIX ME */

	/* unlock the channel again before we exit */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_rsc_req(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t  *sngss7_info = NULL;
	ftdm_channel_t	  *ftdmchan = NULL;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	/* throw the reset flag */
	sngss7_set_ckt_flag(sngss7_info, FLAG_RESET_RX);

	switch (ftdmchan->state) {
	/**************************************************************************/
	case FTDM_CHANNEL_STATE_RESTART:

		/* go to idle so that we can redo the restart state*/
		ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_IDLE);

		break;
	/**************************************************************************/
	default:

		/* set the state of the channel to restart...the rest is done by the chan monitor */
		ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_RESTART);
		break;
	/**************************************************************************/
	}

	/* unlock the channel again before we exit */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_local_rsc_req(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t  *sngss7_info = NULL;
	ftdm_channel_t	  *ftdmchan = NULL;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	/* throw the reset flag */
	sngss7_set_ckt_flag(sngss7_info, FLAG_RESET_RX);

	switch (ftdmchan->state) {
	/**************************************************************************/
	case FTDM_CHANNEL_STATE_RESTART:

		/* go to idle so that we can redo the restart state*/
		ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_IDLE);

		break;
	/**************************************************************************/
	default:

		/* set the state of the channel to restart...the rest is done by the chan monitor */
		ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_RESTART);
		break;
	/**************************************************************************/
	}

	/* unlock the channel again before we exit */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_rsc_rsp(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t  *sngss7_info = NULL;
	ftdm_channel_t	  *ftdmchan = NULL;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	switch (ftdmchan->state) {
	/**********************************************************************/
	case FTDM_CHANNEL_STATE_RESTART:
		
		if ( sngss7_test_ckt_flag(sngss7_info, FLAG_RESET_TX) ) {
			/* throw the FLAG_RESET_TX_RSP to indicate we have acknowledgement from the remote side */
			sngss7_set_ckt_flag(sngss7_info, FLAG_RESET_TX_RSP);

			/* go to DOWN */
			ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_DOWN);
		} else {
			SS7_ERROR("Received RSC-RLC but we're not waiting on a RSC-RLC on CIC #, dropping\n", sngss7_info->circuit->cic);
		}

		break;
	/**********************************************************************/
	case FTDM_CHANNEL_STATE_DOWN:
		
		/* do nothing, just drop the message */
		SS7_DEBUG("Receveived RSC-RLC in down state, dropping\n");
		
		break;
	/**********************************************************************/
	case FTDM_CHANNEL_STATE_TERMINATING:
	case FTDM_CHANNEL_STATE_HANGUP:
	case FTDM_CHANNEL_STATE_HANGUP_COMPLETE:
		
		/* throw the FLAG_RESET_TX_RSP to indicate we have acknowledgement from the remote side */
		sngss7_set_ckt_flag(sngss7_info, FLAG_RESET_TX_RSP);

		/* go to DOWN */
		/*ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_DOWN);*/

		break;
	/**********************************************************************/
	default:
		/* ITU Q764-2.9.5.1.c -> release the circuit */
		if ((siStaEvnt != NULL) &&
			(siStaEvnt->causeDgn.eh.pres ==PRSNT_NODEF) &&
			(siStaEvnt->causeDgn.causeVal.pres == PRSNT_NODEF)) {
			ftdmchan->caller_data.hangup_cause = siStaEvnt->causeDgn.causeVal.val;
		} else {
			ftdmchan->caller_data.hangup_cause = 98;	/* Message not compatiable with call state */
		}

		ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_TERMINATING);
		break;
	/**********************************************************************/
	}

	/* unlock the channel again before we exit */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}
/******************************************************************************/
ftdm_status_t handle_grs_req(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t	*sngss7_info = NULL;
	ftdm_channel_t		*ftdmchan = NULL;
	sngss7_span_data_t	*sngss7_span = NULL; 
	int					range;


	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* extract the range value from the event structure */
	if ((siStaEvnt->rangStat.eh.pres == PRSNT_NODEF) && (siStaEvnt->rangStat.range.pres == PRSNT_NODEF)) {
		range = siStaEvnt->rangStat.range.val;
	} else {
		SS7_ERROR("Received GRS with no range value on CIC = %d\n", sngss7_info->circuit->cic);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* fill in the span structure for this circuit */
	sngss7_span = ftdmchan->span->signal_data;
	sngss7_span->rx_grs.circuit = circuit; 
	sngss7_span->rx_grs.range = range;

	/* the reset will be started in the main thread by "check_if_rx_grs_started" */

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_grs_rsp(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t	*sngss7_info = NULL;
	ftdm_channel_t		*ftdmchan = NULL;
	sngss7_span_data_t	*sngss7_span = NULL; 
	int					range;

	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* extract the range value from the event structure */
	if ((siStaEvnt->rangStat.eh.pres == PRSNT_NODEF) && (siStaEvnt->rangStat.range.pres == PRSNT_NODEF)) {
		range = siStaEvnt->rangStat.range.val;
	} else {
		SS7_ERROR("Received GRA with no range value on CIC = %d\n", sngss7_info->circuit->cic);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* fill in the span structure for this circuit */
	sngss7_span = ftdmchan->span->signal_data;
	sngss7_span->rx_gra.circuit = circuit; 
	sngss7_span->rx_gra.range = range;

	/* check if there is a cause value in the GRA */
	if ((siStaEvnt != NULL) &&
		(siStaEvnt->causeDgn.eh.pres == PRSNT_NODEF) &&
		(siStaEvnt->causeDgn.causeVal.pres == PRSNT_NODEF)) {

		sngss7_span->rx_gra.cause = siStaEvnt->causeDgn.causeVal.val;
	}

	/* the reset will be started in the main thread by "check_if_rx_gra_started" */
	
	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_local_blk(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t  *sngss7_info = NULL;
	ftdm_channel_t	  *ftdmchan = NULL;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	/* check if the circuit is already blocked or not */
	if (sngss7_test_ckt_flag(sngss7_info, FLAG_CKT_LC_BLOCK_RX)) {
		SS7_WARN("Received local BLO on circuit that is already blocked!\n");
	}

	/* throw the ckt block flag */
	sngss7_set_ckt_flag(sngss7_info, FLAG_CKT_LC_BLOCK_RX);

	/* set the channel to suspended state */
	ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_SUSPENDED);

	/* unlock the channel again before we exit */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_local_ubl(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t  *sngss7_info = NULL;
	ftdm_channel_t	  *ftdmchan = NULL;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	/* check if the circuit is already blocked or not */
	if (sngss7_test_ckt_flag(sngss7_info, FLAG_CKT_LC_UNBLK_RX)) {
		SS7_WARN("Received local UBL on circuit that is already unblocked!\n");
	}

	/* throw the ckt block flag */
	sngss7_set_ckt_flag(sngss7_info, FLAG_CKT_LC_UNBLK_RX);

	/* set the channel to suspended state */
	ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_SUSPENDED);

	/* unlock the channel again before we exit */
	ftdm_mutex_unlock(ftdmchan->mutex);

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_ucic(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t	*sngss7_info = NULL;
	sngss7_span_data_t	*sngss7_span = NULL;
	ftdm_channel_t		*ftdmchan = NULL;


	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* check if we just sent a GRS request...*/
	sngss7_span = ftdmchan->span->signal_data;
	if (sngss7_span->tx_grs.circuit > 0) {
		/* we need to put all circuits on this UCIC */
		sngss7_span->ucic.circuit = sngss7_span->tx_grs.circuit;
		sngss7_span->ucic.range = sngss7_span->tx_grs.range;
		goto done;
	}

	/* lock the channel */
	ftdm_mutex_lock(ftdmchan->mutex);

	/* throw the ckt block flag */
	sngss7_set_ckt_flag(sngss7_info, FLAG_CKT_UCIC_BLOCK);

	/* set the channel to suspended state */
	ftdm_set_state_locked(ftdmchan, FTDM_CHANNEL_STATE_SUSPENDED);

	/* unlock the channel again before we exit */
	ftdm_mutex_unlock(ftdmchan->mutex);
done:
	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_cgb_req(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t	*sngss7_info = NULL;
	sngss7_span_data_t	*sngss7_span = NULL;
	ftdm_channel_t		*ftdmchan = NULL;
	int					range;
	uint8_t				status[255];
	int					blockType = 0;
	int					byte = 0;
	int					bit = 0;
	int 				x;
	ftdm_sigmsg_t 		sigev;

	memset(&sigev, 0, sizeof (sigev));
	memset(&status[0], '\0', sizeof(status));

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* grab the span info */
	sngss7_span = ftdmchan->span->signal_data;

	/* figure out what type of block needs to be applied */
	if ((siStaEvnt->cgsmti.eh.pres == PRSNT_NODEF) && (siStaEvnt->cgsmti.typeInd.pres == PRSNT_NODEF)) {
		blockType = siStaEvnt->cgsmti.typeInd.val;
	} else {
		SS7_ERROR("Received CGB with no circuit group supervision value on CIC = %d\n", sngss7_info->circuit->cic);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}	

	/* pull out the range value */
	if ((siStaEvnt->rangStat.eh.pres == PRSNT_NODEF) && (siStaEvnt->rangStat.range.pres == PRSNT_NODEF)) {
		range = siStaEvnt->rangStat.range.val;
	} else {
		SS7_ERROR("Received CGB with no range value on CIC = %d\n", sngss7_info->circuit->cic);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* pull out the status field */
	if ((siStaEvnt->rangStat.eh.pres == PRSNT_NODEF) && (siStaEvnt->rangStat.status.pres == PRSNT_NODEF)) {
		for (x = 0; x < siStaEvnt->rangStat.status.len; x++) {
			status[x] = siStaEvnt->rangStat.status.val[x];
		}
	} else {
		SS7_ERROR("Received CGB with no status value on CIC = %d\n", sngss7_info->circuit->cic);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* save the circuit, range and status */
	sngss7_span->rx_cgb.circuit = circuit;
	sngss7_span->rx_cgb.range = range;
	sngss7_span->rx_cgb.type = blockType;
	for (x = 0; x < siStaEvnt->rangStat.status.len; x++) {
		sngss7_span->rx_cgb.status[x] = status[x];
	}

	/* loop over the cics starting from circuit until range+1 */
	for (x = circuit; x < (circuit + range + 1); x++) {
		/* confirm this is a voice channel */
		if (g_ftdm_sngss7_data.cfg.isupCkt[x].type != VOICE) continue;

		/* grab the circuit in question */
		if (extract_chan_data(x, &sngss7_info, &ftdmchan)) {
			SS7_ERROR("Failed to extract channel data for circuit = %d!\n", x);
			break;
		}
	
		/* lock the channel */
		ftdm_mutex_lock(ftdmchan->mutex);

#if 0
		SS7_ERROR("KONRAD -> circuit=%d, byte=%d, bit=%d, status[byte]=%d, math=%d\n",
					x,
					byte,
					bit,
					status[byte],
					(status[byte] & (1 << bit)));
#endif
		if (status[byte] & (1 << bit)) {
			switch (blockType) {
			/**********************************************************************/
			case 0:	/* maintenance oriented */
				sngss7_set_ckt_flag(sngss7_info, FLAG_GRP_MN_BLOCK_RX);
				break;
			/**********************************************************************/
			case 1: /* hardware failure oriented */
				sngss7_set_ckt_flag(sngss7_info, FLAG_GRP_HW_BLOCK_RX);
				break;
			/**********************************************************************/
			case 2: /* reserved for national use */
				break;
			/**********************************************************************/
			default:
				break;
			/**********************************************************************/
			} /* switch (blockType) */
		}

		sigev.chan_id = ftdmchan->chan_id;
		sigev.span_id = ftdmchan->span_id;
		sigev.channel = ftdmchan;

		/* bring the sig status down */
		sigev.event_id = FTDM_SIGEVENT_SIGSTATUS_CHANGED;
		sigev.ev_data.sigstatus.status = FTDM_SIG_STATE_DOWN;
		ftdm_span_send_signal(ftdmchan->span, &sigev);

		/* unlock the channel again before we exit */
		ftdm_mutex_unlock(ftdmchan->mutex);

		/* update the bit and byte counter*/
		bit ++;
		if (bit == 8) {
			byte++;
			bit = 0;
		}

	} /* for (x = circuit; x < (circuit + range + 1); x++) */

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	ft_to_sngss7_cgba(ftdmchan);

	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_cgu_req(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t	*sngss7_info = NULL;
	sngss7_span_data_t	*sngss7_span = NULL;
	ftdm_channel_t		*ftdmchan = NULL;
	int					range;
	uint8_t				status[255];
	int					blockType = 0;
	int					byte = 0;
	int					bit = 0;
	int 				x;
	ftdm_sigmsg_t 		sigev;

	memset(&sigev, 0, sizeof (sigev));
	memset(&status[0], '\0', sizeof(status));

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* grab the span info */
	sngss7_span = ftdmchan->span->signal_data;

	/* figure out what type of block needs to be applied */
	if ((siStaEvnt->cgsmti.eh.pres == PRSNT_NODEF) && (siStaEvnt->cgsmti.typeInd.pres == PRSNT_NODEF)) {
		blockType = siStaEvnt->cgsmti.typeInd.val;
	} else {
		SS7_ERROR("Received CGU with no circuit group supervision value on CIC = %d\n", sngss7_info->circuit->cic);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}	

	/* pull out the range value */
	if ((siStaEvnt->rangStat.eh.pres == PRSNT_NODEF) && (siStaEvnt->rangStat.range.pres == PRSNT_NODEF)) {
		range = siStaEvnt->rangStat.range.val;
	} else {
		SS7_ERROR("Received CGU with no range value on CIC = %d\n", sngss7_info->circuit->cic);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* pull out the status field */
	if ((siStaEvnt->rangStat.eh.pres == PRSNT_NODEF) && (siStaEvnt->rangStat.status.pres == PRSNT_NODEF)) {
		for (x = 0; x < siStaEvnt->rangStat.status.len; x++) {
			status[x] = siStaEvnt->rangStat.status.val[x];
		}
	} else {
		SS7_ERROR("Received CGU with no status value on CIC = %d\n", sngss7_info->circuit->cic);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* save the circuit, range and status */
	sngss7_span->rx_cgu.circuit = circuit;
	sngss7_span->rx_cgu.range = range;
	sngss7_span->rx_cgu.type = blockType;
	for (x = 0; x < siStaEvnt->rangStat.status.len; x++) {
		sngss7_span->rx_cgu.status[x] = status[x];
	}

	/* loop over the cics starting from circuit until range+1 */
	for (x = circuit; x < (circuit + range + 1); x++) {
		if (g_ftdm_sngss7_data.cfg.isupCkt[x].type != VOICE) continue;
		/* grab the circuit in question */
		if (extract_chan_data(x, &sngss7_info, &ftdmchan)) {
			SS7_ERROR("Failed to extract channel data for circuit = %d!\n", x);
			break;
		}
	
		/* lock the channel */
		ftdm_mutex_lock(ftdmchan->mutex);

		if (status[byte] & (1 << bit)) {
			switch (blockType) {
			/**********************************************************************/
			case 0:	/* maintenance oriented */
				sngss7_clear_ckt_flag(sngss7_info, FLAG_GRP_MN_BLOCK_RX);
				break;
			/**********************************************************************/
			case 1: /* hardware failure oriented */
				sngss7_clear_ckt_flag(sngss7_info, FLAG_GRP_HW_BLOCK_RX);
				break;
			/**********************************************************************/
			case 2: /* reserved for national use */
				break;
			/**********************************************************************/
			default:
				break;
			/**********************************************************************/
			} /* switch (blockType) */
		} /* if (status[byte] & (1 << bit)) */

		sigev.chan_id = ftdmchan->chan_id;
		sigev.span_id = ftdmchan->span_id;
		sigev.channel = ftdmchan;

		/* bring the sig status down */
		sigev.event_id = FTDM_SIGEVENT_SIGSTATUS_CHANGED;
		sigev.ev_data.sigstatus.status = FTDM_SIG_STATE_UP;
		ftdm_span_send_signal(ftdmchan->span, &sigev);
	
		/* unlock the channel again before we exit */
		ftdm_mutex_unlock(ftdmchan->mutex);

		/* update the bit and byte counter*/
		bit ++;
		if (bit == 8) {
			byte++;
			bit = 0;
		}

	} /* for (x = circuit; x < (circuit + range + 1); x++) */

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	ft_to_sngss7_cgua(ftdmchan);

	return FTDM_SUCCESS;
}

/******************************************************************************/
ftdm_status_t handle_olm_msg(uint32_t suInstId, uint32_t spInstId, uint32_t circuit, uint8_t globalFlg, uint8_t evntType, SiStaEvnt *siStaEvnt)
{
	SS7_FUNC_TRACE_ENTER(__FUNCTION__);

	sngss7_chan_data_t	*sngss7_info = NULL;
	ftdm_channel_t		*ftdmchan = NULL;

	/* get the ftdmchan and ss7_chan_data from the circuit */
	if (extract_chan_data(circuit, &sngss7_info, &ftdmchan)) {
		SS7_ERROR("Failed to extract channel data for circuit = %d!\n", circuit);
		SS7_FUNC_TRACE_EXIT(__FUNCTION__);
		return FTDM_FAIL;
	}

	/* handle overload */
	SS7_ERROR_CHAN(ftdmchan,"[CIC:%d]Rx Overload\n", sngss7_info->circuit->cic);

	sng_isup_reg_info_show();

	SS7_FUNC_TRACE_EXIT(__FUNCTION__);
	return FTDM_SUCCESS;
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
