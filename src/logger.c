#include "logger.h"
#if LOG_LEVEL > LEVEL_NO_LOGS

FILE* log_out_file_p = NULL;
FILE* log_err_file_p = NULL;
pthread_mutex_t log_out_mutex;
pthread_mutex_t log_err_mutex;

FILE* get_log_out_file(void) { return log_out_file_p == NULL ? stdout : log_out_file_p; }
FILE* get_log_err_file(void) { return log_err_file_p == NULL ? stderr : log_err_file_p; }
void wrap_free(char** var) { free(*var); }

void _logger_open_out_file(const char* log_out_file_path_str)
{
    log_out_file_p = fopen(log_out_file_path_str, "a");
    if (log_out_file_p == NULL)
    {
        perror("Fatal error: could not open logger out file.");
        exit(-1);
    }
    LOG_INFO("Logger OUT file opened.");
}
void _logger_open_err_file(const char* log_err_file_path_str)
{
    log_err_file_p = fopen(log_err_file_path_str, "a");
    if (log_err_file_p == NULL)
    {
        perror("Fatal error: could not open logger out file.");
        exit(-1);
    }
    LOG_INFO("Logger ERR file opened.");
}

void logger_init(const char* log_out_file_path_str, const char* log_err_file_path_str)
{
    static bool logger_initialized = false;
    if (logger_initialized)
    {
        LOG_ERROR("Logger already initialized.");
        return;
    }
    LOG_INFO("Initializing logger.");
    pthread_mutex_init(&log_out_mutex, NULL);
    pthread_mutex_init(&log_err_mutex, NULL);
    logger_initialized = true;

    if (log_out_file_path_str != NULL && log_err_file_path_str != NULL)
    {
        _logger_open_out_file(log_out_file_path_str);
        if (strcmp(log_out_file_path_str, log_err_file_path_str) == 0)
        {
            log_err_file_p = log_out_file_p;
            LOG_INFO("Logger ERR file matches logger OUT file.");
        }
        else
        {
            _logger_open_err_file(log_err_file_path_str);
        }
    }
    if (log_out_file_path_str == NULL)
    {
        log_out_file_p = stdout;
        LOG_INFO("Logger OUT set to standard out.");
    }
    else if (log_out_file_p == NULL)
    {
        _logger_open_out_file(log_out_file_path_str);
    }

    if (log_err_file_path_str == NULL)
    {
        log_err_file_p = stderr;
        LOG_INFO("Logger ERR set to standard error.");
    }
    else if (log_err_file_p == NULL)
    {
        _logger_open_err_file(log_err_file_path_str);
    }
}

void get_date_time(char* date_time_str)
{
    struct tm result;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &result);
    strftime(date_time_str, 26, "%a %b %d %Y %H:%M:%S.", &result);
#ifdef __linux__
    snprintf(&date_time_str[25], 7, "%06ld", tv.tv_usec);
#else
    snprintf(&date_time_str[25], 7, "%06d", tv.tv_usec);
#endif /* __linux__ */
}

#endif /* LOG_LEVEL > LEVEL_NO_LOGS */
#ifdef TEST
void test_logger(void)
{
    PRINT_BANNER()
    PRINT_TEST_TITLE("Logging all levels");
    PRINT_SEPARATOR();
    LOG_TRACE("Log trace.");
    LOG_DEBUG("Log debug.");
    LOG_INFO("Log info.");
    LOG_WARNING("Log warning.");
    LOG_ERROR("Log error.");
}
#endif /* TEST */
