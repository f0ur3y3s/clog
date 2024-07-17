/**
 * @file clog.c
 * @author f0ur3y3s
 * @brief A threadsafe logging library that writes to STDOUT or a file
 * @version 0.1
 * @date 2024-07-17
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "../include/clog.h"

/**
 * @brief Global logger initialization
 *
 * @note This logger is initialized globally to allow calls to `log` without
 * having to pass in a pointer to a logger struct.
 * @note Any file that includes logger.h will have access to this global logger.
 *
 */
logger_t g_logger = { 0 };

int logger_init (char * p_filename, const log_level_t log_level)
{
    int status = LOG_GEN;

    if ((L_EMERG > log_level) || (L_DEBUG < log_level))
    {
        (void)fprintf(stderr, "[LOG] Invalid log level\n");
        status = LOG_GEN;
        goto EXIT;
    }

    if (0 != pthread_mutex_init(&(g_logger.log_mutex), NULL))
    {
        (void)fprintf(stderr, "[LOG] Could not initialize mutex\n");
        status = LOG_MUTEX;
        goto EXIT;
    }

    g_logger.log_level = log_level;

    if (NULL == p_filename)
    {
        (void)fprintf(stdout,
                      "[LOG] No filename provided, defaulting to STDOUT\n");
        g_logger.p_filename = NULL;
        g_logger.p_file     = stdout;
        status              = LOG_OK;
        goto EXIT;
    }

    // Check for invalid filenames (stdin, stdout, stderr)
    if ((NULL != strstr(p_filename, "stdin"))
        || (NULL != strstr(p_filename, "stdout"))
        || (NULL != strstr(p_filename, "stderr")))
    {
        (void)fprintf(stderr, "[LOG] Invalid filename, defaulting to STDOUT\n");
        g_logger.p_filename = NULL;
        g_logger.p_file     = stdout;
        status              = LOG_NULL;
        goto EXIT;
    }

    FILE * p_fp_file = fopen(p_filename, "w");

    if (NULL == p_fp_file)
    {
        (void)fprintf(stderr,
                      "[LOG] Could not set logger to write to file, defaulting "
                      "to STDOUT\n");
        g_logger.p_filename = NULL;
        g_logger.p_file     = stdout;
        status              = LOG_NULL;
        goto EXIT;
    }

    struct flock lock = { 0 };

    lock.l_type   = F_WRLCK;
    lock.l_start  = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len    = 0;

    if (fcntl(fileno(p_fp_file), F_SETLK, &lock) == -1)
    {
        (void)fprintf(stderr, "[LOG] Could not lock file\n");
        goto EXIT;
    }

    g_logger.p_filename = p_filename;
    g_logger.p_file     = p_fp_file;

    (void)fprintf(stdout, "[LOG] Logger initialized\n");
    status = LOG_OK;

EXIT:
    return (status);
}

int logger_destroy ()
{
    int status = LOG_GEN;

    if (NULL != g_logger.p_filename)
    {
        if (0 != fclose(g_logger.p_file))
        {
            (void)fprintf(stderr, "[LOG] Could not close file\n");
            status = LOG_CLOSE;
            goto EXIT;
        }
    }

    if (0 != pthread_mutex_destroy(&(g_logger.log_mutex)))
    {
        (void)fprintf(stderr, "[LOG] Could not destroy mutex\n");
        status = LOG_MUTEX;
        goto EXIT;
    }

    status = LOG_OK;

EXIT:
    return (status);
}

void logger_log (const log_level_t level,
                 const char *      p_invoking_file,
                 const int         line,
                 const char *      message,
                 ...)
{
    // va_list is not initialized to { 0 } because cppcheck will throw an error
    va_list args;
    va_start(args, message);

    if (LOG_MSG_BUFFLEN < strlen(message))
    {
        (void)fprintf(stderr, "[LOG] Message too long\n");
        goto EXIT_VA;
    }

    if (g_logger.log_level < level)
    {
        goto EXIT;
    }

    if (0 != pthread_mutex_lock(&(g_logger.log_mutex)))
    {
        (void)fprintf(stderr, "[LOG] Could not lock mutex\n");
        goto EXIT_VA;
    }

    const char * p_log_level_str[] = { "EMERG", "ALERT",  "CRIT", "ERROR",
                                       "WARN",  "NOTICE", "INFO", "DEBUG" };
    const char * p_level_str       = p_log_level_str[level];

    char log_msg[LOG_MSG_BUFFLEN] = { 0 };
    (void)vsnprintf(log_msg, sizeof(log_msg), message, args);

    struct tm * p_timeinfo = NULL;
    time_t      rawtime    = { 0 };
    rawtime                = time(NULL);
    p_timeinfo             = localtime(&rawtime);

    if (NULL == g_logger.p_filename)
    {
        goto WRITE_LOG;
    }

    if ((0 != access(g_logger.p_filename, F_OK)))
    {
        (void)fprintf(stderr,
                      "[LOG] Log file not found, attempting to re-open...\n");

        if (0 != fclose(g_logger.p_file))
        {
            (void)fprintf(stderr,
                          "[LOG] Could not close previous file pointer.\n");
            goto EXIT;
        }

        FILE * p_fp_file = fopen(g_logger.p_filename, "a");

        if (NULL != p_fp_file)
        {
            (void)fprintf(stderr, "[LOG] Log file reopened.\n");
            g_logger.p_file = p_fp_file;
        }
        else
        {
            (void)fprintf(stderr,
                          "[LOG] Could not reopen, switching to STDOUT.\n");
            g_logger.p_file     = stdout;
            g_logger.p_filename = NULL;
        }
    }

WRITE_LOG:
    /* This is the following format string:
     *
     * - level of the log
     * - date as year, month, date
     * - time as hour, minute, second
     * - invoking filename
     * - line number
     * - formatted log message
     */
    (void)fprintf(g_logger.p_file,
                  "[%s] %04d-%02d-%02d %02d:%02d:%02d [%s:%d] - %s\n",
                  p_level_str,
                  p_timeinfo->tm_year + YEAR_OFFSET,
                  p_timeinfo->tm_mon + MONTH_OFFSET,
                  p_timeinfo->tm_mday,
                  p_timeinfo->tm_hour,
                  p_timeinfo->tm_min,
                  p_timeinfo->tm_sec,
                  p_invoking_file,
                  line,
                  log_msg);

    if (stdout != g_logger.p_file)
    {
        (void)fflush(g_logger.p_file);
    }

    if (0 != pthread_mutex_unlock(&(g_logger.log_mutex)))
    {
        (void)fprintf(stderr, "[LOG] Could not unlock mutex\n");
    }

EXIT_VA:
    va_end(args);
EXIT:
    return;
}

/*** End of logger.c ***/
