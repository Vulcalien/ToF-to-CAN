/*
 * params.h
 */

#pragma once

int param_get_int(const char * name, int def_val);
float param_get_float(const char * name, float def_val);
char * param_get_string(const char * name, const char * def_val);
