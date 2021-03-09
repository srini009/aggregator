/*
 * (C) 2020 The University of Chicago
 * 
 * See COPYRIGHT in top-level directory.
 */
#include "types.h"
#include "admin.h"
#include "aggregator/aggregator-admin.h"

aggregator_return_t aggregator_admin_init(margo_instance_id mid, aggregator_admin_t* admin)
{
    aggregator_admin_t a = (aggregator_admin_t)calloc(1, sizeof(*a));
    if(!a) return AGGREGATOR_ERR_ALLOCATION;

    a->mid = mid;

    *admin = a;
    return AGGREGATOR_SUCCESS;
}

aggregator_return_t aggregator_admin_finalize(aggregator_admin_t admin)
{
    free(admin);
    return AGGREGATOR_SUCCESS;
}
