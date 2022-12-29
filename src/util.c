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

#include "util.h"

#include <stdlib.h>
#include <errno.h>

#include <xmem.h>

#if defined(PROTEGE_LINUX)

#include <unistd.h>
#include <sys/stat.h>

char *
get_application_directory(void)
{
    char *buffer;
    size_t buffer_len;
    ssize_t bytes_read;
    struct stat statbuf;

    if ( lstat("/proc/self/exe", &statbuf) == -1 )
        return NULL;

    buffer_len = statbuf.st_size + 1;

    /* Sanity checks. */
    if ( buffer_len == 1 || buffer_len > 16384 )
        buffer_len = 1024;    /* Arbitrary default value. */

    buffer = xmalloc(buffer_len);

    bytes_read = readlink("/proc/self/exe", buffer, buffer_len);
    if ( bytes_read == buffer_len )
        bytes_read = 0;  /* Symlink too long for buffer. */

    /* Find the last / */
    while ( bytes_read-- > 0 && buffer[bytes_read] != '/' ) ;

    if ( bytes_read > 0 ) {
        /* Found the last /, truncate the string on it. */
        buffer[bytes_read] = '\0';
    }
    else {
        /*
         * We are here if:
         * - we got an empty string from readlink;
         * - the symlink was too long;
         * - we didn't find a / in the symlink.
         * In any case, we failed to get the application directory.
         */
        free(buffer);
        buffer = NULL;
        errno = ENOENT;
    }

    return buffer;
}

#elif defined(PROTEGE_MACOS)

#include <mach-o/dyld.h>

char *
get_application_directory(void)
{
    char *buffer = NULL;
    uint32_t buffer_len = 0;
    int n = 2;

    (void) _NSGetExecutablePath(buffer, &buffer_len);
    buffer = xmalloc(buffer_len);
    (void) _NSGetExecutablePath(buffer, &buffer_len);

    while ( buffer_len-- > 0 && n > 0 ) {
        if ( buffer[buffer_len] == '/' ) {
            buffer[buffer_len] = '\0';
            n -= 1;
        }
    }

    if ( buffer_len == 0 ) {
        /* No / found in the path. */
        free(buffer);
        buffer = NULL;
        errno = ENOENT;
    }

    return buffer;
}

#elif defined(PROTEGE_WIN32)

#include <windows.h>

char *
get_application_directory(void)
{
    char *buffer;
    DWORD len;

    buffer = xmalloc(PROTEGE_PATH_MAX);
    len = GetModuleFileName(NULL, buffer, PROTEGE_PATH_MAX);

    if ( len == PROTEGE_PATH_MAX ) {
        free(buffer);
        buffer = NULL;
        errno = ENAMETOOLONG;
    }
    else {
        while ( len-- > 0 && buffer[len] != '\\' ) ;
        if ( len > 0 )
            buffer[len] = '\0';
        else {
            free(buffer);
            buffer = NULL;
            errno = ENOENT;
        }
    }

    return buffer;
}

#endif

static int
discard_line(FILE *f)
{
    int c;

    while ( (c = fgetc(f)) != '\n' && c != EOF ) ;

    return c == EOF ? -1 : 0;
}

ssize_t
get_line(FILE *f, char *buffer, size_t len)
{
    int c;
    size_t n = 0;

    while ( (c = fgetc(f)) != '\n' ) {
        if ( c == EOF )
            return -1;

        if ( n >= len - 1 ) {   /* line too long */
            discard_line(f);
            return -1;
        }

        buffer[n++] = (char)c;
    }
    buffer[n] = '\0';

    return n;
}
