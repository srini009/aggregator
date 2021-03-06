/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __AGGREGATOR_COMMON_H
#define __AGGREGATOR_COMMON_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error codes that can be returned by AGGREGATOR functions.
 */
typedef enum aggregator_return_t {
    AGGREGATOR_SUCCESS,
    AGGREGATOR_ERR_INVALID_NAME,      /* Metric creation error - name or ns missing */
    AGGREGATOR_ERR_ALLOCATION,        /* Allocation error */
    AGGREGATOR_ERR_INVALID_ARGS,      /* Invalid argument */
    AGGREGATOR_ERR_INVALID_PROVIDER,  /* Invalid provider id */
    AGGREGATOR_ERR_INVALID_STREAM,    /* Invalid stream id */
    AGGREGATOR_ERR_INVALID_VALUE,     /* Invalid stream update value */
    AGGREGATOR_ERR_INVALID_BACKEND,   /* Invalid backend type */
    AGGREGATOR_ERR_INVALID_CONFIG,    /* Invalid configuration */
    AGGREGATOR_ERR_INVALID_TOKEN,     /* Invalid token */
    AGGREGATOR_ERR_FROM_MERCURY,      /* Mercury error */
    AGGREGATOR_ERR_FROM_ARGOBOTS,     /* Argobots error */
    AGGREGATOR_ERR_OP_UNSUPPORTED,    /* Unsupported operation */
    AGGREGATOR_ERR_OP_FORBIDDEN,      /* Forbidden operation */
    /* ... TODO add more error codes here if needed */
    AGGREGATOR_ERR_OTHER              /* Other error */
} aggregator_return_t;

#define STREAM_BUFFER_SIZE 160000000

/**
 * @brief Identifier for a stream.
 */
typedef uint32_t aggregator_stream_id_t;

typedef enum aggregator_stream_type {
   AGGREGATOR_TYPE_COUNTER,
   AGGREGATOR_TYPE_TIMER,
   AGGREGATOR_TYPE_GAUGE    
} aggregator_stream_type_t;

typedef struct aggregator_stream_sample {
   double time;
   double val;
   uint64_t sample_id;
} aggregator_stream_sample;

typedef aggregator_stream_sample* aggregator_stream_buffer;

typedef struct aggregator_taglist {
    char **taglist;
    int num_tags;
} aggregator_taglist;

typedef aggregator_taglist* aggregator_taglist_t;

inline uint32_t aggregator_hash(char *str);

inline void aggregator_id_from_string_identifiers(char *ns, char *name, char **taglist, int num_tags, aggregator_stream_id_t *id_);

/* djb2 hash from Dan Bernstein */
inline uint32_t
aggregator_hash(char *str)
{
    uint32_t hash = 5381;
    uint32_t c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}


inline void aggregator_id_from_string_identifiers(char *ns, char *name, char **taglist, int num_tags, aggregator_stream_id_t *id_)
{
    aggregator_stream_id_t id, temp_id;

    id = aggregator_hash(ns);
    temp_id = aggregator_hash(name);
    id = id^temp_id;

    /* XOR all the tag ids, so that any ordering of tags returns the same final stream id */
    int i;
    for(i = 0; i < num_tags; i++) {
	temp_id = aggregator_hash(taglist[i]);
	id = id^temp_id;
    }
   
    *id_ = id;
}

#ifdef __cplusplus
}
#endif

#endif
