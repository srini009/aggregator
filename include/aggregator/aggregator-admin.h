/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#ifndef __AGGREGATOR_ADMIN_H
#define __AGGREGATOR_ADMIN_H

#include <margo.h>
#include <aggregator/aggregator-common.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct aggregator_admin* aggregator_admin_t;
#define AGGREGATOR_ADMIN_NULL ((aggregator_admin_t)NULL)

#define AGGREGATOR_STREAM_ID_IGNORE ((aggregator_stream_id_t*)NULL)

/**
 * @brief Creates a AGGREGATOR admin.
 *
 * @param[in] mid Margo instance
 * @param[out] admin AGGREGATOR admin
 *
 * @return AGGREGATOR_SUCCESS or error code defined in aggregator-common.h
 */
aggregator_return_t aggregator_admin_init(margo_instance_id mid, aggregator_admin_t* admin);

/**
 * @brief Finalizes a AGGREGATOR admin.
 *
 * @param[in] admin AGGREGATOR admin to finalize
 *
 * @return AGGREGATOR_SUCCESS or error code defined in aggregator-common.h
 */
aggregator_return_t aggregator_admin_finalize(aggregator_admin_t admin);

#endif
