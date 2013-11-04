/* 
 * File:   logger.h
 * Author: jzendle
 *
 * Created on September 30, 2013, 6:46 PM
 */

#ifndef LOGGER_H
#define	LOGGER_H

#define LOG_PERROR log_errv(0,"function: %s() errno: (%d) strerror: (%s) %s:%d\n", __FUNCTION__, errno, strerror(errno), __FILE__, __LINE__)


void log_info(const char *msg);

void log_infov(const char *str, ...);

void log_err(int exit, const char *msg);

void log_errv(int exit, const char *msg, ...);

#endif	/* LOGGER_H */

