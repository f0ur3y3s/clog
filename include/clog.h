/**
 * @file clog.h
 * @author f0ur3y3s
 * @brief Header file for a threadsafe logger implementation adhering to Cisco
 * logging standards
 * @version 0.2
 * @date 2024-07-17
 *
 * Inspired from https://github.com/rxi/log.c, modified and rewritten to adhere
 * to Cisco logging standard found here:
 * https://www.cisco.com/c/en/us/td/docs/routers/access/wireless/software/guide/
 * SysMsgLogging.html
 *
 * A threadsafe logging library that writes to STDOUT or a file. The logger will
 * only output levels of logging that are less than or equal to the level set,
 * with L_DEBUG being the most verbose and L_EMERG being the least verbose.
 *
 * Example usage:
 *     logger_init("log.txt", L_DEBUG);
 *     clog(L_DEBUG, "This is a debug message");
 *     clog(L_INFO, "This is an info message");
 *     logger_destroy();
 *
 * The logger_destroy function should be called at the end of the program to
 * free resources. Although the logger is stack allocated, not freeing the mutex
 * may cause errors.
 * The logger can log a message up to 256 characters.
 * The logger macro is named `clog` for `custom log` due to a collision with the
 * `log` function in the C math.h library called by unity when testing.
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef LOG_H
#define LOG_H

#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Enum for log levels
 *
 * @param L_EMERG Emergency log level
 * @param L_ALERT Alert log level
 * @param L_CRIT Critical log level
 * @param L_ERR Error log level
 * @param L_WARNING Warning log level
 * @param L_NOTICE Notice log level
 * @param L_INFO Info log level
 * @param L_DEBUG Debug log level
 *
 */
typedef enum log_level_t
{
    L_EMERG = 0,
    L_ALERT,
    L_CRIT,
    L_ERR,
    L_WARNING,
    L_NOTICE,
    L_INFO,
    L_DEBUG,
} log_level_t;

/**
 * @brief Enum for offsets when calculating time
 *
 */
typedef enum offset_t
{
    MONTH_OFFSET    = 1,
    LOG_MSG_BUFFLEN = 1024,
    YEAR_OFFSET     = 1900,
} offset_t;

/**
 * @brief Enum for logger errors
 *
 * @param LOG_GEN General error
 * @param LOG_OK No error
 * @param LOG_EXIST Logger already exists
 * @param LOG_NULL Null pointer
 * @param LOG_OPEN Could not open file
 * @param LOG_CLOSE Could not close file
 * @param LOG_WRITE Could not write to file
 * @param LOG_DESTROY Could not destroy logger
 * @param LOG_MUTEX Could not initialize mutex
 * @param LOG_ALLOC Could not allocate memory
 *
 */
typedef enum logger_error_t
{
    LOG_GEN = -1,
    LOG_OK  = 0,
    LOG_EXIST,
    LOG_NULL,
    LOG_OPEN,
    LOG_CLOSE,
    LOG_WRITE,
    LOG_DESTROY,
    LOG_MUTEX,
    LOG_ALLOC,
} logger_error_t;

/**
 * @brief Struct to hold the logger
 *
 * @param log_mutex Mutex for logging
 * @param log_level Verbosity log level
 * @param p_filename Filename to write logs to, NULL for stdout
 */
typedef struct logger_t
{
    pthread_mutex_t log_mutex;
    log_level_t     log_level;
    char *          p_filename;
    FILE *          p_file;
} logger_t;

/**
 * @brief Macro to log messages
 *
 * @param debug_level Log level
 * @param ... Message to log with format specifiers
 *
 * @note This macro is used to log messages with the file and line number
 */
#define clog(debug_level, ...) \
    logger_log(debug_level, __FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief Initialize the logger with a filename and log level
 *
 * @note If the filename is NULL, the logger will write to STDOUT
 * @note Filenames cannot contain `stdin`, `stdout`, or `stderr`
 *
 * @param p_filename The filename to write logs to.
 * @param log_level The log level to write logs at.
 *
 * @return int
 * @retval LOG_OK If the logger was initialized successfully. (0)
 * @retval Non-zero otherwise
 * @retval LOG_GEN If a general error occurred
 * @retval LOG_OPEN If the file could not be opened
 * @retval LOG_CLOSE If the file could not be closed
 * @retval LOG_MUTEX If the mutex could not be initialized
 * @retval LOG_NULL If the filename is NULL
 */
int logger_init (char * p_filename, const log_level_t log_level);

/**
 * @brief Destroy the logger
 *
 * @return int
 * @retval LOG_OK If the logger was destroyed successfully. (0)
 * @retval Non-zero otherwise
 * @retval LOG_GEN If a general error occurred
 * @retval LOG_MUTEX If the mutex could not be destroyed
 */
int logger_destroy ();

/**
 * @brief Log a message
 *
 * @param level The log level
 * @param p_invoking_file The invoking file
 * @param line The line number
 * @param message The message to log
 * @param ... Format specifiers
 */
void logger_log (const log_level_t level,
                 const char *      p_invoking_file,
                 const int         line,
                 const char *      message,
                 ...);

#endif // LOG_H

/*** end of logger.h ***/
