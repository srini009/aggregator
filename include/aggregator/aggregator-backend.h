/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __AGGREGATOR_BACKEND_H
#define __AGGREGATOR_BACKEND_H

#include <aggregator/aggregator-server.h>
#include <aggregator/aggregator-common.h>

typedef aggregator_return_t (*aggregator_backend_create_fn)(aggregator_provider_t, void**);
typedef aggregator_return_t (*aggregator_backend_open_fn)(aggregator_provider_t, void**);
typedef aggregator_return_t (*aggregator_backend_close_fn)(void*);
typedef aggregator_return_t (*aggregator_backend_destroy_fn)(void*);

/**
 * @brief Implementation of an AGGREGATOR backend.
 */
typedef struct aggregator_backend_impl {
    // backend name
    const char* name;
    // backend management functions
    aggregator_backend_create_fn   create_stream;
    aggregator_backend_open_fn     open_stream;
    aggregator_backend_close_fn    close_stream;
    aggregator_backend_destroy_fn  destroy_stream;
    // RPC functions
    void (*hello)(void*);
    int32_t (*sum)(void*, int32_t, int32_t);
    // ... add other functions here
} aggregator_backend_impl;

/**
 * @brief Registers a backend implementation to be used by the
 * specified provider.
 *
 * Note: the backend implementation will not be copied; it is
 * therefore important that it stays valid in memory until the
 * provider is destroyed.
 *
 * @param provider provider.
 * @param backend_impl backend implementation.
 *
 * @return AGGREGATOR_SUCCESS or error code defined in aggregator-common.h 
 */
aggregator_return_t aggregator_provider_register_backend();

#endif
