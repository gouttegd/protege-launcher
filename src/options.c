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

#include "options.h"

#include <stdio.h>
#include <string.h>

#include <sys/stat.h>

#include <xmem.h>

#include "util.h"

static const char *default_options[] = {
    "-Dlogback.configurationFile=conf/logback.xml",
    "-DentityExpansionLimit=100000000",
    "-Dfile.encoding=UTF-8",
    "-XX:CompileCommand=exclude,javax/swing/text/GlyphView,getBreakSpot",
#if defined(PROTEGE_MACOS)
    "-Dapple.laf.useScreenMenuBar=true",
    "-Dcom.apple.mrj.application.apple.menu.about.name=Protege",
    "-Xdock:name=Protege",
    "-Xdock:icon=Resources/Protege.icns",
#endif
    "-Djava.class.path"
      "=bundles/guava.jar"
      ":bundles/logback-classic.jar"
      ":bundles/logback-core.jar"
      ":bundles/slf4j-api.jar"
      ":bundles/glassfish-corba-orb.jar"
      ":bundles/org.apache.felix.main.jar"
      ":bundles/maven-artifact.jar"
      ":bundles/protege-launcher.jar",
    NULL
};

static size_t n_default_options = sizeof(default_options) / sizeof(char**) - 1;

static void
copy_default_options(struct option_list *list)
{
    size_t n;

    list->allocated = n_default_options + 10;
    list->options = xmalloc(list->allocated * sizeof(char *));
    for ( n = 0; default_options[n]; n++ )
        list->options[n] = (char *)default_options[n];
    list->count = n;
}

static void
append_option(struct option_list *list, char *option)
{
    if ( ! list->allocated )
        copy_default_options(list);

    if ( list->count >= list->allocated - 1 ) {
        list->allocated += 10;
        list->options = xrealloc(list->options, list->allocated);
    }

    list->options[list->count++] = option;
    list->options[list->count] = NULL;
}

static char *
find_configuration_file(const char *app_dir)
{
    char *home, *conf_file = NULL;
    struct stat statbuf;

    if ( (home = getenv("HOME")) ) {
        (void) xasprintf(&conf_file, "%s/.protege/conf/jvm.conf", home);
        if ( stat(conf_file, &statbuf) == -1 ) {
            free(conf_file);
            conf_file = NULL;
        }
    }

    if ( ! conf_file ) {
        (void) xasprintf(&conf_file, "%s/conf/jvm.conf", app_dir);
        if ( stat(conf_file, &statbuf) == -1 ) {
            free(conf_file);
            conf_file = NULL;
        }
    }

    return conf_file;
}

#if defined(PROTEGE_MACOS)

#include <CoreFoundation/CoreFoundation.h>

static void
get_options_from_bundle(struct option_list *list)
{
    CFBundleRef main_bundle;
    CFDictionaryRef info_dict;
    CFArrayRef jvmopts_array;
    CFIndex length, i;

    if ( ! (main_bundle = CFBundleGetMainBundle()) )
        return;

    if ( ! (info_dict = CFBundleGetInfoDictionary(main_bundle)) )
        return;

    if ( ! (jvmopts_array = (CFArrayRef)CFDictionaryGetValue(info_dict, CFSTR("JVMOptions"))) )
        return;

    length = CFArrayGetCount(jvmopts_array);
    for ( i = 0; i < length; i++ ) {
        CFStringRef option = CFArrayGetValueAtIndex(jvmopts_array, i);
        if ( CFStringHasPrefix(option, CFSTR("-Xmx"))
            || CFStringHasPrefix(option, CFSTR("-Xms"))
            || CFStringHasPrefix(option, CFSTR("-Xss")) ) {
            CFIndex option_length;
            char *option_string;

            option_length = CFStringGetLength(option);
            option_string = xmalloc(option_length + 1);
            CFStringGetCString(option, option_string, option_length + 1,
                               kCFStringEncodingMacRoman);

            append_option(list, option_string);
        }
    }
}

#endif

void
get_option_list(const char *app_dir, struct option_list *list)
{
    char *conf_file;

    list->allocated = 0;
    list->count = n_default_options;
    list->options = (char **) default_options;

    if ( (conf_file = find_configuration_file(app_dir)) ) {
        FILE *f;
        char line[100], *opt_value, *opt_string;
        ssize_t n;

        if ( (f = fopen(conf_file, "r")) ) {

            while ( ! feof(f) ) {
                if ( (n = get_line(f, line, sizeof(line))) > 0 ) {
                    if ( line[0] == '#' )
                        continue;

                    if ( ! (opt_value = strchr(line, '=')) )
                        continue;

                    *opt_value++ = '\0';
                    opt_string = NULL;
                    if ( strcmp(line, "max_heap_size") == 0 )
                        (void) xasprintf(&opt_string, "-Xmx%s", opt_value);
                    else if ( strcmp(line, "min_heap_size") == 0 )
                        (void) xasprintf(&opt_string, "-Xms%s", opt_value);
                    else if ( strcmp(line, "stack_size") == 0 )
                        (void) xasprintf(&opt_string, "-Xss%s", opt_value);
                    else if ( strcmp(line, "append") == 0 )
                        opt_string = xstrdup(opt_value);

                    if ( opt_string )
                        append_option(list, opt_string);
                }
            }

            fclose(f);
        }

        free(conf_file);
    }
#if defined(PROTEGE_MACOS)
    else
        get_options_from_bundle(list);
#endif
}

void
free_option_list(struct option_list *list)
{
    if ( list->allocated ) {
        size_t n;

        for ( n = n_default_options; list->options[n]; n++ )
            free(list->options[n]);
        free(list->options);
    }
}
