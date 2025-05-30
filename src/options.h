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

#ifndef ICP20221227_OPTIONS_H
#define ICP20221227_OPTIONS_H

#include <stdlib.h>

#define PROTEGE_FLAG_UI_AUTO_SCALING	0x01

/*
 * Hold a list of options for the launcher.
 */
struct option_list {
    size_t      allocated;  /* Number of strings allocated in options
                               (including terminating NULL). */
    size_t      count;      /* Number of strings present in options
                               (excluding terminating NULL). */
    char      **options;    /* Actual list of option strings. */
    char       *java_home;  /* Additional option specifying a custom
                               location for the JRE to use. */
    unsigned    flags;      /* Misc additional options. */
};

#ifdef __cplusplus
extern "C" {
#endif

void
get_option_list(const char *app_dir, struct option_list *list);

void
free_option_list(struct option_list *list);

#ifdef __cplusplus
}
#endif

#endif /* !ICP20221227_OPTIONS_H */
