/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <string.h>
#include "aggregator/aggregator-backend.h"
#include "../provider.h"
#include "dummy-backend.h"

typedef struct dummy_context {
  int dummy_member;
    /* ... */
} dummy_context;

static aggregator_return_t dummy_create_stream(
        aggregator_provider_t provider,
        void** context)
{
    (void)provider;

    dummy_context* ctx = (dummy_context*)calloc(1, sizeof(*ctx));
    *context = (void*)ctx;
    return AGGREGATOR_SUCCESS;
}

static aggregator_return_t dummy_open_stream(
        aggregator_provider_t provider,
        void** context)
{
    (void)provider;

    dummy_context* ctx = (dummy_context*)calloc(1, sizeof(*ctx));
    *context = (void*)ctx;
    return AGGREGATOR_SUCCESS;
}

static aggregator_return_t dummy_close_stream(void* ctx)
{
    dummy_context* context = (dummy_context*)ctx;
    free(context);
    return AGGREGATOR_SUCCESS;
}

static aggregator_return_t dummy_destroy_stream(void* ctx)
{
    dummy_context* context = (dummy_context*)ctx;
    free(context);
    return AGGREGATOR_SUCCESS;
}

static void dummy_say_hello(void* ctx)
{
    dummy_context* context = (dummy_context*)ctx;
    (void)context;
    printf("Hello World from Dummy stream\n");
}

static int32_t dummy_compute_sum(void* ctx, int32_t x, int32_t y)
{
    (void)ctx;
    return x+y;
}

static aggregator_backend_impl dummy_backend = {
    .name             = "dummy",

    .create_stream  = dummy_create_stream,
    .open_stream    = dummy_open_stream,
    .close_stream   = dummy_close_stream,
    .destroy_stream = dummy_destroy_stream,

    .hello            = dummy_say_hello,
    .sum              = dummy_compute_sum
};

aggregator_return_t aggregator_provider_register_dummy_backend(aggregator_provider_t provider)
{
    return aggregator_provider_register_backend(provider, &dummy_backend);
}
