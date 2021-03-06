/*
 * (C) 2020 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#include <stdio.h>
#include <margo.h>
#include <aggregator/aggregator-server.h>
#include <aggregator/aggregator-admin.h>
#include <aggregator/aggregator-client.h>
#include <aggregator/aggregator-stream.h>
#include "munit/munit.h"

struct test_context {
    margo_instance_id   mid;
    hg_addr_t           addr;
    aggregator_admin_t       admin;
    aggregator_stream_id_t id;
};

static const char* token = "ABCDEFGH";
static const uint16_t provider_id = 42;

static void* test_context_setup(const MunitParameter params[], void* user_data)
{
    (void) params;
    (void) user_data;
    aggregator_return_t      ret;
    margo_instance_id   mid;
    hg_addr_t           addr;
    aggregator_admin_t       admin;
    aggregator_stream_id_t id = 0;
    // create margo instance
    mid = margo_init("na+sm", MARGO_SERVER_MODE, 0, 0);
    munit_assert_not_null(mid);
    // get address of current process
    hg_return_t hret = margo_addr_self(mid, &addr);
    munit_assert_int(hret, ==, HG_SUCCESS);
    // register aggregator provider
    struct aggregator_provider_args args = AGGREGATOR_PROVIDER_ARGS_INIT;
    args.token = token;
    ret = aggregator_provider_register(
            mid, provider_id, &args,
            AGGREGATOR_PROVIDER_IGNORE);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // create an admin
    ret = aggregator_admin_init(mid, &admin);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // create test context
    struct test_context* context = (struct test_context*)calloc(1, sizeof(*context));
    munit_assert_not_null(context);
    context->mid   = mid;
    context->addr  = addr;
    context->admin = admin;
    context->id    = id;
    return context;
}

static void test_context_tear_down(void* fixture)
{
    aggregator_return_t ret;
    struct test_context* context = (struct test_context*)fixture;
    // free the admin
    ret = aggregator_admin_finalize(context->admin);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // free address
    margo_addr_free(context->mid, context->addr);
    // we are not checking the return value of the above function with
    // munit because we need margo_finalize to be called no matter what.
    margo_finalize(context->mid);
}

static MunitResult test_client(const MunitParameter params[], void* data)
{
    (void)params;
    (void)data;
    struct test_context* context = (struct test_context*)data;
    aggregator_client_t client;
    aggregator_return_t ret;
    // test that we can create a client object
    ret = aggregator_client_init(context->mid, &client);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // test that we can free the client object
    ret = aggregator_client_finalize(client);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);

    return MUNIT_OK;
}

static MunitResult test_stream(const MunitParameter params[], void* data)
{
    (void)params;
    (void)data;
    struct test_context* context = (struct test_context*)data;
    aggregator_client_t client;
    aggregator_stream_handle_t rh;
    aggregator_return_t ret;
    // test that we can create a client object
    ret = aggregator_client_init(context->mid, &client);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // test that we can create a stream handle
    ret = aggregator_remote_stream_handle_create(client,
            context->addr, provider_id, context->id, &rh);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // test that we can increase the ref count
    ret = aggregator_remote_stream_handle_ref_incr(rh);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // test that we can destroy the stream handle
    ret = aggregator_remote_stream_handle_release(rh);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // ... and a second time because of the increase ref 
    ret = aggregator_remote_stream_handle_release(rh);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // test that we can free the client object
    ret = aggregator_client_finalize(client);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);

    return MUNIT_OK;
}


static MunitResult test_hello(const MunitParameter params[], void* data)
{
    (void)params;
    (void)data;
    struct test_context* context = (struct test_context*)data;
    aggregator_client_t client;
    aggregator_stream_handle_t rh;
    aggregator_return_t ret;
    // test that we can create a client object
    ret = aggregator_client_init(context->mid, &client);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // test that we can create a stream handle
    ret = aggregator_remote_stream_handle_create(client,
            context->addr, provider_id, context->id, &rh);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // test that we can destroy the stream handle
    ret = aggregator_remote_stream_handle_release(rh);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // test that we can free the client object
    ret = aggregator_client_finalize(client);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);

    return MUNIT_OK;
}

static MunitResult test_sum(const MunitParameter params[], void* data)
{
    (void)params;
    (void)data;
    struct test_context* context = (struct test_context*)data;
    aggregator_client_t client;
    aggregator_stream_handle_t rh;
    aggregator_return_t ret;
    // test that we can create a client object
    ret = aggregator_client_init(context->mid, &client);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // test that we can create a stream handle
    ret = aggregator_remote_stream_handle_create(client,
            context->addr, provider_id, context->id, &rh);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // test that we can send a sum RPC to the stream
    // test that we can destroy the stream handle
    ret = aggregator_remote_stream_handle_release(rh);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // test that we can free the client object
    ret = aggregator_client_finalize(client);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);

    return MUNIT_OK;
}

static MunitResult test_invalid(const MunitParameter params[], void* data)
{
    (void)params;
    (void)data;
    struct test_context* context = (struct test_context*)data;
    aggregator_client_t client;
    aggregator_stream_handle_t rh1, rh2;
    aggregator_stream_id_t invalid_id = 0;
    aggregator_return_t ret;
    // test that we can create a client object
    ret = aggregator_client_init(context->mid, &client);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // create a stream handle for a wrong stream id
    ret = aggregator_remote_stream_handle_create(client,
            context->addr, provider_id, invalid_id, &rh1);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // create a stream handle for a wrong provider id
    ret = aggregator_remote_stream_handle_create(client,
            context->addr, provider_id + 1, context->id, &rh2);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // test sending to the invalid stream id
    ret = aggregator_remote_stream_handle_release(rh1);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // test that we can destroy the stream handle
    ret = aggregator_remote_stream_handle_release(rh2);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);
    // test that we can free the client object
    ret = aggregator_client_finalize(client);
    munit_assert_int(ret, ==, AGGREGATOR_SUCCESS);

    return MUNIT_OK;
}

static MunitTest test_suite_tests[] = {
    { (char*) "/client",   test_client,   test_context_setup, test_context_tear_down, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*) "/stream", test_stream, test_context_setup, test_context_tear_down, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*) "/hello",    test_hello,    test_context_setup, test_context_tear_down, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*) "/sum",      test_sum,      test_context_setup, test_context_tear_down, MUNIT_TEST_OPTION_NONE, NULL },
    { (char*) "/invalid",  test_invalid,  test_context_setup, test_context_tear_down, MUNIT_TEST_OPTION_NONE, NULL },
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = { 
    (char*) "/aggregator/admin", test_suite_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    return munit_suite_main(&test_suite, (void*) "aggregator", argc, argv);
}
