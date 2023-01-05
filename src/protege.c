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

#include <stdlib.h>

#include <err.h>
#include <unistd.h>

#include "util.h"
#include "java.h"
#include "options.h"

#if defined(PROTEGE_WIN32)
#include <windows.h>
#include <stdio.h>
#endif

static char *app_dir = NULL;
struct option_list opt_list;

static void
cleanup(void)
{
    if ( app_dir )
        free(app_dir);

    free_option_list(&opt_list);
}

int
main(int argc, char **argv)
{
    void* jre;
    int ret;

    (void) argc;
    (void) argv;

#if defined(PROTEGE_WIN32)
    /* If the launcher was started from a terminal, let's make sure
     * it is used to display what we send to the output streams. */
    if ( AttachConsole(-1) ) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
#endif

    setprogname("protege");
    (void) atexit(cleanup);

    if ( ! (app_dir = get_application_directory()) )
        err(EXIT_FAILURE, "Cannot get the application directory");

    if ( chdir(app_dir) == -1 )
        err(EXIT_FAILURE, "Cannot change current directory");

    get_option_list(app_dir, &opt_list);

    if ( opt_list.java_home )
        ret = load_jre(opt_list.java_home, 0, &jre);
    else
        ret = load_jre(app_dir, 1, &jre);
    if ( ret != 0 )
        errx(EXIT_FAILURE, "Cannot load JRE: %s", get_java_error(ret));

    if ( (ret = start_java(jre,
                           (const char **)opt_list.options,
                           "org/protege/osgi/framework/Launcher",
                           NULL)) != 0 )
        errx(EXIT_FAILURE, "Cannot start Java: %s", get_java_error(ret));

    return EXIT_SUCCESS;
}
