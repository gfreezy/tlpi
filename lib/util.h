/* 
 * File:   util.h
 * Author: chaofei
 *
 * Created on February 26, 2014, 11:25 PM
 */

#ifndef UTIL_H
#define	UTIL_H

#ifdef	__cplusplus
extern "C" {
#endif
    void trim_whitespace(char *s);
    int strisdigit(char *str);
    char *splitLines(const char *buf, char **pos);
#ifdef	__cplusplus
}
#endif

#endif	/* UTIL_H */

