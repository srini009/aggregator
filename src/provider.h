/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __PROVIDER_H
#define __PROVIDER_H

#include <margo.h>
#include <abt-io.h>
#include "uthash.h"
#include "types.h"

typedef struct aggregator_provider {
    /* Margo/Argobots/Mercury environment */
    margo_instance_id  mid;                 // Margo instance
    uint16_t           provider_id;         // Provider id
    ABT_pool           pool;                // Pool on which to post RPC requests
    abt_io_instance_id abtio;               // ABT-IO instance
    /* Resources and backend types */
    size_t               num_streams;     // number of streams
    aggregator_stream*      streams;         // hash of streams by id
    /* RPC identifiers for clients */
    hg_id_t list_streams_id;
    hg_id_t stream_fetch_id;
    /* ... add other RPC identifiers here ... */
} aggregator_provider;

aggregator_return_t aggregator_provider_stream_create(const char *ns, const char *name, aggregator_stream_type_t t, const char *desc, aggregator_taglist_t tl, aggregator_stream_t* m, aggregator_provider_t provider);

aggregator_return_t aggregator_provider_stream_destroy(aggregator_stream_t m, aggregator_provider_t provider);

aggregator_return_t aggregator_provider_destroy_all_streams(aggregator_provider_t provider);
#endif
