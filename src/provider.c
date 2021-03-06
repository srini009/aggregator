/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <assert.h>
#include "aggregator/aggregator-server.h"
#include "aggregator/aggregator-common.h"
#include "aggregator/aggregator-backend.h"
#include "provider.h"
#include "types.h"

static void aggregator_finalize_provider(void* p);

/* Functions to manipulate the hash of streams */

static inline aggregator_stream* find_stream(
        aggregator_provider_t provider,
        const aggregator_stream_id_t* id);

static inline aggregator_return_t add_stream(
        aggregator_provider_t provider,
        aggregator_stream* stream);

static inline aggregator_return_t remove_stream(
        aggregator_provider_t provider,
        const aggregator_stream_id_t* id);

static inline void remove_all_streams(
        aggregator_provider_t provider);

/* Admin RPCs */

/* Client RPCs */
static DECLARE_MARGO_RPC_HANDLER(aggregator_stream_fetch_ult)
static void aggregator_stream_fetch_ult(hg_handle_t h);
static DECLARE_MARGO_RPC_HANDLER(aggregator_list_streams_ult)
static void aggregator_list_streams_ult(hg_handle_t h);

/* add other RPC declarations here */

int aggregator_provider_register(
        margo_instance_id mid,
        uint16_t provider_id,
        const struct aggregator_provider_args* args,
        aggregator_provider_t* provider)
{
    struct aggregator_provider_args a = AGGREGATOR_PROVIDER_ARGS_INIT;
    if(args) a = *args;
    aggregator_provider_t p;
    hg_id_t id;
    hg_bool_t flag;

    margo_info(mid, "Registering AGGREGATOR provider with provider id %u", provider_id);

    flag = margo_is_listening(mid);
    if(flag == HG_FALSE) {
        margo_error(mid, "Margo instance is not a server");
        return AGGREGATOR_ERR_INVALID_ARGS;
    }

    margo_provider_registered_name(mid, "aggregator_remote_stream_fetch", provider_id, &id, &flag);
    if(flag == HG_TRUE) {
        margo_error(mid, "Provider with the same provider id (%u) already register", provider_id);
        return AGGREGATOR_ERR_INVALID_PROVIDER;
    }

    p = (aggregator_provider_t)calloc(1, sizeof(*p));
    if(p == NULL) {
        margo_error(mid, "Could not allocate memory for provider");
        return AGGREGATOR_ERR_ALLOCATION;
    }

    p->mid = mid;
    p->provider_id = provider_id;
    p->pool = a.pool;
    p->abtio = a.abtio;

    /* Admin RPCs */

    /* Client RPCs */
    id = MARGO_REGISTER_PROVIDER(mid, "aggregator_remote_stream_fetch",
            stream_fetch_in_t, stream_fetch_out_t,
            aggregator_stream_fetch_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->stream_fetch_id = id;

    id = MARGO_REGISTER_PROVIDER(mid, "aggregator_remote_list_streams",
            list_streams_in_t, list_streams_out_t,
            aggregator_list_streams_ult, provider_id, p->pool);
    margo_register_data(mid, id, (void*)p, NULL);
    p->list_streams_id = id;

    /* add other RPC registration here */
    /* ... */

    /* add backends available at compiler time (e.g. default/dummy backends) */

    if(a.push_finalize_callback)
        margo_provider_push_finalize_callback(mid, p, &aggregator_finalize_provider, p);

    if(provider)
        *provider = p;
    margo_info(mid, "AGGREGATOR provider registration done");
    return AGGREGATOR_SUCCESS;
}

static void aggregator_finalize_provider(void* p)
{
    aggregator_provider_t provider = (aggregator_provider_t)p;
    margo_info(provider->mid, "Finalizing AGGREGATOR provider");
    margo_deregister(provider->mid, provider->stream_fetch_id);
    margo_deregister(provider->mid, provider->list_streams_id);
    /* deregister other RPC ids ... */
    remove_all_streams(provider);
    free(provider);
    margo_info(provider->mid, "AGGREGATOR provider successfuly finalized");
}

int aggregator_provider_destroy(
        aggregator_provider_t provider)
{
    margo_instance_id mid = provider->mid;
    margo_info(mid, "Destroying AGGREGATOR provider");
    /* pop the finalize callback */
    margo_provider_pop_finalize_callback(provider->mid, provider);
    /* call the callback */
    aggregator_finalize_provider(provider);
    margo_info(mid, "AGGREGATOR provider successfuly destroyed");
    return AGGREGATOR_SUCCESS;
}

aggregator_return_t aggregator_provider_stream_create(const char *ns, const char *name, aggregator_stream_type_t t, const char *desc, aggregator_taglist_t tl, aggregator_stream_t* m, aggregator_provider_t provider)
{
    if(!ns || !name)
        return AGGREGATOR_ERR_INVALID_NAME;

    /* create an id for the new stream */
    aggregator_stream_id_t id;
    aggregator_id_from_string_identifiers(ns, name, tl->taglist, tl->num_tags, &id);

    /* allocate a stream, set it up, and add it to the provider */
    aggregator_stream* stream = (aggregator_stream*)calloc(1, sizeof(*stream));
    ABT_mutex_create(&stream->stream_mutex);
    stream->id  = id;
    strcpy(stream->name, name);
    strcpy(stream->ns, ns);
    strcpy(stream->desc, desc);
    stream->type = t;
    stream->taglist = tl;
    stream->buffer_index = 0;
    stream->buffer = (aggregator_stream_buffer)calloc(STREAM_BUFFER_SIZE, sizeof(aggregator_stream_sample));
    add_stream(provider, stream);

    fprintf(stderr, "\nCreated stream %d of type %d\n", id, stream->type);
    fprintf(stderr, "Num streams is: %lu\n", provider->num_streams);

    *m = stream;

    return AGGREGATOR_SUCCESS;
}

static void aggregator_stream_fetch_ult(hg_handle_t h)
{
    hg_return_t hret;
    stream_fetch_in_t  in;
    stream_fetch_out_t out;
    hg_bulk_t local_bulk;

    /* find the margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find the provider */
    const struct hg_info* info = margo_get_info(h);
    aggregator_provider_t provider = (aggregator_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_info(provider->mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = AGGREGATOR_ERR_FROM_MERCURY;
        goto finish;
    }

    /* create a bulk region */
    aggregator_stream_buffer b = calloc(in.count, sizeof(aggregator_stream_sample));
    hg_size_t buf_size = in.count * sizeof(aggregator_stream_sample);
    hret = margo_bulk_create(mid, 1, (void**)&b, &buf_size, HG_BULK_READ_ONLY, &local_bulk);

    if(hret != HG_SUCCESS) {
        margo_info(provider->mid, "Could not create bulk_handle (mercury error %d)", hret);
        out.ret = AGGREGATOR_ERR_FROM_MERCURY;
        goto finish;
    }

    aggregator_stream_id_t requested_id = in.stream_id;
    aggregator_stream* stream = find_stream(provider, &(requested_id));
    if(!stream) {
        out.ret = AGGREGATOR_ERR_INVALID_STREAM;
	goto finish;
    }


    out.name = (char*)malloc(36*sizeof(char));
    out.ns = (char*)malloc(36*sizeof(char));
    strcpy(out.name, stream->name);
    strcpy(out.ns, stream->ns);

    /* copyout stream buffer of requested size */
    if(stream->buffer_index < in.count) {
        out.actual_count = stream->buffer_index;
        memcpy(b, stream->buffer, out.actual_count*sizeof(aggregator_stream_sample));
    } else {
	out.actual_count = in.count;
        memcpy(b, stream->buffer + (stream->buffer_index - out.actual_count), out.actual_count*sizeof(aggregator_stream_sample));
    }

    /* do the bulk transfer */
    hret = margo_bulk_transfer(mid, HG_BULK_PUSH, info->addr, in.bulk, 0, local_bulk, 0, buf_size);
    if(hret != HG_SUCCESS) {
        margo_info(provider->mid, "Could not create bulk_handle (mercury error %d)", hret);
        out.ret = AGGREGATOR_ERR_FROM_MERCURY;
        goto finish;
    }

    /* set the response */
    out.ret = AGGREGATOR_SUCCESS;

finish:
    free(b);
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    margo_destroy(h);
    margo_bulk_free(local_bulk);
}
static DEFINE_MARGO_RPC_HANDLER(aggregator_stream_fetch_ult)

aggregator_return_t aggregator_provider_stream_destroy(aggregator_stream_t m, aggregator_provider_t provider)
{

    /* find the stream */
    aggregator_stream* stream = find_stream(provider, &m->id);
    if(!stream) {
        return AGGREGATOR_ERR_INVALID_STREAM;
    }

    /* remove the stream from the provider */
    remove_stream(provider, &stream->id);

    ABT_mutex_free(&stream->stream_mutex);
    free(stream->buffer);

    return AGGREGATOR_SUCCESS;
}

aggregator_return_t aggregator_provider_destroy_all_streams(aggregator_provider_t provider)
{

    remove_all_streams(provider);

    return AGGREGATOR_SUCCESS;
}

static void aggregator_list_streams_ult(hg_handle_t h)
{
    hg_return_t hret;
    list_streams_in_t  in;
    list_streams_out_t out;
    out.ids = NULL;

    /* find margo instance */
    margo_instance_id mid = margo_hg_handle_get_instance(h);

    /* find provider */
    const struct hg_info* info = margo_get_info(h);
    aggregator_provider_t provider = (aggregator_provider_t)margo_registered_data(mid, info->id);

    /* deserialize the input */
    hret = margo_get_input(h, &in);
    if(hret != HG_SUCCESS) {
        margo_error(mid, "Could not deserialize output (mercury error %d)", hret);
        out.ret = AGGREGATOR_ERR_FROM_MERCURY;
        goto finish;
    }

    /* allocate array of stream ids */
    out.ret   = AGGREGATOR_SUCCESS;
    out.count = provider->num_streams < in.max_ids ? provider->num_streams : in.max_ids;
    out.ids   = (aggregator_stream_id_t*)calloc(provider->num_streams, sizeof(*out.ids));

    /* iterate over the hash of streams to fill the array of stream ids */
    unsigned i = 0;
    aggregator_stream *r, *tmp;
    HASH_ITER(hh, provider->streams, r, tmp) {
        out.ids[i++] = r->id;
    }

    margo_debug(mid, "Listed streams");

finish:
    hret = margo_respond(h, &out);
    hret = margo_free_input(h, &in);
    free(out.ids);
    margo_destroy(h);
}
static DEFINE_MARGO_RPC_HANDLER(aggregator_list_streams_ult)

aggregator_return_t aggregator_provider_register_backend()
{
    return AGGREGATOR_SUCCESS;
}

static inline aggregator_stream* find_stream(
        aggregator_provider_t provider,
        const aggregator_stream_id_t* id)
{
    aggregator_stream* stream = NULL;

    HASH_FIND(hh, provider->streams, id, sizeof(aggregator_stream_id_t), stream);
    return stream;
}

static inline aggregator_return_t add_stream(
        aggregator_provider_t provider,
        aggregator_stream* stream)
{

    aggregator_stream* existing = find_stream(provider, &(stream->id));
    if(existing) {
        return AGGREGATOR_ERR_INVALID_STREAM;
    }
    HASH_ADD(hh, provider->streams, id, sizeof(aggregator_stream_id_t), stream);
    provider->num_streams += 1;

    return AGGREGATOR_SUCCESS;
}

static inline aggregator_return_t remove_stream(
        aggregator_provider_t provider,
        const aggregator_stream_id_t* id)
{
    aggregator_stream* stream = find_stream(provider, id);
    if(!stream) {
        return AGGREGATOR_ERR_INVALID_STREAM;
    }
    aggregator_return_t ret = AGGREGATOR_SUCCESS;
    HASH_DEL(provider->streams, stream);
    free(stream);
    provider->num_streams -= 1;
    return ret;
}

static inline void remove_all_streams(
        aggregator_provider_t provider)
{
    aggregator_stream *r, *tmp;
    HASH_ITER(hh, provider->streams, r, tmp) {
        HASH_DEL(provider->streams, r);
        free(r);
    }
    provider->num_streams = 0;
}
