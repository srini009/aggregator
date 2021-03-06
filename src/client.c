/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include <stdarg.h>
#include "types.h"
#include "client.h"
#include "provider.h"
#include "aggregator/aggregator-client.h"
#include "aggregator/aggregator-common.h"

aggregator_return_t aggregator_client_init(margo_instance_id mid, aggregator_client_t* client)
{
    aggregator_client_t c = (aggregator_client_t)calloc(1, sizeof(*c));
    if(!c) return AGGREGATOR_ERR_ALLOCATION;

    c->mid = mid;

    hg_bool_t flag;
    hg_id_t id;
    margo_registered_name(mid, "aggregator_remote_stream_fetch", &id, &flag);

    if(flag == HG_TRUE) {
        margo_registered_name(mid, "aggregator_remote_stream_fetch", &c->stream_fetch_id, &flag);
        margo_registered_name(mid, "aggregator_remote_list_streams", &c->list_streams_id, &flag);
    } else {
        c->stream_fetch_id = MARGO_REGISTER(mid, "aggregator_remote_stream_fetch", stream_fetch_in_t, stream_fetch_out_t, NULL);
        c->list_streams_id = MARGO_REGISTER(mid, "aggregator_remote_list_streams", list_streams_in_t, list_streams_out_t, NULL);
    }

    c->num_stream_handles = 0;
    *client = c;
    return AGGREGATOR_SUCCESS;
}

aggregator_return_t aggregator_client_finalize(aggregator_client_t client)
{
    if(client->num_stream_handles != 0) {
        fprintf(stderr,  
                "Warning: %ld stream handles not released when aggregator_client_finalize was called\n",
                client->num_stream_handles);
    }
    free(client);
    return AGGREGATOR_SUCCESS;
}

/* APIs for microservice clients */
aggregator_return_t aggregator_taglist_create(aggregator_taglist_t *taglist, int num_tags, ...) 
{
    *taglist = (aggregator_taglist_t)malloc(sizeof(aggregator_taglist));
    va_list valist;
    va_start(valist, num_tags);

    (*taglist)->taglist = (char **)malloc(num_tags*sizeof(char*));
    (*taglist)->num_tags = num_tags;
    int i = 0;

    for(i = 0; i < num_tags; i++) {
        (*taglist)->taglist[i] = (char*)malloc(36*sizeof(char*));
	strcpy((*taglist)->taglist[i], va_arg(valist, char*));
    }

    va_end(valist);
    return AGGREGATOR_SUCCESS;

}

aggregator_return_t aggregator_taglist_destroy(aggregator_taglist_t taglist)
{
    int i;
    for(i = 0; i < taglist->num_tags; i++) {
        free(taglist->taglist[i]);
    }

    free(taglist->taglist);
    free(taglist);

    return AGGREGATOR_SUCCESS;

}

aggregator_return_t aggregator_stream_create(const char *ns, const char *name, aggregator_stream_type_t t, const char *desc, aggregator_taglist_t taglist, aggregator_stream_t* m, aggregator_provider_t p)
{
    return aggregator_provider_stream_create(ns, name, t, desc, taglist, m, p);
}

aggregator_return_t aggregator_stream_destroy(aggregator_stream_t m, aggregator_provider_t p)
{
    return aggregator_provider_stream_destroy(m, p);
}

aggregator_return_t aggregator_stream_destroy_all(aggregator_provider_t p)
{
    return aggregator_provider_destroy_all_streams(p);
}

aggregator_return_t aggregator_stream_update(aggregator_stream_t m, double val)
{
    switch(m->type) {
        case AGGREGATOR_TYPE_COUNTER:
          if((m->buffer_index >=1) && (m->buffer[m->buffer_index-1].val > val)) 
              return AGGREGATOR_ERR_INVALID_VALUE;
          break;
        case AGGREGATOR_TYPE_TIMER:
          if(val < 0)
              return AGGREGATOR_ERR_INVALID_VALUE;
          break;
        case AGGREGATOR_TYPE_GAUGE:
          break;
    }

    ABT_unit_id self_id;
  
    ABT_mutex_lock(m->stream_mutex);
    ABT_self_get_thread_id(&self_id);
        
    m->buffer[m->buffer_index].val = val;
    m->buffer[m->buffer_index].time = ABT_get_wtime();
    m->buffer[m->buffer_index].sample_id = self_id;
    m->buffer_index++;

    ABT_mutex_unlock(m->stream_mutex);

    return AGGREGATOR_SUCCESS;
}

aggregator_return_t aggregator_stream_update_gauge_by_fixed_amount(aggregator_stream_t m, double diff)
{
    switch(m->type) {
        case AGGREGATOR_TYPE_COUNTER:
        case AGGREGATOR_TYPE_TIMER:
             return AGGREGATOR_ERR_INVALID_VALUE;
    }

    ABT_unit_id self_id;

    ABT_mutex_lock(m->stream_mutex);
    ABT_self_get_thread_id(&self_id);

    if(m->buffer_index) {
        m->buffer[m->buffer_index].val = m->buffer[m->buffer_index - 1].val + diff;
    } else {
        m->buffer[m->buffer_index].val = 1;
    }

    m->buffer[m->buffer_index].time = ABT_get_wtime();
    m->buffer[m->buffer_index].sample_id = self_id;
    m->buffer_index++;

unlock:
    ABT_mutex_unlock(m->stream_mutex);

    return AGGREGATOR_SUCCESS;
}

aggregator_return_t aggregator_stream_register_retrieval_callback(char *ns, func f)
{
    fprintf(stderr, "Callback function for namespace: %s is not yet implmented\n", ns);

    return AGGREGATOR_SUCCESS;
}

aggregator_return_t aggregator_stream_dump_histogram(aggregator_stream_t m, const char *filename, size_t num_buckets)
{
    double max = 0;
    double min = 9999999999999;

    fprintf(stderr, "Invoked dump histogram\n");
    int i = 0; 
    size_t *buckets = (size_t*)calloc(num_buckets, sizeof(size_t));
    for(i = 0 ; i < m->buffer_index; i++) {
        if(m->buffer[i].val > max)
            max = m->buffer[i].val;
	if(m->buffer[i].val < min)
	    min = m->buffer[i].val;
    }

    int bucket_index;
    for(i = 0 ; i < m->buffer_index; i++) {
        bucket_index = (int)(((m->buffer[i].val - min)/(max - min))*num_buckets);
        buckets[bucket_index]++;
    }

    FILE *fp = fopen(filename, "w");
    fprintf(fp, "%lu, %lf, %lf\n", num_buckets, min, max);
    for(i = 0; i < num_buckets; i++)
        fprintf(fp, "%lu\n", buckets[i]);
    fclose(fp);
    free(buckets);

}

aggregator_return_t aggregator_stream_dump_raw_data(aggregator_stream_t m, const char *filename)
{

    FILE *fp = fopen(filename, "w");
    int i;
    for(i = 0; i < m->buffer_index; i++)
        fprintf(fp, "%.9lf, %.9lf, %lu\n", m->buffer[i].val, m->buffer[i].time, m->buffer[i].sample_id);
    fclose(fp);
}

/* APIs for remote monitoring clients */

aggregator_return_t aggregator_remote_stream_get_id(char *ns, char *name, aggregator_taglist_t taglist, aggregator_stream_id_t* stream_id)
{
    if(!ns || !name)
        return AGGREGATOR_ERR_INVALID_NAME;

    aggregator_id_from_string_identifiers(ns, name, taglist->taglist, taglist->num_tags, stream_id);

    return AGGREGATOR_SUCCESS;
}

aggregator_return_t aggregator_remote_stream_fetch(aggregator_stream_handle_t handle, int64_t *num_samples_requested, aggregator_stream_buffer *buf, char **name, char **ns)
{
    hg_handle_t h;
    stream_fetch_in_t in;
    stream_fetch_out_t out;
    hg_bulk_t local_bulk;

    hg_return_t ret;
    in.stream_id = handle->stream_id;

    if(*num_samples_requested >= STREAM_BUFFER_SIZE || *num_samples_requested < 0)
        *num_samples_requested = STREAM_BUFFER_SIZE;

    in.count = *num_samples_requested;

    aggregator_stream_buffer b = (aggregator_stream_buffer)calloc(*num_samples_requested, sizeof(aggregator_stream_sample));
    hg_size_t segment_sizes[1] = {*num_samples_requested*sizeof(aggregator_stream_sample)};
    void *segment_ptrs[1] = {(void*)b};

    margo_bulk_create(handle->client->mid, 1,  segment_ptrs, segment_sizes, HG_BULK_WRITE_ONLY, &local_bulk);
    in.bulk = local_bulk;

    ret = margo_create(handle->client->mid, handle->addr, handle->client->stream_fetch_id, &h);
    if(ret != HG_SUCCESS)         
        return AGGREGATOR_ERR_FROM_MERCURY; 

    ret = margo_provider_forward(handle->provider_id, h, &in);
    if(ret != HG_SUCCESS) {
        margo_destroy(h);
	return AGGREGATOR_ERR_FROM_MERCURY;
    }

    ret = margo_get_output(h, &out);
    if(ret != HG_SUCCESS) {
	margo_free_output(h, &out);
        margo_destroy(h);
	return AGGREGATOR_ERR_FROM_MERCURY;
    }

    *num_samples_requested = out.actual_count;
    *buf = b;
    *name = (char*)malloc(36*sizeof(char));
    *ns = (char*)malloc(36*sizeof(char));
    strcpy(*name, out.name);
    strcpy(*ns, out.ns);

    margo_free_output(h, &out);
    margo_destroy(h);
    margo_bulk_free(local_bulk);

    return out.ret;
}

aggregator_return_t aggregator_remote_stream_handle_create(
        aggregator_client_t client,
        hg_addr_t addr,
        uint16_t provider_id,
        aggregator_stream_id_t stream_id,
        aggregator_stream_handle_t* handle)
{
    if(client == AGGREGATOR_CLIENT_NULL)
        return AGGREGATOR_ERR_INVALID_ARGS;

    aggregator_stream_handle_t rh =
        (aggregator_stream_handle_t)calloc(1, sizeof(*rh));

    if(!rh) return AGGREGATOR_ERR_ALLOCATION;

    hg_return_t ret = margo_addr_dup(client->mid, addr, &(rh->addr));
    if(ret != HG_SUCCESS) {
        free(rh);
        return AGGREGATOR_ERR_FROM_MERCURY;
    }

    rh->client      = client;
    rh->provider_id = provider_id;
    rh->stream_id   = stream_id;
    rh->refcount    = 1;

    client->num_stream_handles += 1;

    *handle = rh;
    return AGGREGATOR_SUCCESS;
}

aggregator_return_t aggregator_remote_stream_handle_ref_incr(
        aggregator_stream_handle_t handle)
{
    if(handle == AGGREGATOR_STREAM_HANDLE_NULL)
        return AGGREGATOR_ERR_INVALID_ARGS;
    handle->refcount += 1;
    return AGGREGATOR_SUCCESS;
}

aggregator_return_t aggregator_remote_stream_handle_release(aggregator_stream_handle_t handle)
{
    if(handle == AGGREGATOR_STREAM_HANDLE_NULL)
        return AGGREGATOR_ERR_INVALID_ARGS;
    handle->refcount -= 1;
    if(handle->refcount == 0) {
        margo_addr_free(handle->client->mid, handle->addr);
        handle->client->num_stream_handles -= 1;
        free(handle);
    }
    return AGGREGATOR_SUCCESS;
}

aggregator_return_t aggregator_remote_list_streams(aggregator_client_t client, hg_addr_t addr, uint16_t provider_id, aggregator_stream_id_t** ids, size_t* count)
{
    hg_handle_t h;
    list_streams_in_t  in;
    list_streams_out_t out;
    aggregator_return_t ret;
    hg_return_t hret;

    in.max_ids = *count;

    hret = margo_create(client->mid, addr, client->list_streams_id, &h);
    if(hret != HG_SUCCESS)
        return AGGREGATOR_ERR_FROM_MERCURY;

    hret = margo_provider_forward(provider_id, h, &in);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return AGGREGATOR_ERR_FROM_MERCURY;
    }

    hret = margo_get_output(h, &out);
    if(hret != HG_SUCCESS) {
        margo_destroy(h);
        return AGGREGATOR_ERR_FROM_MERCURY;
    }

    ret = out.ret;
    if(ret == AGGREGATOR_SUCCESS) {
        *count = out.count;
        memcpy(*ids, out.ids, out.count*sizeof(aggregator_stream_id_t));
    }
    
    margo_free_output(h, &out);
    margo_destroy(h);
    return ret;
}
