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
    char addr[1024];
    size_t addr_str_size = 1024;
    margo_addr_to_string(mid, addr, &addr_str_size, my_address);

    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    unsigned j=0;
    while(addr[j] != '\0' && addr[j] != ':' && addr[j] != ';') j++;
    char * proto = addr + j;

    // Exchange addresses
    char * addresses_buf = (char*)malloc(1024*size);
    MPI_Gather(addr, 1024, MPI_BYTE, addresses_buf, 1024*size, MPI_BYTE, 0, MPI_COMM_WORLD);
 
    fprintf(stderr, "Server running at address %s, with provider id 42\n", addr);
    margo_addr_free(mid, my_address);
    //if(!rank) {
           //fprintf(stderr, "Server running at address %s, with provider id 42", addr_str);
           fprintf(stderr, "%s", addresses_buf);
    //}

    struct aggregator_provider_args args = AGGREGATOR_PROVIDER_ARGS_INIT;

    aggregator_provider_t provider;
    aggregator_provider_register(mid, 42, &args, &provider);

    margo_wait_for_finalize(mid);

    MPI_Finalize();

    return 0;
}
