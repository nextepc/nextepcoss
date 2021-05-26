/*
 * Copyright (C) 2019,2020 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "sbi-path.h"

#include "npcf-handler.h"

bool pcf_npcf_am_policy_contrtol_handle_create(pcf_ue_t *pcf_ue,
        ogs_sbi_stream_t *stream, ogs_sbi_message_t *message)
{
    OpenAPI_policy_association_request_t *PolicyAssociationRequest = NULL;
    OpenAPI_guami_t *Guami = NULL;

    uint64_t supported_features = 0;

    ogs_assert(pcf_ue);
    ogs_assert(stream);
    ogs_assert(message);

    PolicyAssociationRequest = message->PolicyAssociationRequest;
    if (!PolicyAssociationRequest) {
        ogs_error("[%s] No PolicyAssociationRequest", pcf_ue->supi);
        ogs_sbi_server_send_error(stream, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                message, "[%s] No PolicyAssociationRequest", pcf_ue->supi);
        return false;
    }

    if (!PolicyAssociationRequest->notification_uri) {
        ogs_error("[%s] No notificationUri", pcf_ue->supi);
        ogs_sbi_server_send_error(stream, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                message, "No notificationUri", pcf_ue->supi);
        return false;
    }

    if (!PolicyAssociationRequest->supi) {
        ogs_error("[%s] No supi", pcf_ue->supi);
        ogs_sbi_server_send_error(stream, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                message, "No supi", pcf_ue->supi);
        return false;
    }

    if (!PolicyAssociationRequest->supp_feat) {
        ogs_error("[%s] No suppFeat", pcf_ue->supi);
        ogs_sbi_server_send_error(stream, OGS_SBI_HTTP_STATUS_BAD_REQUEST,
                message, "No suppFeat", pcf_ue->supi);
        return false;
    }

    if (pcf_ue->notification_uri)
        ogs_free(pcf_ue->notification_uri);
    pcf_ue->notification_uri = ogs_strdup(
            PolicyAssociationRequest->notification_uri);

    supported_features =
        ogs_uint64_from_string(PolicyAssociationRequest->supp_feat);
    pcf_ue->am_policy_control_features &= supported_features;

    if (pcf_ue->gpsi)
        ogs_free(pcf_ue->gpsi);
    pcf_ue->gpsi = ogs_strdup(PolicyAssociationRequest->gpsi);

    pcf_ue->access_type = PolicyAssociationRequest->access_type;

    if (pcf_ue->pei)
        ogs_free(pcf_ue->pei);
    pcf_ue->pei = ogs_strdup(PolicyAssociationRequest->pei);

    Guami = PolicyAssociationRequest->guami;
    if (Guami && Guami->amf_id &&
        Guami->plmn_id && Guami->plmn_id->mnc && Guami->plmn_id->mcc) {
        ogs_sbi_parse_guami(&pcf_ue->guami, PolicyAssociationRequest->guami);
    }

    if (PolicyAssociationRequest->rat_type)
        pcf_ue->rat_type = PolicyAssociationRequest->rat_type;

    pcf_ue->policy_association_request =
        OpenAPI_policy_association_request_copy(
                pcf_ue->policy_association_request,
                message->PolicyAssociationRequest);

    if (PolicyAssociationRequest->ue_ambr)
        pcf_ue->subscribed_ue_ambr = OpenAPI_ambr_copy(
                pcf_ue->subscribed_ue_ambr, PolicyAssociationRequest->ue_ambr);

    pcf_ue_sbi_discover_and_send(OpenAPI_nf_type_UDR, pcf_ue, stream, NULL,
            pcf_nudr_dr_build_query_am_data);

    return true;
}

bool pcf_npcf_smpolicycontrtol_handle_create(pcf_sess_t *sess,
        ogs_sbi_stream_t *stream, ogs_sbi_message_t *message)
{
    int status = 0;
    char *strerror = NULL;
    pcf_ue_t *pcf_ue = NULL;

    OpenAPI_sm_policy_context_data_t *SmPolicyContextData = NULL;
    OpenAPI_snssai_t *sliceInfo = NULL;

    uint64_t supported_features = 0;

    ogs_assert(sess);
    pcf_ue = sess->pcf_ue;
    ogs_assert(stream);
    ogs_assert(message);

    SmPolicyContextData = message->SmPolicyContextData;
    if (!SmPolicyContextData) {
        strerror = ogs_msprintf("[%s:%d] No SmPolicyContextData",
                pcf_ue->supi, sess->psi);
        status = OGS_SBI_HTTP_STATUS_BAD_REQUEST;
        goto cleanup;
    }

    if (!SmPolicyContextData->supi) {
        strerror = ogs_msprintf("[%s:%d] No supi", pcf_ue->supi, sess->psi);
        status = OGS_SBI_HTTP_STATUS_BAD_REQUEST;
        goto cleanup;
    }

    if (!SmPolicyContextData->pdu_session_id) {
        strerror = ogs_msprintf("[%s:%d] No pduSessionId",
                pcf_ue->supi, sess->psi);
        status = OGS_SBI_HTTP_STATUS_BAD_REQUEST;
        goto cleanup;
    }

    if (!SmPolicyContextData->pdu_session_type) {
        strerror = ogs_msprintf("[%s:%d] No pduSessionType",
                pcf_ue->supi, sess->psi);
        status = OGS_SBI_HTTP_STATUS_BAD_REQUEST;
        goto cleanup;
    }

    if (!SmPolicyContextData->dnn) {
        strerror = ogs_msprintf("[%s:%d] No dnn", pcf_ue->supi, sess->psi);
        status = OGS_SBI_HTTP_STATUS_BAD_REQUEST;
        goto cleanup;
    }

    if (!SmPolicyContextData->notification_uri) {
        strerror = ogs_msprintf("[%s:%d] No notificationUri",
                pcf_ue->supi, sess->psi);
        status = OGS_SBI_HTTP_STATUS_BAD_REQUEST;
        goto cleanup;
    }

    if (!SmPolicyContextData->ipv4_address &&
        !SmPolicyContextData->ipv6_address_prefix) {
        strerror = ogs_msprintf(
                "[%s:%d] No IPv4 address[%p] or IPv6 prefix[%p]",
                pcf_ue->supi, sess->psi,
                SmPolicyContextData->ipv4_address,
                SmPolicyContextData->ipv6_address_prefix);
        status = OGS_SBI_HTTP_STATUS_BAD_REQUEST;
        goto cleanup;
    }

    sliceInfo = SmPolicyContextData->slice_info;
    if (!sliceInfo) {
        strerror = ogs_msprintf("[%s:%d] No sliceInfo",
                pcf_ue->supi, sess->psi);
        status = OGS_SBI_HTTP_STATUS_BAD_REQUEST;
        goto cleanup;
    }

    if (!sliceInfo->sst) {
        strerror = ogs_msprintf("[%s:%d] No sliceInfo->sst",
                pcf_ue->supi, sess->psi);
        status = OGS_SBI_HTTP_STATUS_BAD_REQUEST;
        goto cleanup;
    }

    if (SmPolicyContextData->supp_feat) {
        supported_features =
            ogs_uint64_from_string(SmPolicyContextData->supp_feat);
        sess->smpolicycontrol_features &= supported_features;
    } else {
        sess->smpolicycontrol_features = 0;
    }

    sess->pdu_session_type = SmPolicyContextData->pdu_session_type;

    if (sess->dnn)
        ogs_free(sess->dnn);
    sess->dnn = ogs_strdup(SmPolicyContextData->dnn);

    if (sess->notification_uri)
        ogs_free(sess->notification_uri);
    sess->notification_uri = ogs_strdup(SmPolicyContextData->notification_uri);

    if (SmPolicyContextData->ipv4_address)
        ogs_assert(true ==
            pcf_sess_set_ipv4addr(sess, SmPolicyContextData->ipv4_address));
    if (SmPolicyContextData->ipv6_address_prefix)
        ogs_assert(true ==
            pcf_sess_set_ipv6prefix(
                sess, SmPolicyContextData->ipv6_address_prefix));

    sess->s_nssai.sst = sliceInfo->sst;
    sess->s_nssai.sd = ogs_s_nssai_sd_from_string(sliceInfo->sd);

    if (SmPolicyContextData->subs_sess_ambr)
        sess->subscribed_sess_ambr = OpenAPI_ambr_copy(
            sess->subscribed_sess_ambr, SmPolicyContextData->subs_sess_ambr);

    if (SmPolicyContextData->subs_def_qos)
        sess->subscribed_default_qos = OpenAPI_subscribed_default_qos_copy(
            sess->subscribed_default_qos, SmPolicyContextData->subs_def_qos);

    pcf_sess_sbi_discover_and_send(OpenAPI_nf_type_UDR, sess, stream, NULL,
            pcf_nudr_dr_build_query_sm_data);

    return true;

cleanup:
    ogs_assert(status);
    ogs_assert(strerror);
    ogs_error("%s", strerror);
    ogs_sbi_server_send_error(stream, status, message, strerror, NULL);
    ogs_free(strerror);

    return false;
}

bool pcf_npcf_smpolicycontrtol_handle_delete(pcf_sess_t *sess,
        ogs_sbi_stream_t *stream, ogs_sbi_message_t *message)
{
    int status = 0;
    char *strerror = NULL;
    pcf_ue_t *pcf_ue = NULL;

    OpenAPI_sm_policy_delete_data_t *SmPolicyDeleteData = NULL;

    ogs_assert(sess);
    pcf_ue = sess->pcf_ue;
    ogs_assert(stream);
    ogs_assert(message);

    SmPolicyDeleteData = message->SmPolicyDeleteData;
    if (!SmPolicyDeleteData) {
        strerror = ogs_msprintf("[%s:%d] No SmPolicyDeleteData",
                pcf_ue->supi, sess->psi);
        status = OGS_SBI_HTTP_STATUS_BAD_REQUEST;
        goto cleanup;
    }

    pcf_sess_sbi_discover_and_send(OpenAPI_nf_type_BSF, sess, stream, NULL,
            pcf_nbsf_management_build_de_register);

    return true;

cleanup:
    ogs_assert(status);
    ogs_assert(strerror);
    ogs_error("%s", strerror);
    ogs_sbi_server_send_error(stream, status, message, strerror, NULL);
    ogs_free(strerror);

    return false;
}

bool pcf_npcf_policyauthorization_handle_create(pcf_sess_t *sess,
        ogs_sbi_stream_t *stream, ogs_sbi_message_t *recvmsg)
{
    int status = 0;
    char *strerror = NULL;
    pcf_ue_t *pcf_ue = NULL;

    OpenAPI_app_session_context_t *AppSessionContext = NULL;
    OpenAPI_app_session_context_req_data_t *AscReqData = NULL;

    uint64_t supported_features = 0;

    ogs_sbi_server_t *server = NULL;
    ogs_sbi_header_t header;
    ogs_sbi_message_t sendmsg;
    ogs_sbi_response_t *response = NULL;

    ogs_assert(sess);
    pcf_ue = sess->pcf_ue;
    ogs_assert(stream);
    ogs_assert(recvmsg);

    ogs_assert(sess->app_session_id);

    server = ogs_sbi_server_from_stream(stream);
    ogs_assert(server);

    AppSessionContext = recvmsg->AppSessionContext;
    if (!AppSessionContext) {
        strerror = ogs_msprintf("[%s:%d] No AppSessionContext",
                pcf_ue->supi, sess->psi);
        status = OGS_SBI_HTTP_STATUS_BAD_REQUEST;
        goto cleanup;
    }

    AscReqData = AppSessionContext->asc_req_data;
    if (!AscReqData) {
        strerror = ogs_msprintf("[%s:%d] No AscReqData",
                pcf_ue->supi, sess->psi);
        status = OGS_SBI_HTTP_STATUS_BAD_REQUEST;
        goto cleanup;
    }

    if (!AscReqData->supp_feat) {
        strerror = ogs_msprintf("[%s:%d] No AscReqData->suppFeat",
                pcf_ue->supi, sess->psi);
        status = OGS_SBI_HTTP_STATUS_BAD_REQUEST;
        goto cleanup;
    }

    if (!AscReqData->notif_uri) {
        strerror = ogs_msprintf("[%s:%d] No AscReqData->notifUri",
                pcf_ue->supi, sess->psi);
        status = OGS_SBI_HTTP_STATUS_BAD_REQUEST;
        goto cleanup;
    }

    supported_features = ogs_uint64_from_string(AscReqData->supp_feat);
    sess->policyauthorization_features &= supported_features;

    memset(&sendmsg, 0, sizeof(sendmsg));

    memset(&header, 0, sizeof(header));
    header.service.name = (char *)OGS_SBI_SERVICE_NAME_NPCF_POLICYAUTHORIZATION;
    header.api.version = (char *)OGS_SBI_API_V1;
    header.resource.component[0] = (char *)OGS_SBI_RESOURCE_NAME_APP_SESSIONS;
    header.resource.component[1] = (char *)sess->app_session_id;
    sendmsg.http.location = ogs_sbi_server_uri(server, &header);
    ogs_assert(sendmsg.http.location);

    response = ogs_sbi_build_response(&sendmsg, OGS_SBI_HTTP_STATUS_CREATED);
    ogs_assert(response);
    ogs_sbi_server_send_response(stream, response);

    ogs_free(sendmsg.http.location);

    return true;

cleanup:
    ogs_assert(status);
    ogs_assert(strerror);
    ogs_error("%s", strerror);
    ogs_sbi_server_send_error(stream, status, recvmsg, strerror, NULL);
    ogs_free(strerror);

    return false;
}
