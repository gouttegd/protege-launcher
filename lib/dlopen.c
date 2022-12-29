/*
 * dlopen - Incenp.org Notch Library: dlopen/dlsym replacement
 * Copyright (C) 2022 Damien Goutte-Gattat
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef HAVE_DLOPEN
#ifdef HAVE_WINDOWS_H

#include <windows.h>

static LPSTR   dlerror_buffer = NULL;
static char   *dlerror_extern = NULL;
static int     dlerror_cleanup_registered = 0;

static void
reset_error_message(void)
{
    if ( dlerror_buffer ) {
        LocalFree(dlerror_buffer);
        dlerror_buffer = NULL;
    }

    dlerror_extern = NULL;
}

static void
store_error_message(void)
{
    DWORD error_code;

    reset_error_message();

    error_code = GetLastError();
    (void) FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                         FORMAT_MESSAGE_FROM_SYSTEM |
                         FORMAT_MESSAGE_IGNORE_INSERTS,
                         NULL,
                         error_code,
                         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                         (LPSTR)&dlerror_buffer,
                         0,
                         NULL);

    dlerror_extern = dlerror_buffer;

    if ( ! dlerror_cleanup_registered ) {
        atexit(reset_error_message);
        dlerror_cleanup_registered = 1;
    }
}

void *
dlopen(const char *filename, int flags)
{
    void *handle;

    (void) flags;

    if ( ! (handle = LoadLibrary(filename)) )
        store_error_message();
    else
        reset_error_message();

    return handle;
}

int
dlclose(void *handle)
{
    FreeLibrary((HMODULE)handle);
    reset_error_message();

    return 0;
}

void *
dlsym(void *handle, const char *symbol)
{
    void *symbol_handle = NULL;

    if ( ! (symbol_handle = GetProcAddress((HMODULE)handle, symbol)) )
        store_error_message();
    else
        reset_error_message();

    return symbol_handle;
}

char *
dlerror(void)
{
    char *tmp = dlerror_extern;
    dlerror_extern = NULL;
    return tmp;
}

#endif /* !HAVE_WINDOWS_H */
#endif /* !HAVE_DLOPEN */
