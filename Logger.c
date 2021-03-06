#ifndef LOGGER_H
#include "Logger.h"
#include "Timer.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

extern int snprintf(char *str, size_t size, const char *format, ...);

void logit(FILE *fp, const char *format, va_list ap) {
    vfprintf(fp, format, ap);
}

void log_info(const char *msg) {
    char time[256];
    char buf [1024];
    timeToString(time, sizeof time);
    snprintf(buf, sizeof (buf), "%s INFO: %s\n", time, msg);
    fprintf(stdout, "%s", buf);
}

void log_infov(const char *format, ...) {
    char time[256];
    timeToString(time, sizeof time);
    char buf[1024];
    snprintf(buf, sizeof (buf), "%s INFO: %s", time, format);
    va_list ap;
    va_start(ap, format);
    logit(stdout, buf, ap);
    va_end(ap);

}

void log_err(int bail, const char *msg) {
    char time[256];
    char buf [1024];
    timeToString(time, sizeof time);
    snprintf(buf, sizeof (buf), "%s ERROR: %s\n", time, msg);
    fprintf(stderr, "%s", buf);
    if (bail != 0) exit(-2);

}

void log_errv(int bail, const char *format, ...) {
    char time[256];
    timeToString(time, sizeof time);
    char buf[1024];
    snprintf(buf, sizeof (buf), "%s ERROR: %s", time, format);

    va_list ap;
    va_start(ap, format);
    logit(stderr, buf, ap);
    va_end(ap);
    if (bail != 0) exit(-2);
}


#endif
