/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <stdio.h>
#include <margo.h>
#include <assert.h>
#include <aggregator/aggregator-client.h>
#include <aggregator/aggregator-stream.h>

#define FATAL(...) \
    do { \
        margo_critical(__VA_ARGS__); \
        exit(-1); \
    } while(0)

int main(int argc, char** argv)
{
    if(argc != 3) {
        fprintf(stderr,"Usage: %s <server address> <provider id>\n", argv[0]);
        exit(-1);
    }

    aggregator_return_t ret;
    hg_return_t hret;
    const char* svr_addr_str = argv[1];
    uint16_t    provider_id  = atoi(argv[2]);

    margo_instance_id mid = margo_init("na+sm://", MARGO_CLIENT_MODE, 0, 0);
    assert(mid);

    hg_addr_t svr_addr;
    hret = margo_addr_lookup(mid, svr_addr_str, &svr_addr);
    if(hret != HG_SUCCESS) {
        FATAL(mid,"margo_addr_lookup failed for address %s", svr_addr_str);
    }

    aggregator_client_t aggregator_clt;
    aggregator_stream_handle_t aggregator_rh;

    margo_info(mid, "Creating AGGREGATOR client");
    ret = aggregator_client_init(mid, &aggregator_clt);
    if(ret != AGGREGATOR_SUCCESS) {
        FATAL(mid,"aggregator_client_init failed (ret = %d)", ret);
    }

    size_t count = 5;
    aggregator_stream_id_t *ids;
    ids = (aggregator_stream_id_t *)malloc(count*sizeof(aggregator_stream_id_t));
    ret = aggregator_remote_list_streams(aggregator_clt, svr_addr, provider_id, &ids, &count);

    aggregator_taglist_t taglist;
    aggregator_taglist_create(&taglist, 3, "tag1", "tag2", "tag3");

    aggregator_stream_id_t id;
    aggregator_remote_stream_get_id("srini", "teststream2", taglist, &id);
    fprintf(stderr, "Retrieved stream id is: %d\n", id);

    if(ret != AGGREGATOR_SUCCESS) {
	fprintf(stderr, "aggregator_remote_list_streams failed (ret = %d)\n", ret);
    } else {
	fprintf(stderr, "Retrieved a total of %lu streams\n", count);
        size_t j = 0;
        for(j = 0; j < count; j++)
           fprintf(stderr, "Retrieved stream with id: %d\n", ids[j]);
    }

    ret = aggregator_remote_stream_handle_create(
            aggregator_clt, svr_addr, provider_id,
            id, &aggregator_rh);

    if(ret != AGGREGATOR_SUCCESS) {
        FATAL(mid,"aggregator_stream_handle_create failed (ret = %d)", ret);
    }

    int64_t num_samples_requested = 5;
    char *name, *ns;
    aggregator_stream_buffer buf;
    ret = aggregator_remote_stream_fetch(aggregator_rh, &num_samples_requested, &buf, &name, &ns);
    fprintf(stderr, "Number of streams fetched %lu, with name %s and ns %s\n", num_samples_requested, name, ns);
    int i;
    aggregator_stream_sample *b = buf;
    for (i = 0; i < num_samples_requested; i++) {
        fprintf(stderr, "Values are : %f, and %f\n", b[i].val, b[i].time);
    }

    margo_info(mid, "Releasing stream handle");
    ret = aggregator_remote_stream_handle_release(aggregator_rh);
    if(ret != AGGREGATOR_SUCCESS) {
        FATAL(mid,"aggregator_stream_handle_release failed (ret = %d)", ret);
    }

    margo_info(mid, "Finalizing client");
    ret = aggregator_client_finalize(aggregator_clt);
    if(ret != AGGREGATOR_SUCCESS) {
        FATAL(mid,"aggregator_client_finalize failed (ret = %d)", ret);
    }

    hret = margo_addr_free(mid, svr_addr);
    if(hret != HG_SUCCESS) {
        FATAL(mid,"Could not free address (margo_addr_free returned %d)", hret);
    }

    margo_finalize(mid);

    return 0;
}
