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

#ifndef ICP20221228_DLOPEN_H
#define ICP20221228_DLOPEN_H

#define RTLD_LAZY       0x0001
#define RTLD_NOW        0x0002
#define RTLD_GLOBAL     0x0100
#define RTLD_LOCAL      0x0000
#define RTLD_NODELETE   0x1000
#define RTLD_NOLOAD     0x0004
#define RTLD_DEEPBIND   0x0008

#ifdef __cplusplus
extern "C" {
#endif

void *dlopen(const char *, int);

int dlclose(void *);

void *dlsym(void *, const char *);

char *dlerror(void);

#ifdef __cplusplus
}
#endif

#endif /* !ICP20221228_DLOPEN_H */
