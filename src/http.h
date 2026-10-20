#ifndef STATICD_HTTP_H
#define STATICD_HTTP_H

#include <stdio.h>

#define MAX_REQUEST_LINE 2048
#define MAX_PATH 1024
#define MAX_FILE (16L * 1024 * 1024)

/* Reads one request from in and writes the response to out. */
void serve(const char *root, FILE *in, FILE *out);

#endif
