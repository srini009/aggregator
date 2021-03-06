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
#include <aggregator/aggregator-server.h>
#include <aggregator/aggregator-stream.h>
#include <aggregator/aggregator-common.h>

aggregator_stream_t m, m2, m3;

void stream_update(int signum)
{  
    static int i = 0;
    aggregator_stream_update(m2, 9+i*1.1);
    signal(signum, stream_update);
    alarm(1);
    i++;
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;
    margo_instance_id mid = margo_init("na+sm://", MARGO_SERVER_MODE, 0, 0);
    assert(mid);

    hg_addr_t my_address;
    margo_addr_self(mid, &my_address);
    char addr_str[128];
    size_t addr_str_size = 128;
    margo_addr_to_string(mid, addr_str, &addr_str_size, my_address);
    margo_addr_free(mid,my_address);
    fprintf(stderr, "Server running at address %s, with provider id 42", addr_str);

    struct aggregator_provider_args args = AGGREGATOR_PROVIDER_ARGS_INIT;

    aggregator_provider_t provider;
    aggregator_provider_register(mid, 42, &args, &provider);

    aggregator_taglist_t taglist, taglist2, taglist3;

    aggregator_taglist_create(&taglist, 5, "tag1", "tag2", "tag3", "tag4", "tag5");
    aggregator_stream_create("srini", "teststream", AGGREGATOR_TYPE_COUNTER, "My first stream", taglist, &m, provider);


    aggregator_taglist_create(&taglist2, 3, "tag1", "tag2", "tag3");
    aggregator_stream_create("srini", "teststream2", AGGREGATOR_TYPE_COUNTER, "My second stream", taglist2, &m2, provider);

    aggregator_taglist_create(&taglist3, 0);
    aggregator_stream_create("srini", "teststream", AGGREGATOR_TYPE_COUNTER, "My third stream", taglist3, &m3, provider);


    signal(SIGALRM, stream_update);
    alarm(2);
    /*aggregator_stream_destroy(m, provider);

    aggregator_taglist_destroy(taglist);*/

    margo_wait_for_finalize(mid);

    return 0;
}
