/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __AGGREGATOR_STREAM_H
#define __AGGREGATOR_STREAM_H

#include <margo.h>
#include <aggregator/aggregator-common.h>
#include <aggregator/aggregator-client.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct aggregator_stream_handle *aggregator_stream_handle_t;
typedef struct aggregator_stream *aggregator_stream_t;
typedef enum aggregator_stream_type aggregator_stream_type_t;
typedef struct aggregator_provider* aggregator_provider_t;
typedef struct aggregator_taglist* aggregator_taglist_t;
typedef struct aggregator_stream_sample* aggregator_stream_buffer;
typedef struct aggregator_stream_sample aggregator_stream_sample;
typedef void (*func)();
#define AGGREGATOR_STREAM_HANDLE_NULL ((aggregator_stream_handle_t)NULL)

/* APIs for providers to record performance data */
aggregator_return_t aggregator_taglist_create(aggregator_taglist_t *taglist, int num_tags, ...);
aggregator_return_t aggregator_taglist_destroy(aggregator_taglist_t taglist);
aggregator_return_t aggregator_stream_create(const char *ns, const char *name, aggregator_stream_type_t t, const char *desc, aggregator_taglist_t taglist, aggregator_stream_t* stream_handle, aggregator_provider_t provider);
aggregator_return_t aggregator_stream_destroy(aggregator_stream_t m, aggregator_provider_t provider);
aggregator_return_t aggregator_stream_destroy_all(aggregator_provider_t provider);
aggregator_return_t aggregator_stream_update(aggregator_stream_t m, double val);
aggregator_return_t aggregator_stream_update_gauge_by_fixed_amount(aggregator_stream_t m, double diff);
aggregator_return_t aggregator_stream_dump_histogram(aggregator_stream_t m, const char *filename, size_t num_buckets);
aggregator_return_t aggregator_stream_dump_raw_data(aggregator_stream_t m, const char *filename);
aggregator_return_t aggregator_stream_class_register_retrieval_callback(char *ns, func f);

/* APIs for remote clients to request for performance data */
aggregator_return_t aggregator_remote_stream_get_id(char *ns, char *name, aggregator_taglist_t taglist, aggregator_stream_id_t* stream_id);
aggregator_return_t aggregator_remote_stream_handle_create(aggregator_client_t client, hg_addr_t addr, uint16_t provider_id, aggregator_stream_id_t stream_id, aggregator_stream_handle_t* handle);
aggregator_return_t aggregator_remote_stream_handle_ref_incr(aggregator_stream_handle_t handle);
aggregator_return_t aggregator_remote_stream_handle_release(aggregator_stream_handle_t handle);
aggregator_return_t aggregator_remote_stream_fetch(aggregator_stream_handle_t handle, int64_t *num_samples_requested, aggregator_stream_buffer *buf, char **name, char **ns);
aggregator_return_t aggregator_remote_list_streams(aggregator_client_t client, hg_addr_t addr, uint16_t provider_id, aggregator_stream_id_t** ids, size_t* count);

#ifdef __cplusplus
}
#endif

#endif
