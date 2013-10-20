/* 
 * File:   logger.h
 * Author: jzendle
 *
 * Created on September 30, 2013, 6:46 PM
 */

#ifndef LOGGER_H
#define	LOGGER_H


void info(const char *msg);

void infov(const char *str, ...);

void err(int exit, const char *msg);

void errv(int exit, const char *msg, ...);

#endif	/* LOGGER_H */

