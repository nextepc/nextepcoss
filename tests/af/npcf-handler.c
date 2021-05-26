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

void af_npcf_policyauthorization_handle_create(
        af_sess_t *sess, ogs_sbi_message_t *recvmsg)
{
    int rv;

    ogs_sbi_message_t message;
    ogs_sbi_header_t header;

    if (!recvmsg->http.location) {
        ogs_error("[%s:%s] No http.location",
                sess->ipv4addr ? sess->ipv4addr : "Unknown",
                sess->ipv6addr ? sess->ipv6addr : "Unknown");
        return;
    }

    memset(&header, 0, sizeof(header));
    header.uri = recvmsg->http.location;

    rv = ogs_sbi_parse_header(&message, &header);
    if (rv != OGS_OK) {
        ogs_error("[%s:%s] Cannot parse http.location [%s]",
                sess->ipv4addr ? sess->ipv4addr : "Unknown",
                sess->ipv6addr ? sess->ipv6addr : "Unknown",
                recvmsg->http.location);
        return;
    }

    if (!message.h.resource.component[1]) {
        ogs_error("[%s:%s] No AppSessionId[%s]",
                sess->ipv4addr ? sess->ipv4addr : "Unknown",
                sess->ipv6addr ? sess->ipv6addr : "Unknown",
                recvmsg->http.location);

        ogs_sbi_header_free(&header);
        return;
    }

    af_sess_set_pcf_app_session_id(sess, message.h.resource.component[1]);

    ogs_sbi_header_free(&header);


    ogs_fatal("TODO");
#if 0
    int i, rv;

    OpenAPI_pcf_binding_t *PcfBinding = NULL;
    OpenAPI_list_t *PcfIpEndPointList = NULL;
    OpenAPI_lnode_t *node = NULL;
    char fqdn[OGS_MAX_FQDN_LEN];

    ogs_assert(sess);
    ogs_assert(recvmsg);

    PcfBinding = recvmsg->PcfBinding;
    ogs_assert(PcfBinding);

    if (!PcfBinding->pcf_fqdn &&
        !PcfBinding->pcf_ip_end_points) {
        ogs_error("No PCF address information [%p:%p]",
                    PcfBinding->pcf_fqdn, PcfBinding->pcf_ip_end_points);
        return;
    }

    if (PcfBinding->pcf_fqdn) {
        ogs_fqdn_parse(fqdn,
                PcfBinding->pcf_fqdn,
                strlen(PcfBinding->pcf_fqdn));
        if (sess->pcf.fqdn)
            ogs_free(sess->pcf.fqdn);
        sess->pcf.fqdn = ogs_strdup(fqdn);
        ogs_assert(sess->pcf.fqdn);
    }

    PcfIpEndPointList = PcfBinding->pcf_ip_end_points;

    for (i = 0; i < sess->pcf.num_of_ip; i++) {
        if (sess->pcf.ip[i].addr)
            ogs_freeaddrinfo(sess->pcf.ip[i].addr);
        if (sess->pcf.ip[i].addr6)
            ogs_freeaddrinfo(sess->pcf.ip[i].addr6);
    }
    sess->pcf.num_of_ip = 0;

    OpenAPI_list_for_each(PcfIpEndPointList, node) {
        OpenAPI_ip_end_point_t *IpEndPoint = node->data;
        ogs_sockaddr_t *addr = NULL, *addr6 = NULL;
        int port = 0;

        if (!IpEndPoint) continue;

        if (sess->pcf.num_of_ip < OGS_SBI_MAX_NUM_OF_IP_ADDRESS) {
            port = IpEndPoint->port;
            if (!port) {
                if (ogs_sbi_default_uri_scheme() ==
                        OpenAPI_uri_scheme_http)
                    port = OGS_SBI_HTTP_PORT;
                else if (ogs_sbi_default_uri_scheme() ==
                        OpenAPI_uri_scheme_https)
                    port = OGS_SBI_HTTPS_PORT;
                else {
                    ogs_fatal("Invalid scheme [%d]",
                        ogs_sbi_default_uri_scheme());
                    ogs_assert_if_reached();
                }
            }

            if (IpEndPoint->ipv4_address) {
                rv = ogs_getaddrinfo(&addr, AF_UNSPEC,
                        IpEndPoint->ipv4_address, port, 0);
                if (rv != OGS_OK) continue;
            }
            if (IpEndPoint->ipv6_address) {
                rv = ogs_getaddrinfo(&addr6, AF_UNSPEC,
                        IpEndPoint->ipv6_address, port, 0);
                if (rv != OGS_OK) continue;
            }

            if (addr || addr6) {
                sess->pcf.ip[sess->pcf.num_of_ip].port = port;
                sess->pcf.ip[sess->pcf.num_of_ip].addr = addr;
                sess->pcf.ip[sess->pcf.num_of_ip].addr6 = addr6;
                sess->pcf.num_of_ip++;
            }
        }
    }

    if (PcfBinding->supi) {
        if (sess->supi)
            ogs_free(sess->supi);
        sess->supi = ogs_strdup(PcfBinding->supi);
    }
    if (PcfBinding->gpsi) {
        if (sess->gpsi)
            ogs_free(sess->gpsi);
        sess->gpsi = ogs_strdup(PcfBinding->gpsi);
    }

    af_sess_associate_pcf_client(sess);
#endif
}
