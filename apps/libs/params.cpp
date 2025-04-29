/*
 * params.cpp
 */

#include <stdlib.h>
#include <ctype.h>
#include "params.h"

int param_get_int(const char * name, int def_val)
{
    char * v = getenv(name);
    if (v == NULL)
        return def_val;
    else
        return atoi(v);
}


float param_get_float(const char * name, float def_val)
{
    char * v = getenv(name);
    if (v == NULL)
        return def_val;
    else
        return atof(v);
}

char * param_get_string(const char * name, const char * def_val)
{
    char * v = getenv(name);
    if (v == NULL)
        return (char *)def_val;
    else
        return v;
}

