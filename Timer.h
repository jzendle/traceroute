/* 
 * File:   timer.h
 * Author: jzendle
 *
 * Created on September 30, 2013, 9:26 PM
 */

#ifndef TIMER_H
#define	TIMER_H

#include <sys/time.h>

    char *timeToString(char *buf, int len);
    float timeDiffMillis(struct timeval *after, struct timeval *before);
 
#endif	/* TIMER_H */

