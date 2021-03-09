/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <bedrock/module.h>
#include "aggregator/aggregator-server.h"
#include "aggregator/aggregator-client.h"
#include "aggregator/aggregator-admin.h"
#include "aggregator/aggregator-provider-handle.h"
#include "client.h"
#include <string.h>

static int aggregator_register_provider(
        bedrock_args_t args,
        bedrock_module_provider_t* provider)
{
    margo_instance_id mid = bedrock_args_get_margo_instance(args);
    uint16_t provider_id  = bedrock_args_get_provider_id(args);

    struct aggregator_provider_args aggregator_args = { 0 };
    aggregator_args.config = bedrock_args_get_config(args);
    aggregator_args.pool   = bedrock_args_get_pool(args);

    aggregator_args.abtio = (abt_io_instance_id)
        bedrock_args_get_dependency(args, "abt_io", 0);

    return aggregator_provider_register(mid, provider_id, &aggregator_args,
                                   (aggregator_provider_t*)provider);
}

static int aggregator_deregister_provider(
        bedrock_module_provider_t provider)
{
    return aggregator_provider_destroy((aggregator_provider_t)provider);
}

static char* aggregator_get_provider_config(
        bedrock_module_provider_t provider) {
    (void)provider;
    // TODO
    return strdup("{}");
}

static int aggregator_init_client(
        margo_instance_id mid,
        bedrock_module_client_t* client)
{
    return aggregator_client_init(mid, (aggregator_client_t*)client);
}

static int aggregator_finalize_client(
        bedrock_module_client_t client)
{
    return aggregator_client_finalize((aggregator_client_t)client);
}

static int aggregator_create_provider_handle(
        bedrock_module_client_t client,
        hg_addr_t address,
        uint16_t provider_id,
        bedrock_module_provider_handle_t* ph)
{
    aggregator_client_t c = (aggregator_client_t)client;
    aggregator_provider_handle_t tmp = calloc(1, sizeof(*tmp));
    margo_addr_dup(c->mid, address, &(tmp->addr));
    tmp->provider_id = provider_id;
    *ph = (bedrock_module_provider_handle_t)tmp;
    return BEDROCK_SUCCESS;
}

static int aggregator_destroy_provider_handle(
        bedrock_module_provider_handle_t ph)
{
    aggregator_provider_handle_t tmp = (aggregator_provider_handle_t)ph;
    margo_addr_free(tmp->mid, tmp->addr);
    free(tmp);
    return BEDROCK_SUCCESS;
}

static struct bedrock_module aggregator = {
    .register_provider       = aggregator_register_provider,
    .deregister_provider     = aggregator_deregister_provider,
    .get_provider_config     = aggregator_get_provider_config,
    .init_client             = aggregator_init_client,
    .finalize_client         = aggregator_finalize_client,
    .create_provider_handle  = aggregator_create_provider_handle,
    .destroy_provider_handle = aggregator_destroy_provider_handle,
    .dependencies            = NULL
};

BEDROCK_REGISTER_MODULE(aggregator, aggregator)
