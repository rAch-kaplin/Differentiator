#include <stdbool.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <regex.h>
#include "logger.h"
#include "color.h"

Logger* GetLogger()
{
    static Logger logger = {LOGL_DEBUG, NULL, DEFAULT_MODE};
    return &logger;
}

ServiceLines* GetServiceLines()
{
    static ServiceLines serv_lines = {};
    return &serv_lines;
}

int LoggerInit(LogLevel levelLogger, const char *log_file_name, OutputMode color_mode)
{
    Logger *log = GetLogger();
    log->levelLogger = levelLogger;
    log->color_mode = color_mode;

    if (log->color_mode != COLOR_MODE)
    {
        log->logFile = fopen(log_file_name, "w+");

        if (log->logFile == NULL)
        {
            fprintf(stderr, "logFile failed open\n");
            return -100;
        }
    }
    else
    {
        log->logFile = stdout;
    }

    return 0;
}

bool shouldLog(LogLevel levelMsg)
{
    return GetLogger()->levelLogger <= levelMsg;
}

const char* ColorLogMsg(const enum LogLevel levelMsg)
{
    switch (levelMsg)
    {
        case LOGL_DEBUG:
            if (GetLogger()->color_mode == COLOR_MODE)
            {
                return COLOR_GREEN "[DEBUG]" COLOR_RESET;
            }
            else
            {
                return "[DEBUG]";
            }
            break;

        case LOGL_INFO:
            if (GetLogger()->color_mode == COLOR_MODE)
            {
                return COLOR_YELLOW "[INFO]" COLOR_RESET;
            }
            else
            {
                return "[INFO]";
            }
            break;

        case LOGL_ERROR:
            if (GetLogger()->color_mode == COLOR_MODE)
            {
                return COLOR_RED "[ERROR]" COLOR_RESET;
            }
            else
            {
                return "[ERROR]";
            }
            break;

        default:
            break;
    }
    return "UNKNOW";
}

void RemoveAnsiCodes(char *str)
{
    if (!str) return;

    regex_t regex;
    regcomp(&regex, "\x1b\\[[0-9;]*m", REG_EXTENDED);

    char *src = str;
    char *dst = str;

    regmatch_t match;
    while (regexec(&regex, src, 1, &match, 0) == 0)
    {
        size_t len = (size_t)match.rm_so;
        memmove(dst, src, len);
        dst += len;

        src += match.rm_eo;
    }

    memmove(dst, src, strlen(src) + 1);
    regfree(&regex);
}
#if 0
void RemoveAnsiCodes(char *str)
{
    if (!str) return;

    char *src = str;
    char *dst = str;

    while (*src)
    {
        if (*src == '\x1b' && *(src + 1) == '[')
        {
            while (*src && *src != 'm')
            {
                src++;
            }
            if (*src == 'm') src++;
        }
        else
        {
            *dst++ = *src++;
        }
    }

    *dst = '\0';
}
#endif

void log(LogLevel levelMsg, const char *file, size_t line, const char *func,  const char *fmt, ...)
{
    Logger *log = GetLogger();
    time_t time_now = time(NULL);
    struct tm *now = localtime(&time_now);
    char time_info[30] = {};
    strftime(time_info, sizeof(time_info), "%Y-%m-%d %H:%M:%S", now);

    if (!log->logFile)
    {
        fprintf(stderr, "logFile is NULL\n");
        return;
    }

    va_list args;
    va_start(args, fmt);

    if (GetLogger()->color_mode != COLOR_MODE)
    {
        RemoveAnsiCodes(GetServiceLines()->serv_lines);
    }

    if (GetLogger()->color_mode == COLOR_MODE)
    {
        fprintf(log->logFile, COLOR_YELLOW "[%s]%s" COLOR_CYAN "[%s][%zu:%s]: " COLOR_RESET, time_info, ColorLogMsg(levelMsg), file , line, func);
    }
    else
    {
        fprintf(log->logFile, "[%s]%s[%s][%zu:%s]: ", time_info, ColorLogMsg(levelMsg), file , line, func);
    }

    vfprintf(log->logFile,  fmt, args);
    va_end(args);
}

void LoggerDeinit()
{
    fclose(GetLogger()->logFile);
    GetLogger()->logFile = NULL;
}

//isatty()
//fileno()
