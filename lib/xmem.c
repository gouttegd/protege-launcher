/*
 * Protégé launcher
 * Copyright (C) 2022 Damien Goutte-Gattat
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xmem.h"

#include <stdio.h>
#include <string.h>
#include <err.h>

void (*xmem_error)(size_t) = NULL;

#define mem_error(a)                                            \
    do {                                                        \
        if ( xmem_error )                                       \
            (*xmem_error)((a));                                 \
        err(EXIT_FAILURE, "Cannot allocate %lu bytes", (a));    \
    } while ( 0 )

void *
xmalloc(size_t s)
{
    void *p;

    if ( ! (p = malloc(s)) )
        mem_error(s);

    return p;
}

void *
xcalloc(size_t n, size_t s)
{
    void *p;

    if ( ! (p = calloc(n, s)) && n && s )
        mem_error(n * s);

    return p;
}

void *
xrealloc(void *p, size_t s)
{
    void *np;

    if ( ! (np = realloc(p, s)) && s )
        mem_error(s);

    return np;
}

char *
xstrdup(const char *s)
{
    char *dup;
    size_t len;

    len = strlen(s);
    if ( ! (dup = malloc(len + 1)) )
        mem_error(len + 1);

    strcpy(dup, s);

    return dup;
}

int
xasprintf(char **s, const char *fmt, ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = xvasprintf(s, fmt, ap);
    va_end(ap);

    return ret;
}

int
xvasprintf(char **s, const char *fmt, va_list ap)
{
    int n;
    va_list aq;

    va_copy(aq, ap);
    n = vsnprintf(NULL, 0, fmt, aq) + 1;
    va_end(aq);

    if ( ! (*s = malloc(n)) )
        mem_error((size_t) n);

    n = vsnprintf(*s, n, fmt, ap);

    return n;
}
