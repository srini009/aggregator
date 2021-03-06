/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __AGGREGATOR_PROVIDER_HANDLE_H
#define __AGGREGATOR_PROVIDER_HANDLE_H

#include <margo.h>
#include <aggregator/aggregator-common.h>

#ifdef __cplusplus
extern "C" {
#endif

struct aggregator_provider_handle {
    margo_instance_id mid;
    hg_addr_t         addr;
    uint16_t          provider_id;
};

typedef struct aggregator_provider_handle* aggregator_provider_handle_t;
#define AGGREGATOR_PROVIDER_HANDLE_NULL ((aggregator_provider_handle_t)NULL)

#ifdef __cplusplus
}
#endif

#endif
