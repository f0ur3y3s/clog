#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "../include/clog.h"
#include "Unity/src/unity.h"

void setUp (void)
{
}

void tearDown (void)
{
}

void test_logger_init ()
{
    logger_init("out.txt", L_DEBUG);
    logger_destroy();
    remove("out.txt");
}

void test_logger_log ()
{
    logger_init("out.txt", L_CRIT);
    clog(L_DEBUG, "test %s", "testing debug");
    clog(L_INFO, "test %s", "testing info");
    clog(L_WARNING, "test %s", "testing warning");
    clog(L_ERR, "test %s", "testing error");
    FILE * p_file = fopen("out.txt", "r");
    TEST_ASSERT_NOT_NULL(p_file);

    char buffer[1024] = { 0 };
    memset(buffer, 0, 1024);

    if (NULL != fgets(buffer, 1024, p_file))
    {
        TEST_ASSERT_NOT_NULL(strstr(buffer, "test testing debug"));
        memset(buffer, 0, 1024);
    }

    if (NULL != fgets(buffer, 1024, p_file))
    {
        TEST_ASSERT_NOT_NULL(strstr(buffer, "test testing info"));
        memset(buffer, 0, 1024);
    }

    if (NULL != fgets(buffer, 1024, p_file))
    {
        TEST_ASSERT_NOT_NULL(strstr(buffer, "test testing warning"));
        memset(buffer, 0, 1024);
    }

    if (NULL != fgets(buffer, 1024, p_file))
    {
        TEST_ASSERT_NOT_NULL(strstr(buffer, "test testing error"));
        memset(buffer, 0, 1024);
    }

    fclose(p_file);
    logger_destroy();
    remove("out.txt");
}

void test_logger_level ()
{
    logger_init("out.txt", L_EMERG);
    clog(L_DEBUG, "test %s", "testing debug");
    clog(L_INFO, "test %s", "testing info");
    clog(L_WARNING, "test %s", "testing warning");
    clog(L_ERR, "test %s", "testing error");
    FILE * p_file = fopen("out.txt", "r");
    TEST_ASSERT_NOT_NULL(p_file);

    char buffer[1024] = { 0 };
    memset(buffer, 0, 1024);

    if (NULL != fgets(buffer, 1024, p_file))
    {
        TEST_ASSERT_NOT_NULL(strstr(buffer, "test testing debug"));
        memset(buffer, 0, 1024);
    }

    TEST_ASSERT_NULL(fgets(buffer, 1024, p_file));

    fclose(p_file);
    logger_destroy();
    remove("out.txt");
}

typedef struct thread_args
{
    int thread_num;
} thread_args;

void * threaded_add (void * p_args)
{
    thread_args * p_thread_args = (thread_args *)p_args;
    clog(L_DEBUG, "Thread %d", p_thread_args->thread_num);
    free(p_thread_args);
    return NULL;
}

void test_logger_threaded ()
{
    logger_init("out.txt", L_DEBUG);
    pthread_t threads[10];

    for (int i = 0; i < 10; i++)
    {
        thread_args * p_args = malloc(sizeof(thread_args));
        p_args->thread_num   = i;
        pthread_create(&threads[i], NULL, threaded_add, p_args);
    }

    for (int i = 0; i < 10; i++)
    {
        pthread_join(threads[i], NULL);
    }

    logger_destroy();

    FILE * p_file = fopen("out.txt", "r");
    TEST_ASSERT_NOT_NULL(p_file);
    char buffer[1024] = { 0 };
    int  line_count   = 0;

    while (NULL != fgets(buffer, 1024, p_file))
    {
        line_count++;
    }

    TEST_ASSERT_EQUAL_INT(10, line_count);
    fclose(p_file);
}

void test_logger_null_filename ()
{
    int status = logger_init(NULL, L_DEBUG);
    TEST_ASSERT_EQUAL_INT(LOG_OK, status);
    clog(L_DEBUG, "test %s", "testing debug");
    logger_destroy();
}

void test_logger_bad_filename ()
{
    int status = logger_init("stdin", L_DEBUG);
    TEST_ASSERT_EQUAL_INT(LOG_NULL, status);
    clog(L_DEBUG, "test %s", "testing debug");
    logger_destroy();
}

int main (void)
{
    UNITY_BEGIN();
    RUN_TEST(test_logger_init);
    RUN_TEST(test_logger_log);
    RUN_TEST(test_logger_null_filename);
    RUN_TEST(test_logger_bad_filename);
    RUN_TEST(test_logger_level);
    RUN_TEST(test_logger_threaded);
    remove("out.txt");
    return UNITY_END();
}