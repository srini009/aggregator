/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef _CLIENT_H
#define _CLIENT_H

#include "types.h"
#include "aggregator/aggregator-client.h"
#include "aggregator/aggregator-stream.h"

typedef struct aggregator_client {
   margo_instance_id mid;
   hg_id_t           stream_fetch_id;
   hg_id_t           list_streams_id;
   uint64_t          num_stream_handles;
} aggregator_client;

typedef struct aggregator_stream_handle {
    aggregator_client_t      client;
    hg_addr_t           addr;
    uint16_t            provider_id;
    uint64_t            refcount;
    aggregator_stream_id_t stream_id;
} aggregator_stream_handle;

#endif
