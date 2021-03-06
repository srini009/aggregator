/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __AGGREGATOR_CLIENT_H
#define __AGGREGATOR_CLIENT_H

#include <margo.h>
#include <aggregator/aggregator-common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct aggregator_client* aggregator_client_t;
#define AGGREGATOR_CLIENT_NULL ((aggregator_client_t)NULL)

/**
 * @brief Creates a AGGREGATOR client.
 *
 * @param[in] mid Margo instance
 * @param[out] client AGGREGATOR client
 *
 * @return AGGREGATOR_SUCCESS or error code defined in aggregator-common.h
 */
aggregator_return_t aggregator_client_init(margo_instance_id mid, aggregator_client_t* client);

/**
 * @brief Finalizes a AGGREGATOR client.
 *
 * @param[in] client AGGREGATOR client to finalize
 *
 * @return AGGREGATOR_SUCCESS or error code defined in aggregator-common.h
 */
aggregator_return_t aggregator_client_finalize(aggregator_client_t client);

#ifdef __cplusplus
}
#endif

#endif
