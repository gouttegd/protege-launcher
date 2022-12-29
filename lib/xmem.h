/*
 * xmem - Incenp.org Notch Library: die-on-error memory functions
 * Copyright (C) 2011 Damien Goutte-Gattat
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

#ifndef ICP20110203_XMEM_H
#define ICP20110203_XMEM_H

#include <stdlib.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void (*xmem_error)(size_t);

void *
xmalloc(size_t);

void *
xcalloc(size_t, size_t);

void *
xrealloc(void *, size_t);

char *
xstrdup(const char *);

char *
xstrndup(const char *, size_t);

int
xasprintf(char **, const char *, ...);

int
xvasprintf(char **, const char *, va_list);

#ifdef __cplusplus
}
#endif

#endif /* !ICP20110203_XMEM_H */
