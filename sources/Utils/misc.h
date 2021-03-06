/**
 * File: misc.h - Various auxiliary routines for MiptVis
 * Copyright (C) 2009  Boris Shurygin
 */
#ifndef MISC_H
#define MISC_H

/**
 * If pointer is not null
 */
inline bool isNotNullP( void *pointer)
{
    return pointer != NULL;
}
/**
 * If pointer is null
 */
inline bool isNullP( void *pointer)
{
    return pointer == NULL;
}

/**
 * Abs template 
 * operators "<" and "-" must be defined for parameter class
 */
template<class Value> inline Value abs(Value val)
{
    if( val < 0)
        return -val;

    return val;
}

#endif