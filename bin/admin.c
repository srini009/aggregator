/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <stdio.h>
#include <assert.h>
#include <margo.h>
#include <aggregator/aggregator-admin.h>

#define FATAL(...) \
    do { \
        margo_critical(__VA_ARGS__); \
        exit(-1); \
    } while(0)

int main(int argc, char** argv)
{
    if(argc != 3) {
        fprintf(stderr,"Usage: %s <server address> <provider id>\n", argv[0]);
        exit(0);
    }

    hg_return_t hret;
    aggregator_return_t ret;
    aggregator_admin_t admin;
    hg_addr_t svr_addr;
    const char* svr_addr_str = argv[1];

    margo_instance_id mid = margo_init("na+sm://", MARGO_CLIENT_MODE, 0, 0);
    assert(mid);

    hret = margo_addr_lookup(mid, svr_addr_str, &svr_addr);
    if(hret != HG_SUCCESS) {
        FATAL(mid,"margo_addr_lookup failed (ret = %d)", hret);
    }

    margo_info(mid,"Initializing admin");
    ret = aggregator_admin_init(mid, &admin);
    if(ret != AGGREGATOR_SUCCESS) {
        FATAL(mid,"aggregator_admin_init failed (ret = %d)", ret);
    }

    margo_info(mid,"Finalizing admin");
    ret = aggregator_admin_finalize(admin);
    if(ret != AGGREGATOR_SUCCESS) {
        FATAL(mid,"aggregator_admin_finalize failed (ret = %d)", ret);
    }

    hret = margo_addr_free(mid, svr_addr);
    if(hret != HG_SUCCESS) {
        FATAL(mid,"margo_addr_free failed (ret = %d)", ret);
    }

    margo_finalize(mid);

    return 0;
}
