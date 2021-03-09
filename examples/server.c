/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <assert.h>
#include <stdio.h>
#include <margo.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <mpi.h>
#include <aggregator/aggregator-server.h>
#include <aggregator/aggregator-stream.h>
#include <aggregator/aggregator-common.h>

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    MPI_Init(&argv, &argv);
    margo_instance_id mid = margo_init("na+sm://", MARGO_SERVER_MODE, 0, 0);
    assert(mid);

    hg_addr_t my_address;
    margo_addr_self(mid, &my_address);
    char addr_str[128];
    size_t addr_str_size = 128;
    margo_addr_to_string(mid, addr_str, &addr_str_size, my_address);
    margo_addr_free(mid, my_address);
    fprintf(stderr, "Server running at address %s, with provider id 42", addr_str);

    struct aggregator_provider_args args = AGGREGATOR_PROVIDER_ARGS_INIT;

    aggregator_provider_t provider;
    aggregator_provider_register(mid, 42, &args, &provider);

    margo_wait_for_finalize(mid);

    MPI_Finalize();

    return 0;
}
